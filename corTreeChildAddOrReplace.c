//
// FILE            corTreeChildAddOrReplace.c -
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <unistd.h>                                            // NULL

#include "corTree/CorNode.h"                                      // CorNode
#include "corTree/corTreeLookup.h"                                    // corTreeLookup
#include "corTree/corTreeBuilder.h"                                   // corTreeChildAdd
#include "corTree/corTreeChildReplace.h"                               // corTreeChildReplace
#include "corTree/corTreeChildAddOrReplace.h"                         // Own interface



// -----------------------------------------------------------------------------
//
// corTreeChildAddOrReplace - replace the child called 'itemName' by 'replacementP', or add it
//
// The replacement node takes the old child's place in the list - the old node is not overwritten:
// a scalar on the tree heap is 32 bytes (CorNode.h), too small to become a container, and its name
// follows it in the same block.
//
// RETURN VALUE
//   The child that was replaced, now out of the tree (the caller frees it if it is on the tree heap),
//   or NULL if 'replacementP' was added.
//
CorNode* corTreeChildAddOrReplace(CorNode* container, const char* itemName, CorNode* replacementP)
{
  CorNode* itemToReplace = corTreeLookup(container, itemName);

  if (itemToReplace == NULL)
    corTreeChildAdd(container, replacementP);
  else
  {
    corTreeChildReplace(container, itemToReplace, replacementP);
    itemToReplace->next = NULL;
  }

  return itemToReplace;
}
