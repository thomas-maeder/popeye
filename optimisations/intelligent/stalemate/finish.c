#include "optimisations/intelligent/stalemate/finish.h"
#include "pyint.h"
#include "pydata.h"
#include "pyslice.h"
#include "optimisations/intelligent/stalemate/intercept_checks.h"
#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "trace.h"

#include <assert.h>

void intelligent_stalemate_continue_after_block(stip_length_type n,
                                                Side side,
                                                square to_be_blocked,
                                                piece blocker_type,
                                                unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (guards(king_square[side],blocker_type,to_be_blocked))
  {
    unsigned int const nr_checks_to_opponent = 0;
    intelligent_stalemate_intercept_checks(n,nr_checks_to_opponent,side);
  }
  else
    intelligent_stalemate_test_target_position(n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_test_target_position(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(!echecc(nbply,Black));
  assert(!echecc(nbply,White));
  if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
    intelligent_stalemate_deal_with_unused_pieces(n);
  else
    intelligent_stalemate_immobilise_black(n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
