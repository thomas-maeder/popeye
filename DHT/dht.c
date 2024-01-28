/* This is dht.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-1999
 *	Institut fuer Informatik, TU Muenchen, Germany
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept intact
 * and in place.
 */

#include "debugging/assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "utilities/boolean.h"

#if defined(__unix__)
#include <unistd.h>
#endif

#if defined(__TURBOC__)   /* TLi */
#  include <mem.h>
#else
#  include <memory.h>
#  if defined(DBMALLOC)
#    include <dbmalloc.h>
#  endif /*DBMALLOC*/
#endif /*__TURBOC__*/

#include "dhtvalue.h"
#include "dht.h"
#include "debugging/trace.h"

#if defined(DEBUG_DHT) || defined(TESTHASH)
static int dhtDebug= 0;
int get_dhtDebug(void) {return dhtDebug;}
void set_dhtDebug(int const d) {dhtDebug = d;}
#endif /*DEBUG_DHT||TESTHASH*/

#if defined(DEBUG_DHT)
#define MYNAME(m)   static char const * const myname = #m;
#define DEBUG_CODE(x)                           \
  do {                                          \
    if (dhtDebug) { x }                         \
  } while (0)
#else
#  define DEBUG_CODE(x)
#  define MYNAME(m)
#endif /*DEBUG_DHT*/

#if !defined(New) /* TODO: Is this the correct check for all of the below lines? */
#  define New(type)    ((type *)fxfAlloc(sizeof(type)))
#  define nNew(n,type) ((type *)nNewImpl(n,sizeof(type)))
#  define Nil(type)    ((type *)0)
static inline void * nNewImpl(size_t const nmemb, size_t const size) {
  return ((size && (nmemb > (((size_t)-1)/size))) ? Nil(void) : fxfAlloc(nmemb*size));
}
#endif /*New*/

/* The next three values are those you may want to change */
#define DefaultMaxLoadFactor    300     /* in percent */
#define DefaultMinLoadFactor    100     /* in percent */


/* One problem in implementing dynamic hashing is the table to hold
 * the pointers to the hash elements. This table should dynamically
 * expand and shrink. The naive approach to use malloc/realloc bears
 * some problems. It may consume too much unused memory if realloc
 * is called with too large increments, or if this increment is too
 * short too many calls to realloc may result in poor performance.
 * We use another approach here, which results in fixed size chunks
 * of memory to be allocated and free'd.
 */

#define LD2_PTR_PER_DIR 8
#define PTR_PER_DIR  (1<<LD2_PTR_PER_DIR)
#define DIR_SIZE     (PTR_PER_DIR*sizeof(void *))
#define DIR_IDX_MASK (PTR_PER_DIR-1)

#define DIR_INDEX(l,x)  (((x)>>((l)*LD2_PTR_PER_DIR)) & DIR_IDX_MASK)

typedef unsigned long   dht_index_t;
#define MAX_LEVEL   ((sizeof(dht_index_t)*8 + LD2_PTR_PER_DIR-1) / LD2_PTR_PER_DIR)

typedef void *ht_dir[PTR_PER_DIR];

typedef struct {
    ht_dir *dir;     /* the last (partial) dir on this level */
    unsigned int valid;    /* the number of entries in partial dir */
} level_descr;

typedef struct {
    unsigned int level;      /* this is the topmost level */
    dht_index_t count;      /* total number of entries in table */
    level_descr ld[MAX_LEVEL];
} dirTable;

typedef struct {
    dirTable *dt;
    ht_dir *current;    /* current dir of this level */
    unsigned int index;   /* index to deliver next */
} dirEnumerate;

typedef struct InternHsElement {
    dhtElement      HsEl;
    struct InternHsElement  *Next;
} InternHsElement;

#define NilInternHsElement  Nil(InternHsElement)
#define NewInternHsElement  New(InternHsElement)
#define FreeInternHsElement(h)  fxfFree(h, sizeof(InternHsElement))

static InternHsElement EndOfTable;

#define freeDir(t)  fxfFree(t, sizeof(ht_dir))

