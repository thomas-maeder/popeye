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

int   len_max (square departure, square arrival, square capture);
	/* determines move length for maximummers */
int   len_min (square departure, square arrival, square capture);
	/* determines move length for minimummer */
int   len_capt (square departure, square arrival, square capture);
	/* determines "move length" for must capture */
int   len_follow (square departure, square arrival, square capture);
	/* determines "move length" for  follow my leader */
int   len_whduell (square departure, square arrival, square capture);
	/* determines "length" of white moves in duellist chess */
int   len_blduell (square departure, square arrival, square capture);
	/* determines "length" of black moves in duellist chess */
int   len_check (square departure, square arrival, square capture);
	/* determines "length" of moves in SchachZwang */
int   len_direct (square departure, square arrival, square capture);
	/* determines "length" of moves in direct reflex */
int   len_alphabetic (square departure, square arrival, square capture);
	/* determines "length" of moves in alphabetic chess */
int len_synchron(square departure, square arrival, square capture);
  /* determines "length" of moves in synchron chess */
int len_antisynchron(square departure, square arrival, square capture);
  /* determines "length" of moves in antisynchron chess */
int len_schwarzschacher (square i, square j, square cab);
	/* determines "length" of moves in schwarzschacher chess */

boolean empile(square departure, square arrival, square capture);
boolean testempile(square departure, square arrival, square capture);


void	gemoarider(square i, couleur camp);
void	gemaorider(square i, couleur camp);
void	gebrid(square i, numvec kbeg, numvec kend);
void	genrid(square i, numvec kbeg, numvec kend);
void    gebleap(square i, numvec kbeg, numvec kend);
void    genleap(square i, numvec kbeg, numvec kend);
void    geriderhopper(square i, numvec kbeg, numvec kend,
		      int run_up, int jump, couleur camp);
void grose(square sq_departure,
           numvec k1, numvec k2, numvec delta_k,
           couleur camp);
void genradialknight(square sq_departure, couleur camp);

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

#endif	/* PY4_H */
