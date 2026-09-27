//
// FILE            corTreeStringValueLookupInArray.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <string.h>                                              // strcmp

#include "corTree/CorNode.h"                                        // CorNode



// -----------------------------------------------------------------------------
//
// corTreeStringValueLookupInArray -
//
// NOTE
//   This lookup function works on string items in an array.
//   The caller needs to be sure that the 'stringArray' is really an array and that ALL its items are Strings
//
CorNode* corTreeStringValueLookupInArray(CorNode* stringArray, const char* value)
{
  if (stringArray != NULL)
  {
    if ((stringArray->type != CorArray) && (stringArray->type != CorObject))
      return NULL;

    for (CorNode* nodeP = stringArray->value.head; nodeP != NULL; nodeP = nodeP->next)
    {
      if (nodeP->type != CorString)
        continue;

      // LM_T(LmtRegMatch, ("Comparing attr '%s' with '%s'", value, nodeP->value.s));
      if (strcmp(value, nodeP->value.s) == 0)
        return nodeP;
    }
  }

  return NULL;
}
