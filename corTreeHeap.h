#ifndef CORTREE_HEAP_H_
#define CORTREE_HEAP_H_

//
// FILE            corTreeHeap.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
// SPDX-License-Identifier: Apache-2.0
//
// The heap of the trees that live on - built with no allocator (kaP == NULL) and freed with corTreeFree.
// A node, its name and its string value are ONE block, as with malloc, but without malloc's 8-byte header
// and 16-byte rounding: blocks are 8-byte aligned, carved from 16 KiB pages that each hold one size class.
// A free looks the class up by the page - nothing in the node is read, so a node whose name or value was
// changed after it was built is freed as exactly what it was allocated as.
//
// Blocks over 1024 bytes are plain malloc, and so is every block in a build with AddressSanitizer and when
// the environment variable COR_TREE_HEAP is "malloc" (valgrind): those see a block's bounds in malloc's
// only. corTreeHeapFree tells the two apart by address.
//
#include <stdint.h>                               // uint64_t

#include "corTree/CorNode.h"                      // CorNode



// -----------------------------------------------------------------------------
//
// corTreeHeapAlloc - a block of 'size' bytes
//
extern void* corTreeHeapAlloc(int size);



// -----------------------------------------------------------------------------
//
// corTreeHeapFree - free a block from corTreeHeapAlloc
//
extern void corTreeHeapFree(void* blockP);



// -----------------------------------------------------------------------------
//
// corTreeHeapBytes - the bytes of the pages handed out (in use + free blocks + page tails)
//
extern uint64_t corTreeHeapBytes(void);

#endif  // CORTREE_HEAP_H_
