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
static unsigned int nr_king_vaulters[nr_sides];

void reset_king_vaulters(void)
{
  nr_king_vaulters[White] = 0;
  nr_king_vaulters[Black] = 0;

  king_vaulters[White][0] = Empty;
  king_vaulters[Black][0] = Empty;
}

void append_king_vaulter(Side side, PieNam p)
{
  king_vaulters[side][nr_king_vaulters[side]] = p;
  ++nr_king_vaulters[side];
  king_vaulters[side][nr_king_vaulters[side]] = Empty;
}

static boolean is_kingsquare_observed(slice_index si)
{
  Side const side = trait[nbply];
  boolean result;

  siblingply(advers(side));
  move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = king_square[side];
  result = is_square_observed_recursive(slices[si].next2,&validate_observation);
  finply();

  return result;
}

/* identifiers of slice determining whether a side's king is observed
 */
static slice_index vaulting_king_square_observer[nr_sides];

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void vaulting_kings_generate_moves_for_piece(slice_index si, PieNam p)
{
  if (p==King)
  {
    Side const side = trait[nbply];

    if (is_kingsquare_observed(vaulting_king_square_observer[side]))
    {
      PieNam const *pi_vaulter;
      for (pi_vaulter = king_vaulters[side]; *pi_vaulter!=Empty; ++pi_vaulter)
        generate_moves_for_piece(slices[si].next1,*pi_vaulter);
    }
    else if (vaulting_kings_transmuting[side])
      return; /* don't generate non-vaulting moves */
  }

  generate_moves_for_piece(slices[si].next1,p);
}

/* Determine whether a square is observed be the side at the move according to
 * Vaulting Kings
 * @param si identifies next slice
 * @return true iff sq_target is observed by the side at the move
 */
boolean vaulting_king_is_square_observed(slice_index si, evalfunction_t *evaluate){
  if (king_square[trait[nbply]]==initsquare)
    return is_square_observed_recursive(slices[si].next1,evaluate);
  else
  {
    Side const side_observing = trait[nbply];
    if (is_kingsquare_observed(vaulting_king_square_observer[side_observing]))
    {
      PieNam const *pi_vaulter;
      for (pi_vaulter = king_vaulters[side_observing]; *pi_vaulter; ++pi_vaulter)
      {
        observing_walk[nbply] = King;
        if ((*checkfunctions[*pi_vaulter])(evaluate))
          return true;
      }

      if (vaulting_kings_transmuting[side_observing])
        return is_square_observed_recursive(slices[si].next2,evaluate);
    }

    return is_square_observed_recursive(slices[si].next1,evaluate);
  }
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

  if (king_vaulters[side][0]==Empty)
  {
    king_vaulters[side][0] = EquiHopper;
    king_vaulters[side][1] = Empty;
  }

  solving_instrument_move_generation(si,side,STVaultingKingsMovesForPieceGenerator);

  vaulting_king_square_observer[side] = instrument_alternative_is_square_observed_king_testing(si,side,STVaultingKingIsSquareObserved);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
