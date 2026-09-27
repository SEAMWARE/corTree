//
// FILE            corTreeStringArraySort.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <string.h>                             // strcmp, memset
#include <unistd.h>                             // NULL

#include "corTree/CorNode.h"                       // CorNode
#include "corTree/corTreeBuilder.h"                    // corTreeArray
#include "corTree/corTreeStringArraySort.h"            // Own interface



// -----------------------------------------------------------------------------
//
// corTreeStringArraySort - sort a CorNode Array
//
// Elements (CorString) are sorted alphabetically by value.
//
void corTreeStringArraySort(CorNode* arrayP)
{
  CorNode  raw;
  CorNode* rawP = &raw;

  memset(&raw, 0, sizeof(raw));
  rawP->type = CorArray;

  //
  // Put all items in rawP, leaving arrayP empty
  //
  rawP->value.head          = arrayP->value.head;
  rawP->value.tail          = arrayP->value.tail;
  arrayP->value.head = NULL;
  arrayP->value.tail        = NULL;

  //
  // Loop over rawP, find smallest, then just move it to arrayP
  //
  while (rawP->value.head != NULL)
  {
    // Set the very first item as the smallest one (minP)
    CorNode* minP = rawP->value.head;

    // Compare with all the rest and set minP accordingly
    for (CorNode* tmpP = minP->next; tmpP != NULL; tmpP = tmpP->next)
    {
      if (strcmp(minP->value.s, tmpP->value.s) > 0)
        minP = tmpP;
    }

    // Move the smallest from rawP to arrayP
    corTreeChildRemove(rawP, minP);
    corTreeChildAdd(arrayP, minP);
  }
}
