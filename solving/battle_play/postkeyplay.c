#include "stipulation/battle_play/postkeyplay.h"
#include "pyoutput.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/branch.h"
#include "trace.h"

#include <assert.h>

/* Allocate a STPostKeyPlaySolutionWriter defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static
slice_index alloc_postkey_solution_writer_slice(stip_length_type length,
                                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_branch(STPostKeyPlaySolutionWriter,length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STPostKeyPlaySuppressor defender slice.
 * @return index of allocated slice
 */
static slice_index alloc_postkeyplay_suppressor_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STPostKeyPlaySuppressor);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
postkey_solution_writer_defend_in_n(slice_index si,
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

  result = defense_defend_in_n(next,n,n_min,n_max_unsolvable);
  write_end_of_solution_phase();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STRefutingVariationWriter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
static
slice_index alloc_refuting_variation_writer_slice(stip_length_type length,
                                                  stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  if (min_length<slack_length_battle)
    min_length += 2;
  result = alloc_branch(STRefutingVariationWriter,length,min_length);

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
refuting_variation_writer_has_solution_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min,
                                            stip_length_type n_max_unsolvable)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
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
stip_length_type
refuting_variation_writer_solve_in_n(slice_index si,
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

  result = attack_solve_in_n(next,n,n_min,n_max_unsolvable);

  if (result>n)
  {
    if (encore())
      write_refutation_mark();
    else
    {
      /* no defense was played - we have been solving threats */
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
postkeyplay_suppressor_defend_in_n(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_min,
                                   stip_length_type n_max_unsolvable)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParam("%u",n_max_unsolvable);
  TraceFunctionParamListEnd();

  output_start_defense_level(si);
  output_end_defense_level();

  result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there are refutations after an attempted key move
 * at non-root level
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
postkeyplay_suppressor_can_defend_in_n(slice_index si,
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


/* The following enumeration type represents the state of postkey
 * handler insertion; this helps us avoiding unnecessary postkey
 * handler slices
 */
typedef enum
{
  postkey_handler_inserted_none,
  postkey_handler_inserted_writer
} postkey_handler_insertion_state;

/* Append a variation writer
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void prepend_refutes_writer(slice_index si, stip_structure_traversal *st)
{
  postkey_handler_insertion_state const * const state = st->param;
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*state==postkey_handler_inserted_writer
      && length>slack_length_battle)
  {
    stip_length_type const min_length = slices[si].u.branch.min_length;
    slice_index const prev = slices[si].prev;
    pipe_append(prev,alloc_refuting_variation_writer_slice(length,min_length));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a variation writer
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void substitute_postkey_solution_writer(slice_index si,
                                               stip_structure_traversal *st)
{
  postkey_handler_insertion_state * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*state==postkey_handler_inserted_none)
  {
    *state = postkey_handler_inserted_writer;
    stip_traverse_structure_children(si,st);
    *state = postkey_handler_inserted_none;

    {
      stip_length_type const length = slices[si].u.branch.length;
      stip_length_type const min_length = slices[si].u.branch.min_length;
      pipe_replace(si,alloc_postkey_solution_writer_slice(length,min_length));
    }
  }
  else
    stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const postkey_handler_inserters[] =
{
  &stip_traverse_structure_children,   /* STProxy */
  &stip_traverse_structure_children,   /* STAttackMove */
  &stip_traverse_structure_children,   /* STDefenseMove */
  &stip_structure_visitor_noop,        /* STHelpMove */
  &stip_structure_visitor_noop,        /* STHelpFork */
  &stip_structure_visitor_noop,        /* STSeriesMove */
  &stip_structure_visitor_noop,        /* STSeriesFork */
  &stip_structure_visitor_noop,        /* STLeafDirect */
  &stip_structure_visitor_noop,        /* STLeafHelp */
  &stip_structure_visitor_noop,        /* STLeafForced */
  &stip_traverse_structure_children,   /* STReciprocal */
  &stip_traverse_structure_children,   /* STQuodlibet */
  &stip_traverse_structure_children,   /* STNot */
  &stip_traverse_structure_children,   /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children,   /* STMoveInverterSolvableFilter */
  &stip_structure_visitor_noop,        /* STMoveInverterSeriesFilter */
  &stip_structure_visitor_noop,        /* STAttackRoot */
  &stip_traverse_structure_children,   /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children,   /* STPostKeyPlaySuppressor */
  &substitute_postkey_solution_writer, /* STContinuationWriter */
  &stip_traverse_structure_children,   /* STRefutationsWriter */
  &stip_traverse_structure_children,   /* STThreatWriter */
  &stip_traverse_structure_children,   /* STThreatEnforcer */
  &stip_traverse_structure_children,   /* STThreatCollector */
  &stip_traverse_structure_children,   /* STRefutationsCollector */
  &prepend_refutes_writer,             /* STVariationWriter */
  &stip_traverse_structure_children,   /* STRefutingVariationWriter */
  &stip_traverse_structure_children,   /* STNoShortVariations */
  &stip_traverse_structure_children,   /* STAttackHashed */
  &stip_structure_visitor_noop,        /* STHelpRoot */
  &stip_structure_visitor_noop,        /* STHelpShortcut */
  &stip_structure_visitor_noop,        /* STHelpHashed */
  &stip_structure_visitor_noop,        /* STSeriesRoot */
  &stip_structure_visitor_noop,        /* STSeriesShortcut */
  &stip_structure_visitor_noop,        /* STParryFork */
  &stip_structure_visitor_noop,        /* STSeriesHashed */
  &stip_traverse_structure_children,   /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children,   /* STSelfCheckGuardDefenderFilter */
  &stip_structure_visitor_noop,        /* STSelfCheckGuardHelpFilter */
  &stip_structure_visitor_noop,        /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STDirectDefenderFilter */
  &stip_structure_visitor_noop,        /* STReflexRootFilter */
  &stip_structure_visitor_noop,        /* STReflexHelpFilter */
  &stip_structure_visitor_noop,        /* STReflexSeriesFilter */
  &stip_traverse_structure_children,   /* STReflexAttackerFilter */
  &stip_traverse_structure_children,   /* STReflexDefenderFilter */
  &stip_traverse_structure_children,   /* STSelfDefense */
  &stip_traverse_structure_children,   /* STRestartGuardRootDefenderFilter */
  &stip_structure_visitor_noop,        /* STRestartGuardHelpFilter */
  &stip_structure_visitor_noop,        /* STRestartGuardSeriesFilter */
  &stip_structure_visitor_noop,        /* STIntelligentHelpFilter */
  &stip_structure_visitor_noop,        /* STIntelligentSeriesFilter */
  &stip_structure_visitor_noop,        /* STGoalReachableGuardHelpFilter */
  &stip_structure_visitor_noop,        /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children,   /* STKeepMatingGuardDefenderFilter */
  &stip_structure_visitor_noop,        /* STKeepMatingGuardHelpFilter */
  &stip_structure_visitor_noop,        /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children,   /* STMaxFlightsquares */
  &stip_traverse_structure_children,   /* STDegenerateTree */
  &stip_traverse_structure_children,   /* STMaxNrNonTrivial */
  &stip_traverse_structure_children,   /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children,   /* STMaxThreatLength */
  &stip_traverse_structure_children,   /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children,   /* STMaxTimeDefenderFilter */
  &stip_structure_visitor_noop,        /* STMaxTimeHelpFilter */
  &stip_structure_visitor_noop,        /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children,   /* STMaxSolutionsRootDefenderFilter */
  &stip_structure_visitor_noop,        /* STMaxSolutionsHelpFilter */
  &stip_structure_visitor_noop,        /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children,   /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_structure_visitor_noop,        /* STStopOnShortSolutionsHelpFilter */
  &stip_structure_visitor_noop         /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument the stipulation representation so that it can write
 * variations
 */
void stip_insert_postkey_handlers(void)
{
  stip_structure_traversal st;
  postkey_handler_insertion_state state = postkey_handler_inserted_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&postkey_handler_inserters,&state);
  stip_traverse_structure(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append a variation writer
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void append_postkeyplay_suppressor(slice_index si,
                                          stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (length>slack_length_battle)
    pipe_append(si,alloc_postkeyplay_suppressor_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const postkey_suppressor_inserters[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &stip_traverse_structure_children, /* STDefenseMove */
  &stip_structure_visitor_noop,      /* STHelpMove */
  &stip_structure_visitor_noop,      /* STHelpFork */
  &stip_structure_visitor_noop,      /* STSeriesMove */
  &stip_structure_visitor_noop,      /* STSeriesFork */
  &stip_structure_visitor_noop,      /* STLeafDirect */
  &stip_structure_visitor_noop,      /* STLeafHelp */
  &stip_structure_visitor_noop,      /* STLeafForced */
  &stip_traverse_structure_children, /* STReciprocal */
  &stip_traverse_structure_children, /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &stip_traverse_structure_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSolvableFilter */
  &stip_structure_visitor_noop,      /* STMoveInverterSeriesFilter */
  &stip_traverse_structure_children, /* STAttackRoot */
  &stip_traverse_structure_children, /* STPostKeyPlaySolutionWriter */
  &stip_traverse_structure_children, /* STPostKeyPlaySuppressor */
  &append_postkeyplay_suppressor,    /* STContinuationWriter */
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
  &stip_structure_visitor_noop,      /* STHelpHashed */
  &stip_structure_visitor_noop,      /* STSeriesRoot */
  &stip_structure_visitor_noop,      /* STSeriesShortcut */
  &stip_structure_visitor_noop,      /* STParryFork */
  &stip_structure_visitor_noop,      /* STSeriesHashed */
  &stip_traverse_structure_children, /* STSelfCheckGuardRootSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardDefenderFilter */
  &stip_structure_visitor_noop,      /* STSelfCheckGuardHelpFilter */
  &stip_structure_visitor_noop,      /* STSelfCheckGuardSeriesFilter */
  &stip_traverse_structure_children, /* STDirectDefenderFilter */
  &stip_traverse_structure_children, /* STReflexRootFilter */
  &stip_structure_visitor_noop,      /* STReflexHelpFilter */
  &stip_structure_visitor_noop,      /* STReflexSeriesFilter */
  &stip_traverse_structure_children, /* STReflexAttackerFilter */
  &stip_traverse_structure_children, /* STReflexDefenderFilter */
  &stip_traverse_structure_children, /* STSelfDefense */
  &stip_traverse_structure_children, /* STRestartGuardRootDefenderFilter */
  &stip_structure_visitor_noop,      /* STRestartGuardHelpFilter */
  &stip_structure_visitor_noop,      /* STRestartGuardSeriesFilter */
  &stip_structure_visitor_noop,      /* STIntelligentHelpFilter */
  &stip_structure_visitor_noop,      /* STIntelligentSeriesFilter */
  &stip_structure_visitor_noop,      /* STGoalReachableGuardHelpFilter */
  &stip_structure_visitor_noop,      /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardAttackerFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardDefenderFilter */
  &stip_structure_visitor_noop,      /* STKeepMatingGuardHelpFilter */
  &stip_structure_visitor_noop,      /* STKeepMatingGuardSeriesFilter */
  &stip_traverse_structure_children, /* STMaxFlightsquares */
  &stip_traverse_structure_children, /* STDegenerateTree */
  &stip_traverse_structure_children, /* STMaxNrNonTrivial */
  &stip_traverse_structure_children, /* STMaxNrNonTrivialCounter */
  &stip_traverse_structure_children, /* STMaxThreatLength */
  &stip_traverse_structure_children, /* STMaxTimeRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxTimeDefenderFilter */
  &stip_structure_visitor_noop,      /* STMaxTimeHelpFilter */
  &stip_structure_visitor_noop,      /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootDefenderFilter */
  &stip_structure_visitor_noop,      /* STMaxSolutionsHelpFilter */
  &stip_structure_visitor_noop,      /* STMaxSolutionsSeriesFilter */
  &stip_traverse_structure_children, /* STStopOnShortSolutionsRootSolvableFilter */
  &stip_structure_visitor_noop,      /* STStopOnShortSolutionsHelpFilter */
  &stip_structure_visitor_noop       /* STStopOnShortSolutionsSeriesFilter */
};

/* Instrument the stipulation representation so that post key play is
 * suppressed from output
 */
void stip_insert_postkeyplay_suppressors(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&postkey_suppressor_inserters,0);
  stip_traverse_structure(root_slice,&st);

  TraceStipulation(root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
