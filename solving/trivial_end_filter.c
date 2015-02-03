#include "solving/trivial_end_filter.h"
#include "solving/has_solution_type.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "solving/binary.h"
#include "solving/ply.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Allocate a STTrivialEndFilter slice.
 * @return index of allocated slice
 */
static slice_index alloc_trivial_end_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_testing_pipe(STTrivialEndFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
void trivial_end_filter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbply>ply_retro_move);
  binary_solve_if_then_else(si,solve_nr_remaining!=previous_move_has_solved);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_output_mode(slice_index si, stip_structure_traversal *st)
{
  output_mode * const mode = st->param;
  output_mode const save_mode = *mode;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *mode = SLICE_U(si).output_mode_selector.mode;
  stip_traverse_structure_children_pipe(si,st);
  *mode = save_mode;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void trivial_varation_filter_insert_self(slice_index si,
                                                stip_structure_traversal *st)
{
  output_mode const * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*mode==output_mode_tree
      && st->context==stip_traversal_context_attack)
  {
    slice_index const prototype = alloc_trivial_end_filter_slice();
    attack_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),&prototype,1,si);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void connect_to_tester(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  SLICE_TESTER(si) = alloc_proxy_slice();
  link_to_branch(SLICE_TESTER(si),SLICE_TESTER(SLICE_NEXT1(si)));
  SLICE_NEXT2(si) = SLICE_TESTER(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor trivial_varation_filter_inserters[] =
{
  { STOutputModeSelector, &remember_output_mode                  },
  { STEndOfBranchGoal,    &trivial_varation_filter_insert_self   },
  { STTrivialEndFilter ,  &connect_to_tester                     },
  { STRefutationsSolver,  &stip_traverse_structure_children_pipe }
};

enum
{
  nr_trivial_varation_filter_inserters
  = (sizeof trivial_varation_filter_inserters
     / sizeof trivial_varation_filter_inserters[0])
};

/* Instrument the solving machinery with trivial variation filters
 * @param si identifies the root of the solving machinery
 */
void solving_insert_trivial_variation_filters(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    trivial_varation_filter_inserters,
                                    nr_trivial_varation_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
