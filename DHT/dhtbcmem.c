/* This is dhtbcmem.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept intact
 * and in place.
 */

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#if defined(__BORLANDC__)
#include <mem.h>
#endif /*__BORLANDC__*/

#include "debugging/assert.h"
#include "dhtvalue.h"
#include "dhtbcmem.h"
#include "dht.h"

static dhtHashValue ConvertBCMemValue(dhtKey m)
{
  BCMemValue const * const toBeConverted = (BCMemValue const *)m.value.object_pointer;
  unsigned int leng;
  unsigned char const *s;
  assert(!!toBeConverted);
  leng = toBeConverted->Leng;
  s = toBeConverted->Data;
  dhtHashValue hash = 0;

  unsigned int i;
  for (i=0; i<leng; i++)
  {
    hash += s[i];
    hash += hash << 10;
    hash ^= hash >> 6;
  }

  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;

  return hash;
}

static int EqualBCMemValue(dhtKey v1, dhtKey v2)
{
  BCMemValue const * const value1 = (BCMemValue const *)v1.value.object_pointer;
  BCMemValue const * const value2 = (BCMemValue const *)v2.value.object_pointer;
  unsigned int length;
  unsigned char const *data1;
  unsigned char const *data2;
  assert(value1 && value2);
  length = value1->Leng;
  data1 = value1->Data;
  assert(data1 || !length);
  data2 = value2->Data;
  assert(data2 || !value2->Leng);

  return ((length == value2->Leng) && !(length && memcmp(data1, data2, length*sizeof *data1)));
}

static int DupBCMemValue(dhtValue kv, dhtValue *output)
{
  BCMemValue const *original = (BCMemValue const *)kv.object_pointer;
  size_t const num_bytes_in_Data = ((sizeof *original) - offsetof(BCMemValue, Data));
  size_t const size_of_element = sizeof original->Data[0];
  BCMemValue *result;
  unsigned short length;
  size_t size = sizeof *original;

  assert(!!output);
  if (!original)
  {
    output->object_pointer = NULL;
    return 0;
  }

  length = original->Leng;
  if (length > (num_bytes_in_Data / size_of_element))
  {
    if (length > ((((size_t)-1) - size + num_bytes_in_Data) / size_of_element))
      return 1;
    size += ((length * size_of_element) - num_bytes_in_Data);
  }

  result = (BCMemValue *)fxfAlloc(size);
  if (result)
  {
    result->Leng = length;
    memcpy(result->Data,original->Data,(length*size_of_element));
    output->object_pointer = result;
    return 0;
  }

  return 1;
}

static void FreeBCMemValue(dhtValue kv)
{
  BCMemValue *freed = (BCMemValue *)kv.object_pointer;
  size_t const num_bytes_in_Data = ((sizeof *freed) - offsetof(BCMemValue, Data));
  size_t const size_of_element = sizeof freed->Data[0];
  if (freed)
  {
    size_t size = sizeof *freed;
    unsigned short length = freed->Leng;
    if (length > (num_bytes_in_Data / size_of_element))
    {
      assert(length <= ((((size_t)-1) - size + num_bytes_in_Data) / size_of_element));
      size += ((length * size_of_element) - num_bytes_in_Data);
    }
    fxfFree(freed,size);
  }
}

static void DumpBCMemValue(dhtValue kv, FILE *f)
{
  BCMemValue const *toBeDumped = (BCMemValue const *)kv.object_pointer;
  unsigned int length;
  unsigned int i;

  assert(toBeDumped && f);

  length = toBeDumped->Leng;
  fprintf(f, "(%u)", length);
  for (i=0; i<length; i++)
    fprintf(f, "%02x", (toBeDumped->Data[i] & 0xffU));
}

dhtValueProcedures dhtBCMemoryProcs =
{
  ConvertBCMemValue,
  EqualBCMemValue,
  DupBCMemValue,
  FreeBCMemValue,
  DumpBCMemValue
};
