//
// FILE            corTreeStringSiblingFind.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <unistd.h>                           // NULL
#include <string.h>                           // strcmp

#include "corTree/CorNode.h"                     // CorNode



// -----------------------------------------------------------------------------
//
// corTreeStringSiblingFind -
//
CorNode* corTreeStringSiblingFind(CorNode* arrayItemP, const char* value)
{
  while (arrayItemP != NULL)
  {
    if (arrayItemP->type == CorString)
    {
      if (strcmp(arrayItemP->value.s, value) == 0)
        return arrayItemP;
    }

    arrayItemP = arrayItemP->next;
  }

  return NULL;
}
