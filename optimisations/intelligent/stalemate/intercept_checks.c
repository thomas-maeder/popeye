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
                                     square to_be_blocked,
                                     Flags blocker_flags,
                                     square blocker_comes_from,
                                     int const check_directions[8],
                                     unsigned int nr_of_check_directions,
                                     unsigned int nr_checks_to_opponent)
{
  /* A rough check whether it is worth thinking about promotions */
  unsigned int time = (blocker_comes_from>=square_a7
                       ? 5
                       : blocker_comes_from/onerow - nr_of_slack_rows_below_board);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TraceSquare(blocker_comes_from);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  assert(time<=5);

  if (to_be_blocked>=square_a2)
    /* square is not on 1st rank -- 1 move necessary to get there */
    ++time;

  if (Nr_remaining_black_moves>=time)
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!uninterceptably_attacks_king(White,to_be_blocked,pp))
      {
        unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(blocker_comes_from,
                                                                                       pp,
                                                                                       to_be_blocked);
        if (time<=Nr_remaining_black_moves)
        {
          boolean const white_check = guards(king_square[White],pp,to_be_blocked);
          if (!(side==White && white_check))
          {
            if (side==Black && white_check)
              ++nr_checks_to_opponent;
            Nr_remaining_black_moves -= time;
            TraceValue("%u\n",Nr_remaining_black_moves);
            SetPiece(pp,to_be_blocked,blocker_flags);
            continue_intercepting_checks(n,
                                         check_directions,
                                         nr_of_check_directions,
                                         nr_checks_to_opponent,
                                         side);
            Nr_remaining_black_moves += time;
          }
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_unpromoted_black_pawn(stip_length_type n,
                                       Side side,
                                       square to_be_blocked,
                                       Flags blocker_flags,
                                       square blocker_comes_from,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions,
                                       unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TraceSquare(blocker_comes_from);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,to_be_blocked,pn))
  {
    unsigned int const nr_required_captures = abs(blocker_comes_from%onerow
                                                  - to_be_blocked%onerow);
    unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                                      blocker_comes_from,
                                                                                      to_be_blocked);
    if (time<=Nr_remaining_black_moves
        && nr_required_captures<=Max_nr_allowed_captures_by_black
        && !(side==White && guards(king_square[White],pn,to_be_blocked)))
    {
      Max_nr_allowed_captures_by_black -= nr_required_captures;
      Nr_remaining_black_moves -= time;
      TraceValue("%u\n",Nr_remaining_black_moves);
      SetPiece(pn,to_be_blocked,blocker_flags);
      continue_intercepting_checks(n,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent,
                                   side);
      Nr_remaining_black_moves += time;
      Max_nr_allowed_captures_by_black += nr_required_captures;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_black_officer(stip_length_type n,
                               Side side,
                               square to_be_blocked,
                               piece blocker_type,
                               Flags blocker_flags,
                               square blocker_comes_from,
                               int const check_directions[8],
                               unsigned int nr_of_check_directions,
                               unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceSquare(blocker_comes_from);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,to_be_blocked,blocker_type))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(blocker_type,
                                                                             blocker_comes_from,
                                                                             blocker_type,
                                                                             to_be_blocked);
    if (time<=Nr_remaining_black_moves)
    {
      boolean const white_check = guards(king_square[White],blocker_type,to_be_blocked);
      if (!(side==White && white_check))
      {
        if (side==Black && white_check)
          ++nr_checks_to_opponent;
        Nr_remaining_black_moves -= time;
        TraceValue("%u\n",Nr_remaining_black_moves);
        SetPiece(blocker_type,to_be_blocked,blocker_flags);
        continue_intercepting_checks(n,
                                     check_directions,
                                     nr_of_check_directions,
                                     nr_checks_to_opponent,
                                     side);
        Nr_remaining_black_moves += time;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_black_piece(stip_length_type n,
                             Side side,
                             square to_be_blocked,
                             int const check_directions[8],
                             unsigned int nr_of_check_directions,
                             unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (Max_nr_allowed_captures_by_white>=1)
  {
    unsigned int i;

    --Max_nr_allowed_captures_by_white;
    TraceValue("%u\n",Max_nr_allowed_captures_by_white);

    for (i = 1; i<MaxPiece[Black]; ++i)
      if (black[i].usage==piece_is_unused)
      {
        piece const blocker_type = black[i].type;
        Flags const blocker_flags = black[i].flags;
        square const blocker_comes_from = black[i].diagram_square;

        black[i].usage = piece_intercepts;

        if (blocker_type==pn)
        {
          with_promoted_black_pawn(n,
                                   side,
                                   to_be_blocked,
                                   blocker_flags,
                                   blocker_comes_from,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent);
          if (to_be_blocked>=square_a2)
            with_unpromoted_black_pawn(n,
                                       side,
                                       to_be_blocked,
                                       blocker_flags,
                                       blocker_comes_from,
                                       check_directions,
                                       nr_of_check_directions,
                                       nr_checks_to_opponent);
        }
        else
          with_black_officer(n,
                             side,
                             to_be_blocked,
                             blocker_type,
                             blocker_flags,
                             blocker_comes_from,
                             check_directions,
                             nr_of_check_directions,
                             nr_checks_to_opponent);

        black[i].usage = piece_is_unused;
      }

    e[to_be_blocked] = vide;
    spec[to_be_blocked] = EmptySpec;

    ++Max_nr_allowed_captures_by_white;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_unpromoted_white_pawn(stip_length_type n,
                                       Side side,
                                       unsigned int blocker_index,
                                       square to_be_blocked,
                                       int const check_directions[8],
                                       unsigned int nr_of_check_directions,
                                       unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,to_be_blocked,pb))
  {
    square const blocks_from = white[blocker_index].diagram_square;
    unsigned int const nr_captures_required = abs(blocks_from%onerow
                                                  - to_be_blocked%onerow);
    if (Max_nr_allowed_captures_by_white>=nr_captures_required)
    {
      unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                                        blocks_from,
                                                                                        to_be_blocked);
      if (time<=Nr_remaining_white_moves
          && !(side==Black && guards(king_square[Black],pb,to_be_blocked)))
      {
        Max_nr_allowed_captures_by_white -= nr_captures_required;
        Nr_remaining_white_moves -= time;
        TraceValue("%u",Max_nr_allowed_captures_by_white);
        TraceValue("%u\n",Nr_remaining_white_moves);
        SetPiece(pb,to_be_blocked,white[blocker_index].flags);
        continue_intercepting_checks(n,
                                     check_directions,
                                     nr_of_check_directions,
                                     nr_checks_to_opponent,
                                     side);
        Nr_remaining_white_moves += time;
        Max_nr_allowed_captures_by_white += nr_captures_required;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_promoted_white_pawn(stip_length_type n,
                                     Side side,
                                     unsigned int blocker_index,
                                     square to_be_blocked,
                                     int const check_directions[8],
                                     unsigned int nr_of_check_directions,
                                     unsigned int nr_checks_to_opponent)
{
  unsigned int const nr_moves_required = (to_be_blocked<=square_h7
                                          ? moves_to_white_prom[blocker_index]+1
                                          : moves_to_white_prom[blocker_index]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nr_moves_required);
  if (Nr_remaining_white_moves>=nr_moves_required)
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!uninterceptably_attacks_king(Black,to_be_blocked,pp))
      {
        unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(white[blocker_index].diagram_square,
                                                                                       pp,
                                                                                       to_be_blocked);
        if (time<=Nr_remaining_white_moves)
        {
          boolean const black_check = guards(king_square[Black],pp,to_be_blocked);
          if (!(side==Black && black_check))
          {
            if (side==White && black_check)
              ++nr_checks_to_opponent;
            Nr_remaining_white_moves -= time;
            TraceValue("%u\n",Nr_remaining_white_moves);
            SetPiece(pp,to_be_blocked,white[blocker_index].flags);
            continue_intercepting_checks(n,
                                         check_directions,
                                         nr_of_check_directions,
                                         nr_checks_to_opponent,
                                         side);
            Nr_remaining_white_moves += time;
          }
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_white_king(stip_length_type n,
                            Side side,
                            square to_be_blocked,
                            int const check_directions[8],
                            unsigned int nr_of_check_directions,
                            unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!would_white_king_guard_from(to_be_blocked)
      && !is_white_king_uninterceptably_attacked_by_non_king(to_be_blocked))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_king(roib,
                                                                         white[index_of_king].diagram_square,
                                                                         to_be_blocked);
    if (time<=Nr_remaining_white_moves)
    {
      Nr_remaining_white_moves -= time;
      TraceValue("%u\n",Nr_remaining_white_moves);
      SetPiece(roib,to_be_blocked,white[index_of_king].flags);
      king_square[White] = to_be_blocked;

      if (is_white_king_interceptably_attacked())
        ++nr_checks_to_opponent;

      continue_intercepting_checks(n,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent,
                                   side);

      king_square[White] = initsquare;
      Nr_remaining_white_moves += time;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_white_officer(stip_length_type n,
                               Side side,
                               piece blocker_type,
                               unsigned int blocker_index,
                               square to_be_blocked,
                               int const check_directions[8],
                               unsigned int nr_of_check_directions,
                               unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TracePiece(blocker_type);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,to_be_blocked,blocker_type))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(blocker_type,
                                                                             white[blocker_index].diagram_square,
                                                                             blocker_type,
                                                                             to_be_blocked);
    if (time<=Nr_remaining_white_moves)
    {
      boolean const black_check = guards(king_square[Black],blocker_type,to_be_blocked);
      if (!(side==Black && black_check))
      {
        if (side==White && black_check)
          ++nr_checks_to_opponent;
        Nr_remaining_white_moves -= time;
        TraceValue("%u\n",Nr_remaining_white_moves);
        SetPiece(blocker_type,to_be_blocked,white[blocker_index].flags);
        continue_intercepting_checks(n,
                                     check_directions,
                                     nr_of_check_directions,
                                     nr_checks_to_opponent,
                                     side);
        Nr_remaining_white_moves += time;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void with_white_piece(stip_length_type n,
                             Side side,
                             square to_be_blocked,
                             int const check_directions[8],
                             unsigned int nr_of_check_directions,
                             unsigned int nr_checks_to_opponent)
{
  unsigned int blocker_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceSquare(to_be_blocked);
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
  {
    white[index_of_king].usage = piece_intercepts;
    with_white_king(n,
                    side,
                    to_be_blocked,
                    check_directions,
                    nr_of_check_directions,
                    nr_checks_to_opponent);
    white[index_of_king].usage = piece_is_unused;
  }

  if (Max_nr_allowed_captures_by_black>=1)
  {
    --Max_nr_allowed_captures_by_black;

    for (blocker_index = 1; blocker_index<MaxPiece[White]; ++blocker_index)
      if (white[blocker_index].usage==piece_is_unused)
      {
        piece const blocker_type = white[blocker_index].type;

        white[blocker_index].usage = piece_intercepts;

        if (blocker_type==pb)
        {
          with_promoted_white_pawn(n,
                                   side,
                                   blocker_index,
                                   to_be_blocked,
                                   check_directions,
                                   nr_of_check_directions,
                                   nr_checks_to_opponent);
          with_unpromoted_white_pawn(n,
                                     side,
                                     blocker_index,
                                     to_be_blocked,
                                     check_directions,
                                     nr_of_check_directions,
                                     nr_checks_to_opponent);
        }
        else
          with_white_officer(n,
                             side,
                             blocker_type,
                             blocker_index,
                             to_be_blocked,
                             check_directions,
                             nr_of_check_directions,
                             nr_checks_to_opponent);

        white[blocker_index].usage = piece_is_unused;
      }

    ++Max_nr_allowed_captures_by_black;
  }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void next_check(stip_length_type n,
                       Side side,
                       int const check_directions[8],
                       unsigned int nr_of_check_directions,
                       unsigned int nr_checks_to_opponent)
{
  square to_be_blocked;
  int const current_dir = check_directions[nr_of_check_directions-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceEnumerator(Side,side,"");
  TraceFunctionParam("%u",nr_of_check_directions);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  assert(nr_of_check_directions>0);
  TraceValue("%d\n",current_dir);

  for (to_be_blocked = king_square[side]+current_dir;
       e[to_be_blocked]==vide;
       to_be_blocked += current_dir)
    if (nr_reasons_for_staying_empty[to_be_blocked]==0)
    {
      with_black_piece(n,
                       side,
                       to_be_blocked,
                       check_directions,
                       nr_of_check_directions-1,
                       nr_checks_to_opponent);
      with_white_piece(n,
                       side,
                       to_be_blocked,
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
