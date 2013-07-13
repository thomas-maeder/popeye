#include "conditions/madrasi.h"
#include "pydata.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "stipulation/stipulation.h"

#include "debugging/trace.h"

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

  if (!rex_mad && sq==king_square[observed_side])
    result = false;
  else
  {
    PieNam const p = get_walk_of_piece_on_square(sq);
    Side const observing_side = advers(observed_side);

    if (number_of_pieces[observing_side][p]==0)
      result = false;
    else
    {
      nextply();
      trait[nbply] = observing_side;
      result = (*checkfunctions[p])(sq,p,&validate_observation_geometry);
      finply();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Can a piece deliver check according to Madrasi
 * @param sq_departure position of the piece
 * @param sq_arrival arrival square of the capture to be threatened
 * @param sq_capture typically the position of the opposite king
 */
static boolean avoid_observation_by_paralysed(square sq_observer,
                                              square sq_landing,
                                              square sq_observee)
{
  return !madrasi_is_moving_piece_observed(sq_observer);
}

static boolean can_piece_move(square sq)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  if (!rex_mad && sq==king_square[trait[nbply]])
    result = true;
  else
  {
    PieNam const candidate = get_walk_of_piece_on_square(sq);

    trait[nbply] = advers(trait[nbply]);
    result = (number_of_pieces[trait[nbply]][candidate]==0
              || !(*checkfunctions[candidate])(sq,
                                               candidate,
                                               &validate_observation_geometry));
    trait[nbply] = advers(trait[nbply]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void madrasi_generate_moves_for_piece(slice_index si,
                                      square sq_departure,
                                      PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (can_piece_move(sq_departure))
    generate_moves_for_piece(slices[si].next1,sq_departure,p);

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

  solving_instrument_move_generation(si,STMadrasiMovesForPieceGenerator);

  register_observer_validator(&avoid_observation_by_paralysed);
  register_observation_validator(&avoid_observation_by_paralysed);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
