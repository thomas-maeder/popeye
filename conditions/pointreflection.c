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

static boolean reflected = false;

static castling_rights_type save_castling_rights;

typedef enum
{
  reflection_reflect,
  reflection_restore
} reflection_type;

static void reflect(slice_index si, reflection_type type)
{
  Side const side_moving = SLICE_STARTER(si);
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",type);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side_moving);
  TraceEOL();

  if (type==reflection_reflect)
  {
    square const square_a = side_moving==White ? square_a1 : square_a8;
    square const square_h = square_a+file_h;
    square const square_a_reflected = transformSquare(square_a,rot180);
    square const square_h_reflected = transformSquare(square_h,rot180);

    save_castling_rights = being_solved.castling_rights;

    if (!is_square_empty(square_a_reflected)
        && game_array.board[square_a_reflected]!=being_solved.board[square_a_reflected])
      CLRCASTLINGFLAGMASK(side_moving,q_castling);

    if (!is_square_empty(square_h_reflected)
        && game_array.board[square_h_reflected]!=being_solved.board[square_h_reflected])
      CLRCASTLINGFLAGMASK(side_moving,k_castling);
  }
  else
    being_solved.castling_rights = save_castling_rights;

  for (bnp = boardnum; *bnp/onerow-square_a1/onerow<nr_rows_on_board/2; ++bnp)
  {
    Flags const spec = being_solved.spec[*bnp];
    piece_walk_type const walk_original = get_walk_of_piece_on_square(*bnp);
    square const reflected = transformSquare(*bnp,rot180);
    piece_walk_type const walk_reflected = get_walk_of_piece_on_square(reflected);
    Flags const spec_reflected = being_solved.spec[reflected];

    if (walk_original!=Empty && walk_reflected!=Empty)
    {
      TraceSquare(*bnp);
      TraceWalk(walk_original);
      TraceWalk(walk_reflected);
      TraceEOL();

      occupy_square(*bnp,walk_reflected,spec);
      occupy_square(reflected,walk_original,spec_reflected);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Temporarily change walks of oppenent's pieces before move generation
 * @param identifies generator slice
 */
void point_reflection_temporarily_change_walks(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(!reflected);
  reflect(si,reflection_reflect);
  reflected = true;

  pipe_solve_delegate(si);

  assert(reflected);
  reflect(si,reflection_restore);
  reflected = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Restore walks of oppenent's pieces
 * @param identifies generator slice
 */
void point_reflection_restore_walks(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(reflected);
  reflect(si,reflection_restore);
  reflected = false;

  pipe_solve_delegate(si);

  assert(!reflected);
  reflect(si,reflection_reflect);
  reflected = true;

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

  solving_instrument_move_generation_simple(si,STPointReflectionTemporaryWalkChanger);
  solving_instrument_move_generation_simple(si,STPointReflectionWalkRestorer);

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
