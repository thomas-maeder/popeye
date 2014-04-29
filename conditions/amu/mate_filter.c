#include "conditions/amu/mate_filter.h"
#include "conditions/amu/attack_counter.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* This module provides slice type STAmuMateFilter
 */

/* Allocate a STAmuMateFilter slice.
 * @return index of allocated slice
 */
static slice_index alloc_amu_mate_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STAmuMateFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void amu_mate_filter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_doesnt_solve_if(si,!amu_attacked_exactly_once[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepend_mate_filter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  pipe_append(slices[si].prev,alloc_amu_mate_filter_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor amu_filter_inserters[] =
{
  { STGoalMateReachedTester, &prepend_mate_filter }
};

enum
{
  nr_amu_filter_inserters = (sizeof amu_filter_inserters
                             / sizeof amu_filter_inserters[0])
};

/* Instrument a stipulation with AMU mate filter slices
 * @param si root of branch to be instrumented
 */
void solving_insert_amu_mate_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    amu_filter_inserters,
                                    nr_amu_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
