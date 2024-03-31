#include "conditions/facetoface.h"
#include "position/position.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "solving/castling.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

static void generate_moves_for_possibly_confronted_piece(slice_index si,
                                                         numvec dir_confronter)
{
  square const confronter_pos = curr_generation->departure+dir_confronter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[confronter_pos],advers(trait[nbply])))
    pipe_move_generation_different_walk_delegate(si,get_walk_of_piece_on_square(confronter_pos));
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean enforce_possibly_confronted_observer_walk(slice_index si,
                                                         numvec dir_confronter)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  Side const side_attacking = trait[nbply];
  square const pos_confronter = sq_departure+dir_confronter;
  piece_walk_type walk;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%d",dir_confronter);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);
  TraceSquare(pos_confronter);
  TraceEOL();

  if (TSTFLAG(being_solved.spec[pos_confronter],advers(side_attacking)))
    walk = get_walk_of_piece_on_square(pos_confronter);
  else
    walk = get_walk_of_piece_on_square(sq_departure);

  TraceWalk(walk);
  TraceWalk(observing_walk[nbply]);
  TraceEOL();

  if (walk==observing_walk[nbply])
    result = pipe_validate_observation_recursive_delegate(si);
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void facetoface_generate_moves_for_piece(slice_index si)
{
  numvec const dir_confronter = trait[nbply]==White ? dir_up : dir_down;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  generate_moves_for_possibly_confronted_piece(si,dir_confronter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make sure that the observer has the expected walk - confronted or originally
 * @return true iff the observation is valid
 */
boolean facetoface_enforce_observer_walk(slice_index si)
{
  numvec const dir_confronter = trait[nbply]==White ? dir_up : dir_down;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = enforce_possibly_confronted_observer_walk(si,dir_confronter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void substitute_enforce_confronted_walk(slice_index si,
                                               stip_structure_traversal *st)
{
  slice_type const * const enforcer = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_substitute(si,alloc_pipe(*enforcer));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Face-to-face Chess
 * @param si identifies root slice of solving machinery
 */
static void initialise_solving(slice_index si,
                               slice_type generator,
                               slice_type enforcer)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_for_piece_generation(si,nr_sides,generator);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,&enforcer);
    stip_structure_traversal_override_single(&st,
                                             STEnforceObserverWalk,
                                             &substitute_enforce_confronted_walk);
    stip_structure_traversal_override_single(&st,
                                             STValidatingObserver,
                                             &stip_structure_visitor_noop);
    stip_structure_traversal_override_single(&st,
                                             STValidatingObservationGeometry,
                                             &stip_structure_visitor_noop);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Face-to-face Chess
 * @param si identifies root slice of solving machinery
 */
void facetoface_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  initialise_solving(si,
                     STFaceToFaceMovesForPieceGenerator,
                     STFaceToFaceEnforceObserverWalk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void backtoback_generate_moves_for_piece(slice_index si)
{
  numvec const dir_confronter = trait[nbply]==White ? dir_down : dir_up;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  generate_moves_for_possibly_confronted_piece(si,dir_confronter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make sure that the observer has the expected walk - confronted or originally
 * @return true iff the observation is valid
 */
boolean backtoback_enforce_observer_walk(slice_index si)
{
  numvec const dir_confronter = trait[nbply]==White ? dir_down : dir_up;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = enforce_possibly_confronted_observer_walk(si,dir_confronter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise the solving machinery with Back-to-back Chess
 * @param si identifies root slice of solving machinery
 */
void backtoback_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  initialise_solving(si,
                     STBackToBackMovesForPieceGenerator,
                     STBackToBackEnforceObserverWalk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void cheektocheek_generate_moves_for_piece(slice_index si)
{
  boolean is_cheeked = false;
  square const pos_left = curr_generation->departure+dir_left;
  square const pos_right = curr_generation->departure+dir_right;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[pos_left],advers(trait[nbply])))
  {
    is_cheeked = true;
    pipe_move_generation_different_walk_delegate(si,get_walk_of_piece_on_square(pos_left));
  }

  if (TSTFLAG(being_solved.spec[pos_right],advers(trait[nbply])))
  {
    is_cheeked = true;
    pipe_move_generation_different_walk_delegate(si,get_walk_of_piece_on_square(pos_right));
  }

  if (!is_cheeked)
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make sure that the observer has the expected walk - confronted or originally
 * @return true iff the observation is valid
 */
boolean cheektocheek_enforce_observer_walk(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (enforce_possibly_confronted_observer_walk(si,dir_left)
            || enforce_possibly_confronted_observer_walk(si,dir_right));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise the solving machinery with Cheek-to-cheek Chess
 * @param si identifies root slice of solving machinery
 */
void cheektocheek_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  initialise_solving(si,
                     STCheekToCheekMovesForPieceGenerator,
                     STCheekToCheekEnforceObserverWalk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
