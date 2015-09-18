 #include "optimisations/intelligent/mate/filter.h"
#include "stipulation/stipulation.h"
#include "stipulation/proxy.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Allocate a STIntelligentMateFilter slice.
 * @param goal_tester_fork fork into the goal goal_tester_fork branch
 * @return allocated slice
 */
slice_index alloc_intelligent_mate_filter(slice_index goal_tester_fork)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal_tester_fork);
  TraceFunctionParamListEnd();

  {
    slice_index const proxy = alloc_proxy_slice();
    link_to_branch(proxy,stip_deep_copy(goal_tester_fork));
    result = alloc_conditional_pipe(STIntelligentMateFilter,proxy);
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
void impose_starter_intelligent_mate_filter(slice_index si,
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
   * we thus have to make sure that the legal move finder always tests for
   * Black.
   */
  *starter = Black;
  stip_traverse_structure_conditional_pipe_tester(si,st);
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
void intelligent_mate_filter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_solves_exactly_if(Intelligent(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
