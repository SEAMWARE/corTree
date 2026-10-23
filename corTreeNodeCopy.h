#ifndef CORTREE_NODECOPY_H_
#define CORTREE_NODECOPY_H_

//
// FILE            corTreeNodeCopy.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
// SPDX-License-Identifier: Apache-2.0
//
#include "corTree/CorNode.h"                      // CorNode



// -----------------------------------------------------------------------------
//
// corTreeNodeCopy - *toP = *fromP, for a 'fromP' that may be a 32-byte node (CorNode.h: the prefix layout)
//
// Only the bytes 'fromP' has are read; a scalar's value.tail in 'toP' is left as it was. 'toP' is a whole
// node. Shallow: a container's copy shares its children.
//
extern void corTreeNodeCopy(CorNode* toP, const CorNode* fromP);

#endif  // CORTREE_NODECOPY_H_
