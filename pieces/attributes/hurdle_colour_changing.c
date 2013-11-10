#include "pieces/attributes/hurdle_colour_changing.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/hoppers.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/structure_traversal.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

static void update_hurdle_colour(void)
{
  square const sq_hurdle = hoppper_moves_auxiliary[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id].sq_hurdle;
  PieNam const pi_hurdle = get_walk_of_piece_on_square(sq_hurdle);

  if (pi_hurdle>King && !is_piece_neutral(spec[sq_hurdle]))
    move_effect_journal_do_side_change(move_effect_reason_hurdle_colour_changing,
                                       sq_hurdle);
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
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

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
    if (TSTFLAG(movingspec,ColourChange))
      update_hurdle_colour();
  }

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
