#include "options/maxsolutions/maxsolutions.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "options/maxsolutions/initialiser.h"
#include "options/maxsolutions/guard.h"
#include "options/maxsolutions/guard.h"
#include "options/maxsolutions/guard.h"
#include "options/maxsolutions/guard.h"
#include "debugging/trace.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

/* number of solutions found in the current phase */
static unsigned int nr_solutions_found_in_phase;

/* maximum number of allowed solutions found in the current phase */
static unsigned int max_nr_solutions_per_phase;

/* has the maximum number of allowed solutions been reached? */
static boolean allowed_nr_solutions_reached;

/* Reset the value of the maxsolutions option
 */
void reset_max_solutions(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  max_nr_solutions_per_phase = UINT_MAX;
  allowed_nr_solutions_reached = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Read the value of the maxsolutions option
 * @return true iff the value could be successfully read
 */
boolean read_max_solutions(char const *token)
{
  boolean result;
  char *end;
  unsigned long const value = strtoul(token,&end,10);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",token);
  TraceFunctionParamListEnd();

  if (*end==0 && value<UINT_MAX)
  {
    max_nr_solutions_per_phase = (unsigned int)value;
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Have we found the maxmimum allowed number of solutions since the
 * last invokation of reset_max_solutions()/read_max_solutions()?
 * @true iff we have found the maxmimum allowed number of solutions
 */
boolean max_solutions_reached(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",allowed_nr_solutions_reached);
  TraceFunctionResultEnd();
  return allowed_nr_solutions_reached;
}

/* Reset the number of found solutions
 */
void reset_nr_found_solutions_per_phase(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_solutions_found_in_phase = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Increase the number of found solutions by 1
 */
void increase_nr_found_solutions(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++nr_solutions_found_in_phase;
  TraceValue("->%u\n",nr_solutions_found_in_phase);

  if (max_nr_solutions_found_in_phase())
    allowed_nr_solutions_reached = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Have we found the maximum allowed number of solutions since the
 * last invokation of reset_nr_found_solutions()?
 * @return true iff the allowed maximum number of solutions have been found
 */
boolean max_nr_solutions_found_in_phase(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = nr_solutions_found_in_phase>=max_nr_solutions_per_phase;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Insert STMaxSolutionsGuard slices
 */
static void insert_maxsolutions_help_filter(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_maxsolutions_guard_slice();
    help_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert STMaxSolutionsGuard slices
 */
static
void insert_maxsolutions_attack_adapter(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] =
    {
      alloc_maxsolutions_guard_slice(),
      alloc_maxsolutions_counter_slice()
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    attack_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert STMaxSolutionsGuard slices
 */
static void insert_maxsolutions_solvable_filter(slice_index si,
                                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_maxsolutions_counter_slice();
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors maxsolutions_filter_inserters[] =
{
  { STMaxSolutionsGuard, &stip_structure_visitor_noop         },
  { STReadyForHelpMove,  &insert_maxsolutions_help_filter     },
  { STAttackAdapter,     &insert_maxsolutions_attack_adapter  },
  { STGoalReachedTester, &insert_maxsolutions_solvable_filter }
};

enum
{
  nr_maxsolutions_filter_inserters =
  (sizeof maxsolutions_filter_inserters
   / sizeof maxsolutions_filter_inserters[0])
};

/* Instrument a stipulation with STMaxSolutions*Filter slices
 * @param si identifies slice where to start
 */
void stip_insert_maxsolutions_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  {
    slice_index const prototypes[] =
    {
      alloc_maxsolutions_initialiser_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    maxsolutions_filter_inserters,
                                    nr_maxsolutions_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
