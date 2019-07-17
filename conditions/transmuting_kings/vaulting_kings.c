#include "conditions/transmuting_kings/vaulting_kings.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/position.h"

#include "debugging/assert.h"

boolean vaulting_kings_transmuting[nr_sides];
piece_walk_type king_vaulters[nr_sides][nr_piece_walks];
unsigned int nr_king_vaulters[nr_sides];

typedef enum
{
  dont_know,
  does_vault,
  does_not_vault
} knowledge;

static knowledge is_king_vaulting[maxply+1];

void reset_king_vaulters(void)
{
  nr_king_vaulters[White] = 0;
  nr_king_vaulters[Black] = 0;
}

void append_king_vaulter(Side side, piece_walk_type p)
{
  king_vaulters[side][nr_king_vaulters[side]] = p;
  ++nr_king_vaulters[side];
}

static boolean is_kingsquare_observed(void)
{
  Side const side = trait[nbply];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (transmuting_kings_testing_transmutation[side])
    result = false;
  else
  {
    transmuting_kings_testing_transmutation[side] = true;

    siblingply(advers(side));
    push_observation_target(being_solved.king_square[side]);
    result = fork_is_square_observed_nested_delegate(temporary_hack_is_square_observed[side],
                                                     EVALUATE(observation));
    finply();

    transmuting_kings_testing_transmutation[side] = false;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void vaulting_kings_generate_moves_for_piece(slice_index si)
{
  Side const side = trait[nbply];
  Flags const mask = BIT(side)|BIT(Royal);

  if (TSTFULLFLAGMASK(being_solved.spec[curr_generation->departure],mask))
  {
    if (is_kingsquare_observed())
    {
      unsigned int i;
      for (i = 0; i!=nr_king_vaulters[side]; ++i)
        pipe_move_generation_different_walk_delegate(si,king_vaulters[side][i]);
    }
    else if (vaulting_kings_transmuting[side])
      return; /* don't generate non-vaulting moves */
  }

  pipe_solve_delegate(si);
}

/* Determine whether a square is observed be the side at the move according to
 * Vaulting Kings
 * @param si identifies next slice
 * @note sets observation_validation_result
 */
void vaulting_king_is_square_observed(slice_index si)
{
  Side const side_observing = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (being_solved.king_square[side_observing]==initsquare)
    pipe_solve_delegate(si);
  else
  {
    is_king_vaulting[nbply] = dont_know;
    pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_king_vaulter(Side side, piece_walk_type walk)
{
  unsigned int i;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_king_vaulters[side]; ++i)
    if (walk==king_vaulters[side][i])
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Make sure to behave correctly while detecting observations by vaulting kings
 */
boolean vaulting_kings_enforce_observer_walk(slice_index si)
{
  boolean result;
  Side const side_observing = trait[nbply];
  square const sq_king = being_solved.king_square[side_observing];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (transmuting_kings_testing_transmutation[advers(side_observing)])
    result = pipe_validate_observation_recursive_delegate(si);
  else if (move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure==sq_king)
  {
    if (is_king_vaulting[nbply]==dont_know)
      is_king_vaulting[nbply] = is_kingsquare_observed() ? does_vault : does_not_vault;

    if (is_king_vaulting[nbply]==does_vault)
    {
      if (is_king_vaulter(side_observing,observing_walk[nbply]))
      {
        piece_walk_type const save_walk = observing_walk[nbply];
        observing_walk[nbply] = get_walk_of_piece_on_square(sq_king);
        result = pipe_validate_observation_recursive_delegate(si);
        observing_walk[nbply] = save_walk;

        if (!result && !vaulting_kings_transmuting[side_observing])
          result = pipe_validate_observation_recursive_delegate(si);
      }
      else
        result = pipe_validate_observation_recursive_delegate(si);
    }
    else
      result = pipe_validate_observation_recursive_delegate(si);
  }
  else
    result = pipe_validate_observation_recursive_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the solving machinery with Vaulting Kings
 * @param si root slice of the solving machinery
 * @param side for whom
 */
void vaulting_kings_initalise_solving(slice_index si, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  if (nr_king_vaulters[side]==0)
  {
    king_vaulters[side][0] = EquiHopper;
    nr_king_vaulters[side] = 1;
  }

  solving_instrument_move_generation(si,side,STVaultingKingsMovesForPieceGenerator);

  stip_instrument_is_square_observed_testing(si,side,STVaultingKingIsSquareObserved);

  stip_instrument_observation_validation(si,side,STVaultingKingsEnforceObserverWalk);
  stip_instrument_check_validation(si,side,STVaultingKingsEnforceObserverWalk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
