#include "conditions/madrasi.h"
#include "pieces/pieces.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"

#include "debugging/trace.h"

boolean madrasi_is_rex_inclusive;

/* Determine whether a particular piece of the moving side is observed
 * @param sq position of the piece
 * @return true iff the piece occupying square sq is observed by the opponent
 */
boolean madrasi_is_moving_piece_observed(square sq)
{
  boolean result;
  Side const observed_side = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  if (!madrasi_is_rex_inclusive && sq==king_square[observed_side])
    result = false;
  else
  {
    PieNam const p = get_walk_of_piece_on_square(sq);
    Side const observing_side = advers(observed_side);

    if (number_of_pieces[observing_side][p]==0)
      result = false;
    else
    {
      nextply(observing_side); /* not siblingply() or ep paralysis causes problems! */
      INIT_CURRMOVE(nbply);
      move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = sq;
      observing_walk[nbply] = p;
      result = (*checkfunctions[p])(&validate_observation_geometry);
      finply();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_paralysed(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  boolean result;
  Side const observed_side = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  if (!madrasi_is_rex_inclusive && sq_departure==king_square[observed_side])
    result = false;
  else
  {
    PieNam const candidate = get_walk_of_piece_on_square(sq_departure);
    Side const observing_side = advers(observed_side);

    siblingply(observing_side);
    ++current_move[nbply];
    move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture = sq_departure;
    observing_walk[nbply] = candidate;
    result = (number_of_pieces[trait[nbply]][candidate]>0
              && (*checkfunctions[candidate])(&validate_observation_geometry));
    finply();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observater according to Madrasi
 * @return true iff the observation is valid
 */
boolean madrasi_validate_observer(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (!is_paralysed(CURRMOVE_OF_PLY(nbply))
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void madrasi_generate_moves_for_piece(slice_index si, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (!is_paralysed(current_generation))
    generate_moves_for_piece(slices[si].next1,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Madrasi
 * @param si identifies root slice of solving machinery
 */
void madrasi_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STMadrasiMovesForPieceGenerator);

  stip_instrument_observer_validation(si,nr_sides,STMadrasiMovesForPieceGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
