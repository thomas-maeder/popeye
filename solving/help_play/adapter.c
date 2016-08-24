#include "solving/help_play/adapter.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/pipe.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/branch.h"
#include "stipulation/binary.h"
#include "stipulation/constraint.h"
#include "stipulation/conditional_pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static void serve_as_intro_hook(slice_index si, stip_structure_traversal *st)
{
  spin_off_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->spun_off[si] = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin the intro slices off a nested help branch
 * @param adapter identifies adapter slice of the nested help branch
 * @param state address of structure holding state
 */
static void spin_off_intro(slice_index adapter, spin_off_state_type *state)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  assert(SLICE_TYPE(adapter)==STHelpAdapter);

  TraceStipulation(adapter);

  {
    slice_index const prototype = alloc_pipe(STEndOfIntro);
    slice_insertion_insert(adapter,&prototype,1);
  }

  {
    slice_index const next = SLICE_NEXT1(adapter);
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,state);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_pipe,
                                                   &pipe_spin_off_copy);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_branch,
                                                   &pipe_spin_off_copy);
    stip_structure_traversal_override_by_structure(&st,
                                                   slice_structure_fork,
                                                   &pipe_spin_off_copy);
    stip_structure_traversal_override_by_contextual(&st,
                                                    slice_contextual_binary,
                                                    &binary_make_root);
    stip_structure_traversal_override_by_contextual(&st,
                                                    slice_contextual_conditional_pipe,
                                                    &conditional_pipe_spin_off_copy);
    stip_structure_traversal_override_single(&st,
                                             STConstraintTester,
                                             &constraint_tester_make_root);
    stip_structure_traversal_override_single(&st,STEndOfIntro,&serve_as_intro_hook);
    stip_traverse_structure(next,&st);

    pipe_link(SLICE_PREV(adapter),next);
    link_to_branch(adapter,state->spun_off[next]);
    state->spun_off[adapter] = state->spun_off[next];
    SLICE_PREV(adapter) = no_slice;
  }

  TraceValue("%u",state->spun_off[adapter]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Wrap the slices representing the nested slices
 * @param adapter identifies solve adapter slice
 * @param st address of structure holding the traversal state
 */
void help_adapter_make_intro(slice_index adapter, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",adapter);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(adapter,st);

  if (st->level==structure_traversal_level_nested
      && SLICE_U(adapter).branch.length>slack_length)
  {
    spin_off_state_type * const state = st->param;
    spin_off_intro(adapter,state);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void help_adapter_solve(slice_index si)
{
  stip_length_type const full_length = SLICE_U(si).branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining==length_unspecified);

  solve_nr_remaining = full_length;
  pipe_solve_delegate(si);
  solve_nr_remaining = length_unspecified;

  if (solve_result==immobility_on_next_move)
    /* oops - unwanted stalemate or illegal move */;
  else
    pipe_this_move_solves_exactly_if(solve_result<=full_length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
