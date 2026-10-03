//
// FILE            corTreeBin.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
// SPDX-License-Identifier: Apache-2.0
//
// The cor format - see corTreeBin.h, and coraine's doc/cor-protocol.md.
//
// Encoder and decoder in one file, deliberately: the wire constants below are the format, and they
// are shared by exactly these two and nothing else.
//
#include <stdbool.h>                              // bool
#include <stdint.h>                               // uint8_t, uint16_t, uint32_t, uint64_t
#include <stdlib.h>                               // malloc, realloc, calloc, free
#include <string.h>                               // memcpy, memmove, memchr, memset, strlen, strcmp, strstr, strncmp
#include <math.h>                                 // fabs
#include <float.h>                                // FLT_MAX

#include "corAlloc/CorAlloc.h"                    // CorAlloc
#include "corAlloc/corAlloc.h"                    // corAlloc
#include "corTree/CorNode.h"                      // CorNode
#include "corTree/corTreeBin.h"                   // Own interface



// -----------------------------------------------------------------------------
//
// The tag byte
//
enum
{
  TAG_TYPE_MASK     = 0x07,     // bits 0-2: CorValueType
  TAG_NAME_SHIFT    = 3,        // bits 3-4: name mode
  TAG_NAME_MASK     = 0x18,
  TAG_FLAGS_SHIFT   = 5,        // bits 5-7: per value type
};

enum
{
  NAME_NONE         = 0,        // an array element
  NAME_TERM         = 1,        // a core-term id
  NAME_NEW          = 2,        // a string - and, room permitting, the names table's next entry
  NAME_INDEX        = 3         // an index into the names table
};

enum
{
  INT_VARINT        = 7,        // flags 0-6 are the integer itself; 7: a zigzag varint follows
  FLOAT_32          = 1,        // flags bit 0: 4 bytes (lossless as a float32), else 8
  STRING_TERM       = 1,        // flags bit 0: the value is a core-term id, not a string
  KIND_BYTE         = 7         // an object's flags are its kind, 0-6; 7: the kind is in the next byte
};

enum
{
  NS_NONE           = 0,        // the string has no namespace part - it is all local
  NS_NEW            = 1,        // a namespace follows inline - and is the namespace table's next entry
  NS_INDEX_BASE     = 2         // 2 + an index into the namespace table
};

enum
{
  TERM_ESCAPE       = 0xFF      // a core-term id: one byte, or 0xFF + two bytes
};



// -----------------------------------------------------------------------------
//
// tableHash - FNV-1a
//
static uint32_t tableHash(const char* s)
{
  uint32_t h = 2166136261u;

  while (*s != 0)
  {
    h ^= (uint8_t) *s++;
    h *= 16777619u;
  }

  return h;
}



// -----------------------------------------------------------------------------
//
// tableInit -
//
static bool tableInit(CorBinTable* tP, int max)
{
  memset(tP, 0, sizeof(CorBinTable));

  tP->max      = max;
  tP->hashSize = 16;

  while (tP->hashSize < max * 2)
    tP->hashSize *= 2;

  tP->stringV = calloc(max > 0 ? max : 1, sizeof(char*));
  tP->hashV   = calloc(tP->hashSize, sizeof(int));

  return (tP->stringV != NULL) && (tP->hashV != NULL);
}



// -----------------------------------------------------------------------------
//
// tableLookup - the index of s, or -1
//
static int tableLookup(CorBinTable* tP, const char* s)
{
  uint32_t mask = tP->hashSize - 1;

  for (uint32_t ix = tableHash(s) & mask; tP->hashV[ix] != 0; ix = (ix + 1) & mask)
  {
    int index = tP->hashV[ix] - 1;

    if (strcmp(tP->stringV[index], s) == 0)
      return index;
  }

  return -1;
}



// -----------------------------------------------------------------------------
//
// tableAdd - the next entry, a copy of s[0..len); false when the table is full (or out of memory)
//
// Writer and reader call this at the same point of the stream, so their tables stay identical - a
// full table is full on both sides.
//
static bool tableAdd(CorBinTable* tP, const char* s, int len)
{
  if (tP->count >= tP->max)
    return false;

  char* copy = malloc(len + 1);
  if (copy == NULL)
    return false;

  memcpy(copy, s, len);
  copy[len] = 0;

  uint32_t mask = tP->hashSize - 1;
  uint32_t ix   = tableHash(copy) & mask;

  while (tP->hashV[ix] != 0)
    ix = (ix + 1) & mask;

  tP->stringV[tP->count] = copy;
  tP->hashV[ix]          = ++tP->count;

  return true;
}



