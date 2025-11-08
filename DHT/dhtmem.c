/* This is dhtmem.c --  Version 1.5
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

#if defined(__BORLANDC__)
#  include <mem.h>
#endif /*__BORLANDC__*/

#include "debugging/assert.h"
#include "dhtvalue.h"
#include "dhtmem.h"
#include "dht.h"

typedef unsigned long uLong;
typedef unsigned char uChar;

STATIC_ASSERT(1 == sizeof NilMemVal->Data[0], "sizeof BCMemValue element must be 1.");

static dhtHashValue HashMemoryValue(dhtKey k)
{
  MemVal const * toBeHashed = (MemVal const *)k.value.object_pointer;
  assert(!!toBeHashed);
  assert(toBeHashed->Data || !toBeHashed->Leng);
  uLong leng= toBeHashed->Leng;
  uChar const *s= toBeHashed->Data;
  dhtHashValue hash= 0;
  uLong i;
  for (i=0; i<leng; ++i) {
    hash+= s[i];
    hash+= hash << 10;
    hash^= hash >> 6;
  }
  hash+= hash << 3;
  hash^= hash >> 11;
  hash+= hash << 15;
  return hash;
}
static int EqualMemoryValue(dhtKey v1, dhtKey v2)
{
  MemVal const * value1 = (MemVal const *)v1.value.object_pointer;
  MemVal const * value2 = (MemVal const *)v2.value.object_pointer;
  uLong length;
  uChar const *data1;
  uChar const *data2;

  assert(value1 && value2);
  length = value1->Leng;
  data1 = value1->Data;
  assert(data1 || !length);
  data2 = value2->Data;
  assert(data2 || !value2->Leng);

  if (length != value2->Leng)
    return 0;
  if (!length) // We'll check this here to avoid (maybe) passing NULL as an argument to memcmp.
    return 1;
  while (length > ((size_t)-1))
  {
    if (memcmp(data1, data2, ((size_t)-1)))
      return 0;
    data1+= ((size_t)-1);
    data2+= ((size_t)-1);
    length-= ((size_t)-1);
  }
  return !memcmp(data1, data2, length);
}
static int DupMemoryValue(dhtValue kv, dhtValue *output)
{
  MemVal const *v= (MemVal const *)kv.object_pointer;
  uChar const *data;
  uLong length;
  MemVal *mv;
  assert(!!output);
  if (!v) {
    output->object_pointer = NilMemVal;
    return 0;
  }
  length= v->Leng;
  data= v->Data;
  assert(data || !length);
  if (length > ((size_t)-1))
    return 1;
  mv= NewMemVal;
  if (mv) {
    mv->Leng= length;
    if (length) {
      uChar *newBuffer= DHTVALUE_ALLOC(length, uChar);
      if (newBuffer) {
        memcpy(newBuffer, data, length);
        mv->Data = newBuffer;
        output->object_pointer = mv;
        return 0;
      } else {
        FreeMemVal(mv);
        return 1;
      }
    } else {
      mv->Data= NULL; // NULL is a valid pointer if Leng == 0
      output->object_pointer= mv;
      return 0;
    }
  }
  return 1;
}
static void FreeMemoryValue(dhtValue kv)
{
  MemVal *v= (MemVal *)kv.object_pointer;
  if (v) {
    DHTVALUE_FREE(v->Data, v->Leng);
    DHTVALUE_FREE(v, sizeof *v);
  }
}
static void DumpMemoryValue(dhtValue kv, FILE *f) {
  MemVal const * v= (MemVal *)kv.object_pointer;
  uLong i;
  assert(v && f);
  assert(v->Data || !v->Leng);
  fprintf(f, "(%lu)", v->Leng);
  for (i=0; i<v->Leng; i++)
    fprintf(f, "%02x", (v->Data[i] & 0xffU));
}

dhtValueProcedures dhtMemoryProcs = {
  HashMemoryValue,
  EqualMemoryValue,
  DupMemoryValue,
  FreeMemoryValue,
  DumpMemoryValue
};
