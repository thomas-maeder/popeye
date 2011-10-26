#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "pyint.h"
#include "pydata.h"
#include "pyslice.h"
#include "platform/maxtime.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/intercept_checks_to_white.h"
#include "optimisations/intelligent/stalemate/black_block.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void continue_after_placement(stip_length_type n,
                                     square to_be_blocked,
                                     piece placed_type,
                                     unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TracePiece(placed_type);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (king_square[White]!=initsquare
      && officer_guards(king_square[White],placed_type,to_be_blocked))
  {
    unsigned int const nr_checks_to_black = 0;
    intelligent_stalemate_intercept_checks_to_white(n,nr_checks_to_black);
  }
  else
    intelligent_stalemate_test_target_position(n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void promoted_pawn(stip_length_type n,
                          square where_to_place,
                          unsigned int placed_index)
{
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_place);
  TraceFunctionParam("%u",placed_index);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(placed_index,
                                                                where_to_place))
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!officer_uninterceptably_attacks_king(White,where_to_place,pp)
          && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_comes_from,
                                                                    pp,
                                                                    where_to_place))
      {
        unsigned int const nr_checks_to_black = 0;
        SetPiece(pp,where_to_place,placed_flags);
        continue_after_placement(n,where_to_place,pp,nr_checks_to_black);
        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpromoted_pawn(stip_length_type n,
                            square where_to_place,
                            unsigned int placed_index)
{
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_place);
  TraceFunctionParam("%u",placed_index);
  TraceFunctionParamListEnd();

  if (!black_pawn_attacks_king(where_to_place)
      && intelligent_reserve_black_pawn_moves_from_to_no_promotion(placed_comes_from,
                                                                   where_to_place))
  {
    SetPiece(pn,where_to_place,placed_flags);
    intelligent_stalemate_test_target_position(n);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void officer(stip_length_type n,
                    square where_to_place,
                    unsigned int placed_index)
{
  piece const placed_type = black[placed_index].type;
  Flags const placed_flags = black[placed_index].flags;
  square const placed_comes_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_place);
  TraceFunctionParam("%u",placed_index);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(White,where_to_place,placed_type)
      && intelligent_reserve_officer_moves_from_to(placed_comes_from,
                                                   where_to_place,
                                                   placed_type))
  {
    unsigned int const nr_checks_to_black = 0;
    SetPiece(placed_type,where_to_place,placed_flags);
    continue_after_placement(n,where_to_place,placed_type,nr_checks_to_black);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_some_piece_on(stip_length_type n, square where_to_place)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_place);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1))
  {
    unsigned int i;
    for (i = 1; i<MaxPiece[Black]; ++i)
      if (black[i].usage==piece_is_unused)
      {
        black[i].usage = piece_blocks;

        if (black[i].type==pn)
        {
          promoted_pawn(n,where_to_place,i);
          if (where_to_place>=square_a2 && where_to_place<=square_h7)
            unpromoted_pawn(n,where_to_place,i);
        }
        else
          officer(n,where_to_place,i);

        black[i].usage = piece_is_unused;
      }

    e[where_to_place] = vide;
    spec[where_to_place] = EmptySpec;

    intelligent_unreserve();
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
      intelligent_stalemate_intercept_checks_to_white(n,nr_of_checks_to_black);
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
        && intelligent_get_nr_remaining_moves(White)==0)
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
