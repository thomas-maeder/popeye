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
 * Slots are identified by their Key pointer:
 *   - EMPTY:   HsEl.Key.value.object_pointer == NULL
 *   - DELETED: HsEl.Key.value.object_pointer == DELETED_MARKER
 *   - OCCUPIED: anything else
 * ============================================================ */

/* Sentinel pointer value for tombstone slots */
static char deleted_sentinel;
#define DELETED_MARKER ((const volatile void *)&deleted_sentinel)

#define INITIAL_TABLE_SIZE 256
#define DEFAULT_MAX_LOAD_FACTOR 70  /* percent */

typedef struct InternHsElement {
    dhtElement   HsEl;
    dhtHashValue HashCache;
} InternHsElement;

#define SLOT_IS_EMPTY(s)   ((s)->HsEl.Key.value.object_pointer == NULL)
#define SLOT_IS_DELETED(s) ((s)->HsEl.Key.value.object_pointer == DELETED_MARKER)
#define SLOT_IS_OCCUPIED(s) (!SLOT_IS_EMPTY(s) && !SLOT_IS_DELETED(s))

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
#define HashTable struct dht
#endif
#define NewHashTable        DHTVALUE_ALLOC(sizeof(dht), HashTable)
#define FreeHashTable(h)    DHTVALUE_FREE(h, sizeof(dht))

