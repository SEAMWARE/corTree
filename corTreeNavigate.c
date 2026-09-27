//
// FILE            corTreeNavigate.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <string.h>                                              // strncpy
#include <unistd.h>                                              // NULL

#include <stdbool.h>                                                // bool
#include "corTree/CorNode.h"                                        // CorNode
#include "corTree/corTreeLookup.h"                                      // corTreeLookup
#include "corTree/corTreeNavigate.h"                                    // Own interface



// -----------------------------------------------------------------------------
//
// corTreeNavigate -
//
CorNode* corTreeNavigate(CorNode* treeP, const char** pathCompV, CorNode** parentPP, bool* onlyLastMissingP)
{
  CorNode* hitP = corTreeLookup(treeP, pathCompV[0]);

  if (parentPP != NULL)
    *parentPP = treeP;

  if (hitP == NULL)  // No hit - we're done
  {
    if (onlyLastMissingP != NULL)
      *onlyLastMissingP = (pathCompV[1] == NULL)? true : false;

    return NULL;
  }

  if (pathCompV[1] == NULL)  // Found it - we're done
    return hitP;

  return corTreeNavigate(hitP, &pathCompV[1], parentPP, onlyLastMissingP);  // Recursive call, one level down
}



// -----------------------------------------------------------------------------
//
// corTreeNavigate2 - true KJ-Tree navigation
//
CorNode* corTreeNavigate2(CorNode* treeP, char** compV)
{
  CorNode* hitP = corTreeLookup(treeP, compV[0]);

  if (hitP == NULL)
    return NULL;

  if (compV[1] == NULL)
    return hitP;

  return corTreeNavigate2(hitP, &compV[1]);
}
