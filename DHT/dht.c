/* This is dht.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-1999
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept intact
 * and in place.
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#if defined(__unix__)
#include <unistd.h>
#endif

#if defined(__TURBOC__)   /* TLi */
#  include <mem.h>
#else
#  include <memory.h>
#  if defined(DBMALLOC)
#    include "dbmalloc.h"
#  endif /*DBMALLOC*/
#endif /*__TURBOC__*/

#include "dhtvalue.h"
#include "dht.h"

#if defined(DEBUG_DHT)
int dhtDebug= 0;
#define MYNAME(m)   static char *myname = #m;
#define DEBUG_CODE(x)                           \
  do {                                          \
    if (dhtDebug) { x }                         \
  } while (0)
#else
#  define DEBUG_CODE(x)
#  define MYNAME(m)
#endif /*DEBUG_DHT*/

#if !defined(New)
#  define New(type)    (type *)fxfAlloc(sizeof(type))
#  define nNew(n,type) (type *)fxfAlloc((n)*sizeof(type))
#  define Nil(type)    (type *)0
#endif /*New*/

/* The next three values are those you may want to change */
#define DefaultMaxLoadFactor    300     /* in percent */
#define DefaultMinLoadFactor    100     /* in percent */


/* One problem in implementing dynamic hashing is the table to hold
 * the pointers to the hash elements. This table should dynamicly
 * expand and shrink. The naiv approach to use malloc/realloc bears
 * some problems. It may consume to much unused memory if realloc
 * is called with too large increments, or if this increment is too
 * short to many calls to realloc may result in poor performance.
 * We use another approach here, which results in fixed size chunks
 * of memory to be allocated and free'd.
 */

#define LD2_PTR_PER_DIR 8
#define PTR_PER_DIR  (1<<LD2_PTR_PER_DIR)
#define DIR_SIZE     (PTR_PER_DIR*sizeof(void *))
#define DIR_IDX_MASK (PTR_PER_DIR-1)

#define DIR_INDEX(l,x)  (((x)>>(l*LD2_PTR_PER_DIR)) & DIR_IDX_MASK)

typedef unsigned long   dht_index_t;
#define MAX_LEVEL   ((sizeof(dht_index_t)*8 + LD2_PTR_PER_DIR-1) / LD2_PTR_PER_DIR)

typedef void *ht_dir[PTR_PER_DIR];

typedef struct {
    ht_dir *dir;     /* the last (partial) dir on this level */
    int    valid;    /* the number of entries in partial dir */
} level_descr;

typedef struct {
    int     level;      /* this is the topmost level */
    dht_index_t count;      /* total number of entries in table */
    level_descr ld[MAX_LEVEL];
} dirTable; 

#define freeDir(t)  fxfFree(t, sizeof(ht_dir))

static void **accessAdr(dirTable *ht, int x) {
  /* whether x is a valid index in the table is not checked */
  ht_dir *dir;
  dir= ht->ld[ht->level].dir;
  switch (ht->level) {  /* all cases fall through */
    /*
      case 7: dir= (ht_dir *)(*dir)[DIR_INDEX(7,x)];
      case 6: dir= (ht_dir *)(*dir)[DIR_INDEX(6,x)];
      case 5: dir= (ht_dir *)(*dir)[DIR_INDEX(5,x)];
      case 4: dir= (ht_dir *)(*dir)[DIR_INDEX(4,x)];
    */
  case 3: dir= (ht_dir *)(*dir)[DIR_INDEX(3,x)];
  case 2: dir= (ht_dir *)(*dir)[DIR_INDEX(2,x)];
  case 1: dir= (ht_dir *)(*dir)[DIR_INDEX(1,x)];
  case 0: return &(*dir)[DIR_INDEX(0,x)];
  }
  /* never reached - just to keep compiler quiet */
  return & (*dir)[0];
}