typedef unsigned long uLong;
typedef unsigned char   uChar;
typedef unsigned short  uShort;

static void **accessAdr(dirTable *dt, uLong x)
{
  /* whether x is a valid index in the table is not checked */
  ht_dir *dir = dt->ld[dt->level].dir;
  void **result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)dt);
  TraceFunctionParam("%lu",x);
  TraceFunctionParamListEnd();

  TraceValue("%lu",dt->level);
  TraceEOL();

  assert(dt->level<4);
  switch (dt->level)
  {  /* all cases fall through */
    /*
      case 7: dir= (ht_dir *)(*dir)[DIR_INDEX(7,x)];
      case 6: dir= (ht_dir *)(*dir)[DIR_INDEX(6,x)];
      case 5: dir= (ht_dir *)(*dir)[DIR_INDEX(5,x)];
      case 4: dir= (ht_dir *)(*dir)[DIR_INDEX(4,x)];
    */
    case 3:
      dir= (ht_dir *)(*dir)[DIR_INDEX(3,x)];
      TraceValue("%p",(void *)dir);
      TraceEOL();
      /* FALLTHRU */
    case 2:
      dir= (ht_dir *)(*dir)[DIR_INDEX(2,x)];
      TraceValue("%p",(void *)dir);
      TraceEOL();
      /* FALLTHRU */
    case 1:
      dir= (ht_dir *)(*dir)[DIR_INDEX(1,x)];
      TraceValue("%p",(void *)dir);
      TraceEOL();
      /* FALLTHRU */
    case 0:
      result = &(*dir)[DIR_INDEX(0,x)];
      TraceValue("%p",(void *)result);
      TraceEOL();
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)result);
  TraceFunctionResultEnd();
  return result;
}

/* Append a dir table element to a dir table. Increases the level of
 * the dir table if all slots are occupied at the current level.
 * As its name indicates, this is a recursive function; the maximal
 * recursion level is dt->level+2.
 * @param dt address of dir table
 * @param elmt_to_append address of element to append
 * @param elmt_depth indicates the depth of the element to be appended.
 *                   elmt_depth==0: append a leaf
 *                   ...
 *                   elmt_depth==dt->level: insert at root level
 *                   elmt_depth==dt->level+1: increase *dt's level to allow
 *                                            appending
 * @return true iff insertion was successful
 */