// -----------------------------------------------------------------------------
//
// tableRelease -
//
static void tableRelease(CorBinTable* tP)
{
  for (int i = 0; i < tP->count; i++)
    free(tP->stringV[i]);

  free(tP->stringV);
  free(tP->hashV);
  memset(tP, 0, sizeof(CorBinTable));
}



// -----------------------------------------------------------------------------
//
// corTreeBinTablesInit -
//
bool corTreeBinTablesInit(CorBinTables* tablesP, int namespacesMax, int namesMax)
{
  return tableInit(&tablesP->namespaces, namespacesMax) && tableInit(&tablesP->names, namesMax);
}



// -----------------------------------------------------------------------------
//
// corTreeBinTablesPreload -
//
bool corTreeBinTablesPreload(CorBinTables* tablesP, const char** namespaceV, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (tableAdd(&tablesP->namespaces, namespaceV[i], strlen(namespaceV[i])) == false)
      return false;
  }

  return true;
}



// -----------------------------------------------------------------------------
//
// corTreeBinTablesRelease -
//
void corTreeBinTablesRelease(CorBinTables* tablesP)
{
  tableRelease(&tablesP->namespaces);
  tableRelease(&tablesP->names);
}



// =============================================================================
//
// ENCODER
//
// =============================================================================



// -----------------------------------------------------------------------------
//
// room - make sure n more bytes fit
//
static bool room(CorBinBuffer* bP, int n)
{
  if (bP->len + n <= bP->size)
    return true;

  //
  // In 64 bits, and refused past an int - CorBinBuffer's length is one. Doubled in an int, a buffer
  // past 1 GiB overflowed and this loop never ended
  //
  long long need = (long long) bP->len + n;
  long long size = (bP->size == 0) ? 256 : bP->size;

  if (need > 0x7FFFFFFF)
    return false;

  while (size < need)
    size *= 2;
  if (size > 0x7FFFFFFF)
    size = 0x7FFFFFFF;

  char* buf = realloc(bP->buf, (size_t) size);
  if (buf == NULL)
    return false;

  bP->buf  = buf;
  bP->size = (int) size;

  return true;
}



// -----------------------------------------------------------------------------
//
// put - n bytes
//
static bool put(CorBinBuffer* bP, const void* p, int n)
{
  if (room(bP, n) == false)
    return false;

  memcpy(&bP->buf[bP->len], p, n);
  bP->len += n;

  return true;
}



// -----------------------------------------------------------------------------
//
// putByte -
//
static bool putByte(CorBinBuffer* bP, uint8_t b)
{
  return put(bP, &b, 1);
}



// -----------------------------------------------------------------------------
//
// varintLen - the bytes a value takes as an unsigned LEB128 varint
//
static int varintLen(uint64_t v)
{
  int n = 1;

  while (v >= 0x80)
  {
    v >>= 7;
    ++n;
  }

  return n;
}



// -----------------------------------------------------------------------------
//
// varintWrite - into p, which has room; the bytes written
//
static int varintWrite(uint8_t* p, uint64_t v)
{
  int n = 0;

  while (v >= 0x80)
  {
    p[n++] = (uint8_t) (v | 0x80);
    v >>= 7;
  }
  p[n++] = (uint8_t) v;

  return n;
}



// -----------------------------------------------------------------------------
//
// putVarint -
//
static bool putVarint(CorBinBuffer* bP, uint64_t v)
{
  uint8_t tmp[10];

  return put(bP, tmp, varintWrite(tmp, v));
}



// -----------------------------------------------------------------------------
//
// putTerm - a core-term id: one byte, or the escape and two
//
static bool putTerm(CorBinBuffer* bP, uint16_t termId)
{
  if (termId < TERM_ESCAPE)
    return putByte(bP, (uint8_t) termId);

  uint8_t v[3] = { TERM_ESCAPE, (uint8_t) (termId & 0xFF), (uint8_t) (termId >> 8) };
  return put(bP, v, 3);
}



// -----------------------------------------------------------------------------
//
// putLocal - varint length, the bytes, the NUL
//
static bool putLocal(CorBinBuffer* bP, const char* s, int len)
{
  return putVarint(bP, len) && put(bP, s, len) && putByte(bP, 0);
}



