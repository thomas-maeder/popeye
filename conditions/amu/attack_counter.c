#include "conditions/amu/attack_counter.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/move_generator.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "position/position.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include "solving/post_move_iteration.h"

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
        && TSTFLAG(being_solved.spec[sq_departure],trait[nbply]))
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
    result = pipe_validate_observation_recursive_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_attacked_exactly_once(square sq_departure, Side trait_ply)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(!are_we_counting);

  are_we_counting = true;

  amu_attack_count = 0;
  single_attacker_departure = initsquare;

  is_square_observed_general_post_move_iterator_solve(advers(trait_ply),
                                                      sq_departure,
                                                      EVALUATE(observation));

  are_we_counting = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",amu_attack_count==1);
  TraceFunctionResultEnd();
  return amu_attack_count==1;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void amu_attack_counter_solve(slice_index si)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  Side const starter = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  amu_attacked_exactly_once[nbply] = is_attacked_exactly_once(sq_departure,starter);
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_amu_attack_counter(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STAMUAttackCounter);
  stip_instrument_observation_validation(si,nr_sides,STAMUObservationCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
