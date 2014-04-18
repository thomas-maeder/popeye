#include "conditions/transmuting_kings/vaulting_kings.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
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
    push_observation_target(king_square[side]);
    result = is_square_observed_recursive(slices[temporary_hack_is_square_observed[side]].next2,EVALUATE(observation));
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

  if (TSTFULLFLAGMASK(spec[curr_generation->departure],mask))
  {
    if (is_kingsquare_observed())
    {
      piece_walk_type const save_current_walk = move_generation_current_walk;
      unsigned int i;
      for (i = 0; i!=nr_king_vaulters[side]; ++i)
      {
        move_generation_current_walk = king_vaulters[side][i];
        generate_moves_for_piece(slices[si].next1);
      }
      move_generation_current_walk = save_current_walk;
    }
    else if (vaulting_kings_transmuting[side])
      return; /* don't generate non-vaulting moves */
  }

  generate_moves_for_piece(slices[si].next1);
}

/* Determine whether a square is observed be the side at the move according to
 * Vaulting Kings
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean vaulting_king_is_square_observed(slice_index si, validator_id evaluate)
{
  boolean result;
  Side const side_observing = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (king_square[side_observing]==initsquare)
    result = is_square_observed_recursive(slices[si].next1,evaluate);
  else
  {
    is_king_vaulting[nbply] = dont_know;
    result = is_square_observed_recursive(slices[si].next1,evaluate);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_king_vaulter(Side side, piece_walk_type walk)
{
  unsigned int i;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
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
  square const sq_king = king_square[side_observing];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (transmuting_kings_testing_transmutation[advers(side_observing)])
    result = validate_observation_recursive(slices[si].next1);
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
        result = validate_observation_recursive(slices[si].next1);
        observing_walk[nbply] = save_walk;

        if (!result && !vaulting_kings_transmuting[side_observing])
          result = validate_observation_recursive(slices[si].next1);
      }
      else
        result = validate_observation_recursive(slices[si].next1);
    }
    else
      result = validate_observation_recursive(slices[si].next1);
  }
  else
    result = validate_observation_recursive(slices[si].next1);

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
  TraceEnumerator(Side,side,"");
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
