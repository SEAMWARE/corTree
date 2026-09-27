//
// FILE            corTreeFree.c - free allocated mem after JSON parse is done
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <string.h>                     // memset
#include <stdlib.h>                     // free

#include <stdbool.h>                         // bool
#include "corLog/corLog.h"                // COR_E, COR_RE, COR_T

#include "corTree/CorNode.h"               // CorNode
#include "corTree/corTreeTraceLevels.h"        // CorTreeTlFree
#include "corTree/corTreeFree.h"               // Own Interface



// -----------------------------------------------------------------------------
//
// corTreeFree
//
void corTreeFree(CorNode* kNodeP)
{
  if (kNodeP->name != NULL)
  {
    COR_T(CorTreeTlFree, "Freeing %s CorNode (%s) at %p", corTreeValueType(kNodeP->type), kNodeP->name, kNodeP);
  }
  else
  {
    COR_T(CorTreeTlFree, "Freeing %s CorNode at %p", corTreeValueType(kNodeP->type), kNodeP);
  }

  if ((kNodeP->type == CorArray) || (kNodeP->type == CorObject))
  {
    CorNode* nodeP = kNodeP->value.head;

    while (nodeP != NULL)
    {
      CorNode* next = nodeP->next;

      corTreeFree(nodeP);
      nodeP = next;
    }
  }
#if 0
  if (kNodeP->type == CorString)
    free(kNodeP->value.s);  // Seems to give problems sometimes with "Invalid free()" ...
#endif

  // Remember that the name of the node (kNodeP->name) is allocated together with the kNodeP itself. Only one free needed
  free(kNodeP);
}
