#include "pyoutput.h"
#include "pystip.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/tree/tree.h"
#include "output/plaintext/line/line.h"

#include "debugging/trace.h"

#ifdef _SE_
#include "se.h"
#endif

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

static void remember_setplay(slice_index si, stip_structure_traversal *st)
{
  boolean * const is_setplay = st->param;
  boolean const save_is_setplay = *is_setplay;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *is_setplay = true;
  stip_traverse_structure_children(si,st);
  *is_setplay = save_is_setplay;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void select_output_mode(slice_index si, stip_structure_traversal *st)
{
  boolean const * const is_setplay = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.output_mode_selector.mode==output_mode_line)
    stip_insert_output_plaintext_line_slices(si);
  else
    stip_insert_output_plaintext_tree_slices(si,*is_setplay);

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
    branch_insert_slices(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_output_slices_binary(slice_index si,
                                        stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_binary_operand1(si,st);
  stip_traverse_structure_binary_operand2(si,st);

  /* don't traverse tester */

  TraceFunctionExit(__func__);
  TraceFunctionParamListEnd();
}

/* Instrument the stipulation structure with slices that implement
 * the selected output mode.
 * @param si identifies slice where to start
 */
void stip_insert_output_slices(slice_index si)
{
  stip_structure_traversal st;
  boolean is_setplay = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&is_setplay);
  stip_structure_traversal_override_single(&st,
                                           STSetplayFork,
                                           &remember_setplay);
  stip_structure_traversal_override_single(&st,
                                           STOutputModeSelector,
                                           &select_output_mode);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_binary,
                                                &insert_output_slices_binary);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
