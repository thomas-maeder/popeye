/* This is dhtcmem.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is keept intact
 * and in place.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dhtvalue.h"
#include "dhtcmem.h"
#include "dht.h"

#if defined(__BORLANDC__)
#  include <mem.h>
#endif /*__BORLANDC__*/

typedef unsigned long uLong;
typedef unsigned char uChar;

static dhtHashValue  ConvertCompactMemoryValue(dhtConstValue m)
{
  uLong leng= ((CompactMemVal const *)m)->Leng;
  uChar const *s= ((CompactMemVal const *)m)->Data;
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

static int EqualCompactMemoryValue(dhtConstValue v1, dhtConstValue v2)
{
  if (((CompactMemVal const *)v1)->Leng != ((CompactMemVal const *)v2)->Leng)
    return 0;
  if (memcmp(((CompactMemVal const *)v1)->Data,
             ((CompactMemVal const *)v2)->Data, ((CompactMemVal const *)v1)->Leng))
    return 0;
  else
    return 1;
}

static dhtConstValue DupCompactMemoryValue(dhtConstValue v)
{
  CompactMemVal *cm= NewCompactMemVal(((CompactMemVal const *)v)->Leng);
  if (cm) {
    cm->Leng= ((CompactMemVal const *)v)->Leng;
    memcpy(cm->Data, ((CompactMemVal const *)v)->Data, cm->Leng);
    return (dhtValue)cm;
  }
  return (dhtValue)cm;
}

static void FreeCompactMemoryValue(dhtValue v)
{
  FreeCompactMemVal(v);
}

static void DumpCompactMemoryValue(dhtConstValue v, FILE *f)
{
  uLong i;
  fprintf(f, "(%lu)", ((CompactMemVal const *)v)->Leng);
  for (i=0; i<((CompactMemVal const *)v)->Leng; i++)
    fprintf(f, "%02x", ((CompactMemVal const *)v)->Data[i] & 0xff);
}

dhtValueProcedures dhtCompactMemoryProcs = {
  ConvertCompactMemoryValue,
  EqualCompactMemoryValue,
  DupCompactMemoryValue,
  FreeCompactMemoryValue,
  DumpCompactMemoryValue
};
