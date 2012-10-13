#include "conditions/norsk.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/move_effect_journal.h"
#include "pieces/walks.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static PieNam norsk_walk(PieNam p)
{
  if (p==standard_walks[Queen])
    return standard_walks[Knight];
  else if (p==standard_walks[Knight])
    return standard_walks[Queen];
  else if (p==standard_walks[Rook])
    return standard_walks[Bishop];
  else if (p==standard_walks[Bishop])
    return standard_walks[Rook];
  else
    return p;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type norsk_arriving_adjuster_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (current_promotion_of_moving[nbply]==Empty)
  {
    square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
    piece const norsked = e[sq_arrival];
    PieNam const norsked_to_walk = norsk_walk(abs(norsked));
    piece const norsked_to = norsked<vide ? -norsked_to_walk : norsked_to_walk;
    if (norsked!=norsked_to)
      move_effect_journal_do_piece_change(move_effect_reason_norsk_chess,
                                          sq_arrival,
                                          norsked_to);
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_norsk_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STNorskArrivingAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
