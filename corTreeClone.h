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

#endif  // CORTREE_CLONE_H_
