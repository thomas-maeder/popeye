#include "pieces/attributes/hurdle_colour_changing.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "stipulation/structure_traversal.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static void update_hurdle_colour(void)
{
  square const sq_hurdle = move_generation_stack[current_move[nbply]].hopper_hurdle;
  piece const pi_hurdle = e[sq_hurdle];

  if (abs(pi_hurdle)>King)
    move_effect_journal_do_side_change(move_effect_reason_hurdle_colour_changing,
                                       sq_hurdle,
                                       e[sq_hurdle]<vide ? White : Black);
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type hurdle_colour_changer_solve(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[move_generation_stack[current_move[nbply]].arrival],
              ColourChange))
    update_hurdle_colour();

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_hurdle_colour_changers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STHurdleColourChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
