# corTree — the Tree, and Nothing Else

The in-memory tree every other part of the coraine stack works on: the node, and
the functions that build, edit, search, clone and sort a tree of them. It has no
parser and no renderer. JSON in and out is **corJson**; any other format — the
binary one the `cor://` protocol and corDB persistence will use, XML, YAML — is
a sibling of corJson that produces or consumes the same tree.

- **Version:** 0.1.0
- **Language:** C
- **License:** [Apache License 2.0](LICENSE)

The only dependencies are **kalloc, ktrace and kbase**.

## Where it comes from

corTree and corJson are the two halves of **kjson**, which was one library that
held both the representation and the JSON parser. That was the design mistake
the split corrects: with the tree on its own, every format is just another
producer or consumer of it. kjson itself is untouched and keeps serving its own
users; this is a copy of its implementation under new names, not a fork.

The one change of substance is the **allocator**. kjson's builders took the
parser's handle, `Kjson*`, and used nothing of it but the allocator inside —
which made the tree depend on the parser. corTree's builders take the allocator
itself:

```c
CorNode* corTreeString(KAlloc* kaP, const char* name, const char* value);
```

`NULL` still means `malloc`, exactly as a `NULL` `Kjson*` did.

## The node

```c
typedef struct CorNode
{
  char*            name;       // NULL/"" for an array item
  CorValueType     type;       // CorString, CorInt, CorFloat, CorBoolean, CorNull, CorObject, CorArray
  unsigned char    flags;      // for the users of the library; in the padding, born 0 under kalloc
  CorValue         value;      // b, i, f, s, or - for a container - head and tail
  struct CorNode*  next;       // next sibling
} CorNode;
```

40 bytes on a 64-bit machine.

## API

| | |
|---|---|
| build | `corTreeObject`, `corTreeArray`, `corTreeString`, `corTreeInteger`, `corTreeFloat`, `corTreeBoolean`, `corTreeNull` |
| edit | `corTreeChildAdd`, `corTreeChildAddSorted`, `corTreeChildAddSortedReverse`, `corTreeChildPrepend`, `corTreeChildRemove`, `corTreeChildMove`, `corTreeChildReplace`, `corTreeChildAddOrReplace`, `corTreeNodeDecouple` |
| find | `corTreeLookup`, `corTreeNavigate`, `corTreeStringSiblingFind`, `corTreeStringValueLookupInArray`, `corTreeChildCount` |
| copy, free | `corTreeClone`, `corTreeFree` |
| sort | `corTreeSort`, `corTreeArraySort`, `corTreeStringArraySort`, `corTreeStringArraySortedInsert` |

## Build

A sibling repo of the rest of the stack: `-I..` and `../corTree/libcorTree.a`.

```sh
make di
```
