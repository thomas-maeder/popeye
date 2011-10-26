#include "optimisations/intelligent/mate/intercept_checks.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/mate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void continue_intercepting_checks(stip_length_type n,
                                         int const check_directions[8],
                                         unsigned int nr_of_check_directions);

static void with_unpromoted_white_pawn(stip_length_type n,
                                       unsigned int placed_index,
                                       square placed_on,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (!white_pawn_attacks_king_region(placed_on,0)
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(white[placed_index].diagram_square,
                                                                   placed_on))
  {
    SetPiece(pb,placed_on,white[placed_index].flags);
    continue_intercepting_checks(n,check_directions,nr_of_check_directions);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_promoted_white_pawn(stip_length_type n,
                                     unsigned int placed_index,
                                     square placed_on,
                                     int const check_directions[8],
                                     unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    square const placed_from = white[placed_index].diagram_square;
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!officer_uninterceptably_attacks_king(Black,placed_on,pp)
          && intelligent_reserve_promoting_white_pawn_moves_from_to(placed_from,
                                                                    pp,
                                                                    placed_on))
      {
        SetPiece(pp,placed_on,white[placed_index].flags);
        continue_intercepting_checks(n,
                                     check_directions,
                                     nr_of_check_directions);
        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_white_officer(stip_length_type n,
                               unsigned int placed_index,
                               piece placed_type, square placed_on,
                               int const check_directions[8],
                               unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TracePiece(placed_type);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(Black,placed_on,placed_type)
      && intelligent_reserve_officer_moves_from_to(white[placed_index].diagram_square,
                                                   placed_on,
                                                   placed_type))
  {
    SetPiece(placed_type,placed_on,white[placed_index].flags);
    continue_intercepting_checks(n,check_directions,nr_of_check_directions);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_check_white(stip_length_type n,
                                  square placed_on,
                                  int const check_directions[8],
                                  unsigned int nr_of_check_directions)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1))
  {
    for (placed_index = 1; placed_index<MaxPiece[White]; ++placed_index)
      if (white[placed_index].usage==piece_is_unused)
      {
        piece const placed_type = white[placed_index].type;

        white[placed_index].usage = piece_intercepts;

        if (placed_type==pb)
        {
          if (placed_on>=square_a2 && placed_on<=square_h7)
            with_unpromoted_white_pawn(n,
                                       placed_index,placed_on,
                                       check_directions,
                                       nr_of_check_directions);
          with_promoted_white_pawn(n,
                                   placed_index,placed_on,
                                   check_directions,
                                   nr_of_check_directions);
        }
        else
          with_white_officer(n,
                             placed_index,placed_type,placed_on,
                             check_directions,
                             nr_of_check_directions);

        white[placed_index].usage = piece_is_unused;
      }

    e[placed_on]= vide;
    spec[placed_on]= EmptySpec;
    intelligent_unreserve();
}

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_promoted_black_pawn(stip_length_type n,
                                     unsigned int placed_index,
                                     square placed_on,
                                     int const check_directions[8],
                                     unsigned int nr_of_check_directions,
                                     boolean is_diagonal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(placed_index,
                                                                placed_on))
  {
    square const placed_from = black[placed_index].diagram_square;

    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      switch (pp)
      {
        case dn:
          break;

        case tn:
          if (is_diagonal
              && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_from,
                                                                        tn,
                                                                        placed_on))
          {
            SetPiece(tn,placed_on,black[placed_index].flags);
            continue_intercepting_checks(n,
                                         check_directions,
                                         nr_of_check_directions);
            intelligent_unreserve();
          }
          break;

        case fn:
          if (!is_diagonal
              && intelligent_reserve_promoting_black_pawn_moves_from_to(placed_from,
                                                                        fn,
                                                                        placed_on))
          {
            SetPiece(fn,placed_on,black[placed_index].flags);
            continue_intercepting_checks(n,
                                         check_directions,
                                         nr_of_check_directions);
            intelligent_unreserve();
          }
          break;

        case cn:
          if (intelligent_reserve_promoting_black_pawn_moves_from_to(placed_from,
                                                                     cn,
                                                                     placed_on))
          {
            SetPiece(cn,placed_on,black[placed_index].flags);
            continue_intercepting_checks(n,
                                         check_directions,
                                         nr_of_check_directions);
            intelligent_unreserve();
          }
          break;

        default:
          assert(0);
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_unpromoted_black_pawn(stip_length_type n,
                                       unsigned int placed_index,
                                       square placed_on,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (!black_pawn_attacks_king(placed_on)
      && intelligent_reserve_black_pawn_moves_from_to_no_promotion(black[placed_index].diagram_square,
                                                                   placed_on))
  {
    SetPiece(pn,placed_on,black[placed_index].flags);
    continue_intercepting_checks(n,
                                 check_directions,
                                 nr_of_check_directions);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_black_officer(stip_length_type n,
                               unsigned int placed_index,
                               square placed_on,
                               int const check_directions[8],
                               unsigned int nr_of_check_directions)
{
  piece const placed_type = black[placed_index].type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",placed_index);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  assert(!officer_guards(king_square[White],placed_type,placed_on));
  if (intelligent_reserve_officer_moves_from_to(black[placed_index].diagram_square,
                                                placed_on,
                                                placed_type))
  {
    SetPiece(placed_type,placed_on,black[placed_index].flags);
    continue_intercepting_checks(n,
                                 check_directions,
                                 nr_of_check_directions);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_check_black(stip_length_type n,
                                  square placed_on,
                                  int const check_directions[8],
                                  unsigned int nr_of_check_directions,
                                  boolean is_diagonal)
{
  unsigned int placed_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(placed_on);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1))
  {
    for (placed_index = 1; placed_index<MaxPiece[Black]; ++placed_index)
      if (black[placed_index].usage==piece_is_unused)
      {
        black[placed_index].usage = piece_intercepts;

        switch (black[placed_index].type)
        {
          case pn:
            if (placed_on>=square_a2 && placed_on<=square_h7)
              with_unpromoted_black_pawn(n,
                                         placed_index,placed_on,
                                         check_directions,
                                         nr_of_check_directions);
            with_promoted_black_pawn(n,
                                     placed_index,placed_on,
                                     check_directions,
                                     nr_of_check_directions,
                                     is_diagonal);
            break;

          case dn:
            break;

          case tn:
            if (is_diagonal)
              with_black_officer(n,
                                 placed_index,placed_on,
                                 check_directions,
                                 nr_of_check_directions);
            break;

          case fn:
            if (!is_diagonal)
              with_black_officer(n,
                                 placed_index,placed_on,
                                 check_directions,
                                 nr_of_check_directions);
            break;

          case cn:
            with_black_officer(n,
                               placed_index,placed_on,
                               check_directions,
                               nr_of_check_directions);
            break;

          default:
            assert(0);
            break;
        }

        black[placed_index].usage = piece_is_unused;
      }

    e[placed_on] = vide;
    spec[placed_on] = EmptySpec;
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_next(stip_length_type n,
                           int const check_directions[8],
                           unsigned int nr_of_check_directions)
{
  square to_be_blocked;
  int const current_dir = check_directions[nr_of_check_directions-1];
  square const start = king_square[White]+current_dir;
  boolean const is_diagonal = SquareCol(king_square[White])==SquareCol(start);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  assert(nr_of_check_directions>0);
  TraceValue("%d\n",current_dir);

  for (to_be_blocked = start;
       e[to_be_blocked]==vide;
       to_be_blocked += current_dir)
    if (nr_reasons_for_staying_empty[to_be_blocked]==0)
    {
      intercept_check_black(n,
                            to_be_blocked,
                            check_directions,
                            nr_of_check_directions-1,
                            is_diagonal);
      intercept_check_white(n,
                            to_be_blocked,
                            check_directions,
                            nr_of_check_directions-1);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void continue_intercepting_checks(stip_length_type n,
                                         int const check_directions[8],
                                         unsigned int nr_of_check_directions)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParamListEnd();

  if (nr_of_check_directions==0)
    intelligent_mate_test_target_position(n);
  else
    intercept_next(n,check_directions,nr_of_check_directions);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_mate_intercept_checks(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    int check_directions[8];
    unsigned int const nr_of_check_directions = find_check_directions(White,
                                                                      check_directions);

    continue_intercepting_checks(n,check_directions,nr_of_check_directions);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
