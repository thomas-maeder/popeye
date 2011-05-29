#include "optimisations/end_of_branch_goal_immobile.h"
#include "pystip.h"
#include "pybrafrk.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "trace.h"

#include <assert.h>

/* Remember the goal imminent after a defense or attack move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void and_immobility(slice_index si, stip_moves_traversal *st)
{
  boolean * const goal_implies_immobility = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves(slices[si].u.binary.op1,st);
  if (*goal_implies_immobility)
  {
    *goal_implies_immobility = false;
    stip_traverse_moves(slices[si].u.binary.op2,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember the goal imminent after a defense or attack move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void or_immobility(slice_index si, stip_moves_traversal *st)
{
  boolean * const goal_implies_immobility = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves(slices[si].u.binary.op1,st);
  if (!*goal_implies_immobility)
    stip_traverse_moves(slices[si].u.binary.op2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether there are more moves after this branch
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void remember_immobility(slice_index si, stip_moves_traversal *st)
{
  boolean * const goal_implies_immobility = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);

  if (slices[si].u.goal_filter.applies_to_who==goal_applies_to_starter)
    *goal_implies_immobility = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Forget a remembered goal because it is to be reached by a move
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void forget_immobility(slice_index si, stip_moves_traversal *st)
{
  boolean * const goal_implies_immobility = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);
  *goal_implies_immobility = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember the goal to be reached
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static void optimise(slice_index si, stip_moves_traversal *st)
{
  boolean const * const goal_implies_immobility = st->param;
  slice_index const to_goal = slices[si].u.fork.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->remaining==0)
  {
    stip_traverse_moves_branch(to_goal,st);
    if (*goal_implies_immobility)
    {
      slice_index const substitute = alloc_branch_fork(STEndOfBranchGoalImmobile,
                                                       to_goal);
      pipe_substitute(si,substitute);
    }
  }
  else
    stip_traverse_moves_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const end_of_branch_goal_optimisers[] =
{
  { STAttackAdapter,             &forget_immobility   },
  { STDefenseAdapter,            &forget_immobility   },
  { STHelpAdapter,               &forget_immobility   },
  { STSeriesAdapter,             &forget_immobility   },
  { STEndOfBranchGoal,           &optimise            },
  { STGoalImmobileReachedTester, &remember_immobility },
  { STOr,                        &and_immobility      },
  { STAnd,                       &or_immobility       }
};

enum
{
  nr_end_of_branch_goal_optimisers = (sizeof end_of_branch_goal_optimisers
                                      / sizeof end_of_branch_goal_optimisers[0])
};

/* Optimise STEndOfBranchGoal slices
 * @param si identifies the root slice of the stipulation
 */
void stip_optimise_with_end_of_branch_goal_immobile(slice_index si)
{
  stip_moves_traversal st;
  boolean goal_implies_immobility = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_moves_traversal_init(&st,&goal_implies_immobility);
  stip_moves_traversal_override(&st,
                                end_of_branch_goal_optimisers,
                                nr_end_of_branch_goal_optimisers);
  stip_traverse_moves(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
