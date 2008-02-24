/* This is dhtstring.c --  Version 1.5
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

static unsigned long ConvertString(dhtValue v) {
    /* I found this hash function on 
     *   http://ourworld.compuserve.com/homepages/bob_jenkins/doobs.htm
     * There are other functions, but this one is has some advantages:
     *    - its small
     *	  - independant from word sizes
     *	  - needs no initialisation
     */
    unsigned char *s= (unsigned char *)v;
    unsigned long hash= 0;
    while (*s) {
	hash+= *s++;
	hash+= hash << 10;
	hash^= hash >> 6;
    }
    hash+= hash << 3;
    hash^= hash >> 11;
    hash+= hash << 15;
    return hash;
}

static int EqualString(dhtValue v1, dhtValue v2) {
	if (strcmp((char *)v1, (char *)v2))
		return 0;
	else
		return 1;
}

extern	dhtStatus	dhtDupStatus;

static dhtValue	DupString(dhtValue v) {
	char *nv;
	nv= (char *)fxfAlloc(strlen((char *)v)+1);
	if (nv == (char *)0)
	    dhtDupStatus= dhtFailedStatus;
	else {
	    strcpy(nv, (char *)v);
	    dhtDupStatus= dhtOkStatus;
	}
	return (dhtValue)nv;
}
static void	FreeString(dhtValue v) {
	fxfFree(v, strlen((char *)v)+1);
	return;
}
static void	DumpString(dhtValue v, FILE *f) {
	fprintf(f, "%s", (char *)v);
	return;
}
dhtValueProcedures dhtStringProcs = {
	ConvertString,
	EqualString,
	DupString,
	FreeString,
	DumpString
};
