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
#include "pieces/vectors.h"

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
square generate_moves_on_line_segment(square sq_departure,
                                      square sq_base,
                                      vec_index_type k);
void generate_locust_capture(square sq_departure, square sq_capture,
                             vec_index_type k,
                             Side camp);
void	rider_generate_moves(Side side, square i, vec_index_type kbeg, vec_index_type kend);
void  leaper_generate_moves(Side side, square i, vec_index_type kbeg, vec_index_type kend);
void  geriderhopper(square i, vec_index_type kbeg, vec_index_type kend,
                    int run_up, int jump, Side camp);
void genradialknight(square sq_departure, Side camp);
void gentreehopper(square sq_departure, Side camp);
void genleafhopper(square sq_departure, Side camp);
void gengreatertreehopper(square sq_departure, Side camp);
void gengreaterleafhopper(square sq_departure, Side camp);

void	reverse_pawn_generate_moves(Side side, square sq_departure);
void	berolina_pawn_generate_moves(Side side, square sq_departure);

void clearedgestraversed();
boolean traversed(square edgesq);
void settraversed(square edgesq);

#endif	/* PY4_H */