int appendDirTable(dirTable *ht, void *x) {
  int i, l;
  ht_dir *dir, *ndir;

  i= ht->ld[0].valid;
  if (i < PTR_PER_DIR) {
    (*ht->ld[0].dir)[i]= x;
    ht->ld[0].valid= i+1;
    ht->count++;
    return 0;
  }
  ndir= New(ht_dir);
  if (ndir == Nil(ht_dir))
    return -1;
  (*ndir)[0]= x;
  dir= ht->ld[0].dir;
  ht->ld[0].dir= (void*)ndir;
  ht->ld[0].valid= 1;
  l= 0;
  while (++l <= ht->level) {
    ht_dir *nndir;
    i= ht->ld[l].valid;
    if (i < PTR_PER_DIR) {
      (*ht->ld[l].dir)[i]= ndir;
      ht->ld[l].valid= i+1;
      ht->count++;
      return 0;
    }
    dir= ht->ld[l].dir;
    nndir= New(ht_dir);
    if (nndir == Nil(ht_dir))
      return -1;
    (*nndir)[0]= (void*)ndir;
    ndir= nndir;
    ht->ld[l].valid= 1;
    ht->ld[l].dir= ndir;
  }
  ht->ld[l].dir= New(ht_dir);
  if (ht->ld[l].dir == Nil(ht_dir))
    return -1;
  (*ht->ld[l].dir)[0]= dir;
  (*ht->ld[l].dir)[1]= ndir;
  ht->ld[l].valid= 2;
  ht->level= l;
  ht->count++;
  return 0;
}

static void shrinkDirTable(dirTable *ht) {
  int i, l;

  i= ht->ld[0].valid;
  if (i > 1) {
    ht->ld[0].valid= i-1;
    ht->count--;
    return;
  }
  if (ht->level == 0) {
    if (i > 0) {
      ht->ld[0].valid= i-1;
      ht->count--;
    }
    return;
  }
  l= 0;
  while (l <= ht->level) {
    if (--ht->ld[l].valid == 0) {
      freeDir(ht->ld[l].dir);
      l++;
    }
    else
      break;
  }
  if (l == ht->level && ht->ld[l].valid == 1) {
    l= l-1;
    ht->level= l;
    ht->ld[l].dir= (*ht->ld[l+1].dir)[0];
    ht->ld[l].valid= PTR_PER_DIR;
    freeDir(ht->ld[l+1].dir);
  }
  while (--l>=0) {
    ht->ld[l].dir= (*ht->ld[l+1].dir)[ht->ld[l+1].valid-1];
    ht->ld[l].valid= PTR_PER_DIR;
  }
  ht->count--;
  return;
}

static void freeDirTable(dirTable *ht) {
  while (ht->count > 1) {
    ht->ld[0].valid= 1;
    ht->count=  ((ht->count-1) & ~DIR_IDX_MASK) + 1;
    shrinkDirTable(ht);
  }
  freeDir(ht->ld[0].dir);
  return;
}

typedef struct {
    dirTable *ht;
    ht_dir *current;    /* current dir of this level */
    unsigned int index;   /* index to deliver next */
} dirEnumerate;

#define EndOfTable  (void *)-1

void *stepDirTable(dirEnumerate *he) {
  if (he->index < he->ht->count) {
    dht_index_t di= he->index & DIR_IDX_MASK;
    if (di == 0)
      he->current= (ht_dir*)accessAdr(he->ht, he->index);
    he->index++;
    return (*he->current)[di];
  }
  return EndOfTable;
}

typedef unsigned long   uLong;
typedef unsigned char   uChar;
typedef unsigned short  uShort;

typedef struct InternHsElement {
    dhtElement      HsEl;
    struct InternHsElement  *Next;
} InternHsElement;
#define NilInternHsElement  Nil(InternHsElement)
#define NewInternHsElement  New(InternHsElement)
#define FreeInternHsElement(h)  fxfFree(h, sizeof(InternHsElement))

