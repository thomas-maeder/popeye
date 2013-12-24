#include "conditions/vaulting_kings.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

boolean vaulting_kings_transmuting[nr_sides];
PieNam king_vaulters[nr_sides][PieceCount];
unsigned int nr_king_vaulters[nr_sides];

void reset_king_vaulters(void)
{
  nr_king_vaulters[White] = 0;
  nr_king_vaulters[Black] = 0;
}

void append_king_vaulter(Side side, PieNam p)
{
  king_vaulters[side][nr_king_vaulters[side]] = p;
  ++nr_king_vaulters[side];
}

static boolean is_kingsquare_observed(slice_index si)
{
  Side const side = trait[nbply];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  siblingply(advers(side));
  push_observation_target(king_square[side]);
  result = is_square_observed_recursive(slices[si].next2,EVALUATE(observation));
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void vaulting_kings_generate_moves_for_piece(slice_index si, PieNam p)
{
  if (p==King)
  {
    Side const side = trait[nbply];

    if (is_kingsquare_observed(temporary_hack_is_square_observed_by_non_king[side]))
    {
      unsigned int i;
      for (i = 0; i!=nr_king_vaulters[side]; ++i)
        generate_moves_for_piece(slices[si].next1,king_vaulters[side][i]);
    }
    else if (vaulting_kings_transmuting[side])
      return; /* don't generate non-vaulting moves */
  }

  generate_moves_for_piece(slices[si].next1,p);
}

static boolean does_vaulting_king_observe(slice_index si, validator_id evaluate)
{
  boolean result = false;
  unsigned int i;
  Side const side_observing = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  testing_observation_by_transmuting_king[nbply] = true;

  for (i = 0; i!=nr_king_vaulters[side_observing]; ++i)
  {
    observing_walk[nbply] = king_vaulters[side_observing][i];
    TraceValue("%u",i);TracePiece(king_vaulters[side_observing][i]);TraceEOL();
    if (is_square_observed_recursive(slices[si].next2,evaluate))
    {
      result = true;
      break;
    }
  }

  testing_observation_by_transmuting_king[nbply] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a square is observed be the side at the move according to
 * Vaulting Kings
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean vaulting_king_is_square_observed(slice_index si, validator_id evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (king_square[trait[nbply]]==initsquare)
    result = is_square_observed_recursive(slices[si].next1,evaluate);
  else
  {
    Side const side_observing = trait[nbply];
    if (is_kingsquare_observed(temporary_hack_is_square_observed_by_non_king[side_observing]))
    {
      if (does_vaulting_king_observe(si,evaluate))
        result = true;
      else if (vaulting_kings_transmuting[side_observing])
        result = false;
      else
        result = is_square_observed_recursive(slices[si].next1,evaluate);
    }
    else
      result = is_square_observed_recursive(slices[si].next1,evaluate);
  }

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

  instrument_alternative_is_square_observed_king_testing(si,side,STVaultingKingIsSquareObserved);

  stip_instrument_observation_validation(si,side,STTransmutingKingsEnforceObserverWalk);
  stip_instrument_check_validation(si,side,STTransmutingKingsEnforceObserverWalk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