static boolean appendDirTable_recursive(dirTable *dt,
                                        void *elmt_to_append,
                                        unsigned int elmt_depth)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)dt);
  TraceFunctionParam("%p",elmt_to_append);
  TraceFunctionParam("%d",elmt_depth);
  TraceFunctionParamListEnd();

  if (elmt_depth>dt->level)
  {
    /* All current slots are occupied -> increase dt->level
     */
    dt->ld[elmt_depth].dir = New(ht_dir);
    if (dt->ld[elmt_depth].dir==Nil(ht_dir))
      result = false;
    else
    {
      TraceValue("%p",(void *)dt->ld[elmt_depth-1].dir);
      TraceEOL();
      (*dt->ld[elmt_depth].dir)[0] = dt->ld[elmt_depth-1].dir;
      (*dt->ld[elmt_depth].dir)[1] = elmt_to_append;
      dt->ld[elmt_depth].valid = 2;
      dt->level = elmt_depth;
    }
  }
  else
  {
    unsigned int const nr_valid = dt->ld[elmt_depth].valid;
    TraceValue("%p",(void *)dt->ld[elmt_depth].dir);
    TraceValue("%d",dt->ld[elmt_depth].valid);
    TraceValue("%d",PTR_PER_DIR);
    TraceEOL();
    if (nr_valid<PTR_PER_DIR)
    {
      /* Insert at the first free slot at level dt->level-elmt_depth
       */
      (*dt->ld[elmt_depth].dir)[nr_valid] = elmt_to_append;
      dt->ld[elmt_depth].valid = nr_valid+1;
    }
    else
    {
      /* Insert a new subtree of height elmt_depth+1. This must happen
       * in the order:
       * 1. allocate subtree root
       * 2. recurse
       * 3. insert or deallocate subtree root, depending on success of
       *    2.
       * If (part of) 3. is done before 2., the tree will be in an
       * inconsistent state (and some memory will be leaked) if
       * 2. fails.
       */
      ht_dir * const subtree_root = New(ht_dir);
      TraceValue("%p",(void *)subtree_root);
      TraceEOL();
      if (subtree_root==Nil(ht_dir))
        result = false;
      else if (appendDirTable_recursive(dt,subtree_root,elmt_depth+1))
      {
        (*subtree_root)[0] = elmt_to_append;
        dt->ld[elmt_depth].dir = subtree_root;
        dt->ld[elmt_depth].valid = 1;
      }
      else
      {
        freeDir(subtree_root);
        result = false;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

/* Append a dir table element to a dir table. Increases the level of
 * the dir table if all slots are occupied at the current level.
 * @param dt address of dir table
 * @param elmt_to_append address of element to append
 * @return true iff insertion was successful
 */
static boolean appendDirTable(dirTable *dt, void *elmt_to_append)
{
  if (appendDirTable_recursive(dt,elmt_to_append,0))
  {
    ++dt->count;
    return true;
  }
  else
    return false;
}

/* TODO recursive implementation */
static void shrinkDirTable(dirTable *dt)
{
  unsigned int const i = dt->ld[0].valid;
  if (i>1)
  {
    dt->ld[0].valid = i-1;
    dt->count--;
  }
  else
  {
    if (dt->level==0)
    {
      if (i>0)
      {
        dt->ld[0].valid = i-1;
        dt->count--;
      }
    }
    else
    {
      unsigned int l = 0;
      while (l<=dt->level)
      {
        --dt->ld[l].valid;
        if (dt->ld[l].valid==0)
        {
          freeDir(dt->ld[l].dir);
          l++;
        }
        else
          break;
      }

      if (l==dt->level && dt->ld[l].valid==1)
      {
        --l;
        dt->level = l;
        dt->ld[l].dir = (*dt->ld[l+1].dir)[0];
        dt->ld[l].valid = PTR_PER_DIR;
        freeDir(dt->ld[l+1].dir);
      }

      while (l>0)
      {
        --l;
        dt->ld[l].dir= (*dt->ld[l+1].dir)[dt->ld[l+1].valid-1];
        dt->ld[l].valid= PTR_PER_DIR;
      }

      dt->count--;
    }
  }
}

static void freeDirTable(dirTable *dt)
{
  while (dt->count > 1)
  {
    dt->ld[0].valid= 1;
    dt->count=  ((dt->count-1) & ~((dht_index_t)DIR_IDX_MASK)) + 1;
    shrinkDirTable(dt);
  }
  TraceText("being freed:");
  TraceValue("%p",(void *)dt->ld[0].dir);
  TraceEOL();
  freeDir(dt->ld[0].dir);
}

#define TMDBG(x) if (0) x

static InternHsElement *stepDirTable(dirEnumerate *enumeration)
{
  InternHsElement *result = &EndOfTable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)enumeration);
  TraceFunctionParamListEnd();

  TraceValue("%lu ",enumeration->index);
  TraceValue("%lu",enumeration->dt->count);
  TraceEOL();
  TMDBG(printf("stepDirTable - index:%u count:%lu\n",enumeration->index,enumeration->dt->count));
  if (enumeration->index<enumeration->dt->count)
  {
    dht_index_t di = enumeration->index & DIR_IDX_MASK;
    TMDBG(printf("stepDirTable - di:%lu\n",di));
    if (di==0)
      enumeration->current= (ht_dir*)accessAdr(enumeration->dt,
                                               enumeration->index);
    assert(enumeration->current!=0);
    enumeration->index++;
    TraceValue("%p",(void *)enumeration->current);
    TraceValue("%p",(void *)*enumeration->current);
    TraceEOL();
    result = (*enumeration->current)[di];
  }
  else
  {
    TMDBG(puts("no further step"));
    TraceText("returning address of end of table pseudo-element\n");
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)result);
  TraceFunctionResultEnd();
  return result;
}

