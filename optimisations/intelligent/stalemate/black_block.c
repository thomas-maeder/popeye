#include "optimisations/intelligent/stalemate/black_block.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "optimisations/intelligent/stalemate/intercept_checks.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void block_first(stip_length_type n,
                        square const to_be_blocked[8],
                        unsigned int nr_to_be_blocked);

static void finalise_blocking(stip_length_type n,
                              square const to_be_blocked[8],
                              unsigned int nr_to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_to_be_blocked);
  TraceFunctionParamListEnd();

  assert(nr_to_be_blocked>=1);

  if (nr_to_be_blocked==1)
  {
    if (echecc(nbply,White))
    {
      boolean const is_black_in_check = false;
      intelligent_stalemate_intercept_checks(n,is_black_in_check,White);
    }
    else
      intelligent_stalemate_test_target_position(n);
  }
  else
    block_first(n,to_be_blocked+1,nr_to_be_blocked-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void promoted_pawn(stip_length_type n,
                          square const to_be_blocked[8],
                          unsigned int nr_to_be_blocked,
                          Flags blocker_flags,
                          square blocker_comes_from)
{
  /* A rough check whether it is worth thinking about promotions */
  unsigned int time = (blocker_comes_from>=square_a7
                       ? 5
                       : blocker_comes_from/onerow - nr_of_slack_rows_below_board);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked[0]);
  TraceFunctionParam("%u",nr_to_be_blocked);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  assert(time<=5);

  if (to_be_blocked[0]>=square_a2)
    /* square is not on 1st rank -- 1 move necessary to get there */
    ++time;

  if (Nr_remaining_black_moves>=time)
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!officer_uninterceptably_attacks_king(White,to_be_blocked[0],pp))
      {
        unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(blocker_comes_from,
                                                                                       pp,
                                                                                       to_be_blocked[0]);
        if (time<=Nr_remaining_black_moves)
        {
          unsigned int diffcol = 0;
          if (pp==fn)
          {
            unsigned int const blocker_comes_from_file = blocker_comes_from%nr_files_on_board;
            square const promotion_square_on_same_file = square_a1+blocker_comes_from_file;
            if (SquareCol(to_be_blocked[0])!=SquareCol(promotion_square_on_same_file))
              diffcol = 1;
          }
          if (diffcol<=Nr_unused_white_masses)
          {
            Nr_remaining_black_moves -= time;
            Nr_unused_white_masses -= diffcol;
            TraceValue("%u",Nr_remaining_black_moves);
            TraceValue("%u\n",Nr_unused_white_masses);
            SetPiece(pp,to_be_blocked[0],blocker_flags);
            finalise_blocking(n,to_be_blocked,nr_to_be_blocked);
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
                            square const to_be_blocked[8],
                            unsigned int nr_to_be_blocked,
                            Flags blocker_flags,
                            square blocker_comes_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked[0]);
  TraceFunctionParam("%u",nr_to_be_blocked);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  if (!black_pawn_attacks_king(to_be_blocked[0]))
  {
    unsigned int const nr_required_captures = abs(blocker_comes_from%onerow
                                                  - to_be_blocked[0]%onerow);
    unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                                      blocker_comes_from,
                                                                                      to_be_blocked[0]);
    if (time<=Nr_remaining_black_moves
        && nr_required_captures<=Nr_unused_white_masses)
    {
      Nr_unused_white_masses -= nr_required_captures;
      Nr_remaining_black_moves -= time;
      TraceValue("%u",Nr_unused_white_masses);
      TraceValue("%u\n",Nr_remaining_black_moves);
      SetPiece(pn,to_be_blocked[0],blocker_flags);
      finalise_blocking(n,to_be_blocked,nr_to_be_blocked);
      Nr_remaining_black_moves += time;
      Nr_unused_white_masses += nr_required_captures;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void officer(stip_length_type n,
                    square const to_be_blocked[8],
                    unsigned int nr_to_be_blocked,
                    piece blocker_type,
                    Flags blocker_flags,
                    square blocker_comes_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked[0]);
  TraceFunctionParam("%u",nr_to_be_blocked);
  TracePiece(blocker_type);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(White,to_be_blocked[0],blocker_type))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(blocker_type,
                                                                             blocker_comes_from,
                                                                             blocker_type,
                                                                             to_be_blocked[0]);
    if (time<=Nr_remaining_black_moves)
    {
      Nr_remaining_black_moves -= time;
      TraceValue("%u\n",Nr_remaining_black_moves);
      SetPiece(blocker_type,to_be_blocked[0],blocker_flags);
      finalise_blocking(n,to_be_blocked,nr_to_be_blocked);
      Nr_remaining_black_moves += time;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void block_first(stip_length_type n,
                        square const to_be_blocked[8],
                        unsigned int nr_to_be_blocked)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked[0]);
  TraceFunctionParam("%u",nr_to_be_blocked);
  TraceFunctionParamListEnd();

  assert(nr_to_be_blocked>0);

  for (i = 1; i<MaxPiece[Black]; ++i)
    if (black[i].usage==piece_is_unused)
    {
      piece const blocker_type = black[i].type;
      Flags const blocker_flags = black[i].flags;
      square const blocker_comes_from = black[i].diagram_square;

      black[i].usage = piece_blocks;

      if (blocker_type==pn)
      {
        promoted_pawn(n,to_be_blocked,nr_to_be_blocked,blocker_flags,blocker_comes_from);
        if (to_be_blocked[0]>=square_a2)
          unpromoted_pawn(n,to_be_blocked,nr_to_be_blocked,blocker_flags,blocker_comes_from);
      }
      else
        officer(n,to_be_blocked,nr_to_be_blocked,blocker_type,blocker_flags,blocker_comes_from);

      black[i].usage = piece_is_unused;
    }

  e[to_be_blocked[0]] = vide;
  spec[to_be_blocked[0]] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_black_block(stip_length_type n,
                                       square const to_be_blocked[8],
                                       unsigned int nr_to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",nr_to_be_blocked);
  TraceFunctionParamListEnd();

  if (Nr_unused_black_masses>=nr_to_be_blocked)
  {
    Nr_unused_black_masses -= nr_to_be_blocked;
    TraceValue("%u\n",Nr_unused_black_masses);

    block_first(n,to_be_blocked,nr_to_be_blocked);

    Nr_unused_black_masses += nr_to_be_blocked;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
