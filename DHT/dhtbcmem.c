/* This is dhtbcmem.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is keept intact
 * and in place.
 */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(__BORLANDC__)
#include <mem.h>
#endif /*__BORLANDC__*/

#include "dhtvalue.h"
#include "dhtbcmem.h"
#include "dht.h"

static unsigned long ConvertBCMemValue(dhtValue m)
{
  BCMemValue const * const toBeConverted = (BCMemValue *)m;
  unsigned long const leng = toBeConverted->Leng; 
  unsigned char const *s = toBeConverted->Data;
  unsigned long hash = 0;

  unsigned long i;
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

static int EqualBCMemValue(dhtValue v1, dhtValue v2)
{
  BCMemValue const * const value1 = (BCMemValue *)v1;
  BCMemValue const * const value2 = (BCMemValue *)v2;
  size_t const size = sizeof *value1 - 1 + value1->Leng;

  return memcmp(value1,value2,size)==0;
}

static dhtValue DupBCMemValue(dhtValue v)
{
  BCMemValue const * const original = (BCMemValue *)v;
  size_t const size = sizeof *original - 1 + original->Leng;

  BCMemValue * const result = fxfAlloc(size);
  if (result!=0)
    memcpy(result,original,size);

  return (dhtValue)result;
}

static void FreeBCMemVal(dhtValue v)
{
  BCMemValue * const toBeFreed = (BCMemValue *)v;
  size_t const size = sizeof *toBeFreed - 1 + toBeFreed->Leng;
  fxfFree(toBeFreed,size);
}

static void DumpBCMemValue(dhtValue v, FILE *f)
{
  BCMemValue const * const toBeDumped = (BCMemValue *)v;
  unsigned int const length = toBeDumped->Leng;
  unsigned int i;

  fprintf(f, "(%d)", toBeDumped->Leng);
  for (i=0; i<length; i++)
    fprintf(f, "%02x", toBeDumped->Data[i] & 0xff);
}

static BCMemValue *BCMemValueCreate(int n)
{
  BCMemValue * const result = fxfAlloc(sizeof *result - 1 + n);
  result->Leng= n;
  return result;
}

dhtValueProcedures dhtBCMemoryProcs =
{
  ConvertBCMemValue,
  EqualBCMemValue,
  DupBCMemValue,
  FreeBCMemVal,
  DumpBCMemValue
};
