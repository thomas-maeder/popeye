#include "optimisations/intelligent/stalemate/filter.h"
#include "solving/machinery/slack_length.h"
#include "solving/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STIntelligentStalemateFilter slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_stalemate_filter(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    slice_index const proxy_branch = alloc_proxy_slice();
    slice_index const help = alloc_help_branch(slack_length+1,
                                               slack_length+1);
    slice_index const proto = alloc_pipe(STIntelligentImmobilisationCounter);
    help_branch_insert_slices(help,&proto,1);
    link_to_branch(proxy_branch,help);

    result = alloc_conditional_pipe(STIntelligentStalemateFilter,proxy_branch);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation.
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
void impose_starter_intelligent_stalemate_filter(slice_index si,
                                                 stip_structure_traversal *st)
{
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  SLICE_STARTER(si) = *starter;
  stip_traverse_structure_children_pipe(si,st);

  /* in duplexes, the colors swapped when looking for the "black solutions".
   * we thus have to make sure that our immobility tester always tests for
   * Black's immobility.
   */
  *starter = Black;
  stip_traverse_structure_next_branch(si,st);
  *starter = SLICE_STARTER(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean Intelligent(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  solutions_found = false;
  intelligent_duplicate_avoider_init();
  IntelligentRegulargoal_types(si);
  intelligent_duplicate_avoider_cleanup();
  result = solutions_found;

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
void intelligent_stalemate_filter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_solves_exactly_if(Intelligent(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
