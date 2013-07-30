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
      nextply(observing_side);
      current_move[nbply] = current_move[nbply-1]+1;
      move_generation_stack[current_move[nbply]].capture = sq;
      move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = initsquare;
      result = (*checkfunctions[p])(sq,p,&validate_observation_geometry);
      finply();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_paralysed(square sq)
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
    PieNam const candidate = get_walk_of_piece_on_square(sq);
    Side const observing_side = advers(observed_side);

    trait[nbply] = observing_side;
    result = (number_of_pieces[trait[nbply]][candidate]>0
              && (*checkfunctions[candidate])(sq,
                                              candidate,
                                              &validate_observation_geometry));
    trait[nbply] = observed_side;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observater according to Madrasi
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean madrasi_validate_observer(slice_index si,
                                  square sq_observer,
                                  square sq_landing,
                                  square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  result = (!is_paralysed(sq_observer)
            && validate_observer_recursive(slices[si].next1,
                                           sq_observer,
                                           sq_landing,
                                           sq_observee));

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

  if (!is_paralysed(sq_departure))
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

  solving_instrument_move_generation(si,nr_sides,STMadrasiMovesForPieceGenerator);

  stip_instrument_observer_validation(si,nr_sides,STMadrasiObserverTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
