#include "conditions/pointreflection.h"
#include "position/position.h"
#include "pieces/walks/classification.h"
#include "solving/observation.h"
#include "solving/castling.h"
#include "solving/move_generator.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"

#include "debugging/trace.h"
#include "debugging/assert.h"

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void point_reflection_generate_moves_for_piece(slice_index si)
{
  square const reflected = transformSquare(curr_generation->departure,rot180);
  piece_walk_type const walk_reflected = get_walk_of_piece_on_square(reflected);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceSquare(curr_generation->departure);
  TraceSquare(reflected);
  TraceWalk(walk_reflected);
  TraceEOL();

  if (walk_reflected==Empty)
  {
    Side const side = trait[nbply];

    if (is_king(move_generation_current_walk)
        && TSTCASTLINGFLAGMASK(side,castlings)>k_cancastle)
    {
      castling_rights_type const save_castling_rights = being_solved.castling_rights;

      square const square_a = side==White ? square_a1 : square_a8;
      square const square_h = square_a+file_h;
      square const square_a_reflected = transformSquare(square_a,rot180);
      square const square_h_reflected = transformSquare(square_h,rot180);

      if (!is_square_empty(square_a_reflected)
          && game_array.board[square_a_reflected]!=being_solved.board[square_a_reflected])
        CLRCASTLINGFLAGMASK(side,q_castling);

      if (!is_square_empty(square_h_reflected)
          && game_array.board[square_h_reflected]!=being_solved.board[square_h_reflected])
        CLRCASTLINGFLAGMASK(side,k_castling);

      /* no need to inspect the king's square here:
       * * reflection of the king belongs to the other branch (i.e. walk_reflected!=Empty)
       * * the castling machinery tests again if the piece on the king's square walks like a king
       */

      pipe_move_generation_delegate(si);

      being_solved.castling_rights = save_castling_rights;
    }
    else
      pipe_move_generation_delegate(si);
  }
  else
    pipe_move_generation_different_walk_delegate(si,walk_reflected);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make sure that the observer has the expected walk - point_reflectionised or originally
 * @return true iff the observation is valid
 */
boolean point_reflection_enforce_observer_walk(slice_index si)
{
  boolean result;
  piece_walk_type const walk_original = get_walk_of_piece_on_square(move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure);
  square const reflected = transformSquare(move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure,rot180);
  piece_walk_type const walk_reflected = get_walk_of_piece_on_square(reflected);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure);
  TraceSquare(reflected);
  TraceWalk(walk_reflected);
  TraceEOL();

  if (walk_reflected==Empty && walk_original==observing_walk[nbply])
    result = pipe_validate_observation_recursive_delegate(si);
  else if (walk_reflected==observing_walk[nbply])
    result = pipe_validate_observation_recursive_delegate(si);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void substitute_enforce_point_reflectionised_walk(slice_index si,
                                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_substitute(si,alloc_pipe(STPointReflectionEnforceObserverWalk));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Annan Chess
 * @param si identifies root slice of solving machinery
 */
void point_reflection_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_for_piece_generation(si,nr_sides,STPointReflectionMovesForPieceGenerator);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STEnforceObserverWalk,
                                             &substitute_enforce_point_reflectionised_walk);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
