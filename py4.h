/******************** MODIFICATIONS to py4.h **************************
**
** Date       Who  What
**
** 1995       TLi  Original
**
** 2006/07/30 SE   New condition: Schwarzschacher
**
**************************** End of List ******************************/

#if !defined(PY4_H)
#define	PY4_H

#include "py.h"

int len_max (square departure, square arrival, square capture);
	/* determines move length for maximummers */
int len_min (square departure, square arrival, square capture);
	/* determines move length for minimummer */
int len_capt (square departure, square arrival, square capture);
	/* determines "move length" for must capture */
int len_follow (square departure, square arrival, square capture);
	/* determines "move length" for  follow my leader */
int len_whduell (square departure, square arrival, square capture);
	/* determines "length" of white moves in duellist chess */
int len_blduell (square departure, square arrival, square capture);
	/* determines "length" of black moves in duellist chess */
int len_check (square departure, square arrival, square capture);
	/* determines "length" of moves in SchachZwang */
int len_direct (square departure, square arrival, square capture);
	/* determines "length" of moves in direct reflex */
int len_alphabetic (square departure, square arrival, square capture);
	/* determines "length" of moves in alphabetic chess */
int len_synchron(square departure, square arrival, square capture);
  /* determines "length" of moves in synchron chess */
int len_antisynchron(square departure, square arrival, square capture);
  /* determines "length" of moves in antisynchron chess */
int len_schwarzschacher (square sq_departure, square sq_arrival, square sq_capture);
	/* determines "length" of moves in schwarzschacher chess */
int len_losingchess(square sq_departure, square sq_arrival, square sq_capture);
	/* determines "length" of moves in losing chess (schlagschach) */

boolean empile(square departure, square arrival, square capture);


void	gemoarider(square i, Side camp);
void	gemaorider(square i, Side camp);
void	gebrid(square i, numvec kbeg, numvec kend);
void	genrid(square i, numvec kbeg, numvec kend);
void    gebleap(square i, numvec kbeg, numvec kend);
void    genleap(square i, numvec kbeg, numvec kend);
void    geriderhopper(square i, numvec kbeg, numvec kend,
		      int run_up, int jump, Side camp);
void grose(square sq_departure,
           numvec k1, numvec k2, numvec delta_k,
           Side camp);
void genradialknight(square sq_departure, Side camp);
void gentreehopper(square sq_departure, Side camp);
void genleafhopper(square sq_departure, Side camp);
void gengreatertreehopper(square sq_departure, Side camp);
void gengreaterleafhopper(square sq_departure, Side camp);
void generate_marine_knight(square sq_departure, Side camp);
void generate_poseidon(square sq_departure, Side moving);

void	genpb(square sq_departure);
	/* generating function for white pawn */
void	genpn(square sq_departure);
	/* generating function for black pawn */
void	genreversepb(square sq_departure);
	/* generating function for white reverse pawn */
void	genreversepn(square sq_departure);
	/* generating function for black reverse pawn */
void	genpbb(square sq_departure);
	/* generating function for white berolina pawn */
void	genpbn(square sq_departure);
	/* generating function for black berolina pawn */

void clearedgestraversed();
boolean traversed(square edgesq);
void settraversed(square edgesq);

#endif	/* PY4_H */
