#ifndef CORTREE_CHILD_MOVE_H_
#define CORTREE_CHILD_MOVE_H_

//
// FILE            corTreeChildMove.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
//
// SPDX-License-Identifier: Apache-2.0
//
#include "corTree/CorNode.h"                // CorNode



// -----------------------------------------------------------------------------
//
// corTreeChildMove - move a child from one container to the end of another
//
// corTreeChildAdd alone is NOT a move: it overwrites child->next, which truncates
// 'from' after 'child' and leaves 'from->value.tail' pointing into 'to'.
//
extern void corTreeChildMove(CorNode* from, CorNode* to, CorNode* child);

#endif  // CORTREE_CHILD_MOVE_H_
