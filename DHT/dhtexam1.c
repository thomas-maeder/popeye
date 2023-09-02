/* This is dhtexample1.c --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept intact
 * and in place.
 */

/* this code reads a /etc/hosts (see hosts(4)) file and creates two
 * hashtables: one establishes a mapping from inet-number to name,
 * the other the reverse mapping.
 * After creation of the tables, one element is deleted from
 * the first table, searched in the second and also deleted
 * there. After this the consistency of both tables is checked.
 * This is repeated, until both tables are emtpy.
 * The code to process the /etc/hosts file depends not on
 * OS specific procedures, but parses the file itself.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dhtvalue.h"
#include "dht.h"
#include "dhtmem.h"

#define	MAXALIASES	35
#define	MAXADDRS	35
static char *host_aliases[MAXALIASES];
static char hostbuf[BUFSIZ+1];
static FILE *hostf = NULL;
static long hostaddr[MAXADDRS];


typedef unsigned long uLong;
typedef unsigned char uChar;

struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	*h_addr;	/* host address */
};


uLong inet_addr(char *cp) {
	/* converts an inet-adr in dot-notation to long */
	uLong addr;

	addr=(strtoul(cp,&cp,10) & 0xFFU);
	if (!(cp=strchr(cp, '.')))
		return 0L;
	addr= (addr<<8)+(strtoul(cp+1,&cp,10) & 0xFFU);
	if (!(cp=strchr(cp, '.')))
		return 0L;
	addr= (addr<<8)+(strtoul(cp+1,&cp,10) & 0xFFU);
	if (!(cp=strchr(cp, '.')))
		return 0L;
	addr= (addr<<8)+(strtoul(cp+1,&cp,10) & 0xFFU);
	return addr;
}

struct hostent *gethent(char *file)
{
	static struct hostent host;
	char *p;
	register char *cp, **q;

