/* This is dhtstring.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept intact
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
	 * There are other functions, but this one has some advantages:
	 *    - it's small
	 *    - independent from word sizes
	 *    - needs no initialisation
	 */
	unsigned char const *s= (unsigned char const *)k.value.object_pointer;
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
	char const *s1= (char const *)v1.value.object_pointer;
	char const *s2= (char const *)v2.value.object_pointer;
	assert(s1 && s2);
	return !strcmp(s1, s2);
}

static int	DupString(dhtValue v, dhtValue *output)
{
	char *nv;
	size_t len;
	char const *original= (char const *)v.object_pointer;
	assert(!!output);
	if (!original) {
		output->object_pointer = NULL;
		return 0;
	}
	len = strlen(original);
	if (len < ((size_t)-1)) {
		nv= (char *)fxfAlloc(len+1);
		if (nv) {
			memcpy(nv, original, len);
			nv[len] = '\0';
			output->object_pointer = nv;
			return 0;
		}
	}
	return 1;
}
static void	FreeString(dhtValue v)
{
	char *s= (char *)v.object_pointer;
	if (s)
	{
		size_t const len = strlen(s);
		assert(len < ((size_t)-1));
		fxfFree(s, len+1);
	}
}
static void	DumpString(dhtValue v, FILE *f)
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
