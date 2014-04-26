#include "conditions/facetoface.h"
#include "position/position.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "solving/castling.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
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

  if (TSTFLAG(spec[confronter_pos],advers(trait[nbply])))
    generate_moves_different_walk(slices[si].next1,
                                  get_walk_of_piece_on_square(confronter_pos));
  else
    generate_moves_delegate(slices[si].next1);

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
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);
  TraceSquare(pos_confronter);
  TraceEOL();

  if (TSTFLAG(spec[pos_confronter],advers(side_attacking)))
    walk = get_walk_of_piece_on_square(pos_confronter);
  else
    walk = get_walk_of_piece_on_square(sq_departure);

  if (walk==observing_walk[nbply])
    result = validate_observation_recursive(slices[si].next1);
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

  solving_instrument_move_generation(si,nr_sides,generator);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,&enforcer);
    stip_structure_traversal_override_single(&st,
                                             STEnforceObserverWalk,
                                             &substitute_enforce_confronted_walk);
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
  numecoup const save_current_move = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  generate_moves_for_possibly_confronted_piece(si,dir_left);
  generate_moves_for_possibly_confronted_piece(si,dir_right);

  remove_duplicate_moves_of_single_piece(save_current_move);

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
