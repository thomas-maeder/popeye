#include "optimisations/intelligent/mate/place_black_piece.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/mate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void promoted_black_pawn(stip_length_type n,
                                unsigned int placed_index,
                                square placed_on)
{
  square const placed_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int time = (placed_from>=square_a7
                         ? 5
                         : placed_from/onerow - nr_of_slack_rows_below_board);
    assert(time<=5);

    if (placed_on>=square_a2)
      /* square is not on 1st rank -- 1 move necessary to get there */
      ++time;

    if (time<=Nr_remaining_black_moves)
    {
      piece pp;
      for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
        if (!uninterceptably_attacks_king(White,placed_on,pp))
        {
          unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(placed_from,
                                                                                         pp,
                                                                                         placed_on);
          if (time<=Nr_remaining_black_moves)
          {
            unsigned int diffcol = 0;
            if (pp==fn)
            {
              unsigned int const placed_from_file = placed_from%nr_files_on_board;
              square const promotion_square_on_same_file = square_a1+placed_from_file;
              if (SquareCol(placed_on)!=SquareCol(promotion_square_on_same_file))
                diffcol = 1;
            }

            if (diffcol<=Nr_unused_white_masses)
            {
              unsigned int const nr_of_checks_to_white = guards(king_square[White],
                                                                pp,
                                                                placed_on);
              Nr_unused_white_masses -= diffcol;
              Nr_remaining_black_moves -= time;
              TraceValue("%u\n",Nr_remaining_black_moves);
              SetPiece(pp,placed_on,black[placed_index].flags);
              intelligent_mate_finish(n,nr_of_checks_to_white);
              Nr_remaining_black_moves += time;
              Nr_unused_white_masses += diffcol;
            }
          }
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpromoted_black_pawn(stip_length_type n,
                                  unsigned int placed_index,
                                  square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,placed_on,pn))
  {
    square const placed_from = black[placed_index].diagram_square;
    unsigned int const diffcol = abs(placed_from%onerow - placed_on%onerow);
    if (diffcol<=Nr_unused_white_masses)
    {
      unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                                        placed_from,
                                                                                        placed_on);
      if (time<=Nr_remaining_black_moves)
      {
        Nr_unused_white_masses -= diffcol;
        Nr_remaining_black_moves -= time;
        TraceValue("%u\n",Nr_remaining_black_moves);
        SetPiece(pn,placed_on,black[placed_index].flags);
        intelligent_mate_test_target_position(n);
        Nr_remaining_black_moves += time;
        Nr_unused_white_masses += diffcol;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void black_officer(stip_length_type n,
                          unsigned int placed_index,
                          piece placed_type, square placed_on)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,placed_on,placed_type))
  {
    square const placed_from = black[placed_index].diagram_square;
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(placed_type,
                                                                             placed_from,
                                                                             placed_type,
                                                                             placed_on);
    if (time<=Nr_remaining_black_moves)
    {
      unsigned int const nr_of_checks_to_white = guards(king_square[White],
                                                        placed_type,
                                                        placed_on);
      Nr_remaining_black_moves -= time;
      TraceValue("%u\n",Nr_remaining_black_moves);
      SetPiece(placed_type,placed_on,black[placed_index].flags);
      intelligent_mate_finish(n,nr_of_checks_to_white);
      Nr_remaining_black_moves += time;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_place_any_black_piece_on(stip_length_type n,
                                               square placed_on)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParamListEnd();

  for (placed_index = 1; placed_index<MaxPiece[Black]; ++placed_index)
    if (black[placed_index].usage==piece_is_unused)
    {
      piece const placed_type = black[placed_index].type;

      black[placed_index].usage = piece_intercepts;

      if (placed_type==pn)
      {
        if (placed_on>=square_a2)
          unpromoted_black_pawn(n,placed_index,placed_on);
        promoted_black_pawn(n,placed_index,placed_on);
      }
      else
        black_officer(n,placed_index,placed_type,placed_on);

      black[placed_index].usage = piece_is_unused;
    }

  e[placed_on] = vide;
  spec[placed_on] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
