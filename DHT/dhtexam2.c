/* This is dhtexample2.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is keept intact
 * and in place.
 */

/* this code is very simple and only test the functionality with
 * a huge amount of entries
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dhtvalue.h"
#include "dht.h"
#include "dhtmem.h"

int main(int argc, char *argv[]) {

	int cnt, EntryCnt= 0;

	struct dummy {
		long l1;
		long l2;
		short s1;
		short s2;
	} dm;

	MemVal mv;
	struct dht *ht;
	int ac,dmpflg=0;
	char **av;


#if defined(FXF)
	/* we want to use only 100K for all hashing */
	fxfInit(100*1024);
#endif /*FXF*/

	ac= 1;
	av= ++argv;
	while (ac < argc) {
		if (strcmp(*av, "-c") == 0) {
			ac++; av++;
			EntryCnt= atoi(*av);
			ac++; av++;
			continue;
		}
		if (strcmp(*av, "-d") == 0) {
			dmpflg=1;
			ac++; av++;
			continue;
		}
		fprintf(stderr, "Usage: %s [ -d ] [ -c EntryCount ]  \n", argv[0]); 
		exit(1);
	}
	if (EntryCnt < 1000) {
		EntryCnt=1000;
		printf("Setting EntryCnt to %d\n", EntryCnt);
	}

	mv.Leng= sizeof(struct dummy);
	mv.Data= (unsigned char *)&dm;

#if !defined(PRE_REGISTER)
	dhtRegisterValue(dhtMemoryValue, 0, &dhtMemoryProcs);
	dhtRegisterValue(dhtSimpleValue, 0, &dhtSimpleProcs);
#endif /*PRE_REGISTER*/

	if ((ht=dhtCreate(dhtMemoryValue, dhtCopy,
			dhtSimpleValue, dhtNoCopy)) == dhtNilHashTable) {
	    fputs("Sorry, no space for HashTables\n",stderr);
	    exit(3);
	}

	for (cnt=0; cnt<EntryCnt; cnt++) {
		dm.l1= cnt;
		dm.l2= -cnt;
		if (dhtEnterElement(ht, (dhtValue)&mv, (dhtValue)&dm) == dhtNilElement) {
			fprintf(stderr, "%s: Sorry, failed to enter %d-th element\n", argv[0], cnt);
			exit(5);
		}
	}
	if (dmpflg) {
		printf("Dumping %d Entries:\n", cnt);
		dhtDump(ht, stderr);
	}
}
