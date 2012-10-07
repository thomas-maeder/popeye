#include "conditions/protean.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type protean_pawn_adjuster_solve(slice_index si,
                                              stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pprise[nbply]!=vide
      && (!rex_protean_ex || !TSTFLAG(spec[sq_arrival],Royal)))
  {
    piece substitute = -pprise[nbply];
    if (pjoue[nbply]<vide)
    {
      if (substitute==pn)
        substitute = reversepn;
      else if (substitute==reversepn)
        substitute = pn;
    }
    else
    {
      if (substitute==pb)
        substitute = reversepb;
      else if (substitute==reversepb)
        substitute = pb;
    }

    move_effect_journal_do_piece_change(move_effect_reason_protean_adjustment,
                                        sq_arrival,
                                        substitute);
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_protean_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STProteanPawnAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
