#include "conditions/circe/cage.h"
#include "conditions/circe/capture_fork.h"
#include "conditions/circe/rebirth_handler.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/temporary_hacks.h"
#include "solving/single_piece_move_generator.h"
#include "solving/post_move_iteration.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static post_move_iteration_id_type prev_post_move_iteration_id_no_cage[maxply+1];

static boolean cage_found_for_current_capture[maxply+1];
static boolean no_cage_for_current_capture[maxply+1];

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_cage_no_cage_fork_solve(slice_index si,
                                                stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]==prev_post_move_iteration_id_no_cage[nbply])
  {
    if (no_cage_for_current_capture[nbply])
      result = solve(slices[si].next2,n);
    else
    {
      result = solve(slices[si].next1,n);

      if (!post_move_iteration_locked[nbply]
          && current_circe_rebirth_square[nbply]==initsquare
          && !cage_found_for_current_capture[nbply])
      {
        no_cage_for_current_capture[nbply] = true;
        lock_post_move_iterations();
      }
    }
  }
  else
  {
    cage_found_for_current_capture[nbply] = false;
    no_cage_for_current_capture[nbply] = false;
    result = solve(slices[si].next1,n);
  }

  prev_post_move_iteration_id_no_cage[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean find_non_capturing_move(square sq_departure, Side moving_side)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceEnumerator(Side,moving_side,"");
  TraceFunctionParamListEnd();

  init_single_piece_move_generator(sq_departure);
  result = solve(slices[temporary_hack_cagecirce_noncapture_finder[moving_side]].next2,length_unspecified)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_cage_cage_tester_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (find_non_capturing_move(current_circe_rebirth_square[nbply],
                              advers(slices[si].starter)))
    result = slack_length-2;
  else
  {
    cage_found_for_current_capture[nbply] = true;
    result = solve(slices[si].next1,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STSuperCirceRebirthHandler),
        alloc_pipe(STCirceCageCageTester)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_cage(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STCageCirceNonCapturingMoveFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  stip_insert_circe_capture_forks(si);

  if (!CondFlag[immuncage])
    stip_insert_rebirth_avoider(si,STCirceCageNoCageFork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
