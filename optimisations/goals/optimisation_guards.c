#include "optimisations/goals/optimisation_guards.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/goals/prerequisite_guards.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/goals/enpassant/filter.h"
#include "optimisations/goals/castling/filter.h"
#include "optimisations/goals/castling/filter.h"
#include "optimisations/goals/castling/filter.h"

#include "debugging/trace.h"

#include "debugging/assert.h"

/* Insert goal optimisation guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_optimisation_guards_castling(slice_index si,
                                              stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_moves_children(si,st);
  state->imminent_goals[goal_castling] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert goal optimisation guards
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
static
void insert_goal_optimisation_guards_enpassant(slice_index si,
                                               stip_moves_traversal *st)
{
  prerequisite_guards_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("0x%x",state);
  TraceEOL();
  stip_traverse_moves_children(si,st);
  state->imminent_goals[goal_ep] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static moves_traversers_visitors const optimisation_guard_inserters[] =
{
  { STGoalCastlingReachedTester,  &insert_goal_optimisation_guards_castling  },
  { STGoalEnpassantReachedTester, &insert_goal_optimisation_guards_enpassant }
};

enum
{
  nr_optimisation_guard_inserters
  = (sizeof optimisation_guard_inserters
     / sizeof optimisation_guard_inserters[0])
};

/* Initialise a structure traversal for the insertion of optimising goal
 * prerequisite testers.
 * @param st to be initialised
 */
void
init_goal_prerequisite_traversal_with_optimisations(stip_moves_traversal *st)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_moves_traversal_override(st,
                                optimisation_guard_inserters,
                                nr_optimisation_guard_inserters);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
/* Determine whether moves that are supposed to reach a particular goal are
 * optimisable
 * @param goal goal to be reached
 * @return true iff moves supposed to reach goal are optimisable
 */
boolean is_goal_reaching_move_optimisable(goal_type goal)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_ep:
    case goal_castling:
      result = true;
      break;

    default:
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a goal optimisation filter slice into a battle branch
 * @param si identifies entry slice into battle branch
 * @param goal goal to provide optimisation for
 * @param context are we instrumenting for a defense or an attack?
 */
void insert_goal_optimisation_battle_filter(slice_index si,
                                            goal_type goal,
                                            stip_traversal_context_type context)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParam("%u",context);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_ep:
    {
      slice_index const prototype = alloc_enpassant_filter_slice();
      if (context==stip_traversal_context_attack)
        attack_branch_insert_slices(si,&prototype,1);
      else
        defense_branch_insert_slices(si,&prototype,1);
      break;
    }

    case goal_castling:
    {
      slice_index const prototype = alloc_castling_filter_slice();
      if (context==stip_traversal_context_attack)
        attack_branch_insert_slices(si,&prototype,1);
      else
        defense_branch_insert_slices(si,&prototype,1);
      break;
    }

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert a goal optimisation filter slice into a help branch
 * @param si identifies entry slice into battle branch
 * @param goal goal to provide optimisation for
 */
void insert_goal_optimisation_help_filter(slice_index si, goal_type goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_ep:
      pipe_append(si,alloc_enpassant_filter_slice());
      break;

    case goal_castling:
      pipe_append(si,alloc_castling_filter_slice());
      break;

    default:
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
