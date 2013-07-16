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
#include "pieces/walks/vectors.h"

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

void	moarider_generate_moves(square i);
void	maorider_generate_moves(square i);
void generate_locust_capture(square sq_departure, square sq_capture,
                             vec_index_type k);
void radialknight_generate_moves(square sq_departure);
void treehopper_generate_moves(square sq_departure);
void leafhopper_generate_moves(square sq_departure);
void greater_treehopper_generate_moves(square sq_departure);
void greater_leafhopper_generate_moves(square sq_departure);

void	reverse_pawn_generate_moves(square sq_departure);
void	berolina_pawn_generate_moves(square sq_departure);

void clearedgestraversed(void);
boolean traversed(square edgesq);
void settraversed(square edgesq);

#endif	/* PY4_H */
