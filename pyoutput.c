#include "pyoutput.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyslice.h"
#include "pypipe.h"
#include "py1.h"
#include "output/plaintext/tree/tree.h"
#include "output/plaintext/tree/check_detector.h"
#include "output/plaintext/tree/move_inversion_counter.h"
#include "output/plaintext/line/move_inversion_counter.h"
#include "output/plaintext/line/line.h"
#include "trace.h"
#ifdef _SE_
#include "se.h"
#endif

#include <assert.h>
#include <stdlib.h>

#define ENUMERATION_TYPENAME output_mode
#define ENUMERATORS \
  ENUMERATOR(output_mode_tree), \
    ENUMERATOR(output_mode_line), \
    ENUMERATOR(output_mode_none)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"

static output_mode current_mode = output_mode_none;

void set_output_mode(output_mode mode)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(output_mode,mode,"");
  TraceFunctionParamListEnd();

  current_mode = mode;

  if (current_mode==output_mode_tree)
    reset_pending_check();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with slices that implement
 * the selected output mode.
 */
void stip_insert_output_slices(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (current_mode==output_mode_tree)
    stip_insert_output_plaintext_tree_slices();
  else
    stip_insert_output_plaintext_line_slices();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void output_mode_treemode(slice_index si, stip_structure_traversal *st)
{
  output_mode * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *mode = output_mode_tree;
  TraceEnumerator(output_mode,*mode,"\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void output_mode_linemode(slice_index si, stip_structure_traversal *st)
{
  output_mode * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *mode = output_mode_line;
  TraceEnumerator(output_mode,*mode,"\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void output_mode_binary(slice_index si, stip_structure_traversal *st)
{
  output_mode * const mode = st->param;
  output_mode mode1;
  output_mode mode2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure(slices[si].u.binary.op1,st);
  mode1 = *mode;

  stip_traverse_structure(slices[si].u.binary.op2,st);
  mode2 = *mode;

  *mode = mode2==output_mode_none ? mode1 : mode2;
  
  TraceEnumerator(output_mode,*mode,"\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void output_mode_self_defense(slice_index si,
                                     stip_structure_traversal *st)
{
  output_mode * const mode = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.branch.length>slack_length_battle)
    *mode = output_mode_tree;
  else
    stip_traverse_structure(slices[si].u.branch.next,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static stip_structure_visitor const output_mode_detectors[] =
{
  &stip_traverse_structure_children, /* STProxy */
  &stip_traverse_structure_children, /* STAttackMove */
  &stip_traverse_structure_children, /* STDefenseMove */
  &stip_traverse_structure_children, /* STHelpMove */
  &output_mode_linemode,             /* STHelpFork */
  &stip_traverse_structure_children, /* STSeriesMove */
  &output_mode_linemode,             /* STSeriesFork */
  &stip_traverse_structure_children, /* STGoalReachedTester */
  &stip_traverse_structure_children, /* STLeaf */
  &output_mode_binary,               /* STReciprocal */
  &output_mode_binary,               /* STQuodlibet */
  &stip_traverse_structure_children, /* STNot */
  &stip_traverse_structure_children, /* STMoveInverterRootSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSolvableFilter */
  &stip_traverse_structure_children, /* STMoveInverterSeriesFilter */
  &output_mode_treemode,             /* STAttackRoot */
  &output_mode_treemode,             /* STDefenseRoot */
  &stip_traverse_structure_children, /* STPostKeyPlaySuppressor */
  &stip_traverse_structure_children, /* STContinuationSolver */
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
  &stip_traverse_structure_children, /* STHelpRoot */
  &stip_traverse_structure_children, /* STHelpShortcut */
  &stip_traverse_structure_children, /* STHelpHashed */
  &stip_traverse_structure_children, /* STSeriesRoot */
  &stip_traverse_structure_children, /* STSeriesShortcut */
  &stip_traverse_structure_children, /* STParryFork */
  &stip_traverse_structure_children, /* STSeriesHashed */
  &pipe_traverse_next,               /* STSelfCheckGuardRootSolvableFilter */
  &pipe_traverse_next,               /* STSelfCheckGuardSolvableFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardAttackerFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardDefenderFilter */
  &pipe_traverse_next,               /* STSelfCheckGuardHelpFilter */
  &stip_traverse_structure_children, /* STSelfCheckGuardSeriesFilter */
  &output_mode_treemode,             /* STDirectDefenderFilter */
  &pipe_traverse_next,               /* STReflexRootFilter */
  &pipe_traverse_next,               /* STReflexHelpFilter */
  &pipe_traverse_next,               /* STReflexSeriesFilter */
  &output_mode_treemode,             /* STReflexAttackerFilter */
  &output_mode_treemode,             /* STReflexDefenderFilter */
  &output_mode_self_defense,         /* STSelfDefense */
  &pipe_traverse_next,               /* STRestartGuardRootDefenderFilter */
  &pipe_traverse_next,               /* STRestartGuardHelpFilter */
  &pipe_traverse_next,               /* STRestartGuardSeriesFilter */
  &pipe_traverse_next,               /* STIntelligentHelpFilter */
  &pipe_traverse_next,               /* STIntelligentSeriesFilter */
  &pipe_traverse_next,               /* STGoalReachableGuardHelpFilter */
  &stip_traverse_structure_children, /* STGoalReachableGuardSeriesFilter */
  &stip_traverse_structure_children, /* STIntelligentDuplicateAvoider */
  &output_mode_treemode,             /* STKeepMatingGuardAttackerFilter */
  &output_mode_treemode,             /* STKeepMatingGuardDefenderFilter */
  &pipe_traverse_next,               /* STKeepMatingGuardHelpFilter */
  &stip_traverse_structure_children, /* STKeepMatingGuardSeriesFilter */
  &pipe_traverse_next,               /* STMaxFlightsquares */
  &pipe_traverse_next,               /* STDegenerateTree */
  &pipe_traverse_next,               /* STMaxNrNonTrivial */
  &pipe_traverse_next,               /* STMaxNrNonTrivialCounter */
  &pipe_traverse_next,               /* STMaxThreatLength */
  &output_mode_treemode,             /* STMaxTimeRootDefenderFilter */
  &output_mode_treemode,             /* STMaxTimeDefenderFilter */
  &pipe_traverse_next,               /* STMaxTimeHelpFilter */
  &stip_traverse_structure_children, /* STMaxTimeSeriesFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootSolvableFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsSolvableFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsRootDefenderFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsHelpFilter */
  &stip_traverse_structure_children, /* STMaxSolutionsSeriesFilter */
  &pipe_traverse_next,               /* STStopOnShortSolutionsRootSolvableFilter */
  &pipe_traverse_next,               /* STStopOnShortSolutionsHelpFilter */
  &pipe_traverse_next,               /* STStopOnShortSolutionsSeriesFilter */
  &pipe_traverse_next,               /* STEndOfPhaseWriter */
  &pipe_traverse_next,               /* STEndOfSolutionWriter */
  &pipe_traverse_next,               /* STRefutationWriter */
  &pipe_traverse_next,               /* STOutputPlaintextTreeCheckDetectorAttackerFilter */
  &pipe_traverse_next,               /* STOutputPlaintextTreeCheckDetectorDefenderFilter */
  &pipe_traverse_next,               /* STOutputPlaintextLineLineWriter */
  &pipe_traverse_next,               /* STOutputPlaintextTreeGoalWriter */
  &pipe_traverse_next,               /* STOutputPlaintextTreeMoveInversionCounter */
  &pipe_traverse_next,               /* STOutputPlaintextLineMoveInversionCounter */
  &pipe_traverse_next                /* STOutputPlaintextLineEndOfIntroSeriesMarker */
};

/* Initialize based on the stipulation
 */
void init_output(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  current_mode = output_mode_none;

  stip_structure_traversal_init(&st,&output_mode_detectors,&current_mode);
  stip_traverse_structure(si,&st);

  TraceEnumerator(output_mode,current_mode,"\n");
  
  if (current_mode==output_mode_tree)
    reset_pending_check();
  else
    current_mode = output_mode_line;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Write the end of a solution phase
 */
void write_end_of_solution_phase(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  Message(NewLine);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
