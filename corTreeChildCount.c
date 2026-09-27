//
// FILE            corTreeChildCount.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <stddef.h>                     // NULL

#include "corTree/CorNode.h"               // CorNode



// -----------------------------------------------------------------------------
//
// corTreeChildCount -
//
int corTreeChildCount(CorNode* containerP)
{
  int     children = 0;
  CorNode* childP   = containerP->value.head;

  while (childP != NULL)
  {
    ++children;
    childP = childP->next;
  }

  return children;
}
