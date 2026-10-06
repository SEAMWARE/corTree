#ifndef CORTREE_CLONE_H_
#define CORTREE_CLONE_H_

//
// FILE            corTreeClone.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include "corAlloc/CorAlloc.h"             // CorAlloc
#include "corTree/CorNode.h"               // CorNode



// -----------------------------------------------------------------------------
//
// corTreeClone - clone a node
//
extern CorNode* corTreeClone(CorAlloc* kaP, CorNode* nodeP);



// -----------------------------------------------------------------------------
//
// CorTreeMarkFn - called for a cloned container whose original carries a mark (see corTreeCloneMarked):
// 'copyP' is the clone, its children in place; 'origP' the original
//
typedef void (*CorTreeMarkFn)(CorAlloc* kaP, CorNode* copyP, CorNode* origP, void* ctx);



// -----------------------------------------------------------------------------
//
// corTreeCloneMarked - corTreeClone, and for every container whose flags have a bit of 'mask': 'fn' on
// its clone, after its children. The clone carries no bit of 'mask'.
//
// For a user that marks containers with something the clone must get and the original leaves out - one
// pass, the clone's own, instead of a walk of its own over every node.
//
extern CorNode* corTreeCloneMarked(CorAlloc* kaP, CorNode* nodeP, unsigned char mask, CorTreeMarkFn fn, void* ctx);

#endif  // CORTREE_CLONE_H_
