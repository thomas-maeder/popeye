/* This is dhtcmem.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept intact
 * and in place.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "debugging/assert.h"
#include "dhtvalue.h"
#include "dhtcmem.h"
#include "dht.h"

#if defined(__BORLANDC__)
#  include <mem.h>
#endif /*__BORLANDC__*/

typedef unsigned long uLong;
typedef unsigned char uChar;

static dhtHashValue ConvertCompactMemoryValue(dhtKey m)
{
  CompactMemVal const * const toBeConverted = (CompactMemVal const *)m.value.object_pointer;
  uLong leng;
  uChar const *s;
  assert(!!toBeConverted);
  leng= toBeConverted->Leng;
  s= toBeConverted->Data;
  dhtHashValue hash= 0;
  uLong i;
  for (i=0; i<leng; i++) {
    hash+= s[i];
    hash+= hash << 10;
    hash^= hash >> 6;
  }
  hash+= hash << 3;
  hash^= hash >> 11;
  hash+= hash << 15;
  return hash;
}

static int EqualCompactMemoryValue(dhtKey v1, dhtKey v2)
{
  CompactMemVal const * const value1 = (CompactMemVal const *)v1.value.object_pointer;
  CompactMemVal const * const value2 = (CompactMemVal const *)v2.value.object_pointer;
  assert(value1 && value2);
  if (value1->Leng != value2->Leng)
    return 0;
  return !memcmp(value1->Data, value2->Data, value1->Leng*sizeof value1->Data[0]);
}

static int DupCompactMemoryValue(dhtValue kv, dhtValue *output)
{
  CompactMemVal const *v = (CompactMemVal const *)kv.object_pointer;
  size_t const num_bytes_in_Data = ((sizeof *v) - offsetof(CompactMemVal, Data));
  size_t const size_of_element = sizeof v->Data[0];
  size_t size = sizeof *v;
  CompactMemVal *result;
  uLong length;

  assert(!!v);

  length = v->Leng;
  if (length > (num_bytes_in_Data / size_of_element))
  {
    if (length > ((((size_t)-1) - size + num_bytes_in_Data) / size_of_element))
      return 1;
    size += ((length * size_of_element) - num_bytes_in_Data);
  }

  result = (CompactMemVal *)fxfAlloc(size);
  if (result)
  {
    result->Leng = length;
    memcpy(result->Data,v->Data,(length*size_of_element));
    output->object_pointer = result;
    return 0;
  }

  return 1;
}

static void FreeCompactMemoryValue(dhtValue kv)
{
  CompactMemVal *v = (CompactMemVal *)kv.object_pointer;
  size_t const num_bytes_in_Data = ((sizeof *v) - offsetof(CompactMemVal, Data));
  size_t const size_of_element = sizeof v->Data[0];
  if (v)
  {
    size_t size = sizeof *v;
    uLong length = v->Leng;
    if (length > (num_bytes_in_Data / size_of_element))
    {
      assert(length <= ((((size_t)-1) - size + num_bytes_in_Data) / size_of_element));
      size += ((length * size_of_element) - num_bytes_in_Data);
    }
    fxfFree(v,size);
  }
}

static void DumpCompactMemoryValue(dhtValue kv, FILE *f)
{
  CompactMemVal const *v = (CompactMemVal const *)kv.object_pointer;
  uLong i;
  assert(v && f);
  fprintf(f, "(%lu)", v->Leng);
  for (i=0; i<v->Leng; i++)
    fprintf(f, "%02x", (v->Data[i] & 0xffU));
}

dhtValueProcedures dhtCompactMemoryProcs = {
  ConvertCompactMemoryValue,
  EqualCompactMemoryValue,
  DupCompactMemoryValue,
  FreeCompactMemoryValue,
  DumpCompactMemoryValue
};
