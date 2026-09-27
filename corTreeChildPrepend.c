//
// FILE            corTreeChildPrepend.c - insert a CorNode at the beginning of a list
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include "corTree/CorNode.h"                                        // CorNode
#include "corTree/corTreeChildPrepend.h"                                // Own interface



// -----------------------------------------------------------------------------
//
// corTreeChildPrepend -
//
void corTreeChildPrepend(CorNode* container, CorNode* child)
{
  child->next = container->value.firstChildP;
  container->value.firstChildP = child;
}