typedef struct
{
    dhtHashValue (*Hash)(dhtKey);
    int         (*Equal)(dhtKey, dhtKey);
    int         (*DupKeyValue)(dhtValue, dhtValue *);
    int         (*DupData)(dhtValue, dhtValue *);
    void        (*FreeKeyValue)(dhtValue);
    void        (*FreeData)(dhtValue);
    void        (*DumpData)(dhtValue,FILE *);
    void        (*DumpKeyValue)(dhtValue,FILE *);
} Procedures;

typedef struct dht {
    uLong       p;     /* Next bucket to split */
    uLong       maxp;  /* Upper bound on p during this expansion */
    uLong       KeyCount;       /* number keys stored in table */
    uShort      MinLoadFactor;  /* Lower bound on the load factor */
    uShort      MaxLoadFactor;  /* Upper bound on the load factor */
    uLong       CurrentSize;
    dirTable        DirTab;     /* The directory table */
    dirEnumerate    DirEnum;        /* stepping through the table */
    InternHsElement *NextStep;      /* the next element to deliver */
    dhtValuePolicy  KeyPolicy;      /* Whether we copy keys */
    dhtValuePolicy  DtaPolicy;      /* Whether we copy data */
    Procedures  procs;
} dht;
#if !defined(HashTable)
#define HashTable struct dht
#endif
#define NewHashTable        ((HashTable *)fxfAlloc(sizeof(dht)))
#define FreeHashTable(h)    fxfFree(h, sizeof(dht))
#define OVERFLOW_SAVE 1
#if defined(OVERFLOW_SAVE)
#define ActualLoadFactor(h)                     \
  ( (h)->CurrentSize < 10000                    \
    ?  ((h)->KeyCount*100) / (h)->CurrentSize   \
    :  (h)->KeyCount / (((h)->CurrentSize/100))   \
    )
#else
#define ActualLoadFactor(h) (((h)->KeyCount*100)/(h)->DirTab.count)
#endif /*OVERFLOW_SAVE*/

unsigned long dhtKeyCount(dht const *h)
{
  return h->KeyCount;
}

char dhtError[128];

char const *dhtErrorMsg(void)
{
  return dhtError;
}

