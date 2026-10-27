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
#include <sys/mman.h>                             // mmap, madvise

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
// The thread caches - each thread's own free blocks per class, taken and freed with no lock. A cache is
// filled from its class (from the free list, or else a range of the page) and given back to it a batch at
// a time, under the class's mutex: CACHE_BYTES worth of blocks, CACHE_BATCH at most - 32 blocks of up to 128 bytes, 4 of
// 1024. At most two batches of a class wait in one thread, 8 KiB. A thread that ends gives its whole cache
// back (cacheRelease) - the blocks stay in the heap, for the other threads.
//
// Without them every node built or freed takes its class's mutex, and the nodes of a tree are of a few
// sizes: the threads that write the store at the same time wait for each other on those few mutexes.
//
enum { CACHE_BATCH = 32, CACHE_BYTES = 4096 };

typedef struct ThreadClass
{
  void*  head;                   // freed blocks, linked through their first 8 bytes, as the class's free list
  int    count;                  // in 'head'
  char*  next;                   // a range of a page never handed out, the thread's own: next..end, whole blocks
  char*  end;
} ThreadClass;

static __thread ThreadClass  cacheV[128];
static __thread bool         cacheKeyed = false;   // cacheKey set: cacheRelease runs when the thread ends
static pthread_key_t         cacheKey;
static bool                  cacheKeyOk = false;
static uint8_t               batchV[128];          // the blocks of a batch of each class - set by heapInit



// -----------------------------------------------------------------------------
//
// classPut - a list of blocks, head to tail, onto its class's free list
//
static void classPut(HeapClass* cP, void* headP, void* tailP)
{
  pthread_mutex_lock(&cP->mutex);
  *(void**) tailP = cP->freeList;
  cP->freeList    = headP;
  pthread_mutex_unlock(&cP->mutex);
}



// -----------------------------------------------------------------------------
//
// cacheKeySet - the thread's cacheRelease armed, at its first use of its cache (and again after a release:
// a later thread-exit destructor may free a tree)
//
static inline void cacheKeySet(void)
{
  if ((cacheKeyed == false) && cacheKeyOk)
  {
    pthread_setspecific(cacheKey, (void*) 1);           // any non-NULL value: the destructor runs
    cacheKeyed = true;
  }
}



// -----------------------------------------------------------------------------
//
// cacheRelease - the ending thread's cache given back to the classes (cacheKey's destructor)
//
static void cacheRelease(void* unused)
{
  (void) unused;

  cacheKeyed = false;                                   // the key is NULL now - a block cached later re-arms it

  for (int ix = 0; ix < 128; ix++)
  {
    ThreadClass* tP    = &cacheV[ix];
    int          bytes = (ix + 1) * 8;

    for (; tP->next != tP->end; tP->next += bytes)      // the range not carved yet: as blocks, to the list
    {
      *(void**) tP->next = tP->head;
      tP->head           = tP->next;
      ++tP->count;
    }
    tP->next = tP->end = NULL;

    if (tP->count == 0)
      continue;

    void* tailP = tP->head;

    while (*(void**) tailP != NULL)
      tailP = *(void**) tailP;

    classPut(&classV[ix], tP->head, tailP);
    tP->head  = NULL;
    tP->count = 0;
  }
}



// -----------------------------------------------------------------------------
//
// heapInit - reserve 64 GiB of addresses, or half of that, down to 256 MiB; nothing reserved = malloc
//
static void heapInit(void)
{
#ifdef __SANITIZE_ADDRESS__
  return;    // ASan sees each block's bounds only in malloc's
#endif

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

      for (int ix = 0; ix < 128; ix++)
        batchV[ix] = (CACHE_BYTES / ((ix + 1) * 8) < CACHE_BATCH) ? CACHE_BYTES / ((ix + 1) * 8) : CACHE_BATCH;

      cacheKeyOk = (pthread_key_create(&cacheKey, cacheRelease) == 0);
      if (cacheKeyOk == false)
        COR_W("corTreeHeap: no thread-exit hook - the blocks cached by a thread that ends are not used again");

      //
      // Out of a core dump until used: a dump to a pipe (apport, systemd-coredump) writes every page of a
      // mapping, the untouched ones as zeros - 64 GiB of them, and the crashing broker took minutes to die.
      // A 64 MiB window is put back in as its first page is handed out (corTreeHeapAlloc)
      //
      madvise(startP, size, MADV_DONTDUMP);

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

  int          ix    = (size + 7) / 8 - 1;
  int          bytes = (ix + 1) * 8;
  ThreadClass* tP    = &cacheV[ix];

  if ((tP->head == NULL) && (tP->next == tP->end))     // a range holds whole blocks: empty is next == end
  {
    //
    // The cache filled under the class's mutex once: a batch from its free list, or else a range of its page
    // - only pointers are moved, the range's memory is first touched when its blocks are used, with no lock
    //
    int        batch = batchV[ix];
    HeapClass* cP    = &classV[ix];

    cacheKeySet();
    pthread_mutex_lock(&cP->mutex);

    while ((tP->count < batch) && (cP->freeList != NULL))
    {
      void* blockP = cP->freeList;

      cP->freeList     = *(void**) blockP;
      *(void**) blockP = tP->head;
      tP->head         = blockP;
      ++tP->count;
    }

    if (tP->head == NULL)
    {
      if (cP->next + bytes > cP->end)
      {
        uint64_t offset = atomic_fetch_add(&heapUsed, 16384);

        if (offset + 16384 > heapSize)
        {
          pthread_mutex_unlock(&cP->mutex);
          return malloc(size);    // the range is full
        }

        if ((offset & ((64ULL << 20) - 1)) == 0)    // a new 64 MiB window: in a core dump from now on
          madvise(heapStart + offset, (heapSize - offset < (64ULL << 20))? heapSize - offset : (64ULL << 20), MADV_DODUMP);

        pageClassV[offset >> 14] = (uint8_t) ix;
        cP->next = heapStart + offset;
        cP->end  = cP->next + 16384;
      }

      long  left  = (long) ((cP->end - cP->next) / bytes);
      long  n     = (left < batch) ? left : batch;

      tP->next  = cP->next;
      tP->end   = cP->next + n * bytes;
      cP->next  = tP->end;
    }

    pthread_mutex_unlock(&cP->mutex);
  }

  void* blockP;

  if (tP->head != NULL)
  {
    blockP   = tP->head;
    tP->head = *(void**) blockP;
    --tP->count;
  }
  else
  {
    blockP    = tP->next;
    tP->next += bytes;
  }

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

  int          ix = pageClassV[(uint64_t) (p - heapStart) >> 14];
  ThreadClass* tP = &cacheV[ix];

  cacheKeySet();

  *(void**) blockP = tP->head;
  tP->head         = blockP;

  int batch = batchV[ix];

  if (++tP->count < 2 * batch)
    return;

  //
  // Too many waiting in this thread: a batch of them back to the class
  //
  void* tailP = tP->head;

  for (int n = 1; n < batch; n++)
    tailP = *(void**) tailP;

  void* headP = tP->head;

  tP->head   = *(void**) tailP;
  tP->count -= batch;

  classPut(&classV[ix], headP, tailP);
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
