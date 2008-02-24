/******************** MODIFICATIONS to pyhash.h **************************
**
** Date       Who  What
** 
** 1994            Original
** 
**************************** End of List ******************************/ 

#if !defined(_PYHASH_H)
#define _PYHASH_H

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

/* exported functions */
void    inithash (void);
void    closehash (void);
void	HashStats(int level, char *trailer);
boolean inhash(hashwhat what, smallint val);
void addtohash(hashwhat what, smallint val);
boolean ser_dsrsol(couleur camp, smallint n, boolean restartenabled);
boolean shsol(couleur camp, smallint n, boolean restartenabled);
boolean mataide(couleur camp, smallint n, boolean restartenabled);
boolean last_dsr_move(couleur camp);

#endif
