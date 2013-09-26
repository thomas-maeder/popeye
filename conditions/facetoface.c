#include "conditions/facetoface.h"
#include "position/position.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "solving/castling.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

static void generate_moves_for_possibly_confronted_piece(slice_index si,
                                                         PieNam p,
                                                         numvec dir_confronter)
{
  square const confronter_pos = curr_generation->departure+dir_confronter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (TSTFLAG(spec[confronter_pos],advers(trait[nbply])))
  {
    PieNam const confronter = get_walk_of_piece_on_square(confronter_pos);
    generate_moves_for_piece(slices[si].next1,confronter);
  }
  else
    generate_moves_for_piece(slices[si].next1,p);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean enforce_possibly_confronted_observer_walk(slice_index si,
                                                         numvec dir_confronter)
{
  square const sq_departure = move_generation_stack[current_move[nbply]-1].departure;
  Side const side_attacking = trait[nbply];
  square const pos_confronter = sq_departure+dir_confronter;
  PieNam walk;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);
  TraceSquare(pos_confronter);
  TraceText("\n");

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
 * @param p walk to be used for generating
 */
void facetoface_generate_moves_for_piece(slice_index si, PieNam p)
{
  numvec const dir_confronter = trait[nbply]==White ? dir_up : dir_down;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  generate_moves_for_possibly_confronted_piece(si,p,dir_confronter)

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

boolean find_square_observer_tracking_back_from_target_unoptimised(slice_index si,
                                                                   evalfunction_t *evaluate)
{
  {
    PieNam pcheck;
    for (pcheck = King; pcheck<=Bishop; ++pcheck)
    {
      observing_walk[nbply] = pcheck;
      if ((*checkfunctions[pcheck])(evaluate))
        return true;
    }
  }

  {
    PieNam const *pcheck;
    for (pcheck = checkpieces; *pcheck; ++pcheck)
    {
      observing_walk[nbply] = *pcheck;
      if ((*checkfunctions[*pcheck])(evaluate))
        return true;
    }
  }

  return is_square_observed_recursive(slices[si].next1,evaluate);
}

typedef struct
{
    slice_type generator;
    slice_type enforcer;
} initialisation_type;

static void substitute_enforce_confronted_walk(slice_index si,
                                                     stip_structure_traversal *st)
{
  initialisation_type const * const init = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_substitute(si,alloc_pipe(init->enforcer));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void substitute_find_square_observed(slice_index si,
                                            stip_structure_traversal *st)
{
  initialisation_type const * const init = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_substitute(si,alloc_pipe(init->generator));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_find_square_observed(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Face-to-face Chess
 * @param si identifies root slice of solving machinery
 */
static void initialise_solving(slice_index si, slice_type generator, slice_type enforcer)
{
  initialisation_type init = { generator, enforcer };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  solving_instrument_move_generation(si,nr_sides,generator);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,&init);
    stip_structure_traversal_override_single(&st,
                                             STEnforceObserverWalk,
                                             &substitute_enforce_confronted_walk);
    stip_structure_traversal_override_single(&st,
                                             STFindSquareObserverTrackingBackKing,
                                             &remove_find_square_observed);
    stip_structure_traversal_override_single(&st,
                                             STFindSquareObserverTrackingBackFairy,
                                             &remove_find_square_observed);
    stip_structure_traversal_override_single(&st,
                                             STFindSquareObserverTrackingBack,
                                             &substitute_find_square_observed);
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
 * @param p walk to be used for generating
 */
void backtoback_generate_moves_for_piece(slice_index si, PieNam p)
{
  numvec const dir_confronter = trait[nbply]==White ? dir_down : dir_up;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  generate_moves_for_possibly_confronted_piece(si,p,dir_confronter)

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
 * @param p walk to be used for generating
 */
void cheektocheek_generate_moves_for_piece(slice_index si, PieNam p)
{
  numecoup const save_current_move = current_move[nbply]-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TracePiece(p);
  TraceFunctionParamListEnd();

  generate_moves_for_possibly_confronted_piece(si,p,dir_left);
  generate_moves_for_possibly_confronted_piece(si,p,dir_right);

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