dht *dhtCreate(dhtValueType KeyType, dhtValuePolicy KeyPolicy,
               dhtValueType DtaType, dhtValuePolicy DataPolicy)
{
  dht *result = Nil(dht);

  TraceFunctionEntry(__func__);
  TraceEOL();

  if (KeyType>=dhtValueTypeCnt)
    sprintf(dhtError,
            "dhtCreate: invalid KeyType: numeric=%u\n", (unsigned int) KeyType);
  else if (dhtProcedures[KeyType]==Nil(dhtValueProcedures))
    sprintf(dhtError,
            "dhtCreate: no procedure registered for KeyType \"%s\"\n",
            dhtValueTypeToString[KeyType]);
  else if (DtaType>=dhtValueTypeCnt)
    sprintf(dhtError,
            "dhtCreate: invalid DataType: numeric=%u\n", (unsigned int) DtaType);
  else if (dhtProcedures[DtaType]==Nil(dhtValueProcedures))
    sprintf(dhtError,
            "dhtCreate: no procedure registered for DtaType \"%s\"\n",
            dhtValueTypeToString[DtaType]);
  else if (KeyPolicy!=dhtNoCopy && KeyPolicy!=dhtCopy)
    sprintf(dhtError,
            "Sorry, unknown KeyPolicy: numeric=%u.", (unsigned int) KeyPolicy);
  else if (DataPolicy!=dhtNoCopy && DataPolicy!=dhtCopy)
    sprintf(dhtError,
            "Sorry, unknown DataPolicy: numeric=%u.", (unsigned int) DataPolicy);
  else
  {
    dht * const ht = NewHashTable;
    if (ht==Nil(dht))
      strcpy(dhtError, "dhtCreate: no memory.");
    else
    {
      ht->DirTab.ld[0].dir = New(ht_dir);
      if (ht->DirTab.ld[0].dir==Nil(ht_dir))
      {
        strcpy(dhtError,
               "dhtCreate: No memory for Directory segment.");
        FreeHashTable(ht);
      }
      else
      {
        ht->DirTab.level= 0;
        ht->DirTab.count= PTR_PER_DIR;
        ht->DirTab.ld[0].valid= PTR_PER_DIR;
        memset(ht->DirTab.ld[0].dir, 0, sizeof(ht_dir));
        ht->p=            0;
        ht->KeyCount=     0;
        ht->maxp=     PTR_PER_DIR;
        ht->CurrentSize=    ht->maxp;
        ht->MinLoadFactor=  DefaultMinLoadFactor;
        ht->MaxLoadFactor=  DefaultMaxLoadFactor;
        ht->KeyPolicy=        KeyPolicy;
        ht->DtaPolicy=        DataPolicy;

        ht->procs.Hash=     dhtProcedures[KeyType]->Hash;
        ht->procs.Equal=    dhtProcedures[KeyType]->Equal;
        ht->procs.DumpData= dhtProcedures[DtaType]->Dump;
        ht->procs.DumpKeyValue=  dhtProcedures[KeyType]->Dump;

        if (KeyPolicy==dhtNoCopy)
        {
          ht->procs.DupKeyValue= dhtProcedures[dhtSimpleValue]->Dup;
          ht->procs.FreeKeyValue= dhtProcedures[dhtSimpleValue]->Free;
        }
        else if (KeyPolicy==dhtCopy)
        {
          ht->procs.DupKeyValue= dhtProcedures[KeyType]->Dup;
          ht->procs.FreeKeyValue= dhtProcedures[KeyType]->Free;
        }

        if (DataPolicy==dhtNoCopy)
        {
          ht->procs.DupData= dhtProcedures[dhtSimpleValue]->Dup;
          ht->procs.FreeData= dhtProcedures[dhtSimpleValue]->Free;
        }
        else if (DataPolicy==dhtCopy)
        {
          ht->procs.DupData= dhtProcedures[DtaType]->Dup;
          ht->procs.FreeData= dhtProcedures[DtaType]->Free;
        }

        result = ht;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)result);
  TraceFunctionResultEnd();
  return result;
}

void dhtDestroy(HashTable *ht)
{
  dirEnumerate dEnum;
  InternHsElement *b;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  dEnum.index= 0;
  dEnum.current = 0;
  dEnum.dt= &ht->DirTab;

  for (b = stepDirTable(&dEnum); b!=&EndOfTable; b = stepDirTable(&dEnum))
    while (b)
    {
      InternHsElement *tmp= b;
      (ht->procs.FreeKeyValue)(b->HsEl.Key.value);
      (ht->procs.FreeData)(b->HsEl.Data);
      b= b->Next;
      FreeInternHsElement(tmp);
    }

  freeDirTable(&ht->DirTab);
  FreeHashTable(ht);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void dhtDumpIndented(int ind, HashTable *ht, FILE *f)
{
  dirEnumerate dEnum;
  int hcnt;
  InternHsElement *b;

  fprintf(f, "%*sSimple Values: \n", ind, "");
  fprintf(f, "%*sp (Next bucket to split) = %6lu\n",
          ind, "", ht->p);
  fprintf(f, "%*smaxp (Upper bound on p)  = %6lu\n",
          ind, "", ht->maxp);
  fprintf(f, "%*sKeyCount                 = %6lu\n",
          ind, "", ht->KeyCount);
  fprintf(f, "%*sCurrentSize              = %6lu\n",
          ind, "", ht->CurrentSize);
  fprintf(f, "%*sDirLevel                 = %6u\n",
          ind, "", ht->DirTab.level);
  hcnt=0;

  dEnum.index= 0;
  dEnum.current = 0;
  dEnum.dt= &ht->DirTab;

  for (b = stepDirTable(&dEnum); b!=&EndOfTable; b = stepDirTable(&dEnum))
    while (b)
    {
      fprintf(f, "%*s    ", ind, "");
      (ht->procs.DumpKeyValue)(b->HsEl.Key.value, f);
      fputs("->", f);
      (ht->procs.DumpData)(b->HsEl.Data, f);
      b= b->Next;
      fputc('\n', f);
      hcnt++;
    }

  fprintf(f, "%*s%d records of %lu dumped\n\n",
          ind, "", hcnt, ht->KeyCount);
}

void dhtDump(HashTable *ht, FILE *f)
{
  dhtDumpIndented(0, ht, f);
}

dhtElement *dhtGetFirstElement(HashTable *ht)
{
  InternHsElement *b;
  dhtElement *result = dhtNilElement;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
  TraceFunctionParamListEnd();

  if (ht->KeyCount>0)
  {
    ht->DirEnum.index= 0;
    ht->DirEnum.dt = &ht->DirTab;

    for (b = stepDirTable(&ht->DirEnum);
         b!=&EndOfTable;
         b = stepDirTable(&ht->DirEnum))
      if (b!=0)
      {
        ht->NextStep= b->Next;
        result = &b->HsEl;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)result);
  TraceFunctionResultEnd();
  return result;
}

dhtElement *dhtGetNextElement(HashTable *ht)
{
  InternHsElement *b;

  if (ht->NextStep)
  {
    dhtElement *de= &ht->NextStep->HsEl;
    ht->NextStep= ht->NextStep->Next;
    return de;
  }

  for (b = stepDirTable(&ht->DirEnum);
       b!=&EndOfTable;
       b = stepDirTable(&ht->DirEnum))
    if (b != 0)
    {
      ht->NextStep= b->Next;
      return &b->HsEl;
    }

  return dhtNilElement;
}

LOCAL uLong DynamicHash(uLong p, uLong maxp, dhtHashValue v)
{
  uLong const h = v % maxp;
  uLong result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%lu ",p);
  TraceFunctionParam("%lu ",maxp);
  TraceFunctionParam("%08x",v);
  TraceFunctionParamListEnd();

  if (h<p)
    result = v % (maxp<<1);
  else
    result = h;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

LOCAL dhtStatus ExpandHashTable(HashTable *ht)
{
  static char const * const myname = "ExpandHashTable";
  /* Need to expand the directory */
  uLong oldp= ht->p;
  uLong newp= ht->maxp + ht->p;
  dhtStatus result = dhtFailedStatus;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
  TraceFunctionParamListEnd();

  TMDBG(printf("ExpandHashTable() - ht->DirTab.ld[0].valid:%u\n",
               ht->DirTab.ld[0].valid));

  if (appendDirTable(&ht->DirTab,0))
  {
    ht->CurrentSize++;

    /* update bucket pointers */
    ht->p++;
    if (ht->p == ht->maxp)
    {
      ht->maxp<<= 1;
      ht->p= 0;
    }

    /* relocate records */
    {
      InternHsElement **newPointer = (InternHsElement **)accessAdr(&ht->DirTab,
                                                                   newp);
      InternHsElement **oldPointer = (InternHsElement **)accessAdr(&ht->DirTab,
                                                                   oldp);

      TraceValue("%lu ",oldp);
      TraceValue("%p",(void *)oldPointer);
      TraceEOL();
      TraceValue("%lu ",newp);
      TraceValue("%p",(void *)newPointer);
      TraceEOL();
      assert(oldPointer!=0);
      while (*oldPointer)
      {
        InternHsElement const *oldElmt = *oldPointer;
        TraceValue("%p ",(void *)*oldPointer);
        {
          dhtHashValue const hashVal = (ht->procs.Hash)(oldElmt->HsEl.Key);
          TraceValue("%lu",hashVal);
          TraceEOL();
          if (DynamicHash(ht->p,ht->maxp,hashVal)==newp)
          {
            *newPointer = *oldPointer;
            *oldPointer = (*oldPointer)->Next;
            newPointer = &(*newPointer)->Next;
            *newPointer = NilInternHsElement;
          }
          else
            oldPointer= &(*oldPointer)->Next;
        }
      }
    }

    result = dhtOkStatus;
  }
  else
    sprintf(dhtError, "%s: no memory\n", myname);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

LOCAL void ShrinkHashTable(HashTable *ht)
{
  InternHsElement   **oldPointer, **newPointer;
  uLong     oldp;

  if (ht->maxp == PTR_PER_DIR && ht->p == 0)
    return;
  if (ht->p == 0)
  {
    ht->maxp>>= 1;
    ht->p= ht->maxp;
  }
  ht->p--;

  TMDBG(puts("ShrinkHashTable()"));
  newPointer= (InternHsElement**)accessAdr(&ht->DirTab, ht->p);
  oldp= ht->p + ht->maxp;
  oldPointer= (InternHsElement**)accessAdr(&ht->DirTab, oldp);

  assert(oldPointer!=0);
  if (*oldPointer)
  {
    while (*newPointer)
      newPointer= &(*newPointer)->Next;
    *newPointer= *oldPointer;
    *oldPointer= NilInternHsElement;
  }
  ht->CurrentSize--;
  shrinkDirTable(&ht->DirTab);
}

LOCAL InternHsElement **LookupInternHsElement(HashTable *ht, dhtKey key)
{
  uLong h;
  InternHsElement **phe;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
  TraceFunctionParam("%jx",key.value.unsigned_integer);
#else
  TraceFunctionParam("%lx",(unsigned long)key.value.unsigned_integer);
#endif
  TraceFunctionParamListEnd();

  h = DynamicHash(ht->p, ht->maxp, (ht->procs.Hash)(key));
  phe = (InternHsElement**)accessAdr(&ht->DirTab, h);
  TMDBG(printf("h:%lu\n",h));

  assert(phe!=0);
  while (*phe)
    if ((ht->procs.Equal)((*phe)->HsEl.Key, key))
    {
      TraceText("found");
      TraceEOL();
      break;
    }
    else
      phe= &((*phe)->Next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)*phe);
  TraceFunctionResultEnd();
  return phe;
}

void dhtRemoveElement(HashTable *ht, dhtKey key)
{
  MYNAME(dhtRemoveElement)
  InternHsElement **phe, *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
  TraceFunctionParam("%jx",key.value.unsigned_integer);
#else
  TraceFunctionParam("%lx",(unsigned long)key.value.unsigned_integer);
#endif
  TraceFunctionParamListEnd();

  phe= LookupInternHsElement(ht, key);
  if (*phe)
  {
    DEBUG_CODE(
      fprintf(stderr, "%s: dumping before removing\n", myname);
      dhtDump(ht, stderr);
      );
    he= *phe;
    if (ht->NextStep == he)
      ht->NextStep= ht->NextStep->Next;

    *phe= he->Next;
    (ht->procs.FreeData)(he->HsEl.Data);
    (ht->procs.FreeKeyValue)(he->HsEl.Key.value);
    FreeInternHsElement(he);
    ht->KeyCount--;
    if (ActualLoadFactor(ht) < ht->MinLoadFactor)
    {
      DEBUG_CODE(
        fprintf(stderr,
                "%s: dumping before shrinking\n", myname);
        dhtDump(ht, stderr);
        );
      ShrinkHashTable(ht);
      DEBUG_CODE(
        fprintf(stderr,
                "%s: dumping after shrinking\n", myname);
        dhtDump(ht, stderr);
        );
    }
    DEBUG_CODE(
      fprintf(stderr, "%s: dumping after removing\n", myname);
      dhtDump(ht, stderr);
      );
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

dhtElement *dhtEnterElement(HashTable *ht, dhtKey key, dhtValue data)
{
  InternHsElement **phe, *he;
  dhtKey KeyV;
  dhtValue DataV;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
  TraceFunctionParam("%jx",key.value.unsigned_integer);
  TraceFunctionParam("%jx",data.unsigned_integer);
#else
  TraceFunctionParam("%lx",(unsigned long)key.value.unsigned_integer);
  TraceFunctionParam("%lx",(unsigned long)data.unsigned_integer);
#endif
  TraceFunctionParamListEnd();

  assert(key.value.object_pointer!=0); /* TODO: This assert assumes that object_pointer is the active member.
                                          Is there a more generic test we could do?  Do we need one? */
  if ((ht->procs.DupKeyValue)(key.value, &KeyV.value))
  {
    TraceText("key duplication failed\n");
    TraceFunctionExit(__func__);
    TraceFunctionResult("%p",(void *)dhtNilElement);
    TraceFunctionResultEnd();
    return dhtNilElement;
  }

  if ((ht->procs.DupData)(data, &DataV))
  {
    (ht->procs.FreeKeyValue)(KeyV.value);
    TraceText("data duplication failed\n");
    TraceFunctionExit(__func__);
    TraceFunctionResult("%p",(void *)dhtNilElement);
    TraceFunctionResultEnd();
    return dhtNilElement;
  }

  phe = LookupInternHsElement(ht,key);
  TraceValue("%p",(void *)phe);
  he = *phe;
  TraceValue("%p",(void *)he);
  TraceEOL();
  if (he==0)
  {
    he = NewInternHsElement;
    TraceValue("%p",(void *)he);
    TraceEOL();
    if (he==0)
    {
      (ht->procs.FreeKeyValue)(KeyV.value);
      (ht->procs.FreeData)(DataV);
      TraceText("allocation of new intern Hs element failed\n");
      TraceFunctionExit(__func__);
      TraceFunctionResult("%p",(void *)dhtNilElement);
      TraceFunctionResultEnd();
      return dhtNilElement;
    }
    else
    {
      *phe = he;
      he->Next = NilInternHsElement;
      ht->KeyCount++;
    }
  }
  else
  {
    if (ht->DtaPolicy == dhtCopy)
      (ht->procs.FreeData)(he->HsEl.Data);
    if (ht->KeyPolicy == dhtCopy)
      (ht->procs.FreeKeyValue)(he->HsEl.Key.value);
  }

  he->HsEl.Key = KeyV;
  he->HsEl.Data = DataV;

  if (ActualLoadFactor(ht)>ht->MaxLoadFactor)
  {
    /*
      fputs("Dumping Hash-Table before expansion\n",stderr);
      fDumpHashTable(ht, stderr);
    */
    if (ExpandHashTable(ht)!=dhtOkStatus)
    {
      TraceText("expansion failed\n");
      TraceFunctionExit(__func__);
      TraceFunctionResult("%p",(void *)dhtNilElement);
      TraceFunctionResultEnd();
      return dhtNilElement;
    }
    /*
      fputs("Dumping Hash-Table after expansion\n",stderr);
      fDumpHashTable(ht, stderr);
    */
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)&he->HsEl);
  TraceFunctionResultEnd();
  return &he->HsEl;
}

dhtElement *dhtLookupElement(HashTable *ht, dhtKey key)
{
  InternHsElement **phe;
  dhtElement *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
  TraceFunctionParamListEnd();

  phe= LookupInternHsElement(ht,key);
  if (*phe)
    result = &(*phe)->HsEl;
  else
    result = dhtNilElement;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int dhtBucketStat(HashTable *ht, unsigned int *counter, unsigned int n)
{
  unsigned int BucketCount = 0;
  dhtElement const *he = dhtGetFirstElement(ht);

  memset(counter, 0, n*sizeof(counter[0]));
  while (he!=Nil(dhtElement))
  {
    unsigned int len = 1;
    InternHsElement *ihe = ((InternHsElement const *)he)->Next;
    while (ihe!=0)
    {
      ++len;
      ht->NextStep = ihe;
      ihe = ihe->Next;
    }

    ++BucketCount;
    if (len<n)
      ++counter[len-1];
    else
      ++counter[n-1];

    he = dhtGetNextElement(ht);
  }

  return BucketCount;
}
