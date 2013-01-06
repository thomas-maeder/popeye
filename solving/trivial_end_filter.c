#include "solving/trivial_end_filter.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "debugging/trace.h"

#include <assert.h>

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

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type trivial_end_filter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (nbply==nil_ply || n==slack_length)
    result = solve(slices[si].next1,n);
  else
    /* variation is trivial - just determine the result */
    result = solve(slices[si].next2,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void remember_output_mode(slice_index si, stip_structure_traversal *st)
{
  output_mode * const mode = st->param;
  output_mode const save_mode = *mode;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *mode = slices[si].u.output_mode_selector.mode;
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
    attack_branch_insert_slices_behind_proxy(slices[si].next2,&prototype,1,si);
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

  slices[si].tester = alloc_proxy_slice();
  link_to_branch(slices[si].tester,slices[slices[si].next1].tester);
  slices[si].next2 = slices[si].tester;

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

/* Instrument a stipulation with trivial variation filters
 * @param si identifies the entry slice of the stipulation to be instrumented
 */
void stip_insert_trivial_variation_filters(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    trivial_varation_filter_inserters,
                                    nr_trivial_varation_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
