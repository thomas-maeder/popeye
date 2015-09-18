#include "optimisations/intelligent/stalemate/finish.h"
#include "solving/machinery/solve.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "options/maxsolutions/maxsolutions.h"
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
  if (!max_nr_solutions_found_in_phase())
  {
    if (!intelligent_stalemate_immobilise_black(si))
      intelligent_stalemate_deal_with_unused_pieces(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
