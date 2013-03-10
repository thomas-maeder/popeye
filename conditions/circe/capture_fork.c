#include "conditions/circe/capture_fork.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/fork.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "conditions/circe/circe.h"
#include "debugging/trace.h"

#include <assert.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_capture_fork_solve(slice_index si,
                                           stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (pprise[nbply]==vide)
  {
    current_circe_rebirth_square[nbply] = initsquare;
    result = solve(slices[si].next2,n);
  }
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index const prototype = alloc_pipe(STLandingAfterCirceRebirthHandler);
  branch_insert_slices_contextual(si,st->context,&prototype,1);
}

typedef struct
{
    slice_index landing;
    slice_type type;
} insertion_state_type;

static void insert_fork(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type const * const state = st->param;
  slice_index const proxy = alloc_proxy_slice();
  slice_index const prototype = alloc_fork_slice(state->type,proxy);

  assert(state->landing!=no_slice);
  link_to_branch(proxy,state->landing);

  branch_insert_slices_contextual(si,st->context,&prototype,1);
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;
  slice_index const save_landing = state->landing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->landing = no_slice;
  insert_landing(si,st);

  stip_traverse_structure_children(si,st);

  insert_fork(si,st);
  state->landing = save_landing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_landing(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(state->landing==no_slice);
  stip_traverse_structure_children_pipe(si,st);
  assert(state->landing==no_slice);
  state->landing = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation with a type of "Circe rebirth avoiders" (i.e.
 * slices that may detour around Circe rebirth under a certain condition;
 * STCaptureFork is an example).
 * @param si identifies root slice of stipulation
 * @param type tye of Circe rebirth avoider
 */
void stip_insert_rebirth_avoider(slice_index si, slice_type type)
{
  stip_structure_traversal st;
  insertion_state_type state = { no_slice, type };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STKingCaptureLegalityTester,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STLandingAfterCirceRebirthHandler,
                                           &remember_landing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_capture_forks(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_insert_rebirth_avoider(si,STCirceCaptureFork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
