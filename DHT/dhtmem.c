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

static dhtHashValue HashMemoryValue(dhtKey k)
{
  MemVal const * toBeHashed = (MemVal const *)k.value.object_pointer;
  assert(!!toBeHashed);
  assert(toBeHashed->Data || !toBeHashed->Leng);
  uLong leng= toBeHashed->Leng;
  uChar const *s= toBeHashed->Data;
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
static int EqualMemoryValue(dhtKey v1, dhtKey v2)
{
  MemVal const * value1 = (MemVal const *)v1.value.object_pointer;
  MemVal const * value2 = (MemVal const *)v2.value.object_pointer;

  assert(value1 && value2);
  assert(value1->Data || !value1->Leng);
  assert(value2->Data || !value2->Leng);
  if (value1->Leng != value2->Leng)
    return 0;
  if (!value1->Leng) // avoid the potential undefined behavior of using memcmp on NULL pointers
    return 1;
  assert(value1->Leng <= (((size_t)-1)/sizeof value1->Data[0]));
  return !memcmp(value1->Data, value2->Data, value1->Leng*sizeof value1->Data[0]);
}
static int DupMemoryValue(dhtValue kv, dhtValue *output)
{
  MemVal const *v= (MemVal const *)kv.object_pointer;
  MemVal *mv;
  assert(!!output);
  if (!v) {
    output->object_pointer = NilMemVal;
    return 0;
  }
  mv= NewMemVal;
  if (mv) {
    mv->Leng= v->Leng;
    if (mv->Leng) {
      mv->Data= (uChar *)fxfAlloc(mv->Leng);
      if (mv->Data) {
        memcpy(mv->Data, v->Data, mv->Leng*sizeof mv->Data[0]);
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
static void	FreeMemoryValue(dhtValue kv)
{
  MemVal *v= (MemVal *)kv.object_pointer;
  if (v) {
    fxfFree(v->Data, v->Leng*sizeof v->Data[0]);
    fxfFree(v, sizeof *v);
  }
}
static void	DumpMemoryValue(dhtValue kv, FILE *f) {
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
