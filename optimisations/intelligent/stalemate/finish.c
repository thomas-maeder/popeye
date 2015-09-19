#include "optimisations/intelligent/stalemate/finish.h"
#include "solving/machinery/solve.h"
#include "solving/machinery/slack_length.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/machinery/slack_length.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Test the position created by the taken operations; if the position is a
 * solvable target position: solve it; otherwise: improve it
 */
void intelligent_stalemate_test_target_position(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /*assert(!echecc(Black));*/
  /*assert(!echecc(White));*/
  if (!intelligent_stalemate_immobilise_black(si))
    intelligent_stalemate_deal_with_unused_pieces(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Impose the starting side on a stipulation.
 * @param si identifies slice
 * @param st address of structure that holds the state of the traversal
 */
void impose_starter_intelligent_stalemate_target_position_tester(slice_index si,
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

/* Allocate a STIntelligentStalemateTargetPositionTester slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_stalemate_target_position_tester(void)
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

    result = alloc_conditional_pipe(STIntelligentStalemateTargetPositionTester,proxy_branch);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
