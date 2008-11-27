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
typedef enum {
	SerNoSucc,
	HelpNoSuccOdd,
	HelpNoSuccEven,
	DirSucc,
	DirNoSucc
} hashwhat;

typedef unsigned char byte;

enum
{
  hashbuf_length = 256
};

typedef union
{
    BCMemValue cmv;
    byte buffer[hashbuf_length];
} HashBuffer;

extern void (*encode)(HashBuffer *);

/* exported functions */
void check_hash_assumptions(void);

void inithash (void);
void closehash (void);
void HashStats(unsigned int level, char *trailer);

typedef unsigned int hash_value_type;

boolean inhash(slice_index si,
               hashwhat what,
               hash_value_type val,
               HashBuffer *);
void addtohash(slice_index si,
               hashwhat what,
               hash_value_type val,
               HashBuffer *);

void IncHashRateLevel(void);
void DecHashRateLevel(void);

extern unsigned long int compression_counter;

#endif
