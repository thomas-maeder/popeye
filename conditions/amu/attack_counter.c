#include "conditions/amu/attack_counter.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/move_generator.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

boolean amu_attacked_exactly_once[maxply+1];

static square single_attacker_departure;
static unsigned int amu_attack_count;
static boolean are_we_counting = false;

boolean amu_count_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (are_we_counting)
  {
    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;

    if (get_walk_of_piece_on_square(sq_departure)==observing_walk[nbply]
        && TSTFLAG(spec[sq_departure],trait[nbply]))
    {
      /* this deals correctly with double attacks by the same piece (e.g. a rose) */
      if (single_attacker_departure==sq_departure)
        result = false;
      else
      {
        ++amu_attack_count;
        single_attacker_departure = sq_departure;
        result = amu_attack_count==2;
      }
    }
    else
      result = false;
  }
  else
    result = validate_observation_recursive(slices[si].next1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_attacked_exactly_once(square sq_departure, Side trait_ply)
{
  assert(!are_we_counting);

  are_we_counting = true;

  amu_attack_count = 0;
  single_attacker_departure = initsquare;

  siblingply(advers(trait_ply));
  move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = sq_departure;
  is_square_observed(&validate_observation);
  finply();

  are_we_counting = false;

  return amu_attack_count==1;
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
stip_length_type amu_attack_counter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  amu_attacked_exactly_once[nbply] = is_attacked_exactly_once(sq_departure,starter);
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_amu_attack_counter(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAMUAttackCounter);
  stip_instrument_observation_validation(si,nr_sides,STAMUObservationCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
