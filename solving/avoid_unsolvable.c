#include "solving/avoid_unsolvable.h"
#include "solving/binary.h"
#include "stipulation/pipe.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/binary.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* maximum number of half-moves that we know have no solution
 */
stip_length_type max_unsolvable;

/* Allocate a STEndOfBranch slice.
 * @param to_goal identifies slice leading towards goal
 * @return index of allocated slice
 */
slice_index alloc_avoid_unsolvable_slice(slice_index proxy_op1,
                                         slice_index proxy_op2)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_op1);
  TraceFunctionParam("%u",proxy_op2);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STAvoidUnsolvable,proxy_op1,proxy_op2);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_reset_unsolvable_attack(slice_index si,
                                           stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_reset_unsolvable_slice();
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_reset_unsolvable_defense(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_defense)
  {
    slice_index const prototype = alloc_reset_unsolvable_slice();
    defense_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_reset_unsolvable_help(slice_index si,
                                         stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_reset_unsolvable_slice();
    slice_insertion_insert(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_avoid_unsolvable(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_attack
      || st->context==stip_traversal_context_defense)
  {
    slice_index const proxy1 = alloc_proxy_slice();
    slice_index const proxy2 = alloc_proxy_slice();
    slice_index const fork = alloc_avoid_unsolvable_slice(proxy1,proxy2);
    pipe_link(SLICE_PREV(si),fork);
    pipe_append(si,proxy2);
    pipe_link(proxy1,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const avoid_unusable_inserters[] =
{
  { STAttackAdapter,         &insert_reset_unsolvable_attack  },
  { STDefenseAdapter,        &insert_reset_unsolvable_attack  },
  { STHelpAdapter,           &insert_reset_unsolvable_help    },
  { STAttackPlayed,          &insert_reset_unsolvable_defense },
  { STDefensePlayed,         &insert_reset_unsolvable_defense },
  { STEndOfBranchGoal,       &insert_avoid_unsolvable         },
  { STCounterMateFilter,     &insert_avoid_unsolvable         },
  { STDoubleMateFilter,      &insert_avoid_unsolvable         },
  { STPrerequisiteOptimiser, &insert_avoid_unsolvable         }
};

enum
{
  nr_avoid_unusable_inserters = (sizeof avoid_unusable_inserters
                                 / sizeof avoid_unusable_inserters[0])
};

/* Instrument the solving machinery with the necessary STAvoidUnusable slices
 * @param root_slice identifies root slice of the solving machinery
 */
void solving_insert_avoid_unsolvable_forks(slice_index root_slice)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_end_of_branch,
                                                  &insert_avoid_unsolvable);
  stip_structure_traversal_override(&st,
                                    avoid_unusable_inserters,
                                    nr_avoid_unusable_inserters);
  stip_traverse_structure(root_slice,&st);

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
void avoid_unsolvable_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining>=previous_move_has_solved);

  TraceValue("%u",max_unsolvable);
  TraceEOL();
  binary_solve_if_then_else(si,max_unsolvable>slack_length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STResetUnsolvable slice
 * @return allocated slice
 */
slice_index alloc_reset_unsolvable_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STResetUnsolvable);

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
void reset_unsolvable_solve(slice_index si)
{
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  max_unsolvable = slack_length;
  TraceValue("->%u",max_unsolvable);
  TraceEOL();

  pipe_solve_delegate(si);

  max_unsolvable = save_max_unsolvable;
  TraceValue("->%u",max_unsolvable);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STLearnUnsolvable slice
 * @return allocated slice
 */
slice_index alloc_learn_unsolvable_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STLearnUnsolvable);

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
void learn_unsolvable_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  if (solve_result>MOVE_HAS_SOLVED_LENGTH())
  {
    max_unsolvable = MOVE_HAS_SOLVED_LENGTH();
    TraceValue("->%u",max_unsolvable);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