unsigned long dhtKeyCount(dht const *h)
{
  return h->KeyCount;
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
  size_t bytes = size * sizeof(InternHsElement);
  InternHsElement *t = (InternHsElement *)malloc(bytes);
  if (t)
    memset(t, 0, bytes);
  return t;
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
  uLong new_size = old_size * 2;
  InternHsElement *old_table = ht->table;
  InternHsElement *new_table = allocTable(new_size);
  uLong i;

  if (!new_table)
  {
    strcpy(dhtError, "growTable: no memory");
    return dhtFailedStatus;
  }

  ht->table = new_table;
  ht->table_size = new_size;

  for (i = 0; i < old_size; i++)
  {
    if (SLOT_IS_OCCUPIED(&old_table[i]))
    {
      uLong idx = old_table[i].HashCache & (new_size - 1);
      while (!SLOT_IS_EMPTY(&new_table[idx]))
        idx = (idx + 1) & (new_size - 1);
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

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i < ht->table_size; i++)
  {
    if (SLOT_IS_OCCUPIED(&ht->table[i]))
    {
      (ht->procs.FreeData)(ht->table[i].HsEl.Data);
      (ht->procs.FreeKeyValue)(ht->table[i].HsEl.Key.value);
    }
  }

  freeTable(ht->table, ht->table_size);
  FreeHashTable(ht);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void dhtDumpIndented(int ind, HashTable *ht, FILE *f)
{
  uLong i;
  int hcnt = 0;

  fprintf(f, "%*sSimple Values: \n", ind, "");
  fprintf(f, "%*sKeyCount                 = %6lu\n",
          ind, "", ht->KeyCount);
  fprintf(f, "%*sTableSize                = %6lu\n",
          ind, "", ht->table_size);

  for (i = 0; i < ht->table_size; i++)
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

  fprintf(f, "%*s%d records of %lu dumped\n\n",
          ind, "", hcnt, ht->KeyCount);
}

void dhtDump(HashTable *ht, FILE *f)
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
  while (ht->enum_idx < ht->table_size)
  {
    InternHsElement *slot = &ht->table[ht->enum_idx];
    ht->enum_idx++;
    if (SLOT_IS_OCCUPIED(slot))
      return &slot->HsEl;
  }
  return dhtNilElement;
}

/* Find a slot for the given key. Returns the index of the slot containing
 * the key, or the index of the first EMPTY slot if not found.
 * If insert_mode is true, a DELETED slot encountered first will be returned
 * for reuse (but only if the key is not found further along the probe). */
static InternHsElement *lookupSlot(dht *ht, dhtKey key, dhtHashValue hashVal,
                                   InternHsElement **first_deleted)
{
  uLong mask = ht->table_size - 1;
  uLong idx = hashVal & mask;
  InternHsElement *del = NULL;

  for (;;)
  {
    InternHsElement *slot = &ht->table[idx];
    if (SLOT_IS_EMPTY(slot))
    {
      if (first_deleted)
        *first_deleted = del;
      return slot;
    }
    if (SLOT_IS_DELETED(slot))
    {
      if (!del)
        del = slot;
    }
    else if (slot->HashCache == hashVal && (ht->procs.Equal)(slot->HsEl.Key, key))
    {
      if (first_deleted)
        *first_deleted = NULL;
      return slot;
    }
    idx = (idx + 1) & mask;
  }
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
  slot = lookupSlot(ht, key, hashVal, NULL);

  if (SLOT_IS_OCCUPIED(slot))
  {
    DEBUG_CODE(
      fprintf(stderr, "%s: dumping before removing\n", myname);
      dhtDump(ht, stderr);
    );
    (ht->procs.FreeData)(slot->HsEl.Data);
    (ht->procs.FreeKeyValue)(slot->HsEl.Key.value);
    /* Mark as tombstone */
    slot->HsEl.Key.value.object_pointer = DELETED_MARKER;
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

  assert(key.value.object_pointer!=0);

  hashVal = (ht->procs.Hash)(key);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
  return dhtEnterElementWithHash(ht, key, data, hashVal);
}

dhtElement *dhtEnterElementWithHash(HashTable *ht, dhtKey key, dhtValue data, dhtHashValue hashVal)
{
  InternHsElement *slot;
  InternHsElement *first_deleted;
  dhtValue DataV;
  dhtValue *KeyVPtr, *DataVPtr;
  dhtKey KeyK;

  TraceFunctionEntry(__func__);

  assert(key.value.object_pointer!=0);

  slot = lookupSlot(ht, key, hashVal, &first_deleted);

  if (SLOT_IS_OCCUPIED(slot))
  {
    /* Key exists — update in place */
    KeyVPtr = &KeyK.value;
    DataVPtr = &DataV;

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
  /* Choose insertion slot: reuse tombstone if available, else use the empty slot */
  if (first_deleted)
    slot = first_deleted;

  KeyVPtr = &KeyK.value;
  DataVPtr = &DataV;

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

  slot->HsEl.Key = KeyK;
  slot->HsEl.Data = DataV;
  slot->HashCache = hashVal;
  ht->KeyCount++;

  /* Check load factor and grow if needed */
  if (ht->KeyCount * 100 > (uLong)ht->MaxLoadFactor * ht->table_size)
  {
    if (growTable(ht) != dhtOkStatus)
    {
      /* Element was inserted but table couldn't grow.
       * Signal failure to the caller by returning dhtNilElement. */
      TraceFunctionExit(__func__);
      TraceFunctionResult("%p",(void *)dhtNilElement);
      TraceFunctionResultEnd();
      return dhtNilElement;
    }
    /* After rehash, slot pointer is invalid — look up again */
    slot = lookupSlot(ht, key, hashVal, NULL);
    assert(SLOT_IS_OCCUPIED(slot));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)&slot->HsEl);
  TraceFunctionResultEnd();
  return &slot->HsEl;
}

dhtElement *dhtLookupElement(HashTable *ht, dhtKey key)
{
  dhtHashValue hashVal;
  InternHsElement *slot;
  dhtElement *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
  TraceFunctionParamListEnd();

  hashVal = (ht->procs.Hash)(key);
  slot = lookupSlot(ht, key, hashVal, NULL);

  if (SLOT_IS_OCCUPIED(slot))
    result = &slot->HsEl;
  else
    result = dhtNilElement;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%p",(void *)result);
  TraceFunctionResultEnd();
  return result;
}

dhtElement *dhtLookupElementWithHash(HashTable *ht, dhtKey key, dhtHashValue hashVal)
{
  InternHsElement *slot;
  dhtElement *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",(void *)ht);
  TraceFunctionParamListEnd();

  slot = lookupSlot(ht, key, hashVal, NULL);

  if (SLOT_IS_OCCUPIED(slot))
    result = &slot->HsEl;
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
  uLong i;

  memset(counter, 0, n*sizeof(counter[0]));

  for (i = 0; i < ht->table_size; i++)
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
