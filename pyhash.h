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

/* typedefs */
typedef unsigned char   byte;
typedef enum {
	SerNoSucc,
	IntroSerNoSucc,
	WhHelpNoSucc,
	BlHelpNoSucc,
	WhDirSucc,
	WhDirNoSucc
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

void    inithash (void);
void    closehash (void);
void	HashStats(int level, char *trailer);
boolean inhash(hashwhat what, int val, HashBuffer *);
void addtohash(hashwhat what, int val, HashBuffer *);

boolean is_a_mating_piece_left(Side mating_side);

void IncHashRateLevel(void);
void DecHashRateLevel(void);

#endif
