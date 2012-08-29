#include "conditions/amu/attack_counter.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

boolean amu_attacked_exactly_once[maxply+1];

static square single_attacker_departure;
static unsigned int amu_attack_count;

static boolean eval_amu_attack(square sq_departure,
                               square sq_arrival,
                               square sq_capture)
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

static boolean is_attacked_exactly_once(square sq_departure, Side trait_ply)
{
  square const save_king_square = king_square[trait_ply];

  king_square[trait_ply] = sq_departure;
  amu_attack_count = 0;
  single_attacker_departure = initsquare;
  rechec[trait_ply](&eval_amu_attack);
  king_square[trait_ply] = save_king_square;

  return amu_attack_count==1;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type amu_attack_counter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  amu_attacked_exactly_once[nbply] = is_attacked_exactly_once(sq_departure,starter);
  result = attack(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type amu_attack_counter_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  Side const starter = slices[si].starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  amu_attacked_exactly_once[nbply] = is_attacked_exactly_once(sq_departure,starter);
  result = defend(slices[si].next1,n);

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

  stip_instrument_moves_no_replay(si,STAMUAttackCounter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
