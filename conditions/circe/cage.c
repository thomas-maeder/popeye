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

static post_move_iteration_id_type prev_post_move_iteration_id_rebirth[maxply+1];
static post_move_iteration_id_type prev_post_move_iteration_id_no_cage[maxply+1];

static boolean cage_found_for_current_capture[maxply+1];
static boolean no_cage_for_current_capture[maxply+1];

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_cage_no_cage_fork_attack(slice_index si,
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
      result = attack(slices[si].next2,n);
    else
    {
      result = attack(slices[si].next1,n);

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
    result = attack(slices[si].next1,n);
  }

  prev_post_move_iteration_id_no_cage[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_cage_no_cage_fork_defend(slice_index si,
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
      result = defend(slices[si].next2,n);
    else
    {
      result = attack(slices[si].next1,n);

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
    result = defend(slices[si].next1,n);
  }

  prev_post_move_iteration_id_no_cage[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static square advance_rebirth_square(square current)
{
  TraceFunctionEntry(__func__);
  TraceSquare(current);
  TraceFunctionParamListEnd();

  do
  {
    ++current;
  } while (current<=square_h8 && e[current]!=vide);

  TraceSquare(current);TraceText("\n");

  TraceFunctionExit(__func__);
  TraceSquare(current);
  TraceFunctionResultEnd();
  return current;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_cage_rebirth_handler_attack(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  square sq_rebirth;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]==prev_post_move_iteration_id_rebirth[nbply])
    sq_rebirth = current_circe_rebirth_square[nbply];
  else
    sq_rebirth = advance_rebirth_square(square_a1-1);

  if (sq_rebirth<=square_h8)
  {
    /* rebirth on current cage */
    circe_do_rebirth(sq_rebirth,pprise[nbply],pprispec[nbply]);
    result = attack(slices[si].next1,n);
    circe_undo_rebirth(sq_rebirth);

    if (!post_move_iteration_locked[nbply])
    {
      current_circe_rebirth_square[nbply] = advance_rebirth_square(sq_rebirth);
      if (current_circe_rebirth_square[nbply]<=square_h8)
        lock_post_move_iterations();
      else
        current_circe_rebirth_square[nbply] = initsquare;
    }

    prev_post_move_iteration_id_rebirth[nbply] = post_move_iteration_id[nbply];
  }
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type circe_cage_rebirth_handler_defend(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  square sq_rebirth;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]==prev_post_move_iteration_id_rebirth[nbply])
    sq_rebirth = current_circe_rebirth_square[nbply];
  else
    sq_rebirth = advance_rebirth_square(square_a1-1);

  if (sq_rebirth<=square_h8)
  {
    /* rebirth on current cage */
    circe_do_rebirth(sq_rebirth,pprise[nbply],pprispec[nbply]);
    result = defend(slices[si].next1,n);
    circe_undo_rebirth(sq_rebirth);

    if (!post_move_iteration_locked[nbply])
    {
      current_circe_rebirth_square[nbply] = advance_rebirth_square(sq_rebirth);
      if (current_circe_rebirth_square[nbply]<=square_h8)
        lock_post_move_iterations();
      else
        current_circe_rebirth_square[nbply] = initsquare;
    }

    prev_post_move_iteration_id_rebirth[nbply] = post_move_iteration_id[nbply];
  }
  else
    result = n+2;

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

  init_single_piece_move_generator(sq_departure,e[sq_departure]);
  result = attack(slices[temporary_hack_cagecirce_noncapture_finder[moving_side]].next2,length_unspecified)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_cage_cage_tester_attack(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  square const sq_cage = current_circe_rebirth_square[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_cage==initsquare)
    result = attack(slices[si].next1,n);
  else if (find_non_capturing_move(sq_cage,advers(slices[si].starter)))
    result = n+2;
  else
  {
    cage_found_for_current_capture[nbply] = true;
    result = attack(slices[si].next1,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type circe_cage_cage_tester_defend(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;
  square const sq_cage = current_circe_rebirth_square[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (sq_cage==initsquare)
    result = defend(slices[si].next1,n);
  else if (find_non_capturing_move(sq_cage,advers(slices[si].starter)))
    result = slack_length-1;
  else
  {
    cage_found_for_current_capture[nbply] = true;
    result = defend(slices[si].next1,n);
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
        alloc_pipe(STCirceCageRebirthHandler),
        alloc_pipe(STCirceRebirthPromoter),
        alloc_pipe(STCirceCageCageTester)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static void instrument_move_replay(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STCirceCageRebirthHandler),
        alloc_pipe(STCirceRebirthPromoter)
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
                                           STReplayingMoves,
                                           &instrument_move_replay);
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
