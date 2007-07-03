/******************** MODIFICATIONS to py4.h **************************
**
** Date       Who  What
** 
** 1995       TLi  Original
** 
** 2006/07/30 SE   New condition: Schwarzschacher  
**
**************************** End of List ******************************/ 
 
#ifndef	PY4_H
#define	PY4_H

short   len_max (square i, square j, square cab);
	/* determines move length for maximummers */
short   len_min (square i, square j, square cab);
	/* determines move length for minimummer */
short   len_capt (square i, square j, square cab);
	/* determines "move length" for must capture */
short   len_follow (square i, square j, square cab);
	/* determines "move length" for  follow my leader */
short   len_whduell (square i, square j, square cab);
	/* determines "length" of white moves in duellist chess */
short   len_blduell (square i, square j, square cab);
	/* determines "length" of black moves in duellist chess */
short   len_check (square i, square j, square cab);
	/* determines "length" of moves in SchachZwang */
short   len_direct (square i, square j, square cab);
	/* determines "length" of moves in direct reflex */
short   len_alphabetic (square i, square j, square cab);
	/* determines "length" of moves in alphabetic chess */
short len_synchron(square id, square ia, square ip);
  /* determines "length" of moves in synchron chess */
short len_antisynchron(square id, square ia, square ip);
  /* determines "length" of moves in antisynchron chess */
short len_schwarzschacher (square i, square j, square cab);
	/* determines "length" of moves in schwarzschacher chess */

boolean empile(square id, square ia, square ip);
boolean testempile(square id, square ia, square ip);


void	gemoarider(square i, couleur camp);
void	gemaorider(square i, couleur camp);
void	gebrid(square i, numvec kbeg, numvec kend);
void	genrid(square i, numvec kbeg, numvec kend);
void    gebleap(square i, numvec kbeg, numvec kend);
void    genleap(square i, numvec kbeg, numvec kend);
void    geriderhopper(square i, numvec kbeg, numvec kend,
		      smallint run_up, smallint jump, couleur camp);
void	grose(square i, int type, couleur camp);

void	genpb(square i);
	/* generating function for white pawn */
void	genpn(square i);
	/* generating function for black pawn */
void	genpbb(square i);
	/* generating function for white berolina pawn */
void	genpbn(square i);
	/* generating function for black berolina pawn */

#endif	/* PY4_H */
