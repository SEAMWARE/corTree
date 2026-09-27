//
// FILE            corTreeChildReplace.c - replace a CorNode child with another
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <unistd.h>                             // NULL

#include "corTree/CorNode.h"                       // CorNode



// -----------------------------------------------------------------------------
//
// corTreeChildReplace -
//
void corTreeChildReplace(CorNode* container, CorNode* outP, CorNode* inP)
{
  CorNode* prev  = NULL;
  int     found = 0;

  for (CorNode* childP = container->value.firstChildP; childP != NULL; childP = childP->next)
  {
    if (childP == outP)
    {
      found = 1;
      break;
    }

    prev = childP;
  }

  if (found == 0)
    return;

  if (prev == NULL)
  {
    container->value.firstChildP = inP;
    inP->next = outP->next;
  }
  else
  {
    prev->next = inP;
    inP->next  = outP->next;
  }

  if (container->lastChild == outP)
    container->lastChild = inP;
}
