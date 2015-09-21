#include "conditions/disparate.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/move_effect_journal.h"
#include "solving/pipe.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

static boolean can_piece_move(numecoup n)
{
  boolean result = true;
  ply const parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (parent>ply_retro_move && trait[nbply]!=trait[parent])
  {
    move_effect_journal_index_type const parent_base = move_effect_journal_base[parent];
    move_effect_journal_index_type const parent_movement = parent_base+move_effect_journal_index_offset_movement;
    if (parent_movement>=move_effect_journal_base[parent+1])
    {
      /* we are solving a threat - no disparate effect there */
    }
    else
    {
      square const sq_departure = move_generation_stack[n].departure;
      assert(move_effect_journal[parent_movement].type==move_effect_piece_movement);
      piece_walk_type const pi_parent_moving = move_effect_journal[parent_movement].u.piece_movement.moving;
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
 */
void disparate_generate_moves_for_piece(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (can_piece_move(current_generation))
    pipe_move_generation_delegate(si);

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

  if (can_piece_move(CURRMOVE_OF_PLY(nbply)))
    result = pipe_validate_observation_recursive_delegate(si);
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

  stip_instrument_observation_validation(si,nr_sides,STDisparateMovesForPieceGenerator);
  stip_instrument_check_validation(si,nr_sides,STDisparateMovesForPieceGenerator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
