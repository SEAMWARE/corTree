//
// FILE            corTreeNodeCopy.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
// SPDX-License-Identifier: Apache-2.0
//
#include <stddef.h>                               // offsetof
#include <string.h>                               // memcpy

#include "corTree/CorNode.h"                      // CorNode
#include "corTree/corTreeNodeCopy.h"              // Own interface



// -----------------------------------------------------------------------------
//
// corTreeNodeCopy -
//
void corTreeNodeCopy(CorNode* toP, const CorNode* fromP)
{
  if ((fromP->type == CorObject) || (fromP->type == CorArray))
    memcpy(toP, fromP, sizeof(CorNode));
  else
    memcpy(toP, fromP, offsetof(CorNode, value.tail));
}
