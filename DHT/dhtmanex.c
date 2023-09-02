/* This is dhtmanex.c --  Version 1.5
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

#if !defined(__FreeBSD__)
#include <search.h>
#endif /*__FreeBSD__*/

#include "dhtvalue.h"
#include "dht.h"

struct info {		/* this is the info stored in table */
	int age, room;	/* other than the key */
};

int main( )
{

    char	string_to_enter[128];
    char	name_to_find[128];
    struct info *info_to_enter;
    int 	Age, Room;
    struct dht	*OurTable;
    dhtElement	*he;
    dhtKey k;
    dhtValue v;

#if defined(FXF)
    /* we want to use only 100K for all hashing */
    fxfInit(100*1024);
#endif /*FXF*/


    /* create table */
    OurTable= dhtCreate(dhtStringValue, dhtCopy,
				dhtSimpleValue, dhtNoCopy);

    while (scanf("%127s%d%d", string_to_enter, &Age, &Room) != EOF) {
    	/* put info in structure */
    	info_to_enter= (struct info*)malloc(sizeof(struct info));
        if (!info_to_enter) {
            (void)fprintf(stderr,
                         "no memory to store key = %s, "
                         "age = %d, "
                         "room = %d; giving up\n",
                         string_to_enter,
                         Age,
                         Room);
            break;
        }
    	info_to_enter->age= Age;
    	info_to_enter->room= Room;
    	/* the string will be duplicated the info not */
        k.key_data.object_pointer = string_to_enter;
        v.value_data.object_pointer = info_to_enter;
    	dhtEnterElement(OurTable, k, v);
    }

    /* access table */
    while (scanf("%127s", name_to_find) != EOF) {
    k.key_data.object_pointer = name_to_find;
	he= dhtLookupElement(OurTable, k);
	if (he != dhtNilElement) {
	    /* if item is in the table */
	    (void)printf("found %s, age = %d, room = %d\n",
			(char *)he->Key.key_data.object_pointer,
			((struct info *)he->Data.value_data.object_pointer)->age,
			((struct info *)he->Data.value_data.object_pointer)->room);
	} else {
	    (void)printf("no such employee %s\n", name_to_find);
	}
    }
    /* now delete all struct info in the table */
    he= dhtGetFirstElement(OurTable);
    while (he) {
    	free(he->Data.value_data.object_pointer);
	he= dhtGetNextElement(OurTable);
    }
    /* now destroy the whole table */
    dhtDestroy(OurTable);

#if defined(FXF)
    fxfTeardown();
#endif /*FXF*/

    return 0;
}