// -----------------------------------------------------------------------------
//
// namespaceSplit - where an IRI's namespace ends (after its last '/', '#' or ':'), or 0 for no split
//
// Only an IRI is split: something with "://", or a URN. A timestamp has ':' too, and splitting it
// would only fill the table with dates.
//
static int namespaceSplit(const char* s)
{
  if ((strstr(s, "://") == NULL) && (strncmp(s, "urn:", 4) != 0))
    return 0;

  int split = 0;

  for (int i = 0; s[i] != 0; i++)
  {
    if ((s[i] == '/') || (s[i] == '#') || (s[i] == ':'))
      split = i + 1;
  }

  return split;
}



// -----------------------------------------------------------------------------
//
// putString - a string as namespace reference + local part
//
static bool putString(CorBinBuffer* bP, CorBinTables* tablesP, const char* s)
{
  int len   = strlen(s);
  int split = (tablesP != NULL) ? namespaceSplit(s) : 0;

  if (split == 0)
    return putVarint(bP, NS_NONE) && putLocal(bP, s, len);

  char ns[split + 1];

  memcpy(ns, s, split);
  ns[split] = 0;

  int index = tableLookup(&tablesP->namespaces, ns);

  if (index >= 0)
    return putVarint(bP, NS_INDEX_BASE + index) && putLocal(bP, &s[split], len - split);

  //
  // A new namespace: inline, and the table's next entry - unless the table is full, when the
  // whole string simply goes as it is (the reader's table is full at the same point)
  //
  if (tableAdd(&tablesP->namespaces, ns, split) == false)
    return putVarint(bP, NS_NONE) && putLocal(bP, s, len);

  return putVarint(bP, NS_NEW) && putLocal(bP, ns, split) && putLocal(bP, &s[split], len - split);
}



