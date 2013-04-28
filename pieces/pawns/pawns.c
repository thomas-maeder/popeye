#include "pieces/pawns/pawns.h"
#include "conditions/einstein/en_passant.h"
#include "conditions/wormhole.h"
#include "solving/en_passant.h"
#include "pydata.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

void pawns_generate_ep_capture_move(Side side,
                                    square sq_departure,
                                    square sq_arrival,
                                    square sq_arrival_singlestep)
{
  ply const ply_parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_arrival_singlestep);
  TraceFunctionParamListEnd();

  TraceSquare(ep[ply_parent]);
  TraceSquare(einstein_ep[ply_parent]);
  TraceText("\n");

  if (abs(e[sq_departure])!=Orphan /* orphans cannot capture ep */
      && (sq_arrival_singlestep==ep[ply_parent]
          || sq_arrival_singlestep==einstein_ep[ply_parent]))
  {
    /* ep capture */
    square sq_capture;

    if (nbply==2)
    {
      /* ep.-key  standard pawn */
      int const dir_backward = side==White ? dir_down : dir_up;
      sq_capture = sq_arrival_singlestep+dir_backward;
    }
    else if (trait[ply_parent]==advers(side))
      sq_capture = move_generation_stack[current_move[ply_parent]].arrival;
    else
      sq_capture = initsquare;

    if (TSTFLAG(sq_spec[sq_capture],Wormhole))
      sq_capture = wormhole_positions[wormhole_next_transfer[ply_parent]-1];

    if (piece_belongs_to_opponent(sq_capture,side))
    {
      empile(sq_departure,sq_arrival,sq_capture);
      move_generation_stack[current_move[nbply]].auxiliary = sq_arrival_singlestep;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* generates move of a pawn of side camp on departure capturing a piece on
 * arrival
 */
void pawns_generate_capture_move(Side side,
                                 square sq_departure,
                                 numvec dir)
{
  square const sq_arrival = sq_departure+dir;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (piece_belongs_to_opponent(sq_arrival,side))
    empile(sq_departure,sq_arrival,sq_arrival);
  else
    pawns_generate_ep_capture_move(side,sq_departure,sq_arrival,sq_arrival);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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
