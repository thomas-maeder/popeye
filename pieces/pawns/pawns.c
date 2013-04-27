#include "pieces/pawns/pawns.h"
#include "conditions/einstein/en_passant.h"
#include "conditions/wormhole.h"
#include "solving/en_passant.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* generates move of a pawn of side camp on departure capturing a piece on
 * arrival
 */
void pawns_generate_capture_move(Side side,
                                 square sq_departure,
                                 numvec dir)
{
  Side const opponent = advers(side);
  square const sq_arrival = sq_departure+dir;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (piece_belongs_to_opponent(e[sq_arrival],side))
    /* normal capture */
    empile(sq_departure,sq_arrival,sq_arrival);
  else if (abs(e[sq_departure])!=Orphan /* orphans cannot capture ep */
           && (sq_arrival==ep[parent_ply[nbply]]
               || sq_arrival==einstein_ep[parent_ply[nbply]]))
  {
    /* ep capture */
    square prev_arrival;

    if (nbply==2)
    {
      /* ep.-key  standard pawn */
      int const dir_backward = side==White ? dir_down : dir_up;
      prev_arrival = sq_arrival+dir_backward;
    }
    else if (trait[parent_ply[nbply]]==opponent)
      prev_arrival = move_generation_stack[current_move[parent_ply[nbply]]].arrival;
    else
      prev_arrival = initsquare;

    if (TSTFLAG(sq_spec[prev_arrival],Wormhole))
      prev_arrival = wormhole_positions[wormhole_next_transfer[parent_ply[nbply]]-1];

    if (TSTFLAG(spec[prev_arrival],opponent))
      empile(sq_departure,sq_arrival,prev_arrival);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* end pawns_generate_capture_move */

/* generates moves of a pawn in direction dir where steps single steps are
 * possible.
 */
void pawns_generate_nocapture_moves(square sq_departure, numvec dir, int steps)
{
  square sq_arrival= sq_departure+dir;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParam("%d",dir);
  TraceFunctionParam("%d",steps);
  TraceFunctionParamListEnd();

  while (steps--)
  {
    TraceSquare(sq_arrival);
    TracePiece(e[sq_arrival]);
    TraceText("\n");
    if (e[sq_arrival]==vide && empile(sq_departure,sq_arrival,sq_arrival))
      sq_arrival+= dir;
    else
      break;
    TraceValue("%d\n",steps);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