// -----------------------------------------------------------------------------
//
// nodeEncode -
//
static bool nodeEncode(CorNode* nodeP, const CorBinCodec* codecP, CorBinTables* tablesP, CorBinBuffer* bP, bool inArray)
{
  uint8_t  type      = (uint8_t) nodeP->type;
  uint8_t  flags     = 0;
  uint16_t valueTerm = 0;
  uint8_t  kind      = 0;
  CorNode* foldedP   = NULL;

  //
  // The name: none in an array, else a core-term id, a names-table index, or a new string
  //
  uint8_t  nameMode  = NAME_NONE;
  uint16_t nameTerm  = 0;
  int      nameIndex = -1;

  if ((inArray == false) && (nodeP->name != NULL))
  {
    //
    // Only through the callback: node->termId is the user's, and opaque here - corNgsild keeps a
    // "not a core term" marker in it (0x8000), which as an id on the wire would be nonsense
    //
    if ((codecP != NULL) && (codecP->nameTermId != NULL))
      nameTerm = codecP->nameTermId(nodeP);

    if ((nameTerm != 0) && (codecP != NULL) && (codecP->termName != NULL))
      nameMode = NAME_TERM;
    else
    {
      nameIndex = (tablesP != NULL) ? tableLookup(&tablesP->names, nodeP->name) : -1;
      nameMode  = (nameIndex >= 0) ? NAME_INDEX : NAME_NEW;
    }
  }

  //
  // The value's flags
  //
  switch (nodeP->type)
  {
  case CorInt:
    flags = ((nodeP->value.i >= 0) && (nodeP->value.i < INT_VARINT)) ? (uint8_t) nodeP->value.i : INT_VARINT;
    break;

  case CorFloat:
    //
    // Outside float's range the cast itself is undefined behaviour - so the range comes first
    //
    flags = ((fabs(nodeP->value.f) <= FLT_MAX) && (((double) (float) nodeP->value.f) == nodeP->value.f)) ? FLOAT_32 : 0;
    break;

  case CorBoolean:
    flags = (nodeP->value.b == true) ? 1 : 0;
    break;

  case CorString:
    if ((codecP != NULL) && (codecP->valueTermId != NULL) && (codecP->termName != NULL))
      valueTerm = codecP->valueTermId(nodeP->value.s);
    flags = (valueTerm != 0) ? STRING_TERM : 0;
    break;

  case CorObject:
    if ((codecP != NULL) && (codecP->objectKind != NULL))
      kind = codecP->objectKind(nodeP, &foldedP) & 0x0F;
    flags = (kind < KIND_BYTE) ? kind : KIND_BYTE;
    break;

  default:
    break;
  }

  if (putByte(bP, type | (nameMode << TAG_NAME_SHIFT) | (flags << TAG_FLAGS_SHIFT)) == false)
    return false;

  if ((nodeP->type == CorObject) && (flags == KIND_BYTE) && (putByte(bP, kind) == false))
    return false;

  //
  // The name
  //
  if (nameMode == NAME_TERM)
  {
    if (putTerm(bP, nameTerm) == false)
      return false;
  }
  else if (nameMode == NAME_INDEX)
  {
    if (putVarint(bP, nameIndex) == false)
      return false;
  }
  else if (nameMode == NAME_NEW)
  {
    if (putString(bP, tablesP, nodeP->name) == false)
      return false;

    if (tablesP != NULL)
      tableAdd(&tablesP->names, nodeP->name, strlen(nodeP->name));
  }

  //
  // The value
  //
  switch (nodeP->type)
  {
  case CorInt:
    if (flags == INT_VARINT)
    {
      uint64_t zz = ((uint64_t) nodeP->value.i << 1) ^ (uint64_t) (nodeP->value.i >> 63);
      return putVarint(bP, zz);
    }
    return true;

  case CorFloat:
    if (flags == FLOAT_32)
    {
      float f = (float) nodeP->value.f;
      return put(bP, &f, 4);
    }
    return put(bP, &nodeP->value.f, 8);

  case CorString:
    if (valueTerm != 0)
      return putTerm(bP, valueTerm);
    return putString(bP, tablesP, nodeP->value.s);

  case CorObject:
  case CorArray:
    {
      //
      // The byte length precedes the children but is known only after them: one byte is reserved,
      // and only a container of 128 bytes or more moves its children to make room for a longer one
      //
      if (putByte(bP, 0) == false)
        return false;

      int lenAt = bP->len - 1;

      for (CorNode* childP = nodeP->value.head; childP != NULL; childP = childP->next)
      {
        if (childP == foldedP)
          continue;

        if (nodeEncode(childP, codecP, tablesP, bP, nodeP->type == CorArray) == false)
          return false;
      }

      uint64_t childBytes = bP->len - (lenAt + 1);
      int      vlen       = varintLen(childBytes);

      if (vlen > 1)
      {
        if (room(bP, vlen - 1) == false)
          return false;

        memmove(&bP->buf[lenAt + vlen], &bP->buf[lenAt + 1], childBytes);
        bP->len += vlen - 1;
      }

      varintWrite((uint8_t*) &bP->buf[lenAt], childBytes);
    }
    return true;

  default:                                         // boolean, null: all in the tag
    return true;
  }
}



// -----------------------------------------------------------------------------
//
// corTreeBinEncode -
//
bool corTreeBinEncode(CorNode* treeP, const CorBinCodec* codecP, CorBinTables* tablesP, CorBinBuffer* outP)
{
  return nodeEncode(treeP, codecP, tablesP, outP, false);
}



// =============================================================================
//
// DECODER
//
// Every read is bounds-checked against the end of the input: malformed bytes are an error, never a
// read past the buffer.
//
// =============================================================================



typedef struct Reader
{
  const uint8_t*      p;
  const uint8_t*      end;
  const CorBinCodec*  codecP;
  CorBinTables*       tablesP;
  CorAlloc*           kaP;
  const char*         error;
} Reader;



// -----------------------------------------------------------------------------
//
// fail -
//
static void* fail(Reader* rP, const char* error)
{
  if (rP->error == NULL)
    rP->error = error;

  return NULL;
}



// -----------------------------------------------------------------------------
//
// memAlloc - from the allocator, or malloc
//
static void* memAlloc(Reader* rP, int size)
{
  //
  // Rounded up to 8: the allocator hands out consecutive bytes, so an odd-length joined string would
  // leave the next CorNode misaligned - slow on x86, a fault on some ARM. (Found by UBSan.)
  //
  size = (size + 7) & ~7;

  return (rP->kaP != NULL) ? corAlloc(rP->kaP, size) : malloc(size);
}



// -----------------------------------------------------------------------------
//
// getByte -
//
static bool getByte(Reader* rP, uint8_t* bP)
{
  if (rP->p >= rP->end)
  {
    fail(rP, "truncated");
    return false;
  }

  *bP = *rP->p++;
  return true;
}



