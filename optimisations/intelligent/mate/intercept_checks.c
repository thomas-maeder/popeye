#include "optimisations/intelligent/mate/intercept_checks.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/mate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void continue_intercepting_checks(unsigned int nr_remaining_white_moves,
                                         unsigned int nr_remaining_black_moves,
                                         unsigned int max_nr_allowed_captures_by_white,
                                         unsigned int max_nr_allowed_captures_by_black,
                                         stip_length_type n,
                                         int const check_directions[8],
                                         unsigned int nr_of_check_directions);

static void with_unpromoted_white_pawn(unsigned int nr_remaining_white_moves,
                                       unsigned int nr_remaining_black_moves,
                                       unsigned int max_nr_allowed_captures_by_white,
                                       unsigned int max_nr_allowed_captures_by_black,
                                       stip_length_type n,
                                       unsigned int placed_index,
                                       square placed_on,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions)
{
  square const placed_from = white[placed_index].diagram_square;
  unsigned int const diffcol = abs(placed_from%onerow - placed_on%onerow);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (diffcol<=max_nr_allowed_captures_by_white
      && !uninterceptably_attacks_king(Black,placed_on,pb))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                                      placed_from,
                                                                                      placed_on);
    if (time<=nr_remaining_white_moves)
    {
      SetPiece(pb,placed_on,white[placed_index].flags);
      continue_intercepting_checks(nr_remaining_white_moves-time,
                                   nr_remaining_black_moves,
                                   max_nr_allowed_captures_by_white-diffcol,
                                   max_nr_allowed_captures_by_black,
                                   n,
                                   check_directions,
                                   nr_of_check_directions);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_promoted_white_pawn(unsigned int nr_remaining_white_moves,
                                     unsigned int nr_remaining_black_moves,
                                     unsigned int max_nr_allowed_captures_by_white,
                                     unsigned int max_nr_allowed_captures_by_black,
                                     stip_length_type n,
                                     unsigned int placed_index,
                                     square placed_on,
                                     int const check_directions[8],
                                     unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  {
    /* A rough check whether it is worth thinking about promotions */
    unsigned int time = moves_to_white_prom[placed_index];
    if (placed_on<=square_h7)
      /* square is not on 8th rank -- 1 move necessary to get there */
      ++time;

    if (time<=nr_remaining_white_moves)
    {
      square const placed_from = white[placed_index].diagram_square;
      piece pp;
      for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
        if (!uninterceptably_attacks_king(Black,placed_on,pp))
        {
          unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(placed_from,
                                                                                         pp,
                                                                                         placed_on);
          unsigned int diffcol;
          if (pp==fb && SquareCol(placed_on)==SquareCol(placed_from%onerow))
            diffcol= 1;
          else
            diffcol= 0;
          TracePiece(pp);
          TraceValue("%u",diffcol);
          TraceValue("%u\n",time);

          if (diffcol<=max_nr_allowed_captures_by_white
              && time<=nr_remaining_white_moves)
          {
            SetPiece(pp,placed_on,white[placed_index].flags);
            continue_intercepting_checks(nr_remaining_white_moves-time,
                                         nr_remaining_black_moves,
                                         max_nr_allowed_captures_by_white-diffcol,
                                         max_nr_allowed_captures_by_black,
                                         n,
                                         check_directions,
                                         nr_of_check_directions);
          }
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_white_officer(unsigned int nr_remaining_white_moves,
                               unsigned int nr_remaining_black_moves,
                               unsigned int max_nr_allowed_captures_by_white,
                               unsigned int max_nr_allowed_captures_by_black,
                               stip_length_type n,
                               unsigned int placed_index,
                               piece placed_type, square placed_on,
                               int const check_directions[8],
                               unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,placed_on,placed_type))
  {
    square const placed_from = white[placed_index].diagram_square;
    unsigned int const time= intelligent_count_nr_of_moves_from_to_no_check(placed_type,
                                                                            placed_from,
                                                                            placed_type,
                                                                            placed_on);
    if (time<=nr_remaining_white_moves)
    {
      Flags const placed_flags = white[placed_index].flags;
      SetPiece(placed_type,placed_on,placed_flags);
      continue_intercepting_checks(nr_remaining_white_moves-time,
                                   nr_remaining_black_moves,
                                   max_nr_allowed_captures_by_white,
                                   max_nr_allowed_captures_by_black,
                                   n,
                                   check_directions,
                                   nr_of_check_directions);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_check_white(unsigned int nr_remaining_white_moves,
                                  unsigned int nr_remaining_black_moves,
                                  unsigned int max_nr_allowed_captures_by_white,
                                  unsigned int max_nr_allowed_captures_by_black,
                                  stip_length_type n,
                                  square placed_on,
                                  int const check_directions[8],
                                  unsigned int nr_of_check_directions)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  for (placed_index = 1; placed_index<MaxPiece[White]; ++placed_index)
    if (white[placed_index].usage==piece_is_unused)
    {
      piece const placed_type = white[placed_index].type;

      white[placed_index].usage = piece_intercepts;

      if (placed_type==pb)
      {
        if (placed_on<=square_h7)
          with_unpromoted_white_pawn(nr_remaining_white_moves,
                                     nr_remaining_black_moves,
                                     max_nr_allowed_captures_by_white,
                                     max_nr_allowed_captures_by_black,
                                     n,
                                     placed_index,placed_on,
                                     check_directions,
                                     nr_of_check_directions);
        with_promoted_white_pawn(nr_remaining_white_moves,
                                 nr_remaining_black_moves,
                                 max_nr_allowed_captures_by_white,
                                 max_nr_allowed_captures_by_black,
                                 n,
                                 placed_index,placed_on,
                                 check_directions,
                                 nr_of_check_directions);
      }
      else
        with_white_officer(nr_remaining_white_moves,
                           nr_remaining_black_moves,
                           max_nr_allowed_captures_by_white,
                           max_nr_allowed_captures_by_black,
                           n,
                           placed_index,placed_type,placed_on,
                           check_directions,
                           nr_of_check_directions);

      white[placed_index].usage = piece_is_unused;
    }

  e[placed_on]= vide;
  spec[placed_on]= EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_promoted_black_pawn(unsigned int nr_remaining_white_moves,
                                     unsigned int nr_remaining_black_moves,
                                     unsigned int max_nr_allowed_captures_by_white,
                                     unsigned int max_nr_allowed_captures_by_black,
                                     stip_length_type n,
                                     unsigned int placed_index,
                                     square placed_on,
                                     int const check_directions[8],
                                     unsigned int nr_of_check_directions)
{
  square const placed_from = black[placed_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
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

    if (time<=nr_remaining_black_moves)
    {
      piece pp;
      for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
        if (!guards(king_square[White],pp,placed_on))
        {
          unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(placed_from,
                                                                                         pp,
                                                                                         placed_on);
          unsigned int diffcol = 0;
          if (pp==fn)
          {
            unsigned int const placed_from_file = placed_from%nr_files_on_board;
            square const promotion_square_on_same_file = square_a1+placed_from_file;
            if (SquareCol(placed_on)!=SquareCol(promotion_square_on_same_file))
              diffcol = 1;
          }

          if (diffcol<=max_nr_allowed_captures_by_black
              && time<=nr_remaining_black_moves)
          {
            SetPiece(pp,placed_on,black[placed_index].flags);
            continue_intercepting_checks(nr_remaining_white_moves,
                                         nr_remaining_black_moves-time,
                                         max_nr_allowed_captures_by_white,
                                         max_nr_allowed_captures_by_black-diffcol,
                                         n,
                                         check_directions,
                                         nr_of_check_directions);
          }
        }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_unpromoted_black_pawn(unsigned int nr_remaining_white_moves,
                                       unsigned int nr_remaining_black_moves,
                                       unsigned int max_nr_allowed_captures_by_white,
                                       unsigned int max_nr_allowed_captures_by_black,
                                       stip_length_type n,
                                       unsigned int placed_index,
                                       square placed_on,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,placed_on,pn))
  {
    square const placed_from = black[placed_index].diagram_square;
    unsigned int const diffcol = abs(placed_from%onerow - placed_on%onerow);
    if (diffcol<=max_nr_allowed_captures_by_black)
    {
      unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                                        placed_from,
                                                                                        placed_on);
      if (time<=nr_remaining_black_moves)
      {
        SetPiece(pn,placed_on,black[placed_index].flags);
        continue_intercepting_checks(nr_remaining_white_moves,
                                     nr_remaining_black_moves-time,
                                     max_nr_allowed_captures_by_white,
                                     max_nr_allowed_captures_by_black-diffcol,
                                     n,
                                     check_directions,
                                     nr_of_check_directions);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_black_officer(unsigned int nr_remaining_white_moves,
                               unsigned int nr_remaining_black_moves,
                               unsigned int max_nr_allowed_captures_by_white,
                               unsigned int max_nr_allowed_captures_by_black,
                               stip_length_type n,
                               unsigned int placed_index,
                               piece placed_type, square placed_on,
                               int const check_directions[8],
                               unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (!guards(king_square[White],placed_type,placed_on))
  {
    square const placed_from = black[placed_index].diagram_square;
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(placed_type,
                                                                             placed_from,
                                                                             placed_type,
                                                                             placed_on);
    if (time<=nr_remaining_black_moves)
    {
      SetPiece(placed_type,placed_on,black[placed_index].flags);
      continue_intercepting_checks(nr_remaining_white_moves,
                                   nr_remaining_black_moves-time,
                                   max_nr_allowed_captures_by_white,
                                   max_nr_allowed_captures_by_black,
                                   n,
                                   check_directions,
                                   nr_of_check_directions);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_check_black(unsigned int nr_remaining_white_moves,
                                  unsigned int nr_remaining_black_moves,
                                  unsigned int max_nr_allowed_captures_by_white,
                                  unsigned int max_nr_allowed_captures_by_black,
                                  stip_length_type n,
                                  square placed_on,
                                  int const check_directions[8],
                                  unsigned int nr_of_check_directions)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  for (placed_index = 1; placed_index<MaxPiece[Black]; ++placed_index)
    if (black[placed_index].usage==piece_is_unused)
    {
      piece const placed_type = black[placed_index].type;

      black[placed_index].usage = piece_intercepts;

      if (placed_type==pn)
      {
        if (placed_on>=square_a2)
          with_unpromoted_black_pawn(nr_remaining_white_moves,
                                     nr_remaining_black_moves,
                                     max_nr_allowed_captures_by_white,
                                     max_nr_allowed_captures_by_black,
                                     n,
                                     placed_index,placed_on,
                                     check_directions,
                                     nr_of_check_directions);
        with_promoted_black_pawn(nr_remaining_white_moves,
                                 nr_remaining_black_moves,
                                 max_nr_allowed_captures_by_white,
                                 max_nr_allowed_captures_by_black,
                                 n,
                                 placed_index,placed_on,
                                 check_directions,
                                 nr_of_check_directions);
      }
      else
        with_black_officer(nr_remaining_white_moves,
                           nr_remaining_black_moves,
                           max_nr_allowed_captures_by_white,
                           max_nr_allowed_captures_by_black,
                           n,
                           placed_index,placed_type,placed_on,
                           check_directions,
                           nr_of_check_directions);

      black[placed_index].usage = piece_is_unused;
    }

  e[placed_on] = vide;
  spec[placed_on] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_next(unsigned int nr_remaining_white_moves,
                           unsigned int nr_remaining_black_moves,
                           unsigned int max_nr_allowed_captures_by_white,
                           unsigned int max_nr_allowed_captures_by_black,
                           stip_length_type n,
                           int const check_directions[8],
                           unsigned int nr_of_check_directions)
{
  square to_be_blocked;
  int const current_dir = check_directions[nr_of_check_directions-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  assert(nr_of_check_directions>0);
  TraceValue("%d\n",current_dir);

  for (to_be_blocked = king_square[White]+current_dir;
       e[to_be_blocked]==vide;
       to_be_blocked += current_dir)
    if (nr_reasons_for_staying_empty[to_be_blocked]==0)
    {
      intercept_check_black(nr_remaining_white_moves,
                            nr_remaining_black_moves,
                            max_nr_allowed_captures_by_white,
                            max_nr_allowed_captures_by_black,
                            n,
                            to_be_blocked,
                            check_directions,
                            nr_of_check_directions-1);
      intercept_check_white(nr_remaining_white_moves,
                            nr_remaining_black_moves,
                            max_nr_allowed_captures_by_white,
                            max_nr_allowed_captures_by_black,
                            n,
                            to_be_blocked,
                            check_directions,
                            nr_of_check_directions-1);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void continue_intercepting_checks(unsigned int nr_remaining_white_moves,
                                         unsigned int nr_remaining_black_moves,
                                         unsigned int max_nr_allowed_captures_by_white,
                                         unsigned int max_nr_allowed_captures_by_black,
                                         stip_length_type n,
                                         int const check_directions[8],
                                         unsigned int nr_of_check_directions)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (nr_of_check_directions==0)
    intelligent_mate_test_target_position(nr_remaining_white_moves,
                                          nr_remaining_black_moves,
                                          max_nr_allowed_captures_by_white,
                                          max_nr_allowed_captures_by_black,
                                          n);
  else
    intercept_next(nr_remaining_white_moves,
                   nr_remaining_black_moves,
                   max_nr_allowed_captures_by_white,
                   max_nr_allowed_captures_by_black,
                   n,
                   check_directions,
                   nr_of_check_directions);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_intercept_checks(unsigned int nr_remaining_white_moves,
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

  {
    int check_directions[8];
    unsigned int const nr_of_check_directions = find_check_directions(White,
                                                                      check_directions);

    continue_intercepting_checks(nr_remaining_white_moves,
                                 nr_remaining_black_moves,
                                 max_nr_allowed_captures_by_white,
                                 max_nr_allowed_captures_by_black,
                                 n,
                                 check_directions,
                                 nr_of_check_directions);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
