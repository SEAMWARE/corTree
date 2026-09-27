//
// FILE            corTreeLookup.c - lookup a member of a container
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <stdio.h>                      // NULL
#include <string.h>                     // strcmp

#include "kbase/kStrEq.h"               // kStrEq
#include "kbase/kLibLog.h"              // K Log macros

#include "corTree/corTreeTraceLevels.h"        // Kjl*
#include "corTree/CorNode.h"               // CorNode
#include "corTree/corTreeLookup.h"             // Own Interface



// -----------------------------------------------------------------------------
//
// corTreeLookup - look up a node named 'name' in container 'container'
//
// For containers of type CorObject only, the name of the nodes (children of the object)
// are compared tothe parameter 'name' and if equal, a pointer to the node is returned.
//
CorNode* corTreeLookup(CorNode* container, const char* name)
{
  if (name == NULL)
    return NULL;

  if (container->type == CorObject)
  {
    CorNode* current;

    for (current = container->value.firstChildP; current != NULL; current = current->next)
    {
      if (kStrEq(current->name, name) == true)
        return current;
    }
  }

  return NULL;
}





// -----------------------------------------------------------------------------
//
// corTreeLookup - look up a node named 'name' in container 'container'
//
// For containers of type CorObject only, the name of the nodes (children of the object)
// are compared tothe parameter 'name' and if equal, a pointer to the node is returned.
//
CorNode* corTreeLookupWithStrcmp(CorNode* container, char* name)
{
  if (name == NULL)
    return NULL;

  if (container->type == CorObject)
  {
    CorNode* current;

    for (current = container->value.firstChildP; current != NULL; current = current->next)
    {
      if (strcmp(current->name, name) == 0)
        return current;
    }
  }

  return NULL;
}
