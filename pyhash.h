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

extern  void (*encode)(HashBuffer*);

/* exported functions */
void check_hash_assumptions(void);

void    inithash (void);
void    closehash (void);
void	HashStats(int level, char *trailer);
boolean inhash(hashwhat what, int val, HashBuffer *);
void addtohash(hashwhat what, int val, HashBuffer *);
void sr_find_write_final_move(couleur defender, slice_index si);
boolean ser_find_write_solutions(couleur camp,
                                 int n,
                                 boolean restartenabled,
                                 slice_index si);
boolean h_find_write_solutions(couleur camp,
                               int n,
                               boolean restartenabled,
                               slice_index si);

void IncHashRateLevel(void);
void DecHashRateLevel(void);

#endif
