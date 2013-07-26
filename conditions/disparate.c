#include "conditions/disparate.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

static boolean can_piece_move(square sq)
{
  boolean result = true;
  ply const parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(sq);
  TraceFunctionParamListEnd();

  if (nbply>2 && trait[nbply]!=trait[parent])
  {
    move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
    move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
    if (parent_movement>=move_effect_journal_top[parent])
    {
      /* we are solving a threat - no disparate effect there */
    }
    else
    {
      PieNam const pi_parent_moving = move_effect_journal[parent_movement].u.piece_movement.moving;
      if (get_walk_of_piece_on_square(sq)==pi_parent_moving)
        result = false;
    }
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
void disparate_generate_moves_for_piece(slice_index si,
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

/* Validate an observation according to Disparate Chess
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean disparate_validate_observation(slice_index si,
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

  if (can_piece_move(sq_observer))
    result = validate_observation_recursive(slices[si].next1,
                                            sq_observer,
                                            sq_landing,
                                            sq_observee);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise the solving machinery with Disparate chess
 * @param si identifies root slice of solving machinery
 */
void disparate_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,STDisparateMovesForPieceGenerator);

  stip_instrument_observation_validation(si,nr_sides,STValidatingObservationDisparate);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