	if (hostf == NULL && (hostf = fopen(file, "r" )) == NULL)
		return (NULL);
again:
	if ((p = fgets(hostbuf, BUFSIZ, hostf)) == NULL)
		return (NULL);
	if (*p == '#')
		goto again;
	cp = strpbrk(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';
	cp = strpbrk(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	host.h_addr = (char *)hostaddr;
	*(uLong *)host.h_addr = inet_addr(p);
	host.h_length = sizeof (uLong);
	while (*cp == ' ' || *cp == '\t')
		cp++;
	host.h_name = cp;
	q = host.h_aliases = host_aliases;
	cp = strpbrk(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &host_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = strpbrk(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&host);
}


int main(int argc, char *argv[]) {
	struct dht *NameToInet,*InetToName;
	dhtElement	*hhe, *he;
	struct hostent	*host;
	int		i;
	char		*hostsfile;
	dhtKey k;
	dhtValue v;


	if (argc > 1) {
	    hostsfile= argv[1];
	}
	else
	    hostsfile= "/etc/hosts";
	
#if defined(FXF)
	/* we want to use only 300K for all hashing */
	fxfInit(300*1024);
#endif /*FXF*/

	/*
	fputs("MallocInfo before first malloc\n",stderr);
	fDumpMallinfo(stderr);
	*/
#if defined(USE_MEMVAL)
#if !defined(PRE_REGISTER)
	dhtRegisterValue(dhtMemoryValue, 0, &dhtMemoryProcs);
	dhtRegisterValue(dhtSimpleValue, 0, &dhtSimpleProcs);
#endif
	if ((NameToInet=dhtCreate(dhtMemoryValue, dhtCopy, dhtSimpleValue, dhtNoCopy)) == NilHashTable) {
		fputs("Sorry, no space for more HashTables\n",stderr);
		exit(5);
	}

	if ((InetToName=CreateHashTable(dhtSimpleValue, dhtNoCopy, dhtMemoryValue, dhtCopy)) == NilHashTable) {
		fputs("Sorry, no space for more HashTables\n",stderr);
		exit(7);
	}
#else
#if !defined(PRE_REGISTER)
	dhtRegisterValue(dhtStringValue, 0, &dhtStringProcs);
	dhtRegisterValue(dhtSimpleValue, 0, &dhtSimpleProcs);
#endif
	if ((NameToInet=dhtCreate(dhtStringValue, dhtCopy,
			dhtSimpleValue, dhtNoCopy)) == dhtNilHashTable) {
		fputs("Sorry, no space for more HashTables\n",stderr);
		exit(5);
	}

	if ((InetToName=dhtCreate(dhtSimpleValue, dhtNoCopy,
			dhtStringValue, dhtCopy)) == dhtNilHashTable) {
		fputs("Sorry, no space for more HashTables\n",stderr);
		exit(7);
	}
#endif /*USE_MEMVAL*/

	/*
	fputs("MallocInfo after creating the HashTables\n",stderr);
	fDumpMallinfo(stderr);
	*/
#if defined(FXF)
	fputs("fxf-Info after creation of hash tables\n",stderr);
	fxfInfo(stderr);
#endif /*FXF*/

#define BYT(x)	(uChar)((x)&0xff)
	while ((host=gethent(hostsfile))) {
		unsigned long InetAddr;
		char *h;
#if defined(USE_MEMVAL)
		MemVal mvHostName, *HostName= &mvHostName;
#else
		char *HostName;
#endif /*USE_MEMVAL*/

		h= host->h_addr;
#if defined(USE_MEMVAL)
		mvHostName.Leng= strlen(host->h_name);
		mvHostName.Data= (uChar *)host->h_name;
#else
		HostName= host->h_name;
#endif /*USE_MEMVAL*/
		/*
		*/
		InetAddr= *(uLong *)h; 

		k.key_data.object_pointer = HostName;
		if (dhtLookupElement(NameToInet, k)) {
			fprintf(stderr, "Hostname %s already entered\n", host->h_name);
		}
		v.value_data.unsigned_integer = InetAddr;
		dhtEnterElement(NameToInet, k, v);
		k.key_data.unsigned_integer = InetAddr; 
		if (dhtLookupElement(InetToName, k)) {
			fprintf(stderr, "InetAddr 0x%08lx already entered\n", InetAddr);
		}
		v.value_data.object_pointer = HostName;
		dhtEnterElement(InetToName, k, v);
	}
#if defined(FXF)
	fputs("fxf-Info after filling the hash tables\n",stderr);
	fxfInfo(stderr);
#endif /*FXF*/

	/* Dump both Tables */
	fputs("Dumping the hashtables ...", stdout); fflush(stdout);
	dhtDump(NameToInet,stderr);
	dhtDump(InetToName,stderr);
	puts(" done");

	/*
	fputs("MallocInfo after filling the HashTables\n",stderr);
	fDumpMallinfo(stderr);
	*/

	fputs("Testing if we get all entries from NameToInet via GetFirst and GetNext...\n",stderr); 
	he= dhtGetFirstElement(NameToInet);
	i= 0;
	while (he) {
		i++;
		he= dhtGetNextElement(NameToInet);
	}
	fprintf(stderr, "Got %d entries\n", i);

	fputs("Testing if we get alle entries from InetToName via GetFirst and GetNext...\n",stderr); 
	he= dhtGetFirstElement(InetToName);
	i= 0;
	while (he) {
		i++;
		he= dhtGetNextElement(InetToName);
	}
	fprintf(stderr, "Got %d entries\n", i);

	he= dhtGetFirstElement(NameToInet);
	while (he) {
		uLong adr= (uLong)he->Data.value_data.unsigned_integer;
		char *Name;
#if defined(USE_MEMVAL)
		strncpy(str,
		  (char *)((MemVal *)he->Key.object_pointer)->Data,
		  ((MemVal *)he->Key.object_pointer)->Leng);
		str[((MemVal *)he->Key.object_pointer)->Leng]='\0';
		Name= str;
#else
		Name= (char *)he->Key.key_data.object_pointer;
#endif /*USE_MEMVAL*/
		printf("%3u.%3u.%3u.%3u = %s   ", BYT(adr), BYT(adr>>8),
			BYT(adr>>16), BYT(adr>>24), Name);

		dhtRemoveElement(NameToInet, he->Key);
		k.key_data = he->Data.value_data;
		dhtRemoveElement(InetToName, k);
		printf("   Deleting and checking consistency (Load=%lu... ", dhtKeyCount(NameToInet));
		i=0;
		hhe= dhtGetFirstElement(NameToInet);
		fputs("    ", stdout); fflush(stdout);
		while (hhe) {
			k.key_data = hhe->Data.value_data;
			dhtElement *he1= dhtLookupElement(InetToName, k);
			if (strcmp((char *)he1->Data.value_data.object_pointer, (char *)hhe->Key.key_data.object_pointer) != 0) {
				puts("\nSorry, Mismatch");
				exit(1);
			}
			if (he1->Key.key_data.unsigned_integer != hhe->Data.value_data.unsigned_integer) {
				puts("\nSorry, Mismatch");
				exit(2);
			}
			i=i+1;
			/*
			printf("\b\b\b\b%4d",i); fflush(stdout);
			*/
			hhe= dhtGetNextElement(NameToInet);
		}
		puts(" done");
		he= dhtGetFirstElement(NameToInet);
		/*he= GetNextHashElement(NameToInet);*/
	}
#if defined(FXF)
	fputs("fxf-Info after emptying the hash tables\n",stderr);
	fxfInfo(stderr);
#endif /*FXF*/
	fputs("Dumping the hashtables after removing ...", stdout); fflush(stdout);
	dhtDump(NameToInet,stderr);
	dhtDump(InetToName,stderr);
	/*
	fputs("MallocInfo after emptying the HashTables\n",stderr);
	fDumpMallinfo(stderr);
	*/


	dhtDestroy(NameToInet);
	dhtDestroy(InetToName);

#if defined(FXF)
	fputs("fxf-Info after destroying the hash tables\n",stderr);
	fxfInfo(stderr);
#endif /*FXF*/

	/*
	fputs("MallocInfo after emptying the HashTables\n",stderr);
	fDumpMallinfo(stderr);
	*/

#if defined(FXF)
	fxfTeardown();
#endif
	return 0;
}
