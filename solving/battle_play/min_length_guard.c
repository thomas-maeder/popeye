#include "solving/battle_play/min_length_guard.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/branch.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "stipulation/battle_play/branch.h"
#include "solving/battle_play/min_length_optimiser.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <stdlib.h>

/* Allocate a STMinLengthGuard slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static slice_index alloc_min_length_guard(stip_length_type length,
                                          stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STMinLengthGuard,length,min_length);

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
void min_length_guard_solve(slice_index si)
{
  slice_index const length = SLICE_U(si).branch.length;
  slice_index const min_length = SLICE_U(si).branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (solve_result>previous_move_is_illegal
      && solve_nr_remaining+min_length>length+solve_result)
    /* the defender has refuted by reaching the goal too early */
    solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    stip_length_type defense_length;
    stip_length_type defense_min_length;
} insertion_state_type;

static void remember_defense_length(slice_index si,
                                    stip_structure_traversal *st,
                                    int delta)
{
  insertion_state_type * const state = st->param;
  insertion_state_type const save_state = *state;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO there is an integer underflow in some structured stipulations
  // for some reason, it doesn't seem to harm, but we should
  // eventually get rid of it and activate these assertions:

//  // TODO does this overflow work on all implementations?
//  assert(abs(delta)<=1);
//  assert(SLICE_U(si).branch.length>0 || delta>=0);
  state->defense_length = SLICE_U(si).branch.length+(unsigned int)delta;

//  // TODO does this overflow work on all implementations?
//  assert(abs(delta)<=1);
//  assert(SLICE_U(si).branch.min_length>0 || delta>=0);
  state->defense_min_length = SLICE_U(si).branch.min_length+(unsigned int)delta;

  stip_traverse_structure_children_pipe(si,st);

  *state = save_state;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_defense_adapter_length(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  remember_defense_length(si,st,0);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_intro_min_length(slice_index si,
                                    stip_structure_traversal *st)
{
  stip_length_type const length = SLICE_U(si).branch.length;
  stip_length_type const min_length = SLICE_U(si).branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  remember_defense_length(si,st,-1);

  if (min_length>slack_length+1)
  {
    slice_index const prototype = alloc_min_length_optimiser_slice(length,
                                                                   min_length);
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_nested_min_length(slice_index si,
                                     stip_structure_traversal *st)
{
  insertion_state_type const * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_defense
      && state->defense_min_length>slack_length+1)
  {
    slice_index const prototype = alloc_min_length_optimiser_slice(state->defense_length-1,
                                                                   state->defense_min_length-1);
    defense_branch_insert_slices(si,&prototype,1);

    if (st->activity==stip_traversal_activity_testing)
    {
      slice_index const prototype = alloc_min_length_guard(state->defense_length-1,
                                                           state->defense_min_length-1);
      defense_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation with minimum length functionality
 * @param si identifies the root slice of the stipulation
 */
void solving_insert_min_length(slice_index si)
{
  stip_structure_traversal st;
  insertion_state_type state = { slack_length, slack_length };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_single(&st,
                                           STAttackAdapter,
                                           &insert_intro_min_length);
  stip_structure_traversal_override_single(&st,
                                           STDefenseAdapter,
                                           &remember_defense_adapter_length);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &insert_nested_min_length);
  stip_structure_traversal_override_single(&st,
                                           STEndOfBranchForced,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STEndOfBranchTester,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STConstraintTester,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STConstraintSolver,
                                           &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
