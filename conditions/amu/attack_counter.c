#include "conditions/amu/attack_counter.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

boolean amu_attacked_exactly_once[maxply+1];

static square single_attacker_departure;
static unsigned int amu_attack_count;

static boolean eval_amu_attack(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;

  if (get_walk_of_piece_on_square(sq_departure)==observing_walk[nbply])
  {
    /* this deals correctly with double attacks by the same piece (e.g. a rose) */
    if (single_attacker_departure==sq_departure)
      return false;
    else
    {
      ++amu_attack_count;
      single_attacker_departure = sq_departure;
      return amu_attack_count==2;
    }
  }
  else
    return false;
}

static boolean is_attacked_exactly_once(square sq_departure, Side trait_ply)
{
  amu_attack_count = 0;
  single_attacker_departure = initsquare;

  siblingply(advers(trait_ply));
  current_move[nbply] = current_move[nbply-1]+1;
  move_generation_stack[current_move[nbply]-1].capture = sq_departure;
  is_square_observed(&eval_amu_attack);
  finply();

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
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
