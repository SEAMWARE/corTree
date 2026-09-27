//
// FILE            corTreeNodeDecouple.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <unistd.h>                           // NULL

#include "corTree/CorNode.h"                     // CorNode
#include "corTree/corTreeNodeDecouple.h"             // Own interface



// -----------------------------------------------------------------------------
//
// corTreeNodeDecouple -
//
void corTreeNodeDecouple(CorNode* parent, CorNode* nodeToDecouple, CorNode* prev)
{
  if (prev != NULL)
    prev->next = nodeToDecouple->next;
  else
    parent->value.head = nodeToDecouple->next;

  if (parent->value.tail == nodeToDecouple)
    parent->value.tail = prev;
}
