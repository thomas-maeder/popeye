#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "pyint.h"
#include "pydata.h"
#include "pyslice.h"
#include "platform/maxtime.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/intercept_checks.h"
#include "optimisations/intelligent/stalemate/black_block.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void promoted_pawn(stip_length_type n,
                          square where_to_place,
                          Flags blocker_flags,
                          square blocker_comes_from)
{
  /* A rough check whether it is worth thinking about promotions */
  unsigned int time = (blocker_comes_from>=square_a7
                       ? 5
                       : blocker_comes_from/onerow - nr_of_slack_rows_below_board);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_place);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  assert(time<=5);

  if (where_to_place>=square_a2)
    /* square is not on 1st rank -- 1 move necessary to get there */
    ++time;

  if (Nr_remaining_black_moves>=time)
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!officer_uninterceptably_attacks_king(White,where_to_place,pp))
      {
        unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(blocker_comes_from,
                                                                                       pp,
                                                                                       where_to_place);
        if (time<=Nr_remaining_black_moves)
        {
          unsigned int diffcol = 0;
          if (pp==fn)
          {
            unsigned int const blocker_comes_from_file = blocker_comes_from%nr_files_on_board;
            square const promotion_square_on_same_file = square_a1+blocker_comes_from_file;
            if (SquareCol(where_to_place)!=SquareCol(promotion_square_on_same_file))
              diffcol = 1;
          }
          if (diffcol<=Nr_unused_white_masses)
          {
            unsigned int const nr_checks_to_black = 0;
            Nr_remaining_black_moves -= time;
            Nr_unused_white_masses -= diffcol;
            TraceValue("%u",Nr_remaining_black_moves);
            TraceValue("%u\n",Nr_unused_white_masses);
            SetPiece(pp,where_to_place,blocker_flags);
            intelligent_stalemate_continue_after_block(n,
                                                       White,
                                                       where_to_place,
                                                       pp,
                                                       nr_checks_to_black);
            Nr_unused_white_masses += diffcol;
            Nr_remaining_black_moves += time;
          }
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpromoted_pawn(stip_length_type n,
                            square where_to_place,
                            Flags blocker_flags,
                            square blocker_comes_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_place);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  if (!black_pawn_attacks_king(where_to_place))
  {
    unsigned int const nr_required_captures = abs(blocker_comes_from%onerow
                                                  - where_to_place%onerow);
    unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                                      blocker_comes_from,
                                                                                      where_to_place);
    if (time<=Nr_remaining_black_moves
        && nr_required_captures<=Nr_unused_white_masses)
    {
      Nr_unused_white_masses -= nr_required_captures;
      Nr_remaining_black_moves -= time;
      TraceValue("%u",Nr_unused_white_masses);
      TraceValue("%u\n",Nr_remaining_black_moves);
      SetPiece(pn,where_to_place,blocker_flags);
      intelligent_stalemate_test_target_position(n);
      Nr_remaining_black_moves += time;
      Nr_unused_white_masses += nr_required_captures;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void officer(stip_length_type n,
                    square where_to_place,
                    piece blocker_type,
                    Flags blocker_flags,
                    square blocker_comes_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_place);
  TracePiece(blocker_type);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(White,where_to_place,blocker_type))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(blocker_type,
                                                                             blocker_comes_from,
                                                                             blocker_type,
                                                                             where_to_place);
    if (time<=Nr_remaining_black_moves)
    {
      unsigned int const nr_checks_to_black = 0;
      Nr_remaining_black_moves -= time;
      TraceValue("%u\n",Nr_remaining_black_moves);
      SetPiece(blocker_type,where_to_place,blocker_flags);
      intelligent_stalemate_continue_after_block(n,
                                                 White,
                                                 where_to_place,
                                                 blocker_type,
                                                 nr_checks_to_black);
      Nr_remaining_black_moves += time;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_some_piece_on(stip_length_type n, square where_to_place)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_place);
  TraceFunctionParamListEnd();

  if (Nr_unused_black_masses>=1)
  {
    --Nr_unused_black_masses;
    TraceValue("%u\n",Nr_unused_black_masses);

    for (i = 1; i<MaxPiece[Black]; ++i)
      if (black[i].usage==piece_is_unused)
      {
        piece const blocker_type = black[i].type;
        Flags const blocker_flags = black[i].flags;
        square const blocker_comes_from = black[i].diagram_square;

        black[i].usage = piece_blocks;

        if (blocker_type==pn)
        {
          promoted_pawn(n,where_to_place,blocker_flags,blocker_comes_from);
          if (where_to_place>=square_a2)
            unpromoted_pawn(n,where_to_place,blocker_flags,blocker_comes_from);
        }
        else
          officer(n,where_to_place,blocker_type,blocker_flags,blocker_comes_from);

        black[i].usage = piece_is_unused;
      }

    e[where_to_place] = vide;
    spec[where_to_place] = EmptySpec;

    ++Nr_unused_black_masses;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_some_piece(stip_length_type n)
{
  square const *bnp;
  square const * const save_start = where_to_start_placing_unused_black_pieces;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  for (bnp = where_to_start_placing_unused_black_pieces; *bnp; ++bnp)
    if (e[*bnp]==vide && nr_reasons_for_staying_empty[*bnp]==0)
    {
      where_to_start_placing_unused_black_pieces = bnp;
      place_some_piece_on(n,*bnp);
    }

  where_to_start_placing_unused_black_pieces = save_start;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void fix_white_king_on_diagram_square(stip_length_type n)
{
  TraceFunctionEntry(__func__);
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
      intelligent_stalemate_intercept_checks(n,nr_of_checks_to_black,White);
    }
    else
      intelligent_stalemate_test_target_position(n);

    e[king_square[White]] = vide;
    spec[king_square[White]] = EmptySpec;
    king_square[White] = initsquare;

    white[index_of_king].usage = piece_is_unused;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_deal_with_unused_pieces(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (!hasMaxtimeElapsed())
  {
    if (white[index_of_king].usage==piece_is_unused
        && white[index_of_king].diagram_square!=square_e1
        && Nr_remaining_white_moves==0)
    {
      if (e[white[index_of_king].diagram_square]==vide
          && nr_reasons_for_staying_empty[white[index_of_king].diagram_square]==0)
        fix_white_king_on_diagram_square(n);
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
        place_some_piece(n);

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