// -----------------------------------------------------------------------------
//
// getVarint -
//
static bool getVarint(Reader* rP, uint64_t* vP)
{
  uint64_t v     = 0;
  int      shift = 0;
  uint8_t  b;

  do
  {
    if ((shift > 63) || (getByte(rP, &b) == false))
    {
      fail(rP, "bad varint");
      return false;
    }

    v     |= (uint64_t) (b & 0x7F) << shift;
    shift += 7;
  } while ((b & 0x80) != 0);

  *vP = v;
  return true;
}



// -----------------------------------------------------------------------------
//
// getTerm - a core-term id, and its name
//
static const char* getTerm(Reader* rP, uint16_t* termIdP)
{
  uint8_t b;

  if (getByte(rP, &b) == false)
    return NULL;

  uint16_t termId = b;

  if (b == TERM_ESCAPE)
  {
    uint8_t lo, hi;

    if ((getByte(rP, &lo) == false) || (getByte(rP, &hi) == false))
      return NULL;

    termId = (uint16_t) (lo | (hi << 8));
  }

  const char* name = ((rP->codecP != NULL) && (rP->codecP->termName != NULL)) ? rP->codecP->termName(termId) : NULL;

  if (name == NULL)
    return fail(rP, "unknown core-term id");

  *termIdP = termId;
  return name;
}



// -----------------------------------------------------------------------------
//
// getLocal - a varint-length string with its NUL, in place; *lenP its length
//
static const char* getLocal(Reader* rP, int* lenP)
{
  uint64_t len;

  if (getVarint(rP, &len) == false)
    return NULL;

  if (len >= (uint64_t) (rP->end - rP->p))                       // the bytes and the NUL
    return fail(rP, "string runs past the end");

  const char* s = (const char*) rP->p;

  if (s[len] != 0)
    return fail(rP, "string without its NUL");

  if (memchr(s, 0, len) != NULL)
    return fail(rP, "string with a NUL inside");

  rP->p += len + 1;
  *lenP  = (int) len;

  return s;
}



// -----------------------------------------------------------------------------
//
// getString - namespace reference + local part; in place unless it has a namespace
//
static const char* getString(Reader* rP)
{
  uint64_t    ns;
  int         len;
  const char* nsString;

  if (getVarint(rP, &ns) == false)
    return NULL;

  if (ns == NS_NONE)
    return getLocal(rP, &len);

  if (rP->tablesP == NULL)
    return fail(rP, "a namespace, and no tables");

  if (ns == NS_NEW)
  {
    int nsLen;

    if ((nsString = getLocal(rP, &nsLen)) == NULL)
      return NULL;

    if (tableAdd(&rP->tablesP->namespaces, nsString, nsLen) == false)
      return fail(rP, "a new namespace, and the table is full");
  }
  else
  {
    uint64_t index = ns - NS_INDEX_BASE;

    if (index >= (uint64_t) rP->tablesP->namespaces.count)
      return fail(rP, "namespace index out of range");

    nsString = rP->tablesP->namespaces.stringV[index];
  }

  const char* local = getLocal(rP, &len);
  if (local == NULL)
    return NULL;

  int   nsLen = strlen(nsString);
  char* joined = memAlloc(rP, nsLen + len + 1);

  if (joined == NULL)
    return fail(rP, "out of memory");

  memcpy(joined, nsString, nsLen);
  memcpy(&joined[nsLen], local, len + 1);

  return joined;
}



