/* This is dhtbcmem.c --  Version 1.5
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
#if defined(FBSD)
#include <stdlib.h>
#else
#include <malloc.h>
#endif /*FBSD*/
#if defined(__BORLANDC__)
#include <mem.h>
#endif /*__BORLANDC__*/

#include "dhtvalue.h"
#include "dhtbcmem.h"

typedef unsigned long uLong;

static unsigned long  ConvertBCMemValue(dhtValue m) {
    uLong leng= ((BCMemValue *)m)->Leng; 
    uChar *s= ((BCMemValue *)m)->Data;
    unsigned long hash= 0;
    int i;
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

static int EqualBCMemValue(dhtValue v1, dhtValue v2) {
	if (((BCMemValue *)v1)->Leng != ((BCMemValue *)v2)->Leng)
		return 0;
	if (memcmp(((BCMemValue *)v1)->Data,
		((BCMemValue *)v2)->Data, ((BCMemValue *)v1)->Leng))
		return 0;
	else
		return 1;
}
extern	dhtStatus	dhtDupStatus;

static dhtValue	DupBCMemValue(dhtValue v) {
	BCMemValue *cm= NewBCMemValue(((BCMemValue *)v)->Leng);
	if (cm) {
		cm->Leng= ((BCMemValue *)v)->Leng;
		memcpy(cm->Data, ((BCMemValue *)v)->Data, cm->Leng);
		dhtDupStatus= dhtOkStatus;
		return (dhtValue)cm;
	}
	dhtDupStatus= dhtFailedStatus;
	return (dhtValue)cm;
}
static void	FreeBCMemVal(dhtValue v) {
	FreeBCMemValue(v);
	return;
}
static void	DumpBCMemValue(dhtValue v, FILE *f) {
	int i;
	fprintf(f, "(%d)", ((BCMemValue *)v)->Leng);
	for (i=0; i<(int)((BCMemValue*)v)->Leng; i++)
		fprintf(f, "%02x", ((BCMemValue*)v)->Data[i] & 0xff);
	return;
}

BCMemValue *BCMemValueCreate(int n) {
    BCMemValue *bcm= NewBCMemValue(n);  
    bcm->Leng= n;
    return bcm;
}

dhtValueProcedures dhtBCMemoryProcs = {
	ConvertBCMemValue,
	EqualBCMemValue,
	DupBCMemValue,
	FreeBCMemVal,
	DumpBCMemValue
};
