#include "solving/trivial_end_filter.h"
#include "pydata.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "trace.h"

#include <assert.h>

/* Used to inform STTrivialEndFilter about when to filter out trivial
 * variations (e.g. short mates in self stipulations if there are defenses that
 * don't deliver mate).
 *
 * Initialise element 1 to true to cause immediate mates in set and postkey only
 * play in self stipulations.
 *
 * This causes some superfluous moves and check tests to be performed in ser-r
 * stipulations, but any "correct" implementation would be significantly more
 * complex.
 */
boolean do_write_trivial_ends[maxply+1] = { false, true };

/* Allocate a STTrivialEndFilter slice.
 * @return index of allocated slice
 */
static slice_index alloc_trivial_end_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STTrivialEndFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type trivial_end_filter_can_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = can_attack(slices[si].u.pipe.next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type trivial_end_filter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (nbply==nil_ply || do_write_trivial_ends[parent_ply[nbply]])
    result = attack(next,n);
  else
    /* variation is trivial - just determine the result */
    result = can_attack(next,n);

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
  stip_traverse_structure_children(si,st);
  *mode = save_mode;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void trivial_varation_filter_insert_constraint(slice_index si,
                                                      stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_pipe(si,st);

  {
    slice_index const adapter = branch_find_slice(STAttackAdapter,
                                                  slices[si].u.fork.fork);
    if (adapter!=no_slice)
    {
      slice_index const prototype = alloc_trivial_end_filter_slice();
      battle_branch_insert_slices(adapter,&prototype,1);
    }
  }

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

  stip_traverse_structure_pipe(si,st);

  if (*mode==output_mode_tree
      && st->context==stip_traversal_context_defense)
  {
    slice_index const adapter = branch_find_slice(STAttackAdapter,
                                                  slices[si].u.fork.fork);
    assert(adapter!=no_slice);
    pipe_append(adapter,alloc_trivial_end_filter_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors trivial_varation_filter_inserters[] =
{
  { STOutputModeSelector,      &remember_output_mode                      },
  { STConstraint,              &trivial_varation_filter_insert_constraint },
  { STEndOfBranchGoal,         &trivial_varation_filter_insert_self       },
  { STEndOfBranchGoalImmobile, &trivial_varation_filter_insert_self       }
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
void stip_insert_trivial_varation_filters(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override(&st,
                                    trivial_varation_filter_inserters,
                                    nr_trivial_varation_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
