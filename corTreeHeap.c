//
// FILE            corTreeHeap.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
// SPDX-License-Identifier: Apache-2.0
//
#define _DEFAULT_SOURCE                           // MAP_NORESERVE

#include <pthread.h>                              // pthread_mutex_*, pthread_once
#include <stdatomic.h>                            // atomic_fetch_add
#include <stdbool.h>                              // bool
#include <stdint.h>                               // uint8_t, uint64_t
#include <stdlib.h>                               // malloc, free, getenv
#include <string.h>                               // strcmp
#include <sys/mman.h>                             // mmap

#include "corLog/corLog.h"                        // COR_W
#include "corTree/corTreeHeap.h"                  // Own interface



// -----------------------------------------------------------------------------
//
// HeapClass - the blocks of one size: (index + 1) * 8 bytes
//
typedef struct HeapClass
{
  pthread_mutex_t  mutex;
  void*            freeList;     // freed blocks, linked through their first 8 bytes
  char*            next;         // the current page's next uncarved block
  char*            end;          // the current page's end
} HeapClass;



// -----------------------------------------------------------------------------
//
// The heap - one address range reserved at the first allocation, its pages handed out in order.
// pageClassV[page] is the class of each page handed out. Both are MAP_NORESERVE: RAM is what is touched.
//
static pthread_once_t    heapOnce   = PTHREAD_ONCE_INIT;
static char*             heapStart  = NULL;
static uint64_t          heapSize   = 0;
static _Atomic uint64_t  heapUsed   = 0;
static uint8_t*          pageClassV = NULL;
static HeapClass         classV[128];



// -----------------------------------------------------------------------------
//
// heapInit - reserve 64 GiB of addresses, or half of that, down to 256 MiB; nothing reserved = malloc
//
static void heapInit(void)
{
  const char* env = getenv("COR_TREE_HEAP");

  if ((env != NULL) && (strcmp(env, "malloc") == 0))
    return;

  for (uint64_t size = 64ULL << 30; size >= (256ULL << 20); size /= 2)
  {
    void* startP = mmap(NULL, size,       PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    void* tableP = mmap(NULL, size >> 14, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);

    if ((startP != MAP_FAILED) && (tableP != MAP_FAILED))
    {
      for (int ix = 0; ix < 128; ix++)
        pthread_mutex_init(&classV[ix].mutex, NULL);

      pageClassV = (uint8_t*) tableP;
      heapSize   = size;
      heapStart  = (char*) startP;    // last: corTreeHeapFree's range check reads it
      return;
    }

    if (startP != MAP_FAILED) munmap(startP, size);
    if (tableP != MAP_FAILED) munmap(tableP, size >> 14);
  }

  COR_W("corTreeHeap: no address range could be reserved - the trees use malloc");
}



// -----------------------------------------------------------------------------
//
// corTreeHeapAlloc -
//
void* corTreeHeapAlloc(int size)
{
  pthread_once(&heapOnce, heapInit);

  if ((heapStart == NULL) || (size > 1024) || (size <= 0))
    return malloc(size);

  int         ix     = (size + 7) / 8 - 1;
  int         bytes  = (ix + 1) * 8;
  HeapClass*  cP     = &classV[ix];
  void*       blockP;

  pthread_mutex_lock(&cP->mutex);

  if (cP->freeList != NULL)
  {
    blockP       = cP->freeList;
    cP->freeList = *(void**) blockP;
  }
  else
  {
    if (cP->next + bytes > cP->end)
    {
      uint64_t offset = atomic_fetch_add(&heapUsed, 16384);

      if (offset + 16384 > heapSize)
      {
        pthread_mutex_unlock(&cP->mutex);
        return malloc(size);    // the range is full
      }

      pageClassV[offset >> 14] = (uint8_t) ix;
      cP->next = heapStart + offset;
      cP->end  = cP->next + 16384;
    }

    blockP    = cP->next;
    cP->next += bytes;
  }

  pthread_mutex_unlock(&cP->mutex);

  return blockP;
}



// -----------------------------------------------------------------------------
//
// corTreeHeapFree -
//
void corTreeHeapFree(void* blockP)
{
  char* p = (char*) blockP;

  if ((heapStart == NULL) || (p < heapStart) || (p >= heapStart + heapSize))
  {
    free(blockP);
    return;
  }

  HeapClass* cP = &classV[pageClassV[(uint64_t) (p - heapStart) >> 14]];

  pthread_mutex_lock(&cP->mutex);
  *(void**) blockP = cP->freeList;
  cP->freeList     = blockP;
  pthread_mutex_unlock(&cP->mutex);
}



// -----------------------------------------------------------------------------
//
// corTreeHeapBytes -
//
uint64_t corTreeHeapBytes(void)
{
  uint64_t used = atomic_load(&heapUsed);

  return (used < heapSize)? used : heapSize;
}
