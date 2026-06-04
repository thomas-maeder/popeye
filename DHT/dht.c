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

#if !defined(New)
#  define New(type)    DHTVALUE_ALLOC(sizeof(type), type)
#  define nNew(n,type) ((type *)nNewImpl(n,sizeof(type),ALIGNMENT_OF_TYPE(type)))
#  define Nil(type)    ((type *)0)
static inline void * nNewImpl(size_t const nmemb, size_t const size, size_t desired_alignment) {
  return ((size && (nmemb > (((size_t)-1)/size))) ? Nil(void) : DHTVALUE_ALLOC_RAW(nmemb*size, desired_alignment));
}
#endif /*New*/

typedef unsigned long uLong;
typedef unsigned char uChar;
typedef unsigned short uShort;

/* ============================================================
 * Open-addressing hash table with linear probing.
 * Slots are identified by their HashCache value:
 *   - EMPTY:   HashCache == 0
 *   - DELETED: HashCache == 1
 *   - OCCUPIED: anything else
 * ============================================================ */

#define INITIAL_TABLE_SIZE 256
#define DEFAULT_MAX_LOAD_FACTOR 70  /* percent */ /* TODO: Is this a good value?  Wikipedia suggests
                                                           that 50 is a typical upper bound. */

STATIC_ASSERT(DEFAULT_MAX_LOAD_FACTOR < 100, "DEFAULT_MAX_LOAD_FACTOR must be < 100.");
STATIC_ASSERT(DEFAULT_MAX_LOAD_FACTOR > (49/INITIAL_TABLE_SIZE), "DEFAULT_MAX_LOAD_FACTOR is too small.");
STATIC_ASSERT((INITIAL_TABLE_SIZE > 0) && !(INITIAL_TABLE_SIZE & (INITIAL_TABLE_SIZE - 1U)), "INITIAL_TABLE_SIZE must be a power of 2.");

typedef struct InternHsElement {
    dhtElement   HsEl;
    dhtHashValue HashCache; /* We'll force hash values to > 1
                               and use 0 and 1 as our indicators. */
} InternHsElement;

#define SMALL_HASH_ADJUSTMENT ((((uLong) -1) >> 1) + 1) /* top bit of uLong, so bottom bits are preserved
                                                           and placement is unlikely to be affected */

#define SLOT_IS_EMPTY(s)    ((s)->HashCache == 0)
#define SLOT_IS_DELETED(s)  ((s)->HashCache == 1)
#define SLOT_IS_OCCUPIED(s) ((s)->HashCache > 1)

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
    Procedures       procs;
    InternHsElement *table;
    uLong            table_size;    /* power of 2 */
    uLong            KeyCount;
    uLong            enum_idx;      /* iteration index */
    uShort           MaxLoadFactor; /* percent */
    dhtValuePolicy   KeyPolicy;
    dhtValuePolicy   DtaPolicy;
} dht;

#if !defined(HashTable)
typedef struct dht HashTable;
#endif
#define NewHashTable        DHTVALUE_ALLOC(sizeof(dht), HashTable)
#define FreeHashTable(h)    DHTVALUE_FREE(h, sizeof(dht))

unsigned long dhtKeyCount(dht const *ht)
{
  return ht->KeyCount;
}

char dhtError[128];

char const *dhtErrorMsg(void)
{
  return dhtError;
}

/* Allocate and zero-initialize a table of given size.
 * We use malloc/free directly because the flat table exceeds FXF's
 * maximum allocation size. */
static InternHsElement *allocTable(uLong size)
{
  if (size > ((size_t) -1))
    return NULL;
  return (InternHsElement *)calloc(size, sizeof(InternHsElement));
}

static void freeTable(InternHsElement *t, uLong size)
{
  (void)size;
  free(t);
}

