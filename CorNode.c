//
// FILE            CorNode.c - utility functions for node values
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <stdio.h>                      // sprintf

#include "corBase/corMacros.h"          // COR_FT, et al

#include "corTree/CorNode.h"               // Own interface



// -----------------------------------------------------------------------------
//
// corTreeValueType -
//
const char* corTreeValueType(CorValueType vt)
{
  switch (vt)
  {
  case CorNone:     return "None";
  case CorString:   return "String";
  case CorInt:      return "Int";
  case CorFloat:    return "Float";
  case CorBoolean:  return "Bool";
  case CorNull:     return "Null";
  case CorObject:   return "Object";
  case CorArray:    return "Array";
  }

  return "Unknown";
}



// -----------------------------------------------------------------------------
//
// corTreeValue -
//
char* corTreeValue(CorNode* nodeP, char* buf, int bufLen)
{
  switch (nodeP->type)
  {
  case CorNone:     return "None";
  case CorString:   return nodeP->value.s;
  case CorObject:   return "Object";
  case CorArray:    return "Array";
  case CorBoolean:  return COR_FT(nodeP->value.b == true);
  case CorNull:     return "Null";

  case CorInt:
      snprintf(buf, bufLen, "%lld", nodeP->value.i);

    return buf;

  case CorFloat:
      snprintf(buf, bufLen, "%f", nodeP->value.f);
    return buf;
  }

  return "unknown value";
}
