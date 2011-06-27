#include "pyoutput.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "output/plaintext/move_inversion_counter.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/tree/tree.h"
#include "output/plaintext/line/line.h"

#include "trace.h"

#ifdef _SE_
#include "se.h"
#endif

#define ENUMERATION_TYPENAME output_mode
#define ENUMERATORS \
  ENUMERATOR(output_mode_tree), \
    ENUMERATOR(output_mode_line), \
    ENUMERATOR(output_mode_none)

#define ENUMERATION_MAKESTRINGS

#include "pyenum.h"

/* identifies a slice whose starter is the nominal starter of the stipulation
 * before any move inversions are applied
 * (e.g. in a h#N.5, this slice's starter is Black)
 */
slice_index output_plaintext_slice_determining_starter = no_slice;

/* Allocate an STOutputModeSelector slice
 * @param mode output mode to be selected by the allocated slice
 * @return identifier of the allocated slice
 */
slice_index alloc_output_mode_selector(output_mode mode)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceEnumerator(output_mode,mode,"");
  TraceFunctionParamListEnd();

  result = alloc_pipe(STOutputModeSelector);
  slices[result].u.output_mode_selector.mode = mode;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void select_output_mode(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.output_mode_selector.mode==output_mode_line)
    stip_insert_output_plaintext_line_slices(si);
  else
    stip_insert_output_plaintext_tree_slices(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_move_inversion_counter(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_append(si,alloc_output_plaintext_move_inversion_counter_slice());
  output_plaintext_slice_determining_starter = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the stipulation structure with slices that implement
 * the selected output mode.
 * @param si identifies slice where to start
 */
void stip_insert_output_slices(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  output_plaintext_slice_determining_starter = no_slice;

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STOutputModeSelector,
                                           &select_output_mode);
  stip_structure_traversal_override_single(&st,
                                           STMoveInverter,
                                           &insert_move_inversion_counter);
  stip_structure_traversal_override_single(&st,
                                           STTemporaryHackFork,
                                           &stip_traverse_structure_pipe);
  stip_traverse_structure(si,&st);

  {
    slice_index const prototypes[] =
    {
      alloc_illegal_selfcheck_writer_slice(),
      alloc_end_of_phase_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    root_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
