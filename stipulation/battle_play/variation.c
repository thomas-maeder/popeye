#include "stipulation/battle_play/variation.h"
#include "pyoutput.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_play.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STVariationWriter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static slice_index alloc_variation_writer_slice(stip_length_type length,
                                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STVariationWriter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
variation_writer_has_solution_in_n(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_min,
                                   stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  result = attack_has_solution_in_n(next,n,n_min,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type variation_writer_solve_in_n(slice_index si,
                                             stip_length_type n,
                                             stip_length_type n_min,
                                             stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  if (encore())
    write_battle_move();
  else
  {
    /* no defense was played - we are solving threats */
  }

  result = attack_solve_in_n(next,n,n_min,n_max_unsolvable);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type variation_writer_solve(slice_index si)
{
  has_solution_type result;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;
  stip_length_type const n_max_unsolvable = min_length-2;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type nr_moves;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_moves = attack_has_solution_in_n(next,length,min_length,n_max_unsolvable);

  if (nr_moves<min_length)
    result = opponent_self_check;
  else if (nr_moves<=length)
  {
    result = has_solution;
    write_battle_move();
    {
      stip_length_type const solve_result = attack_solve_in_n(next,
                                                              length,min_length,
                                                              n_max_unsolvable);
      assert(solve_result<=length);
    }
  }
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Inserting variation handlers in both parts of a binary
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void variation_handler_insert_binary(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.binary.op1,st);
  stip_traverse_structure(slices[si].u.binary.op2,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* The following enumeration type represents the state of variation
 * handler insertion
 */
typedef enum
{
  variation_handler_none,
  variation_handler_needed,
  variation_handler_inserted
} variation_handler_insertion_state;

/* Append a variation writer
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void variation_writer_append(slice_index si,
                                    stip_structure_traversal *st)
{
  variation_handler_insertion_state * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*state==variation_handler_needed)
  {
    *state = variation_handler_inserted;
    stip_traverse_structure_children(si,st);
    *state = variation_handler_needed;

    pipe_append(si,alloc_variation_writer_slice(length,min_length));
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prepend a variation writer
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void variation_writer_prepend_leaf(slice_index si,
                                          stip_structure_traversal *st)
{
  variation_handler_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*state==variation_handler_needed)
    pipe_append(slices[si].prev,
                alloc_variation_writer_slice(slack_length_battle,
                                             slack_length_battle));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a variation writer if none has been inserted before
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void variation_writer_insert_self_defense(slice_index si,
                                                 stip_structure_traversal *st)
{
  variation_handler_insertion_state * const state = st->param;
  variation_handler_insertion_state const save_state = *state;
  slice_index const next = slices[si].u.branch_fork.next;
  slice_index const to_goal = slices[si].u.branch_fork.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(next,st);
  *state = save_state;
  stip_traverse_structure(to_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inform slices traversed after a STDefenseMove slice that a
 * variation writer slice is needed 
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void mark_need_for_handler(slice_index si,
                                  stip_structure_traversal *st)
{
  variation_handler_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *state = variation_handler_needed;
  stip_traverse_structure_children(si,st);
  *state = variation_handler_none;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const variation_handler_inserters[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &mark_need_for_handler,            /* STDefenseMove */
  &stip_structure_visitor_noop,      /* STHelpMove */
  &stip_structure_visitor_noop,      /* STHelpFork */
  &stip_structure_visitor_noop,      /* STSeriesMove */
  &stip_structure_visitor_noop,      /* STSeriesFork */
  &stip_structure_visitor_noop,      /* STLeafDirect */
  &stip_structure_visitor_noop,      /* STLeafHelp */
  &variation_writer_prepend_leaf,    /* STLeafForced */
  &variation_handler_insert_binary,  /* STReciprocal */
  &variation_handler_insert_binary,  /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &stip_traverse_structure_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children, /* STAttackRoot */
  &stip_traverse_structure_children, /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children, /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children, /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children, /* STContinuationWriter */
  &stip_traverse_structure_children, /* STRefutationsWriter */
  &stip_traverse_structure_children, /* STThreatWriter */
  &stip_traverse_structure_children, /* STThreatEnforcer */
  &stip_traverse_structure_children, /* STThreatCollector */
  &stip_traverse_structure_children, /* STRefutationsCollector */
  &stip_traverse_structure_children, /* STVariationWriter */
  &stip_traverse_structure_children, /* STRefutingVariationWriter */
  &stip_traverse_structure_children, /* STNoShortVariations */
  &stip_traverse_structure_children, /* STAttackHashed */
  &stip_structure_visitor_noop,      /* STHelpRoot */
  &stip_structure_visitor_noop,      /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &stip_structure_visitor_noop,      /* STSeriesRoot */
  &stip_structure_visitor_noop,      /* STSeriesShortcut */
  &stip_traverse_structure_children, /* STParryFork */
  &stip_traverse_structure_children, /* STSeriesHashed */
  &stip_traverse_structure_children, /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSolvableFilter */
  &variation_writer_append,          /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardDefenderFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children, /* STDirectDefenderFilter */
  &stip_traverse_structure_children, /* STReflexRootFilter */
  &stip_traverse_structure_children, /* STReflexHelpFilter */
  &stip_traverse_structure_children, /* STReflexSeriesFilter */
  &stip_traverse_structure_children, /* STReflexAttackerFilter */
  &stip_traverse_structure_children, /* STReflexDefenderFilter */
  &variation_writer_insert_self_defense, /* STSelfDefense */
  &stip_traverse_structure_children, /* STRestartGuardRootDefenderFilter */
  &stip_traverse_structure_children, /* STRestartGuardHelpFilter */
  &stip_traverse_structure_children, /* STRestartGuardSeriesFilter */
  &stip_traverse_structure_children, /* STIntelligentHelpFilter */
  &stip_traverse_structure_children, /* STIntelligentSeriesFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardDefenderFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children, /* STMaxFlightsquares */
  &stip_traverse_structure_children, /* STDegenerateTree */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
  &stip_traverse_structure_children, /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children, /* STMaxThreatLength */
  &stip_traverse_structure_children, /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children, /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsHelpFilter */
  &stip_traverse_structure_children  /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument the stipulation representation so that it can deal with
 * variations
 */
void stip_insert_variation_handlers(void)
{
  stip_structure_traversal st;
  variation_handler_insertion_state state = variation_handler_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&variation_handler_inserters,&state);
  stip_traverse_structure(root_slice,&st);

  assert(state==variation_handler_none);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
