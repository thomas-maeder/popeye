#include "optimisations/intelligent/stalemate/finish.h"
#include "pyint.h"
#include "pydata.h"
#include "pyslice.h"
#include "optimisations/intelligent/stalemate/intercept_checks.h"
#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "trace.h"

#include <assert.h>

void intelligent_stalemate_continue_after_block(unsigned int nr_remaining_white_moves,
                                                unsigned int nr_remaining_black_moves,
                                                unsigned int max_nr_allowed_captures_by_white,
                                                unsigned int max_nr_allowed_captures_by_black,
                                                stip_length_type n,
                                                Side side,
                                                square to_be_blocked,
                                                piece blocker_type,
                                                unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (guards(king_square[side],blocker_type,to_be_blocked))
  {
    unsigned int const nr_checks_to_opponent = 0;
    intelligent_stalemate_intercept_checks(nr_remaining_white_moves,
                                           nr_remaining_black_moves,
                                           max_nr_allowed_captures_by_white,
                                           max_nr_allowed_captures_by_black,
                                           n,
                                           nr_checks_to_opponent,
                                           side);
  }
  else
    intelligent_stalemate_test_target_position(nr_remaining_white_moves,
                                               nr_remaining_black_moves,
                                               max_nr_allowed_captures_by_white,
                                               max_nr_allowed_captures_by_black,
                                               n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_test_target_position(unsigned int nr_remaining_white_moves,
                                                unsigned int nr_remaining_black_moves,
                                                unsigned int max_nr_allowed_captures_by_white,
                                                unsigned int max_nr_allowed_captures_by_black,
                                                stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(!echecc(nbply,Black));
  assert(!echecc(nbply,White));
  if (slice_has_solution(slices[current_start_slice].u.fork.fork)==has_solution)
    intelligent_stalemate_deal_with_unused_pieces(nr_remaining_white_moves,
                                                  nr_remaining_black_moves,
                                                  max_nr_allowed_captures_by_white,
                                                  max_nr_allowed_captures_by_black,
                                                  n);
  else
    intelligent_stalemate_immobilise_black(nr_remaining_white_moves,
                                           nr_remaining_black_moves,
                                           max_nr_allowed_captures_by_white,
                                           max_nr_allowed_captures_by_black,
                                           n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
