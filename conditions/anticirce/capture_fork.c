#include "conditions/anticirce/capture_fork.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/circe/circe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/fork.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type anticirce_capture_fork_solve(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_no_piece_removal)
    result = solve(slices[si].next2,n);
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index const prototype = alloc_pipe(STLandingAfterAnticirceRebirth);
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

static void instrument(slice_index si, stip_structure_traversal *st)
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

static void insert_rebirth_avoider(slice_index si, slice_type type)
{
  stip_structure_traversal st;
  insertion_state_type state = { no_slice, type };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STAnticirceConsideringRebirth,
                                           &instrument);
  stip_structure_traversal_override_single(&st,
                                           STLandingAfterAnticirceRebirth,
                                           &remember_landing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_anticirce_capture_forks(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  insert_rebirth_avoider(si,STAnticirceCaptureFork);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
