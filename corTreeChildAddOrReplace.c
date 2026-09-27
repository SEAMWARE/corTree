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
#include "corTree/corTreeChildAddOrReplace.h"                         // Own interface



// -----------------------------------------------------------------------------
//
// corTreeChildAddOrReplace -
//
void corTreeChildAddOrReplace(CorNode* container, const char* itemName, CorNode* replacementP)
{
  CorNode* itemToReplace = corTreeLookup(container, itemName);

  if (itemToReplace == NULL)
    corTreeChildAdd(container, replacementP);
  else
  {
    itemToReplace->type  = replacementP->type;
    itemToReplace->value = replacementP->value;
    // CorNode::cSum and CorNode::valueString aren't used
  }
}