/* Grow the table to double its current size and rehash all elements */
static dhtStatus growTable(dht *ht)
{
  uLong old_size = ht->table_size;
  uLong new_size;
  InternHsElement *old_table;
  InternHsElement *new_table;
  uLong i;
  assert((old_size > 0) && !(old_size & (old_size - 1)));
#if defined(FXF)
  /* Enforce memory budget: the table backbone (outside FXF) is capped at
   * 1/3 of the arena size (~25% of the original -maxmem budget, since
   * arena = 75% of budget). This keeps total process memory within the
   * user's specified limit. */
  if (old_size > ((fxfArenaSize()/sizeof(InternHsElement))/6))
  {
    strcpy(dhtError, "growTable: exceeds memory budget");
    return dhtFailedStatus;
  }
#endif
  if (old_size > ((uLong) -1)/2)
  {
    strcpy(dhtError, "growTable: no memory");
    return dhtFailedStatus;
  }
  new_size = old_size * 2;
  new_table = allocTable(new_size); /* TODO: Can we do better with a strategy involving realloc? */
  if (!new_table)
  {
    strcpy(dhtError, "growTable: no memory");
    return dhtFailedStatus;
  }
  old_table = ht->table;

  ht->table = new_table;
  ht->table_size = new_size;
  --new_size; /* Now it's the mask we need. */

  for (i = 0; i < old_size; i++)
  {
    if (SLOT_IS_OCCUPIED(&old_table[i]))
    {
      uLong idx = old_table[i].HashCache & new_size;
      while (!SLOT_IS_EMPTY(&new_table[idx]))
        idx = (idx + 1) & new_size;
      new_table[idx] = old_table[i];
    }
  }

  freeTable(old_table, old_size);
  return dhtOkStatus;
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
      ht->table = allocTable(INITIAL_TABLE_SIZE);
      if (!ht->table)
      {
        strcpy(dhtError, "dhtCreate: No memory for table.");
        FreeHashTable(ht);
      }
      else
      {
        ht->table_size = INITIAL_TABLE_SIZE;
        ht->KeyCount = 0;
        ht->enum_idx = 0;
        ht->MaxLoadFactor = DEFAULT_MAX_LOAD_FACTOR;
        ht->KeyPolicy = KeyPolicy;
        ht->DtaPolicy = DataPolicy;

        ht->procs.Hash     = dhtProcedures[KeyType]->Hash;
        ht->procs.Equal    = dhtProcedures[KeyType]->Equal;
        ht->procs.DumpData = dhtProcedures[DtaType]->Dump;
        ht->procs.DumpKeyValue = dhtProcedures[KeyType]->Dump;

        if (KeyPolicy==dhtNoCopy)
        {
          ht->procs.DupKeyValue  = dhtProcedures[dhtSimpleValue]->Dup;
          ht->procs.FreeKeyValue = dhtProcedures[dhtSimpleValue]->Free;
        }
        else
        {
          ht->procs.DupKeyValue  = dhtProcedures[KeyType]->Dup;
          ht->procs.FreeKeyValue = dhtProcedures[KeyType]->Free;
        }

        if (DataPolicy==dhtNoCopy)
        {
          ht->procs.DupData  = dhtProcedures[dhtSimpleValue]->Dup;
          ht->procs.FreeData = dhtProcedures[dhtSimpleValue]->Free;
        }
        else
        {
          ht->procs.DupData  = dhtProcedures[DtaType]->Dup;
          ht->procs.FreeData = dhtProcedures[DtaType]->Free;
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
  uLong i;
  uLong size = ht->table_size;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i < size; i++)
  {
    if (SLOT_IS_OCCUPIED(&ht->table[i]))
    {
      (ht->procs.FreeData)(ht->table[i].HsEl.Data);
      (ht->procs.FreeKeyValue)(ht->table[i].HsEl.Key.value);
    }
  }

  freeTable(ht->table, size);
  FreeHashTable(ht);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void dhtDumpIndented(int ind, HashTable const *ht, FILE *f)
{
  uLong i;
  uLong hcnt = 0;
  uLong size = ht->table_size;

  fprintf(f, "%*sSimple Values: \n", ind, "");
  fprintf(f, "%*sKeyCount                 = %6lu\n",
          ind, "", ht->KeyCount);
  fprintf(f, "%*sTableSize                = %6lu\n",
          ind, "", size);

  for (i = 0; i < size; i++)
  {
    if (SLOT_IS_OCCUPIED(&ht->table[i]))
    {
      fprintf(f, "%*s    ", ind, "");
      (ht->procs.DumpKeyValue)(ht->table[i].HsEl.Key.value, f);
      fputs("->", f);
      (ht->procs.DumpData)(ht->table[i].HsEl.Data, f);
      fputc('\n', f);
      hcnt++;
    }
  }

  fprintf(f, "%*s%lu records of %lu dumped\n\n",
          ind, "", hcnt, ht->KeyCount);
}

void dhtDump(HashTable const *ht, FILE *f)
{
  dhtDumpIndented(0, ht, f);
}

dhtElement *dhtGetFirstElement(HashTable *ht)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
  TraceFunctionParamListEnd();

  ht->enum_idx = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
  return dhtGetNextElement(ht);
}

dhtElement *dhtGetNextElement(HashTable *ht)
{
  uLong size = ht->table_size;
  uLong enum_idx = ht->enum_idx;
  while (enum_idx < size)
  {
    InternHsElement *slot = &ht->table[enum_idx++];
    if (SLOT_IS_OCCUPIED(slot))
    {
      ht->enum_idx = enum_idx;
      return &slot->HsEl;
    }
  }
  ht->enum_idx = enum_idx;
  return dhtNilElement;
}

/* Find a slot for the given key. Returns true if the key is found, false otherwise.
 * If slot isn't NULL, returns the address of key if found and the address of an
 * available slot otherwise (a DELETED slot if available, otherwise an EMPTY one).
 * We expect the caller to ensure that hashVal is > 1, since smaller values
 * indicate emptiness or deletion. */
static boolean lookupSlot(dht const *ht, dhtKey key, dhtHashValue hashVal,
                          InternHsElement **slot)
{
  uLong mask = ht->table_size - 1;
  uLong idx = hashVal & mask;
  uLong const starting_idx = idx;
  InternHsElement *del = NULL;

  assert(hashVal > 1);

  do
  {
    InternHsElement *tbl_idx = &ht->table[idx];
    if (SLOT_IS_EMPTY(tbl_idx))
    {
      /* We're done searching; the element isnt there. */
      if (slot)
      {
        /* Return the proper insertion point. */
        if (del)
          *slot = del;
        else
          *slot = tbl_idx;
      }
      return false;
    }
    if (SLOT_IS_DELETED(tbl_idx))
    {
      if (!del)
        del = tbl_idx;
    }
    else if ((tbl_idx->HashCache == hashVal) && (ht->procs.Equal)(tbl_idx->HsEl.Key, key))
    {
      /* TODO: Consider lazy deletion (https://en.wikipedia.org/wiki/Lazy_deletion). */
      if (slot)
        *slot = tbl_idx;
      return true;
    }
    idx = (idx + 1) & mask;
  }
  while (idx != starting_idx);
  /* table is a graveyard; there must be at least one tombstone */
  assert(!!del);
  if (slot)
    *slot = del;
  return false;
}

void dhtRemoveElement(HashTable *ht, dhtKey key)
{
  MYNAME(dhtRemoveElement)
  dhtHashValue hashVal;
  InternHsElement *slot;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
  TraceFunctionParamListEnd();

  hashVal = (ht->procs.Hash)(key);
  if (hashVal < 2)
    hashVal += SMALL_HASH_ADJUSTMENT;

  if (lookupSlot(ht, key, hashVal, &slot))
  {
    DEBUG_CODE(
      fprintf(stderr, "%s: dumping before removing\n", myname);
      dhtDump(ht, stderr);
    );
    (ht->procs.FreeData)(slot->HsEl.Data);
    (ht->procs.FreeKeyValue)(slot->HsEl.Key.value);
    /* Mark as tombstone */
    slot->HashCache = 1;
    ht->KeyCount--;
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
  dhtHashValue hashVal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
  TraceFunctionParamListEnd();

  hashVal = (ht->procs.Hash)(key);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
  return dhtEnterElementWithHash(ht, key, data, hashVal);
}

static int tableNeedsToGrow(HashTable const *ht)
{
  uLong table_size = ht->table_size;
  uLong KeyCount = ht->KeyCount;
  uShort MaxLoadFactor = ht->MaxLoadFactor;

  /* perform the necessary comparison(s) while being
     cognizant of possible arithmetic overflow */
  uLong maxULong = ((uLong) -1);
  if (table_size > (maxULong/MaxLoadFactor))
  {
    /* (MaxLoadFactor * table_size) would overflow. */
    if (KeyCount > ((maxULong - 99)/100))
    {
      /* Annoying case:
           1. (MaxLoadFactor * table_size) overflows.
           2. (KeyCount * 100) might overflow, and regardless it's close
              enough that (1) doesn't guarantee that there's enough room.
         I'm not sure there's an easy way to perform the desired comparison here.
         TODO: Handle this situation better. */
      if ((table_size - KeyCount) < 2) /* We have to ensure that we don't completely fill the table. */
        return 1;
      return (((KeyCount + 1) / (long double)table_size) > (MaxLoadFactor / 100.0L)); /* Close enough? */
    }
    return 0;
  }
  assert((MaxLoadFactor * table_size) >= (KeyCount * 100));
  return (((MaxLoadFactor * table_size) - (KeyCount * 100)) < 100);
}

dhtElement *dhtEnterElementWithHash(HashTable *ht, dhtKey key, dhtValue data, dhtHashValue hashVal)
{
  InternHsElement *slot;
  dhtValue DataV;
  dhtKey KeyK;
  dhtValue *KeyVPtr = &KeyK.value;
  dhtValue *DataVPtr = &DataV;
  boolean found;

  TraceFunctionEntry(__func__);

  assert((ht->procs.Hash)(key) == hashVal);

  if ((ht->procs.DupKeyValue)(key.value, KeyVPtr))
  {
    TraceFunctionExit(__func__);
    TraceFunctionResult("%p",(void *)dhtNilElement);
    TraceFunctionResultEnd();
    return dhtNilElement;
  }
  if ((ht->procs.DupData)(data, DataVPtr))
  {
    (ht->procs.FreeKeyValue)(*KeyVPtr);
    TraceFunctionExit(__func__);
    TraceFunctionResult("%p",(void *)dhtNilElement);
    TraceFunctionResultEnd();
    return dhtNilElement;
  }

  if (hashVal < 2)
    hashVal += SMALL_HASH_ADJUSTMENT;

  found = lookupSlot(ht, key, hashVal, &slot);
  assert(slot && ((!found) == !SLOT_IS_OCCUPIED(slot)));

  if (found)
  {
    /* Key exists — update in place */
    
    if (ht->DtaPolicy == dhtCopy)
      (ht->procs.FreeData)(slot->HsEl.Data);
    if (ht->KeyPolicy == dhtCopy)
      (ht->procs.FreeKeyValue)(slot->HsEl.Key.value);

    slot->HsEl.Key = KeyK;
    slot->HsEl.Data = DataV;
    slot->HashCache = hashVal;

    TraceFunctionExit(__func__);
    TraceFunctionResult("%p",(void *)&slot->HsEl);
    TraceFunctionResultEnd();
    return &slot->HsEl;
  }

  /* Key not found — insert new element */

  /* First, check load factor and grow if needed */
  if (tableNeedsToGrow(ht))
  {
    if (growTable(ht) != dhtOkStatus)
    {
      /* Signal failure to the caller by returning dhtNilElement. */
      (ht->procs.FreeKeyValue)(*KeyVPtr);
      (ht->procs.FreeData)(*DataVPtr);
      TraceFunctionExit(__func__);
      TraceFunctionResult("%p",(void *)dhtNilElement);
      TraceFunctionResultEnd();
      return dhtNilElement;
    }
    /* After rehash, slot pointer is invalid — look up again */
    found = lookupSlot(ht, key, hashVal, &slot);
    assert(slot && (!found) && SLOT_IS_EMPTY(slot));
  }

  slot->HsEl.Key = KeyK;
  slot->HsEl.Data = DataV;
  slot->HashCache = hashVal;
  ht->KeyCount++;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)&slot->HsEl);
  TraceFunctionResultEnd();
  return &slot->HsEl;
}