// -----------------------------------------------------------------------------
//
// nodeDecode -
//
static CorNode* nodeDecode(Reader* rP, int depth)
{
  uint8_t tag;

  if (depth > 256)
    return fail(rP, "nested too deep");

  if (getByte(rP, &tag) == false)
    return NULL;

  uint8_t type     = tag & TAG_TYPE_MASK;
  uint8_t nameMode = (tag & TAG_NAME_MASK) >> TAG_NAME_SHIFT;
  uint8_t flags    = tag >> TAG_FLAGS_SHIFT;
  uint8_t kind     = 0;

  if ((type < CorString) || (type > CorArray))
    return fail(rP, "unknown value type");

  if (type == CorObject)
  {
    kind = flags;

    if ((flags == KIND_BYTE) && (getByte(rP, &kind) == false))
      return NULL;
  }

  CorNode* nodeP = memAlloc(rP, sizeof(CorNode));
  if (nodeP == NULL)
    return fail(rP, "out of memory");

  memset(nodeP, 0, sizeof(CorNode));
  nodeP->type = (CorValueType) type;

  //
  // The name
  //
  if (nameMode == NAME_TERM)
  {
    uint16_t termId;
    const char* name = getTerm(rP, &termId);

    if (name == NULL)
      return NULL;

    nodeP->name   = (char*) name;
    nodeP->termId = termId;
  }
  else if (nameMode == NAME_INDEX)
  {
    uint64_t index;

    if ((rP->tablesP == NULL) || (getVarint(rP, &index) == false) || (index >= (uint64_t) rP->tablesP->names.count))
      return fail(rP, "name index out of range");

    nodeP->name = rP->tablesP->names.stringV[index];
  }
  else if (nameMode == NAME_NEW)
  {
    const char* name = getString(rP);
    if (name == NULL)
      return NULL;

    nodeP->name = (char*) name;

    if (rP->tablesP != NULL)
      tableAdd(&rP->tablesP->names, name, strlen(name));
  }

  //
  // The value
  //
  switch (nodeP->type)
  {
  case CorInt:
    if (flags == INT_VARINT)
    {
      uint64_t zz;

      if (getVarint(rP, &zz) == false)
        return NULL;

      nodeP->value.i = (long long) ((zz >> 1) ^ (~(zz & 1) + 1));
    }
    else
      nodeP->value.i = flags;
    break;

  case CorFloat:
    if ((flags & FLOAT_32) != 0)
    {
      float f;

      if (rP->end - rP->p < 4)
        return fail(rP, "truncated");
      memcpy(&f, rP->p, 4);
      rP->p += 4;
      nodeP->value.f = f;
    }
    else
    {
      if (rP->end - rP->p < 8)
        return fail(rP, "truncated");
      memcpy(&nodeP->value.f, rP->p, 8);
      rP->p += 8;
    }
    break;

  case CorBoolean:
    nodeP->value.b = ((flags & 1) != 0);
    break;

  case CorNull:
    break;

  case CorString:
    if ((flags & STRING_TERM) != 0)
    {
      uint16_t termId;
      const char* s = getTerm(rP, &termId);

      if (s == NULL)
        return NULL;
      nodeP->value.s = (char*) s;
    }
    else
    {
      const char* s = getString(rP);

      if (s == NULL)
        return NULL;
      nodeP->value.s = (char*) s;
    }
    break;

  case CorObject:
  case CorArray:
    {
      uint64_t len;

      if (getVarint(rP, &len) == false)
        return NULL;

      if (len > (uint64_t) (rP->end - rP->p))
        return fail(rP, "container runs past the end");

      const uint8_t* end      = rP->p + len;
      const uint8_t* outerEnd = rP->end;

      rP->end = end;                                 // a child cannot run past its container
      while (rP->p < end)
      {
        CorNode* childP = nodeDecode(rP, depth + 1);

        if (childP == NULL)
          return NULL;

        if ((nodeP->type == CorArray) && (childP->name != NULL))
          return fail(rP, "a named array element");

        if ((nodeP->type == CorObject) && (childP->name == NULL))
          return fail(rP, "an unnamed object member");

        if (nodeP->value.tail == NULL)
          nodeP->value.head = childP;
        else
          nodeP->value.tail->next = childP;
        nodeP->value.tail = childP;
      }
      rP->end = outerEnd;

      if ((kind != 0) && (rP->codecP != NULL) && (rP->codecP->objectKindExpand != NULL))
        rP->codecP->objectKindExpand(nodeP, kind, rP->kaP);
    }
    break;

  default:
    return fail(rP, "unknown value type");
  }

  return nodeP;
}



// -----------------------------------------------------------------------------
//
// corTreeBinDecode -
//
CorNode* corTreeBinDecode(const char* buf, int len, const CorBinCodec* codecP, CorBinTables* tablesP, CorAlloc* kaP, const char** errorP)
{
  Reader r = { (const uint8_t*) buf, (const uint8_t*) buf + len, codecP, tablesP, kaP, NULL };

  CorNode* treeP = nodeDecode(&r, 0);

  if ((treeP != NULL) && (r.p != r.end))
  {
    treeP   = NULL;
    r.error = "bytes after the tree";
  }

  if (errorP != NULL)
    *errorP = (treeP == NULL) ? r.error : NULL;

  return treeP;
}
