#if !defined(DHT_INCLUDED)
#define DHT_INCLUDED
/* This is dht.h --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept intact
 * and in place.
 */
typedef enum {
	dhtCopy, dhtNoCopy
} dhtValuePolicy;

/* Now finally this is the HashElement */
typedef struct {
  dhtKey	  Key;
  dhtValue	Data;
} dhtElement;
#define dhtNilElement  ((dhtElement *)0)

struct dht;
#define dhtNilHashTable			((struct dht *)0)

/* procedures */
struct dht *dhtCreate(dhtValueType KeyType, dhtValuePolicy KeyPolicy,
                      dhtValueType DtaType, dhtValuePolicy DataPolicy);
dhtElement *dhtEnterElement(struct dht *, dhtKey key, dhtValue data);
unsigned int dhtBucketStat	(struct dht *, unsigned int *counter, unsigned int n);
void	      dhtDestroy	(struct dht *);
void	      dhtDump		(struct dht *, FILE *);
void	      dhtDumpIndented	(int ind, struct dht *, FILE *);
void	      dhtRemoveElement	(struct dht *, dhtKey key);
dhtElement   *dhtLookupElement	(struct dht *, dhtKey key);
dhtElement   *dhtGetFirstElement(struct dht *);
dhtElement   *dhtGetNextElement	(struct dht *);
unsigned long dhtKeyCount	(struct dht const *);
char const   *dhtErrorMsg	(void);

extern char dhtError[];

#if defined(OLD_NAMES)
#define HashTable		struct dht
#define NilHashTable		dhtNilHashTable
#define	CreateHashTable		dhtCreate
#define DestroyHashTable	dhtDestroy
#define EnterHashElement	dhtEnterElement
#define RemoveHashElement	dhtRemoveElement
#define LookupHashElement	dhtLookupElement
#define GetFirstHashElement	dhtGetFirstElement
#define GetNextHashElement	dhtGetNextElement
#define	fDumpHashTable		dhtDump
#endif /*OLD_NAMES*/

#if defined(DEBUG_DHT) || defined(TESTHASH)
int get_dhtDebug(void);
void set_dhtDebug(int d);
#endif
#endif /*DHT_INCLUDED*/
