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

static unsigned long  ConvertCompactMemoryValue(dhtValue m) {
  uLong leng= ((CompactMemVal *)m)->Leng; 
  uChar *s= ((CompactMemVal *)m)->Data;
  unsigned long hash= 0;
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

static int EqualCompactMemoryValue(dhtValue v1, dhtValue v2) {
  if (((CompactMemVal *)v1)->Leng != ((CompactMemVal *)v2)->Leng)
    return 0;
  if (memcmp(((CompactMemVal *)v1)->Data,
             ((CompactMemVal *)v2)->Data, ((CompactMemVal *)v1)->Leng))
    return 0;
  else
    return 1;
}

static dhtValue DupCompactMemoryValue(dhtValue v) {
  CompactMemVal *cm= NewCompactMemVal(((CompactMemVal *)v)->Leng);
  if (cm) {
    cm->Leng= ((CompactMemVal *)v)->Leng;
    memcpy(cm->Data, ((CompactMemVal *)v)->Data, cm->Leng);
    dhtDupStatus= dhtOkStatus;
    return (dhtValue)cm;
  }
  dhtDupStatus= dhtFailedStatus;
  return (dhtValue)cm;
}

static void FreeCompactMemoryValue(dhtValue v) {
  FreeCompactMemVal(v);
  return;
}

static void DumpCompactMemoryValue(dhtValue v, FILE *f) {
  uLong i;
  fprintf(f, "(%lu)", ((CompactMemVal *)v)->Leng);
  for (i=0; i<((CompactMemVal*)v)->Leng; i++)
    fprintf(f, "%02x", ((CompactMemVal*)v)->Data[i] & 0xff);
  return;
}

dhtValueProcedures dhtCompactMemoryProcs = {
  ConvertCompactMemoryValue,
  EqualCompactMemoryValue,
  DupCompactMemoryValue,
  FreeCompactMemoryValue,
  DumpCompactMemoryValue
};
