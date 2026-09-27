//
// FILE            corTreeSort.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <string.h>                     // strcmp

#include "corTree/CorNode.h"               // CorNode
#include "corTree/corTreeBuilder.h"            // corTreeChildRemove



// -----------------------------------------------------------------------------
//
// corTreeSort - sort a CorNode Object
//
// Objects are sorted alphabetically by key (ascending: 'a' before 'z').
// Arrays are not sorted - use corTreeStringArraySort for that.
//
// Implementation: in-place selection sort. Each iteration finds the minimum
// node in the unsorted suffix and splices it onto the tail of the sorted
// prefix. The previous implementation had a bug — it always prepended to the
// global firstChildP rather than the head of the unsorted suffix, so already-
// sorted prefix elements got displaced and the result was neither ascending
// nor descending. (It was symmetric across calls though, which is why
// MongoCommonUpdate.cpp's compare-after-sort still produced the right answer
// for change detection.)
//
void corTreeSort(CorNode* nodeP)
{
  if ((nodeP->type != CorObject) && (nodeP->type != CorArray))  // Arrays can contain objects, that can be sorted,
    return;

  if (nodeP->value.firstChildP == NULL)
    return;

  // Recursive calls for all child items that are Object or Array
  for (CorNode* currentP = nodeP->value.firstChildP; currentP != NULL; currentP = currentP->next)
  {
    if ((currentP->type == CorObject) || (currentP->type == CorArray))
      corTreeSort(currentP);
  }

  // Arrays aren't sorted
  if (nodeP->type == CorArray)
    return;

  // Selection sort the children: find min in unsorted suffix, splice it onto
  // the tail of the sorted prefix.
  CorNode* sortedTail = NULL;  // Last node in already-sorted prefix; NULL when prefix is empty

  while (1)
  {
    CorNode* unsortedHead = (sortedTail == NULL) ? nodeP->value.firstChildP : sortedTail->next;

    if ((unsortedHead == NULL) || (unsortedHead->next == NULL))
      break;

    // Find minimum in unsorted suffix
    CorNode* minP = unsortedHead;
    for (CorNode* p = unsortedHead->next; p != NULL; p = p->next)
    {
      if (strcmp(minP->name, p->name) > 0)
        minP = p;
    }

    if (minP == unsortedHead)
    {
      // Already at head of unsorted suffix; just extend the sorted prefix.
      sortedTail = unsortedHead;
      continue;
    }

    // Splice minP out of its current position and place it at the head of the
    // unsorted suffix (i.e. just after sortedTail).
    corTreeChildRemove(nodeP, minP);
    if (sortedTail == NULL)
    {
      minP->next = nodeP->value.firstChildP;
      nodeP->value.firstChildP = minP;
    }
    else
    {
      minP->next = sortedTail->next;
      sortedTail->next = minP;
    }
    sortedTail = minP;
  }

  // Refresh lastChild (corTreeChildRemove may have left it stale)
  CorNode* lastP = nodeP->value.firstChildP;
  if (lastP != NULL)
  {
    while (lastP->next != NULL)
      lastP = lastP->next;
    nodeP->lastChild = lastP;
  }
}
