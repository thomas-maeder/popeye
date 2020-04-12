/* This is dhtbcmem.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is keept intact
 * and in place.
 */

#include "debugging/assert.h"
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

static dhtHashValue ConvertBCMemValue(dhtConstValue m)
{
  BCMemValue const * const toBeConverted = (BCMemValue const *)m;
  unsigned int leng = toBeConverted->Leng;
  unsigned char const *s = toBeConverted->Data;
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

static int EqualBCMemValue(dhtConstValue v1, dhtConstValue v2)
{
  BCMemValue const * const value1 = (BCMemValue const *)v1;
  BCMemValue const * const value2 = (BCMemValue const *)v2;
  size_t const size = sizeof *value1 - sizeof value1->Data + value1->Leng;

  return memcmp(value1,value2,size)==0;
}

static dhtConstValue DupBCMemValue(dhtConstValue v)
{
  BCMemValue const * const original = (BCMemValue const *)v;
  size_t const size = (sizeof *original
                       - sizeof original->Data
                       + original->Leng);

  BCMemValue * const result = fxfAlloc(size);
  if (result!=0)
    memcpy(result,original,size);

  return (dhtConstValue)result;
}

static void FreeBCMemVal(dhtValue v)
{
  BCMemValue * const freed = (BCMemValue *)v;
  size_t const size = sizeof *freed - sizeof freed->Data + freed->Leng;
  fxfFree(freed,size);
}

static void DumpBCMemValue(dhtConstValue v, FILE *f)
{
  BCMemValue const * const toBeDumped = (BCMemValue const *)v;
  unsigned int const length = toBeDumped->Leng;
  unsigned int i;

  fprintf(f, "(%d)", toBeDumped->Leng);
  for (i=0; i<length; i++)
    fprintf(f, "%02x", toBeDumped->Data[i] & 0xff);
}

dhtValueProcedures dhtBCMemoryProcs =
{
  ConvertBCMemValue,
  EqualBCMemValue,
  DupBCMemValue,
  FreeBCMemVal,
  DumpBCMemValue
};