typedef struct {
    uLong       (*Hash)(dhtValue);
    int     (*Equal)(dhtValue, dhtValue);
    dhtValue    (*DupKey)(dhtValue);
    dhtValue    (*DupData)(dhtValue);
    void        (*FreeKey)(dhtValue);
    void        (*FreeData)(dhtValue);
    void        (*DumpData)(dhtValue,FILE *);
    void        (*DumpKey)(dhtValue,FILE *);
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
#define NewHashTable        (HashTable *)fxfAlloc(sizeof(dht))
#define FreeHashTable(h)    fxfFree(h, sizeof(dht))
#if defined(OVERFLOW_SAVE)
#define ActualLoadFactor(h)                     \
  ( (h)->CurrentSize < 10000                    \
    ?  ((h)->KeyCount*100) / (h)->CurrentSize   \
    :  (h)->KeyCount / ((h->CurrentSize/100))   \
    )
#else
#define ActualLoadFactor(h) (((h)->KeyCount*100)/(h)->DirTab.count)
#endif /*OVERFLOW_SAVE*/

unsigned long dhtActualLoad(dht *h) {
  return ActualLoadFactor(h);
}
unsigned long dhtKeyCount(dht *h) {
  return h->KeyCount;
}

char dhtError[128];

char *dhtErrorMsg() {
  return dhtError;
}

dht *dhtCreate(dhtValueType KeyType, dhtValuePolicy KeyPolicy,
               dhtValueType DtaType, dhtValuePolicy DataPolicy) {
  dht       *ht;

  if (KeyType >= dhtValueTypeCnt) {
    sprintf(dhtError,
            "dhtCreate: invalid KeyType: numeric=%d\n", KeyType);
    return Nil(dht);
  }
  if (dhtProcedures[KeyType] == Nil(dhtValueProcedures)) {
    sprintf(dhtError,
            "dhtCreate: no procedure registered for KeyType \"%s\"\n",
            dhtValueTypeToString[KeyType]);
    return Nil(dht);
  }
  if (DtaType >= dhtValueTypeCnt) {
    sprintf(dhtError,
            "dhtCreate: invalid DataType: numeric=%d\n", DtaType);
    return Nil(dht);
  }
  if (dhtProcedures[DtaType] == Nil(dhtValueProcedures)) {
    sprintf(dhtError,
            "dhtCreate: no procedure registered for DtaType \"%s\"\n",
            dhtValueTypeToString[DtaType]);
    return Nil(dht);
  }

  if ((ht=NewHashTable) == Nil(dht)) {
    strcpy(dhtError, "dhtCreate: no memory.");
    return Nil(dht);
  }
  ht->DirTab.level= 0;
  ht->DirTab.count= PTR_PER_DIR;
  ht->DirTab.ld[0].valid= PTR_PER_DIR;
  ht->DirTab.ld[0].dir= New(ht_dir);
  if (ht->DirTab.ld[0].dir == Nil(ht_dir)) {
    strcpy(dhtError,
           "dhtCreate: No memory for Directory segment.");
    FreeHashTable(ht);
    return Nil(dht);
  }
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
  ht->procs.DumpKey=  dhtProcedures[KeyType]->Dump;

  if (KeyPolicy==dhtNoCopy) {
    ht->procs.DupKey= dhtProcedures[dhtSimpleValue]->Dup;
    ht->procs.FreeKey= dhtProcedures[dhtSimpleValue]->Free;
  }
  else if (KeyPolicy==dhtCopy) {
    ht->procs.DupKey= dhtProcedures[KeyType]->Dup;
    ht->procs.FreeKey= dhtProcedures[KeyType]->Free;
  }
  else {
    sprintf(dhtError,
            "Sorry, unknown KeyPolicy: numeric=%d.", KeyPolicy);
    freeDirTable(&ht->DirTab);
    FreeHashTable(ht);
    return Nil(dht);
  }

  if (DataPolicy==dhtNoCopy) {
    ht->procs.DupData= dhtProcedures[dhtSimpleValue]->Dup;
    ht->procs.FreeData= dhtProcedures[dhtSimpleValue]->Free;
  }
  else if (DataPolicy==dhtCopy) {
    ht->procs.DupData= dhtProcedures[DtaType]->Dup;
    ht->procs.FreeData= dhtProcedures[DtaType]->Free;
  }
  else {
    sprintf(dhtError,
            "Sorry, unknown DataPolicy: numeric=%d.", DataPolicy);
    freeDirTable(&ht->DirTab);
    FreeHashTable(ht);
    return Nil(dht);
  }

  return ht;
}

void dhtDestroy(HashTable *ht) {
  dirEnumerate dEnum;
  void *e;

  dEnum.index= 0;
  dEnum.current = 0;
  dEnum.ht= &ht->DirTab;

  while ((e=stepDirTable(&dEnum)) != EndOfTable) {
    InternHsElement *b= (InternHsElement *)e;
    while (b) {
      InternHsElement *tmp= b;
      (ht->procs.FreeKey)(b->HsEl.Key);
      (ht->procs.FreeData)(b->HsEl.Data);
      b= b->Next;
      FreeInternHsElement(tmp);
    }
  }
  freeDirTable(&ht->DirTab);
  FreeHashTable(ht);
}

void dhtDumpIndented(int ind, HashTable *ht, FILE *f) {
  dirEnumerate dEnum;
  int dcnt, hcnt;
  void *e;

  fprintf(f, "%*sSimple Values: \n", ind, "");
  fprintf(f, "%*sp (Next bucket to split) = %6lu\n",
          ind, "", ht->p);
  fprintf(f, "%*smaxp (Upper bound on p)  = %6lu\n",
          ind, "", ht->maxp);
  fprintf(f, "%*sKeyCount                 = %6lu\n",
          ind, "", ht->KeyCount);
  fprintf(f, "%*sCurrentSize              = %6lu\n",
          ind, "", ht->CurrentSize);
  fprintf(f, "%*sMinLoadFactor (%%)        = %6u\n",
          ind, "", ht->MinLoadFactor);
  fprintf(f, "%*sMaxLoadFactor (%%)        = %6u\n",
          ind, "", ht->MaxLoadFactor);
  fprintf(f, "%*sActual LoadFactor (%%)    = %6ld\n",
          ind, "", ActualLoadFactor(ht));
  fprintf(f, "%*sDirLevel                 = %6u\n",
          ind, "", ht->DirTab.level);
  dcnt= hcnt=0;

  dEnum.index= 0;
  dEnum.current = 0;
  dEnum.ht= &ht->DirTab;

  while ((e=stepDirTable(&dEnum)) != EndOfTable) {
    InternHsElement *b= (InternHsElement *)e;
    while (b) {
      fprintf(f, "%*s    ", ind, "");
      (ht->procs.DumpKey)(b->HsEl.Key, f);
      fputs("->", f);
      (ht->procs.DumpData)(b->HsEl.Data, f);
      b= b->Next;
      fputc('\n', f);
      hcnt++;
    }
  }
  fprintf(f, "%*s%d records of %ld dumped\n\n",
          ind, "", hcnt, ht->KeyCount);
}

void dhtDump(HashTable *ht, FILE *f) {
  dhtDumpIndented(0, ht, f);
}

dhtElement *dhtGetFirstElement(HashTable *ht) {
  void *e;

  if (ht->KeyCount == 0)
    return dhtNilElement;

  ht->DirEnum.index= 0;
  ht->DirEnum.ht= &ht->DirTab;

  while ((e=stepDirTable(&ht->DirEnum)) != EndOfTable) {
    if (e != 0) {
      InternHsElement *b= (InternHsElement *)e;
      ht->NextStep= b->Next;
      return &b->HsEl;
    }
  }
  return dhtNilElement;
}

dhtElement *dhtGetNextElement(HashTable *ht) {
  void *e;
  if (ht->NextStep) {
    dhtElement *de= &ht->NextStep->HsEl;
    ht->NextStep= ht->NextStep->Next;
    return de;
  }
  while ((e=stepDirTable(&ht->DirEnum)) != EndOfTable) {
    if (e != 0) {
      InternHsElement *b= (InternHsElement *)e;
      ht->NextStep= b->Next;
      return &b->HsEl;
    }
  }
  return dhtNilElement;
}

LOCAL uLong DynamicHash(uLong p, uLong maxp, uLong v) {
  uLong h; 

  h= v % maxp;
  if (h < p)
    return v % (maxp<<1);
  else
    return h;
}

LOCAL dhtStatus ExpandHashTable(HashTable *ht) {
  static char *myname= "ExpandHashTable";
  /* Need to expand the directory */
  uLong oldp= ht->p;
  uLong newp= ht->maxp + ht->p;

  if (appendDirTable(&ht->DirTab, 0) < 0) {
    sprintf(dhtError, "%s: no memory\n", myname);
    return dhtFailedStatus;
  }
  ht->CurrentSize++;

  /* update bucket pointers */
  ht->p++;
  if (ht->p == ht->maxp) {
    ht->maxp<<= 1;
    ht->p= 0;
  }

  /* relocate records */
  {
    InternHsElement **new, **old;

    new= (InternHsElement **)accessAdr(&ht->DirTab, newp);
    old= (InternHsElement **)accessAdr(&ht->DirTab, oldp);
    
    while (*old) {
      if (DynamicHash(ht->p, ht->maxp,
                      (ht->procs.Hash)((*old)->HsEl.Key)) == newp) {
        *new= *old;
        *old= (*old)->Next;
        new= &(*new)->Next;
        *new= NilInternHsElement;
      }
      else
        old= &(*old)->Next;
    }
  }
  return dhtOkStatus;
}

LOCAL void ShrinkHashTable(HashTable *ht) {
  InternHsElement   **old, **new;
  uLong     oldp;

  if (ht->maxp == PTR_PER_DIR && ht->p == 0)
    return;
  if (ht->p == 0) {
    ht->maxp>>= 1;
    ht->p= ht->maxp;
  }
  ht->p--;

  new= (InternHsElement**)accessAdr(&ht->DirTab, ht->p);
  oldp= ht->p + ht->maxp;
  old= (InternHsElement**)accessAdr(&ht->DirTab, oldp);

  if (*old) {
    while (*new)
      new= &(*new)->Next;
    *new= *old;
    *old= NilInternHsElement;
  }
  ht->CurrentSize--;
  shrinkDirTable(&ht->DirTab);
  return;
}

LOCAL InternHsElement **LookupInternHsElement(HashTable *ht, dhtValue key) {
  uLong     h;
  InternHsElement **phe;
    
  h= DynamicHash(ht->p, ht->maxp, (ht->procs.Hash)(key));
  phe= (InternHsElement**)accessAdr(&ht->DirTab, h);

  while (*phe) {
    if ((ht->procs.Equal)((*phe)->HsEl.Key, key))
      break;
    else
      phe= &((*phe)->Next);
  }
  return phe;
}

void dhtRemoveElement(HashTable *ht, dhtValue key) {
  MYNAME(dhtRemoveElement)
    InternHsElement **phe, *he;

  phe= LookupInternHsElement(ht, key);
  if (*phe) {
    DEBUG_CODE(
      fprintf(stderr, "%s: dumping before removing\n", myname);
      dhtDump(ht, stderr);
      );
    he= *phe;
    if (ht->NextStep == he)
      ht->NextStep= ht->NextStep->Next;
        
    *phe= he->Next;
    (ht->procs.FreeData)(he->HsEl.Data);
    (ht->procs.FreeKey)(he->HsEl.Key);
    FreeInternHsElement(he);
    ht->KeyCount--;
    if (ActualLoadFactor(ht) < ht->MinLoadFactor) {
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
  return;
}

dhtStatus   dhtDupStatus;

dhtElement *dhtEnterElement(HashTable *ht, dhtValue key, dhtValue data) {
  InternHsElement **phe, *he;
  dhtValue KeyV, DataV;
  dhtStatus stat= dhtOkStatus;

  KeyV= (ht->procs.DupKey)(key);
  if (dhtDupStatus != dhtOkStatus)
    return dhtNilElement;
  DataV= (ht->procs.DupData)(data);
  if (dhtDupStatus != dhtOkStatus) {
    (ht->procs.FreeKey)(KeyV);
    return dhtNilElement;
  }
  phe= LookupInternHsElement(ht, key);
  he= *phe;
  if (!he) {
    if (!(he=NewInternHsElement))
      return dhtNilElement;
    *phe= he;
    he->Next= NilInternHsElement;
    ht->KeyCount++;
  }
  else {
    if (ht->DtaPolicy == dhtCopy)
      (ht->procs.FreeData)(he->HsEl.Data);
    if (ht->KeyPolicy == dhtCopy)
      (ht->procs.FreeKey)(he->HsEl.Key);
  }
  he->HsEl.Key= KeyV;
  he->HsEl.Data= DataV;
  if (ActualLoadFactor(ht) > ht->MaxLoadFactor) {
    /*
      fprintf(stderr, "Dumping Hash-Table before expansion\n");
      fDumpHashTable(ht, stderr);
    */
    if ((stat=ExpandHashTable(ht)) != dhtOkStatus)
      return dhtNilElement;
    /*
      fprintf(stderr, "Dumping Hash-Table after expansion\n");
      fDumpHashTable(ht, stderr);
    */
  }
  return &he->HsEl;
}

dhtElement *dhtLookupElement(HashTable *ht, dhtValue key) {
  InternHsElement **phe= LookupInternHsElement(ht, key);
  if (*phe)
    return &(*phe)->HsEl;
  else
    return dhtNilElement;
}

int dhtBucketStat(HashTable *ht, int *counter, int n) {
  int       BucketCount;
  dhtElement    *he;

  memset(counter, 0, n*sizeof(counter[0]));
  BucketCount= 0;
  he= dhtGetFirstElement(ht);
  while (he != Nil(dhtElement)) {
    int len= 1;
    InternHsElement *ihe= ((InternHsElement *)he)->Next;
    while (ihe) {
      len++;
      ht->NextStep= ihe;
      ihe= ihe->Next;
    }
    
    BucketCount++;
    if (len < n)
      counter[len-1]++;
    else
      counter[n-1]++;
    
    he= dhtGetNextElement(ht);
  }
  return BucketCount;
}
