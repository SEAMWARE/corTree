//
// FILE            corTreeStringArraySortedInsert.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <unistd.h>                              // NULL
#include <string.h>                              // strcmp

#include "corTree/CorNode.h"                        // CorNode
#include "corTree/corTreeStringArraySortedInsert.h"     // Own interface



// -----------------------------------------------------------------------------
//
// corTreeStringArraySortedInsert -
//
void corTreeStringArraySortedInsert(CorNode* arrayP, CorNode* newItemP)
{
  CorNode* prev  = NULL;
  CorNode* itemP = arrayP->value.firstChildP;

  while (itemP != NULL)
  {
    int cmp = strcmp(itemP->value.s, newItemP->value.s);  // <0 if itemP < newItemP,  ==0 id equal

    if (cmp < 0)
      prev = itemP;
    else if (cmp == 0)
      return;  // Already present values are skipped

    itemP = itemP->next;
  }

  if (prev != NULL)
  {
    if (prev->next == NULL)  // Insert as last item
    {
      prev->next        = newItemP;
      newItemP->next    = NULL;
      arrayP->lastChild = newItemP;
    }
    else  // Insert the middle
    {
      newItemP->next = prev->next;
      prev->next     = newItemP;
    }
  }
  else  // Insert as first item
  {
    newItemP->next = arrayP->value.firstChildP;
    arrayP->value.firstChildP = newItemP;

    if (arrayP->lastChild == NULL)
      arrayP->lastChild = newItemP;
  }
}
