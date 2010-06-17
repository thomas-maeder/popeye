#include "stipulation/battle_play/postkeyplay.h"
#include "pypipe.h"
#include "stipulation/battle_play/defense_root.h"
#include "trace.h"

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

/* Try to defend after an attacking move
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

  result = n;

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


/* Remove the STContinuationSolver slice not used in postkey play
 * @param si identifies slice around which to insert try handlers
 * @param st address of structure defining traversal
 */
static void substitute_defense_root(slice_index si, stip_structure_traversal *st)
{
  stip_length_type const length = slices[si].u.branch.length;
  stip_length_type const min_length = slices[si].u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_replace(si,alloc_defense_root_slice(length,min_length));

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
  &stip_traverse_structure_children,   /* STDefenseRoot */
  &stip_traverse_structure_children,   /* STPostKeyPlaySuppressor */
  &substitute_defense_root,            /* STContinuationSolver */
  &stip_traverse_structure_children,   /* STContinuationWriter */
  &stip_traverse_structure_children,   /* STBattlePlaySolver */
  &stip_traverse_structure_children,   /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children,   /* STThreatSolver */
  &stip_traverse_structure_children,   /* STZugzwangWriter */
  &stip_traverse_structure_children,   /* STThreatEnforcer */
  &stip_traverse_structure_children,   /* STThreatCollector */
  &stip_traverse_structure_children,   /* STRefutationsCollector */
  &stip_traverse_structure_children,   /* STVariationWriter */
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
  &stip_structure_visitor_noop,        /* STStopOnShortSolutionsSeriesFilter */
  &stip_structure_visitor_noop,        /* STEndOfPhaseWriter */
  &stip_structure_visitor_noop,        /* STEndOfSolutionWriter */
  &stip_structure_visitor_noop,        /* STRefutationWriter */
  &stip_structure_visitor_noop,        /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_structure_visitor_noop         /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
};

/* Instrument the stipulation representation so that it can write
 * variations
 */
void stip_insert_postkey_handlers(void)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceStipulation(root_slice);

  stip_structure_traversal_init(&st,&postkey_handler_inserters,0);
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
  &stip_traverse_structure_children, /* STDefenseRoot */
  &stip_traverse_structure_children, /* STPostKeyPlaySuppressor */
  &append_postkeyplay_suppressor,    /* STContinuationSolver */
  &stip_traverse_structure_children, /* STContinuationWriter */
  &stip_traverse_structure_children, /* STBattlePlaySolver */
  &stip_traverse_structure_children, /* STBattlePlaySolutionWriter */
  &stip_traverse_structure_children, /* STThreatSolver */
  &stip_traverse_structure_children, /* STZugzwangWriter */
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
  &stip_structure_visitor_noop,      /* STStopOnShortSolutionsSeriesFilter */
  &stip_structure_visitor_noop,      /* STEndOfPhaseWriter */
  &stip_structure_visitor_noop,      /* STEndOfSolutionWriter */
  &stip_structure_visitor_noop,      /* STRefutationWriter */
  &stip_structure_visitor_noop,      /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &stip_structure_visitor_noop       /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
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
