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
#include "dht.h"

static unsigned long ConvertString(dhtConstValue v)
{
    /* I found this hash function on 
     *   http://ourworld.compuserve.com/homepages/bob_jenkins/doobs.htm
     * There are other functions, but this one is has some advantages:
     *    - its small
     *	  - independant from word sizes
     *	  - needs no initialisation
     */
    unsigned char const *s= (unsigned char const *)v;
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

static int EqualString(dhtConstValue v1, dhtConstValue v2)
{
	if (strcmp((char const *)v1, (char const *)v2))
		return 0;
	else
		return 1;
}

static dhtConstValue	DupString(dhtConstValue v)
{
	char *nv;
	nv= (char *)fxfAlloc(strlen((char const *)v)+1);
	if (nv!=0)
		strcpy(nv, (char const *)v);
	return (dhtConstValue)nv;
}
static void	FreeString(dhtValue v)
{
	fxfFree(v, strlen((char const *)v)+1);
	return;
}
static void	DumpString(dhtConstValue v, FILE *f)
{
	fputs((char const *)v,f);
	return;
}

dhtValueProcedures dhtStringProcs = {
	ConvertString,
	EqualString,
	DupString,
	FreeString,
	DumpString
};
