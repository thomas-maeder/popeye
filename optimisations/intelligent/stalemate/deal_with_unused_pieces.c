#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "pyint.h"
#include "pydata.h"
#include "pyslice.h"
#include "platform/maxtime.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/stalemate/intercept_checks.h"
#include "optimisations/intelligent/stalemate/black_block.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>

static void place_piece(unsigned int nr_remaining_white_moves,
                        unsigned int nr_remaining_black_moves,
                        unsigned int max_nr_allowed_captures_by_white,
                        unsigned int max_nr_allowed_captures_by_black,
                        stip_length_type n)
{
  square const *bnp;
  square const * const save_start = where_to_start_placing_unused_black_pieces;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (bnp = where_to_start_placing_unused_black_pieces; *bnp; ++bnp)
    if (e[*bnp]==vide && nr_reasons_for_staying_empty[*bnp]==0)
    {
      where_to_start_placing_unused_black_pieces = bnp;
      intelligent_stalemate_black_block(nr_remaining_white_moves,
                                        nr_remaining_black_moves,
                                        max_nr_allowed_captures_by_white,
                                        max_nr_allowed_captures_by_black,
                                        n,
                                        *bnp);
    }

  where_to_start_placing_unused_black_pieces = save_start;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void fix_white_king_on_diagram_square(unsigned int nr_remaining_white_moves,
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

  if (nr_reasons_for_staying_empty[white[index_of_king].diagram_square]==0
      && !would_white_king_guard_from(white[index_of_king].diagram_square)
      && !is_white_king_uninterceptably_attacked_by_non_king(white[index_of_king].diagram_square))
  {
    white[index_of_king].usage = piece_is_fixed_to_diagram_square;

    king_square[White] = white[index_of_king].diagram_square;
    SetPiece(roib,king_square[White],white[index_of_king].flags);

    if (is_white_king_interceptably_attacked())
    {
      unsigned int const nr_of_checks_to_black = 0;
      intelligent_stalemate_intercept_checks(nr_remaining_white_moves,
                                             nr_remaining_black_moves,
                                             max_nr_allowed_captures_by_white,
                                             max_nr_allowed_captures_by_black,
                                             n,
                                             nr_of_checks_to_black,
                                             White);
    }
    else
      intelligent_stalemate_test_target_position(nr_remaining_white_moves,
                                                 nr_remaining_black_moves,
                                                 max_nr_allowed_captures_by_white,
                                                 max_nr_allowed_captures_by_black,
                                                 n);

    e[king_square[White]] = vide;
    spec[king_square[White]] = EmptySpec;
    king_square[White] = initsquare;

    white[index_of_king].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_deal_with_unused_pieces(unsigned int nr_remaining_white_moves,
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

  if (!hasMaxtimeElapsed())
  {
    if (white[index_of_king].usage==piece_is_unused
        && white[index_of_king].diagram_square!=square_e1
        && nr_remaining_white_moves==0)
    {
      if (e[white[index_of_king].diagram_square]==vide
          && nr_reasons_for_staying_empty[white[index_of_king].diagram_square]==0)
        fix_white_king_on_diagram_square(nr_remaining_white_moves,
                                         nr_remaining_black_moves,
                                         max_nr_allowed_captures_by_white,
                                         max_nr_allowed_captures_by_black,
                                         n);
    }
    else
    {
      unsigned int unused = 0;

      {
        unsigned int i;
        for (i = 1; i<MaxPiece[Black]; ++i)
          if (black[i].usage==piece_is_unused)
            ++unused;
      }

      TraceValue("%u\n",piece_is_unused);
      TraceValue("%u\n",MovesLeft[White]);
      if (unused>0)
        place_piece(nr_remaining_white_moves,
                    nr_remaining_black_moves,
                    max_nr_allowed_captures_by_white,
                    max_nr_allowed_captures_by_black,
                    n);

      if (unused<=MovesLeft[White])
      {
        CapturesLeft[1] = unused;
        solve_target_position(n);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
