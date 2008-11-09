/******************** MODIFICATIONS to pyhash.h **************************
**
** Date       Who  What
** 
** 1994            Original
** 
**************************** End of List ******************************/ 

#if !defined(PYHASH_H)
#define PYHASH_H

#include "DHT/dhtbcmem.h"
#include "py.h"

/* typedefs */
typedef unsigned char   byte;
typedef enum {
	SerNoSucc,
	HelpNoSuccOdd,
	HelpNoSuccEven,
	DirSucc,
	DirNoSucc
} hashwhat;

typedef union
{
    BCMemValue cmv;
    byte buffer[256];
} HashBuffer;

extern boolean flag_hashall;

extern  void (*encode)(HashBuffer*);

/* exported functions */
void check_hash_assumptions(void);

void inithash (void);
void closehash (void);
void HashStats(int level, char *trailer);
boolean inhash(slice_index si, hashwhat what, int val, HashBuffer *);
void addtohash(slice_index si, hashwhat what, int val, HashBuffer *);

void IncHashRateLevel(void);
void DecHashRateLevel(void);

#endif
