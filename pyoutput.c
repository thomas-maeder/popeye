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
 * @param si identifies slice where to start
 */
void stip_insert_output_slices(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (current_mode==output_mode_tree)
    stip_insert_output_plaintext_tree_slices(si);
  else
    stip_insert_output_plaintext_line_slices(si);

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
    stip_traverse_structure_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitors output_mode_detectors[] =
{
  { STHelpFork,                       &output_mode_linemode         },
  { STSeriesFork,                     &output_mode_linemode         },
  { STReciprocal,                     &output_mode_binary           },
  { STQuodlibet,                      &output_mode_binary           },
  { STAttackRoot,                     &output_mode_treemode         },
  { STDefenseRoot,                    &output_mode_treemode         },
  { STDirectDefenderFilter,           &output_mode_treemode         },
  { STReflexRootFilter,               &stip_traverse_structure_pipe },
  { STReflexHelpFilter,               &stip_traverse_structure_pipe },
  { STReflexSeriesFilter,             &stip_traverse_structure_pipe },
  { STReflexAttackerFilter,           &output_mode_treemode         },
  { STReflexDefenderFilter,           &output_mode_treemode         },
  { STSelfDefense,                    &output_mode_self_defense     }
};

enum
{
  nr_output_mode_detectors = (sizeof output_mode_detectors
                              / sizeof output_mode_detectors[0])
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

  stip_structure_traversal_init(&st,&current_mode);
  stip_structure_traversal_override(&st,
                                    output_mode_detectors,
                                    nr_output_mode_detectors);
  stip_traverse_structure(si,&st);

  TraceEnumerator(output_mode,current_mode,"\n");
  
  if (current_mode==output_mode_tree)
    reset_pending_check();
  else
    current_mode = output_mode_line;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
