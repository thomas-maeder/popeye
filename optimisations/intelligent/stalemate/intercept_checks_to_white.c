#include "optimisations/intelligent/stalemate/intercept_checks_to_white.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/intercept_checks_to_black.h"
#include "optimisations/intelligent/stalemate/deal_with_unused_pieces.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void continue_intercepting_checks(stip_length_type n,
                                                   int const check_directions[8],
                                                   unsigned int nr_of_check_directions,
                                                   unsigned int nr_checks_to_black);

static void with_promoted_black_pawn(stip_length_type n,
                                     square where_to_intercept,
                                     unsigned int intercepter_index,
                                     int const check_directions[8],
                                     unsigned int nr_of_check_directions,
                                     unsigned int nr_checks_to_black)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",intercepter_index);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(intercepter_index,
                                                                where_to_intercept))
  {
    Flags const intercepter_flags = black[intercepter_index].flags;
    square const intercepter_comes_from = black[intercepter_index].diagram_square;

    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!(king_square[White]!=initsquare
            && officer_guards(king_square[White],pp,where_to_intercept))
          && intelligent_reserve_promoting_black_pawn_moves_from_to(intercepter_comes_from,
                                                                    pp,
                                                                    where_to_intercept))
      {
        SetPiece(pp,where_to_intercept,intercepter_flags);
        continue_intercepting_checks(n,
                                     check_directions,
                                     nr_of_check_directions,
                                     nr_checks_to_black);
        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_unpromoted_black_pawn(stip_length_type n,
                                       square where_to_intercept,
                                       unsigned int intercepter_index,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions,
                                       unsigned int nr_checks_to_black)
{
  Flags const intercepter_flags = black[intercepter_index].flags;
  square const intercepter_comes_from = black[intercepter_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",intercepter_index);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  if (!black_pawn_attacks_king(where_to_intercept)
      && intelligent_reserve_black_pawn_moves_from_to_no_promotion(intercepter_comes_from,
                                                                   where_to_intercept))
  {
    SetPiece(pn,where_to_intercept,intercepter_flags);
    continue_intercepting_checks(n,
                                 check_directions,
                                 nr_of_check_directions,
                                 nr_checks_to_black);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_black_rider(stip_length_type n,
                             square where_to_intercept,
                             unsigned int intercepter_index,
                             int const check_directions[8],
                             unsigned int nr_of_check_directions,
                             unsigned int nr_checks_to_black)
{
  piece const intercepter_type = black[intercepter_index].type;
  Flags const intercepter_flags = black[intercepter_index].flags;
  square const intercepter_comes_from = black[intercepter_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",intercepter_index);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  assert(!officer_guards(king_square[White],intercepter_type,where_to_intercept));
  if (intelligent_reserve_officer_moves_from_to(intercepter_comes_from,
                                                where_to_intercept,
                                                intercepter_type))
  {
    SetPiece(intercepter_type,where_to_intercept,intercepter_flags);
    continue_intercepting_checks(n,
                                 check_directions,
                                 nr_of_check_directions,
                                 nr_checks_to_black);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_black_knight(stip_length_type n,
                              square where_to_intercept,
                              unsigned int intercepter_index,
                              int const check_directions[8],
                              unsigned int nr_of_check_directions,
                              unsigned int nr_checks_to_black)
{
  Flags const intercepter_flags = black[intercepter_index].flags;
  square const intercepter_comes_from = black[intercepter_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",intercepter_index);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  if (!knight_guards(king_square[White],where_to_intercept)
      && intelligent_reserve_officer_moves_from_to(intercepter_comes_from,
                                                   where_to_intercept,
                                                   cn))
  {
    SetPiece(cn,where_to_intercept,intercepter_flags);
    continue_intercepting_checks(n,
                                 check_directions,
                                 nr_of_check_directions,
                                 nr_checks_to_black);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_black_piece(stip_length_type n,
                             square where_to_intercept,
                             int const check_directions[8],
                             unsigned int nr_of_check_directions,
                             unsigned int nr_checks_to_black,
                             boolean is_diagonal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParam("%u",is_diagonal);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1))
  {
    unsigned int i;
    for (i = 1; i<MaxPiece[Black]; ++i)
      if (black[i].usage==piece_is_unused)
      {
        black[i].usage = piece_intercepts;

        switch (black[i].type)
        {
          case pn:
            with_promoted_black_pawn(n,
                                     where_to_intercept,
                                     i,
                                     check_directions,
                                     nr_of_check_directions,
                                     nr_checks_to_black);
            if (where_to_intercept>=square_a2 && where_to_intercept<=square_h7)
              with_unpromoted_black_pawn(n,
                                         where_to_intercept,
                                         i,
                                         check_directions,
                                         nr_of_check_directions,
                                         nr_checks_to_black);
            break;

          case dn:
            break;

          case tn:
            if (is_diagonal)
              with_black_rider(n,
                               where_to_intercept,
                               i,
                               check_directions,
                               nr_of_check_directions,
                               nr_checks_to_black);
            break;

          case fn:
            if (!is_diagonal)
              with_black_rider(n,
                               where_to_intercept,
                               i,
                               check_directions,
                               nr_of_check_directions,
                               nr_checks_to_black);
            break;

          case cn:
            with_black_knight(n,
                              where_to_intercept,
                              i,
                              check_directions,
                              nr_of_check_directions,
                              nr_checks_to_black);
            break;

          default:
            assert(0);
            break;
        }

        black[i].usage = piece_is_unused;
      }

    e[where_to_intercept] = vide;
    spec[where_to_intercept] = EmptySpec;

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_unpromoted_white_pawn(stip_length_type n,
                                       unsigned int blocker_index,
                                       square where_to_intercept,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions,
                                       unsigned int nr_checks_to_black)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  if (!white_pawn_attacks_king_region(where_to_intercept,0)
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(white[blocker_index].diagram_square,
                                                                   where_to_intercept))
  {
    SetPiece(pb,where_to_intercept,white[blocker_index].flags);
    continue_intercepting_checks(n,
                                 check_directions,
                                 nr_of_check_directions,
                                 nr_checks_to_black);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_promoted_white_pawn(stip_length_type n,
                                     unsigned int blocker_index,
                                     square where_to_intercept,
                                     int const check_directions[8],
                                     unsigned int nr_of_check_directions,
                                     unsigned int nr_checks_to_black)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(blocker_index,
                                                                where_to_intercept))
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!officer_uninterceptably_attacks_king(Black,where_to_intercept,pp)
          && intelligent_reserve_promoting_white_pawn_moves_from_to(white[blocker_index].diagram_square,
                                                                    pp,
                                                                    where_to_intercept))
      {
        if (officer_guards(king_square[Black],pp,where_to_intercept))
          ++nr_checks_to_black;
        SetPiece(pp,where_to_intercept,white[blocker_index].flags);
        continue_intercepting_checks(n,
                                     check_directions,
                                     nr_of_check_directions,
                                     nr_checks_to_black);

        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_white_officer(stip_length_type n,
                               unsigned int blocker_index,
                               square where_to_intercept,
                               int const check_directions[8],
                               unsigned int nr_of_check_directions,
                               unsigned int nr_checks_to_black)
{
  piece const intercepter_type = white[blocker_index].type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(Black,where_to_intercept,intercepter_type)
      && intelligent_reserve_officer_moves_from_to(white[blocker_index].diagram_square,
                                                   where_to_intercept,
                                                   intercepter_type))
  {
    if (officer_guards(king_square[Black],intercepter_type,where_to_intercept))
      ++nr_checks_to_black;
    SetPiece(intercepter_type,where_to_intercept,white[blocker_index].flags);
    continue_intercepting_checks(n,
                                 check_directions,
                                 nr_of_check_directions,
                                 nr_checks_to_black);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_white_piece(stip_length_type n,
                             square where_to_intercept,
                             int const check_directions[8],
                             unsigned int nr_of_check_directions,
                             unsigned int nr_checks_to_black)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1))
  {
    unsigned int blocker_index;
    for (blocker_index = 1; blocker_index<MaxPiece[White]; ++blocker_index)
      if (white[blocker_index].usage==piece_is_unused)
      {
        white[blocker_index].usage = piece_intercepts;

        if (white[blocker_index].type==pb)
        {
          with_promoted_white_pawn(n,
                                   blocker_index,
                                   where_to_intercept,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_black);
          if (where_to_intercept>=square_a2 && where_to_intercept<=square_h7)
            with_unpromoted_white_pawn(n,
                                       blocker_index,
                                       where_to_intercept,
                                       check_directions,
                                       nr_of_check_directions,
                                       nr_checks_to_black);
        }
        else
          with_white_officer(n,
                             blocker_index,
                             where_to_intercept,
                             check_directions,
                             nr_of_check_directions,
                             nr_checks_to_black);

        white[blocker_index].usage = piece_is_unused;
      }

    intelligent_unreserve();
  }

  e[where_to_intercept] = vide;
  spec[where_to_intercept] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void next_check(stip_length_type n,
                       int const check_directions[8],
                       unsigned int nr_of_check_directions,
                       unsigned int nr_checks_to_black)
{
  square where_to_intercept;
  int const current_dir = check_directions[nr_of_check_directions-1];
  square const start = king_square[White]+current_dir;
  boolean const is_diagonal = SquareCol(start)==SquareCol(king_square[White]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  assert(nr_of_check_directions>0);
  TraceValue("%d\n",current_dir);

  for (where_to_intercept = start;
       e[where_to_intercept]==vide;
       where_to_intercept += current_dir)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0)
    {
      /* avoid testing the same position twice */
      if (*where_to_start_placing_unused_black_pieces<where_to_intercept)
        with_black_piece(n,
                         where_to_intercept,
                         check_directions,
                         nr_of_check_directions-1,
                         nr_checks_to_black,
                         is_diagonal);
      with_white_piece(n,
                       where_to_intercept,
                       check_directions,
                       nr_of_check_directions-1,
                       nr_checks_to_black);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void continue_intercepting_checks(stip_length_type n,
                                         int const check_directions[8],
                                         unsigned int nr_of_check_directions,
                                         unsigned int nr_checks_to_black)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  if (nr_of_check_directions==0)
  {
    if (nr_checks_to_black>0)
    {
      unsigned int nr_checks_to_white = 0;
      intelligent_stalemate_intercept_checks_to_black(n,nr_checks_to_white);
    }
    else
      intelligent_stalemate_test_target_position(n);
  }
  else
    next_check(n,check_directions,nr_of_check_directions,nr_checks_to_black);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_intercept_checks_to_white(stip_length_type n,
                                                     unsigned int nr_checks_to_black)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_checks_to_black);
  TraceFunctionParamListEnd();

  {
    int check_directions[8];
    unsigned int const nr_of_check_directions = find_check_directions(White,
                                                                      check_directions);

    continue_intercepting_checks(n,
                                 check_directions,
                                 nr_of_check_directions,
                                 nr_checks_to_black);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
