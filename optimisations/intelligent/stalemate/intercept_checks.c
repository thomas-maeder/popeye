#include "optimisations/intelligent/stalemate/intercept_checks.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void continue_intercepting_checks(stip_length_type n,
                                                   int const check_directions[8],
                                                   unsigned int nr_of_check_directions,
                                                   unsigned int nr_checks_to_opponent,
                                                   Side side);

static void with_promoted_black_pawn(stip_length_type n,
                                     Side side,
                                     square where_to_intercept,
                                     unsigned int intercepter_index,
                                     int const check_directions[8],
                                     unsigned int nr_of_check_directions,
                                     unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",intercepter_index);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(intercepter_index,
                                                                where_to_intercept))
  {
    Flags const blocker_flags = black[intercepter_index].flags;
    square const blocker_comes_from = black[intercepter_index].diagram_square;

    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!guards(king_square[White],pp,where_to_intercept)
          && intelligent_reserve_promoting_pawn_moves_from_to(blocker_comes_from,
                                                              pp,
                                                              where_to_intercept))
      {
        boolean const white_check = guards(king_square[White],pp,where_to_intercept);
        if (!(side==White && white_check))
        {
          if (side==Black && white_check)
            ++nr_checks_to_opponent;
          SetPiece(pp,where_to_intercept,blocker_flags);
          continue_intercepting_checks(n,
                                       check_directions,
                                       nr_of_check_directions,
                                       nr_checks_to_opponent,
                                       side);
        }

        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_unpromoted_black_pawn(stip_length_type n,
                                       Side side,
                                       square where_to_intercept,
                                       unsigned int intercepter_index,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions,
                                       unsigned int nr_checks_to_opponent)
{
  Flags const blocker_flags = black[intercepter_index].flags;
  square const blocker_comes_from = black[intercepter_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",intercepter_index);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!guards(king_square[White],pn,where_to_intercept)
      && intelligent_reserve_black_pawn_moves_from_to_no_promotion(blocker_comes_from,
                                                                   where_to_intercept))
  {
    if (!(side==White && guards(king_square[White],pn,where_to_intercept)))
    {
      SetPiece(pn,where_to_intercept,blocker_flags);
      continue_intercepting_checks(n,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent,
                                   side);
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_black_officer(stip_length_type n,
                               Side side,
                               square where_to_intercept,
                               unsigned int intercepter_index,
                               int const check_directions[8],
                               unsigned int nr_of_check_directions,
                               unsigned int nr_checks_to_opponent)
{
  piece const blocker_type = black[intercepter_index].type;
  Flags const blocker_flags = black[intercepter_index].flags;
  square const blocker_comes_from = black[intercepter_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",intercepter_index);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!guards(king_square[White],blocker_type,where_to_intercept)
      && intelligent_reserve_officer_moves_from_to(blocker_comes_from,
                                                   where_to_intercept,
                                                   blocker_type))
  {
    boolean const white_check = guards(king_square[White],blocker_type,where_to_intercept);
    if (!(side==White && white_check))
    {
      if (side==Black && white_check)
        ++nr_checks_to_opponent;
      SetPiece(blocker_type,where_to_intercept,blocker_flags);
      continue_intercepting_checks(n,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent,
                                   side);
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_black_piece(stip_length_type n,
                             Side side,
                             square where_to_intercept,
                             int const check_directions[8],
                             unsigned int nr_of_check_directions,
                             unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(Black,1))
  {
    unsigned int i;
    for (i = 1; i<MaxPiece[Black]; ++i)
      if (black[i].usage==piece_is_unused)
      {
        black[i].usage = piece_intercepts;

        if (black[i].type==pn)
        {
          with_promoted_black_pawn(n,
                                   side,
                                   where_to_intercept,
                                   i,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent);
          if (where_to_intercept>=square_a2)
            with_unpromoted_black_pawn(n,
                                       side,
                                       where_to_intercept,
                                       i,
                                       check_directions,
                                       nr_of_check_directions,
                                       nr_checks_to_opponent);
        }
        else
          with_black_officer(n,
                             side,
                             where_to_intercept,
                             i,
                             check_directions,
                             nr_of_check_directions,
                             nr_checks_to_opponent);

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
                                       Side side,
                                       unsigned int blocker_index,
                                       square where_to_intercept,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions,
                                       unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!white_pawn_attacks_king(where_to_intercept)
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(white[blocker_index].diagram_square,
                                                                   where_to_intercept))
  {
    if (!(side==Black && guards(king_square[Black],pb,where_to_intercept)))
    {
      SetPiece(pb,where_to_intercept,white[blocker_index].flags);
      continue_intercepting_checks(n,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent,
                                   side);
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_promoted_white_pawn(stip_length_type n,
                                     Side side,
                                     unsigned int blocker_index,
                                     square where_to_intercept,
                                     int const check_directions[8],
                                     unsigned int nr_of_check_directions,
                                     unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(blocker_index,
                                                                where_to_intercept))
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!officer_uninterceptably_attacks_king(Black,where_to_intercept,pp)
          && intelligent_reserve_promoting_pawn_moves_from_to(white[blocker_index].diagram_square,
                                                              pp,
                                                              where_to_intercept))
      {
        boolean const black_check = guards(king_square[Black],pp,where_to_intercept);
        if (!(side==Black && black_check))
        {
          if (side==White && black_check)
            ++nr_checks_to_opponent;
          SetPiece(pp,where_to_intercept,white[blocker_index].flags);
          continue_intercepting_checks(n,
                                       check_directions,
                                       nr_of_check_directions,
                                       nr_checks_to_opponent,
                                       side);
        }

        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_white_king(stip_length_type n,
                            Side side,
                            square where_to_intercept,
                            int const check_directions[8],
                            unsigned int nr_of_check_directions,
                            unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!would_white_king_guard_from(where_to_intercept)
      && !is_white_king_uninterceptably_attacked_by_non_king(where_to_intercept)
      && intelligent_reserve_king_moves_from_to(White,
                                                white[index_of_king].diagram_square,
                                                where_to_intercept))
  {
    SetPiece(roib,where_to_intercept,white[index_of_king].flags);
    king_square[White] = where_to_intercept;

    if (is_white_king_interceptably_attacked())
      ++nr_checks_to_opponent;

    continue_intercepting_checks(n,
                                 check_directions,
                                 nr_of_check_directions,
                                 nr_checks_to_opponent,
                                 side);

    king_square[White] = initsquare;
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_white_officer(stip_length_type n,
                               Side side,
                               unsigned int blocker_index,
                               square where_to_intercept,
                               int const check_directions[8],
                               unsigned int nr_of_check_directions,
                               unsigned int nr_checks_to_opponent)
{
  piece const blocker_type = white[blocker_index].type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(Black,where_to_intercept,blocker_type)
      && intelligent_reserve_officer_moves_from_to(white[blocker_index].diagram_square,
                                                   where_to_intercept,
                                                   blocker_type))
  {
    boolean const black_check = guards(king_square[Black],blocker_type,where_to_intercept);
    if (!(side==Black && black_check))
    {
      if (side==White && black_check)
        ++nr_checks_to_opponent;
      SetPiece(blocker_type,where_to_intercept,white[blocker_index].flags);
      continue_intercepting_checks(n,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent,
                                   side);
    }

    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_white_piece(stip_length_type n,
                             Side side,
                             square where_to_intercept,
                             int const check_directions[8],
                             unsigned int nr_of_check_directions,
                             unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(where_to_intercept);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
  {
    white[index_of_king].usage = piece_intercepts;
    with_white_king(n,
                    side,
                    where_to_intercept,
                    check_directions,
                    nr_of_check_directions,
                    nr_checks_to_opponent);
    white[index_of_king].usage = piece_is_unused;
  }

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
                                   side,
                                   blocker_index,
                                   where_to_intercept,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent);
          with_unpromoted_white_pawn(n,
                                     side,
                                     blocker_index,
                                     where_to_intercept,
                                     check_directions,
                                     nr_of_check_directions,
                                     nr_checks_to_opponent);
        }
        else
          with_white_officer(n,
                             side,
                             blocker_index,
                             where_to_intercept,
                             check_directions,
                             nr_of_check_directions,
                             nr_checks_to_opponent);

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
                       Side side,
                       int const check_directions[8],
                       unsigned int nr_of_check_directions,
                       unsigned int nr_checks_to_opponent)
{
  square where_to_intercept;
  int const current_dir = check_directions[nr_of_check_directions-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  assert(nr_of_check_directions>0);
  TraceValue("%d\n",current_dir);

  for (where_to_intercept = king_square[side]+current_dir;
       e[where_to_intercept]==vide;
       where_to_intercept += current_dir)
    if (nr_reasons_for_staying_empty[where_to_intercept]==0)
    {
      with_black_piece(n,
                       side,
                       where_to_intercept,
                       check_directions,
                       nr_of_check_directions-1,
                       nr_checks_to_opponent);
      with_white_piece(n,
                       side,
                       where_to_intercept,
                       check_directions,
                       nr_of_check_directions-1,
                       nr_checks_to_opponent);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void continue_intercepting_checks(stip_length_type n,
                                         int const check_directions[8],
                                         unsigned int nr_of_check_directions,
                                         unsigned int nr_checks_to_opponent,
                                         Side side)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  if (nr_of_check_directions==0)
  {
    Side const opponent = advers(side);
    if (nr_checks_to_opponent>0)
    {
      unsigned int nr_checks_to_black = 0;
      intelligent_stalemate_intercept_checks(n,nr_checks_to_black,opponent);
    }
    else
      intelligent_stalemate_test_target_position(n);
  }
  else
    next_check(n,
               side,
               check_directions,
               nr_of_check_directions,
               nr_checks_to_opponent);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_intercept_checks(stip_length_type n,
                                            unsigned int nr_checks_to_opponent,
                                            Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  {
    int check_directions[8];
    unsigned int const nr_of_check_directions = find_check_directions(side,
                                                                      check_directions);

    continue_intercepting_checks(n,
                                 check_directions,
                                 nr_of_check_directions,
                                 nr_checks_to_opponent,
                                 side);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
