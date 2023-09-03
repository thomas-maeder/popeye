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
  assert(value1 && value2)
  
  length = value1->Leng;
  if (length != value2->Leng)
    return 0;

  return !memcmp(value1->Data, value2->Data, length*sizeof value1->Data[0]);
}

static int DupBCMemValue(dhtValue kv, dhtValue *output)
{
  BCMemValue const *original = (BCMemValue const *)kv.object_pointer;
  size_t const num_bytes_in_Data = ((sizeof *original) - offsetof(BCMemValue, Data));
  size_t const size_of_element = sizeof original->Data[0];
  size_t const num_elements_in_Data = (num_bytes_in_Data / size_of_element);
  size_t const remainder = (num_bytes_in_Data % size_of_element);
  size_t size = sizeof *original;
  BCMemValue *result;
  unsigned char length;

  assert(!!original);

  length = original->Leng;
  if (length > num_elements_in_Data)
  {
    size_t const num_new_elements = (length - num_elements_in_Data);
    if (num_new_elements > (((size_t)-1) - size + remainder) / size_of_element)
      return 1;
    size += ((num_new_elements * size_of_element) - remainder);
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
  size_t const num_elements_in_Data = (num_bytes_in_Data / size_of_element);
  size_t const remainder = (num_bytes_in_Data % size_of_element);
  if (freed)
  {
    size_t size = sizeof *freed;
    unsigned char length = freed->Leng;
    if (length > num_elements_in_Data)
    {
      size_t const num_new_elements = (length - num_elements_in_Data);
      assert(num_new_elements <= ((((size_t)-1) - size + remainder) / size_of_element));
      size += ((num_new_elements * size_of_element) - remainder);
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
