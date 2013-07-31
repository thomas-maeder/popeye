#include "conditions/transmuting_kings/reflective_kings.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void reflective_kings_generate_moves_for_piece(slice_index si,
                                               square sq_departure,
                                               PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (p==King)
  {
    numecoup const save_nbcou = current_move[nbply];
    generate_moves_for_piece(slices[si].next1,sq_departure,King);
    if (generate_moves_of_transmuting_king(si,sq_departure))
      remove_duplicate_moves_of_single_piece(save_nbcou);
  }
  else
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a square is observed be the side at the move according to
 * Reflective Kings
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean reflective_king_is_square_observed(slice_index si, evalfunction_t *evaluate)
{
  if (number_of_pieces[trait[nbply]][King]>0)
  {
    Side const side_attacking = trait[nbply];
    Side const side_attacked = advers(side_attacking);

    PieNam *ptrans;

    for (ptrans = transmpieces[side_attacking]; *ptrans; ptrans++)
      if (number_of_pieces[side_attacked][*ptrans]>0
          && is_king_transmuting_as(*ptrans,evaluate)
          && (*checkfunctions[*ptrans])(move_generation_stack[current_move[nbply]].capture,King,evaluate))
        return true;
  }

  return is_square_observed_recursive(slices[si].next1,evaluate);
}

/* Inialise the solving machinery with reflective kings
 * @param si identifies root slice of solving machinery
 * @param side for whom
 */
void reflective_kings_initialise_solving(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
   TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,side,STReflectiveKingsMovesForPieceGenerator);
  instrument_alternative_is_square_observed_king_testing(si,side,STReflectiveKingIsSquareObserved);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
