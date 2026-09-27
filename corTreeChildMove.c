//
// FILE            corTreeChildMove.c - move a child from one container to another
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
//
// SPDX-License-Identifier: Apache-2.0
//
#include <stddef.h>                                                 // NULL

#include "corTree/CorNode.h"                                        // CorNode
#include "corTree/corTreeBuilder.h"                                 // corTreeChildRemove, corTreeChildAdd
#include "corTree/corTreeChildMove.h"                               // Own interface



// -----------------------------------------------------------------------------
//
// corTreeChildMove - move a child from one container to the end of another
//
// corTreeChildRemove only unlinks (the node's memory belongs to its allocator),
// so the node is intact and can be appended to 'to' right away.
// A child that is not in 'from' is left alone - appending it would cut short
// whatever list it IS in.
//
void corTreeChildMove(CorNode* from, CorNode* to, CorNode* child)
{
  if (corTreeChildRemove(from, child) != NULL)
    corTreeChildAdd(to, child);
}
