#include "stipulation/battle_play/continuation.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/defense_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STContinuationSolver defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_continuation_solver_slice(stip_length_type length,
                                            stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STContinuationSolver,length,min_length);

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
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
continuation_solver_defend_in_n(slice_index si,
                                stip_length_type n,
                                stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;
  unsigned int const max_nr_allowed_refutations = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = defense_can_defend_in_n(next,
                                   n,n_max_unsolvable,
                                   max_nr_allowed_refutations);
  if (result<=n)
  {
    stip_length_type const
        defend_result = defense_defend_in_n(next,result,n_max_unsolvable);
    assert(defend_result==result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
continuation_solver_can_defend_in_n(slice_index si,
                                    stip_length_type n,
                                    stip_length_type n_max_unsolvable,
                                    unsigned int max_nr_refutations)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  result = defense_can_defend_in_n(next,n,n_max_unsolvable,max_nr_refutations);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

typedef enum
{
  continuation_handler_not_needed,
  continuation_handler_needed,
  continuation_handler_inserted
} continuation_handler_insertion_state;

/* Append a continuation solver if none has been inserted before
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void continuation_solver_prepend(slice_index si,
                                        stip_structure_traversal *st)
{
  continuation_handler_insertion_state * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  pipe_append(slices[si].prev,
              alloc_continuation_solver_slice(length,min_length));
  *state = continuation_handler_inserted;
  TraceValue("->%u\n",*state);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inform predecessors that a continuation solver is needed
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void continuation_solver_mark_need(slice_index si,
                                          stip_structure_traversal *st)
{
  continuation_handler_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  *state = continuation_handler_needed;
  TraceValue("->%u\n",*state);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a continuation solver if none has been inserted before
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static
void continuation_solver_insert_defender_filter(slice_index si,
                                                stip_structure_traversal *st)
{
  continuation_handler_insertion_state * const state = st->param;
  continuation_handler_insertion_state const save_state = *state;
  slice_index const proxy_to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(slices[proxy_to_goal].type==STProxy);

  stip_traverse_structure(proxy_to_goal,st);

  TraceValue("%u\n",*state);
  if (*state==continuation_handler_needed)
    pipe_append(proxy_to_goal,
                alloc_continuation_solver_slice(slack_length_battle,
                                                slack_length_battle));
  *state = save_state;
  stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors continuation_handler_inserters[] =
{
  { STReadyForDefense,                  &continuation_solver_prepend        },
  { STGoalReachedTester,                &continuation_solver_mark_need      },
  { STHelpRoot,                         &stip_structure_visitor_noop        },
  { STSeriesRoot,                       &stip_structure_visitor_noop        },
  { STDirectDefenderFilter,     &continuation_solver_insert_defender_filter }
};

enum
{
  nr_continuation_handler_inserters =
  (sizeof continuation_handler_inserters
   / sizeof continuation_handler_inserters[0])
};

/* Instrument the stipulation representation so that it can deal with
 * continuations
 * @param si identifies slice where to start
 */
void stip_insert_continuation_handlers(slice_index si)
{
  stip_structure_traversal st;
  continuation_handler_insertion_state state = continuation_handler_not_needed;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    continuation_handler_inserters,
                                    nr_continuation_handler_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
