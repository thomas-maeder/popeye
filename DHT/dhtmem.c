/* This is dhtmem.c --  Version 1.5
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

#if defined(__BORLANDC__)
#  include <mem.h>
#endif /*__BORLANDC__*/

#include "dhtvalue.h"
#include "dhtmem.h"
#include "dht.h"

typedef unsigned long uLong;
typedef unsigned char uChar;

static dhtHashValue HashMemoryValue(dhtConstValue v)
{
  uLong leng= ((MemVal const *)v)->Leng;
  uChar const *s= ((MemVal const *)v)->Data;
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
static int EqualMemoryValue(dhtConstValue v1, dhtConstValue v2)
{
  if (((MemVal const *)v1)->Leng != ((MemVal const *)v2)->Leng)
    return 0;
  if (memcmp(((MemVal const *)v1)->Data, ((MemVal const *)v2)->Data, ((MemVal const *)v1)->Leng))
    return 0;
  else
    return 1;
}

static dhtValue	DupMemoryValue(dhtConstValue v)
{
  MemVal *mv;

  mv= NewMemVal;
  if (mv) {
    mv->Data= (unsigned char *)fxfAlloc(((MemVal const *)v)->Leng);
    if (!mv->Data)
      FreeMemVal(mv);
    else {
      mv->Leng= ((MemVal*)v)->Leng;
      memcpy(mv->Data, ((MemVal*)v)->Data, mv->Leng);
      return (dhtValue)mv;
    }
  }
  return (dhtValue)mv;
}
static void	FreeMemoryValue(dhtValue v)
{
  DeleteMemVal(v);
  return;
}
static void	DumpMemoryValue(dhtConstValue v, FILE *f) {
  uLong i;
  fprintf(f, "(%lu)", ((MemVal const *)v)->Leng);
  for (i=0; i<((MemVal const *)v)->Leng; i++)
    fprintf(f, "%02x", ((MemVal const *)v)->Data[i] & 0xff);
  return;
}

dhtValueProcedures dhtMemoryProcs = {
  HashMemoryValue,
  EqualMemoryValue,
  DupMemoryValue,
  FreeMemoryValue,
  DumpMemoryValue
};
