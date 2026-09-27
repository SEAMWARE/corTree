#ifndef CORTREE_NODE_DECOUPLE_H_
#define CORTREE_NODE_DECOUPLE_H_

//
// FILE            corTreeNodeDecouple.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include "corTree/CorNode.h"               // CorNode



// -----------------------------------------------------------------------------
//
// corTreeNodeDecouple -
//
extern void corTreeNodeDecouple(CorNode* parent, CorNode* nodeToDecouple, CorNode* prev);

#endif  // CORTREE_NODE_DECOUPLE_H_
