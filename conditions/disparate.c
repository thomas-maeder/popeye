#include "conditions/disparate.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pydata.h"

static boolean can_piece_move(numecoup n)
{
  boolean result = true;
  ply const parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
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
      square const sq_departure = move_generation_stack[n].departure;
      PieNam const pi_parent_moving = move_effect_journal[parent_movement].u.piece_movement.moving;
      if (get_walk_of_piece_on_square(sq_departure)==pi_parent_moving)
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
 * @param p walk to be used for generating
 */
void disparate_generate_moves_for_piece(slice_index si, PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (can_piece_move(current_generation))
    generate_moves_for_piece(slices[si].next1,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate an observation according to Disparate Chess
 * @return true iff the observation is valid
 */
boolean disparate_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (can_piece_move(current_move[nbply]-1))
    result = validate_observation_recursive(slices[si].next1);
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
  stip_instrument_check_validation(si,nr_sides,STValidatingObservationDisparate);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
