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
                          unsigned blocker_index)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked[0]);
  TraceFunctionParam("%u",nr_to_be_blocked);
  TraceFunctionParam("%u",blocker_index);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_black_pawn_theoretically_move_to(blocker_index,
                                                                to_be_blocked[0]))
  {
    Flags const blocker_flags = black[blocker_index].flags;
    square const blocker_comes_from = black[blocker_index].diagram_square;

    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!officer_uninterceptably_attacks_king(White,to_be_blocked[0],pp)
          && intelligent_reserve_promoting_black_pawn_moves_from_to(blocker_comes_from,
                                                                    pp,
                                                                    to_be_blocked[0]))
      {
        SetPiece(pp,to_be_blocked[0],blocker_flags);
        finalise_blocking(n,to_be_blocked,nr_to_be_blocked);
        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpromoted_pawn(stip_length_type n,
                            square const to_be_blocked[8],
                            unsigned int nr_to_be_blocked,
                            unsigned blocker_index)
{
  Flags const blocker_flags = black[blocker_index].flags;
  square const blocker_comes_from = black[blocker_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked[0]);
  TraceFunctionParam("%u",nr_to_be_blocked);
  TraceFunctionParam("%u",blocker_index);
  TraceFunctionParamListEnd();

  if (!black_pawn_attacks_king(to_be_blocked[0])
      && intelligent_reserve_black_pawn_moves_from_to_no_promotion(blocker_comes_from,
                                                                   to_be_blocked[0]))
  {
    SetPiece(pn,to_be_blocked[0],blocker_flags);
    finalise_blocking(n,to_be_blocked,nr_to_be_blocked);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void officer(stip_length_type n,
                    square const to_be_blocked[8],
                    unsigned int nr_to_be_blocked,
                    unsigned blocker_index)
{
  piece const blocker_type = black[blocker_index].type;
  Flags const blocker_flags = black[blocker_index].flags;
  square const blocker_comes_from = black[blocker_index].diagram_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked[0]);
  TraceFunctionParam("%u",nr_to_be_blocked);
  TraceFunctionParam("%u",blocker_index);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(White,to_be_blocked[0],blocker_type)
      && intelligent_reserve_officer_moves_from_to(blocker_comes_from,
                                                   to_be_blocked[0],
                                                   blocker_type))
  {
    SetPiece(blocker_type,to_be_blocked[0],blocker_flags);
    finalise_blocking(n,to_be_blocked,nr_to_be_blocked);
    intelligent_unreserve();
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
      black[i].usage = piece_blocks;

      if (black[i].type==pn)
      {
        promoted_pawn(n,to_be_blocked,nr_to_be_blocked,i);
        if (to_be_blocked[0]>=square_a2)
          unpromoted_pawn(n,to_be_blocked,nr_to_be_blocked,i);
      }
      else
        officer(n,to_be_blocked,nr_to_be_blocked,i);

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

  if (intelligent_reserve_masses(Black,nr_to_be_blocked))
  {
    block_first(n,to_be_blocked,nr_to_be_blocked);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
