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

#include "debugging/assert.h"
#include "dhtvalue.h"
#include "dht.h"

static dhtHashValue ConvertString(dhtKey k)
{
    /* I found this hash function on 
     *   http://ourworld.compuserve.com/homepages/bob_jenkins/doobs.htm
     * There are other functions, but this one is has some advantages:
     *    - it's small
     *	  - independent from word sizes
     *	  - needs no initialisation
     */
    unsigned char const *s= (unsigned char const *)k.key_data.object_pointer;
    unsigned long hash;
    assert(!!s);
    hash= 0;
    while (*s) {
	hash+= *s++;
	hash+= hash << 10;
	hash^= hash >> 6;
    }
    hash+= hash << 3;
    hash^= hash >> 11;
    hash+= hash << 15;
    return (dhtHashValue)hash;
}

static int EqualString(dhtKey v1, dhtKey v2)
{
	char const *s1= (char const *)v1.key_data.object_pointer;
	char const *s2= (char const *)v2.key_data.object_pointer;
	assert(s1 && s2);
	return !strcmp(s1, s2);
}

static int	DupString(dhtKeyOrValue v, dhtKeyOrValue *output)
{
	char *nv;
	assert(!!output);
	char const *original= (char const *)v.object_pointer;
	assert(!!original);
	nv= (char *)fxfAlloc(strlen(original)+1);
	if (nv) {
		strcpy(nv, original);
		output->object_pointer = nv;
		return 0;
	}
	return 1;
}
static void	FreeString(dhtKeyOrValue v)
{
	char *s= (char *)v.object_pointer;
	if (s)
		fxfFree(s, strlen(s)+1);
}
static void	DumpString(dhtKeyOrValue v, FILE *f)
{
	char const *s= (char const *)v.object_pointer;
	assert(s && f);
	fputs(s,f);
}

dhtValueProcedures dhtStringProcs = {
	ConvertString,
	EqualString,
	DupString,
	FreeString,
	DumpString
};
