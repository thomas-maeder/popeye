#include "conditions/koeko/anti.h"
#include "solving/observation.h"
#include "solving/single_move_generator.h"
#include "solving/move_effect_journal.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/temporary_hacks.h"
#include "debugging/trace.h"
#include "pydata.h"

nocontactfunc_t antikoeko_nocontact;


/* Validate an observation according to Patrol Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
static boolean avoid_contact_while_observing(square sq_observer,
                                             square sq_landing,
                                             square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  init_single_move_generator(sq_observer,sq_landing,sq_observee);
  result = solve(slices[temporary_hack_king_capture_legality_tester[trait[nbply]]].next2,length_unspecified)==next_move_has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
stip_length_type antikoeko_legality_tester_solve(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_top[nbply-1];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
   if ((*antikoeko_nocontact)(pos))
      result = solve(slices[si].next1,n);
    else
      result = previous_move_is_illegal;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise solving in Anti-Koeko
 * @param si identifies the root slice of the stipulation
 */
void antikoeko_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_instrument_moves(si,STAntiKoekoLegalityTester);

  register_observer_validator(&avoid_contact_while_observing);
  register_observation_geometry_validator(&avoid_contact_while_observing);
  register_observation_validator(&avoid_contact_while_observing);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