dhtElement *dhtLookupElement(HashTable const *ht, dhtKey key)
{
  dhtHashValue hashVal;
  InternHsElement *slot;
  dhtElement *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
  TraceFunctionParamListEnd();

  hashVal = (ht->procs.Hash)(key);
  if (hashVal < 2)
    hashVal += SMALL_HASH_ADJUSTMENT;

  if (lookupSlot(ht, key, hashVal, &slot))
    result = &slot->HsEl;
  else
    result = dhtNilElement;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)result);
  TraceFunctionResultEnd();
  return result;
}

dhtElement *dhtLookupElementWithHash(HashTable const *ht, dhtKey key, dhtHashValue hashVal)
{
  InternHsElement *slot;
  dhtElement *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
  TraceFunctionParamListEnd();

  assert((ht->procs.Hash)(key) == hashVal);

  if (hashVal < 2)
    hashVal += SMALL_HASH_ADJUSTMENT;

  if (lookupSlot(ht, key, hashVal, &slot))
    result = &slot->HsEl;
  else
    result = dhtNilElement;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)result);
  TraceFunctionResultEnd();
  return result;
}

unsigned int dhtBucketStat(HashTable const *ht, unsigned int *counter, unsigned int n)
{
  unsigned int BucketCount = 0;
  uLong i;
  uLong size = ht->table_size;

  if (n > (((size_t) -1)/sizeof(counter[0])))
  {
    unsigned int *zero_position = counter;
    unsigned int num_left_to_zero = n;
    do {
        memset(zero_position, 0, ((((size_t) -1)/sizeof(zero_position[0]))*sizeof(zero_position[0])));
        zero_position += (((size_t) -1)/sizeof(zero_position[0]));
        num_left_to_zero -= (unsigned int)(((size_t) -1)/sizeof(zero_position[0]));
    }
    while (num_left_to_zero > (unsigned int)(((size_t) -1)/sizeof(zero_position[0])));
    memset(zero_position, 0, num_left_to_zero*sizeof(zero_position[0]));
  }
  else
    memset(counter, 0, n*sizeof(counter[0]));

  for (i = 0; i < size; i++)
  {
    if (SLOT_IS_OCCUPIED(&ht->table[i]))
    {
      /* In open addressing each occupied slot is its own "bucket" of length 1 */
      BucketCount++;
      if (n > 0)
        ++counter[0]; /* all chains have length 1 */
    }
  }

  return BucketCount;
}
