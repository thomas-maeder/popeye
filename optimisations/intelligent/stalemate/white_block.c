#include "optimisations/intelligent/stalemate/white_block.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/intercept_checks_to_white.h"
#include "optimisations/intelligent/stalemate/intercept_checks_to_black.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void continue_after_block(stip_length_type n,
                                 square to_be_blocked,
                                 piece blocker_type,
                                 unsigned int nr_checks_to_opponent)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceFunctionParam("%u",nr_checks_to_opponent);
  TraceFunctionParamListEnd();

  if (officer_guards(king_square[Black],blocker_type,to_be_blocked))
  {
    unsigned int const nr_checks_to_opponent = 0;
    intelligent_stalemate_intercept_checks_to_black(n,nr_checks_to_opponent);
  }
  else
    intelligent_stalemate_test_target_position(n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpromoted_pawn(stip_length_type n,
                            unsigned int blocker_index,
                            square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (!white_pawn_attacks_king_region(to_be_blocked,0)
      && intelligent_reserve_white_pawn_moves_from_to_no_promotion(white[blocker_index].diagram_square,
                                                                   to_be_blocked))
  {
    SetPiece(pb,to_be_blocked,white[blocker_index].flags);
    intelligent_stalemate_test_target_position(n);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void promoted_pawn(stip_length_type n,
                          unsigned int blocker_index,
                          square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (intelligent_can_promoted_white_pawn_theoretically_move_to(blocker_index,
                                                                to_be_blocked))
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!officer_uninterceptably_attacks_king(Black,to_be_blocked,pp)
          && intelligent_reserve_promoting_white_pawn_moves_from_to(white[blocker_index].diagram_square,
                                                                    pp,
                                                                    to_be_blocked))
      {
        unsigned int const nr_checks_to_white = 0;
        SetPiece(pp,to_be_blocked,white[blocker_index].flags);
        continue_after_block(n,to_be_blocked,pp,nr_checks_to_white);
        intelligent_unreserve();
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void white_king(stip_length_type n, square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (!would_white_king_guard_from(to_be_blocked)
      && !is_white_king_uninterceptably_attacked_by_non_king(to_be_blocked)
      && intelligent_reserve_white_king_moves_from_to(white[index_of_king].diagram_square,
                                                      to_be_blocked))
  {
    SetPiece(roib,to_be_blocked,white[index_of_king].flags);
    king_square[White] = to_be_blocked;

    if (is_white_king_interceptably_attacked())
    {
      unsigned int const nr_of_checks_to_black = 0;
      intelligent_stalemate_intercept_checks_to_white(n,nr_of_checks_to_black);
    }
    else
      intelligent_stalemate_test_target_position(n);

    king_square[White] = initsquare;
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void officer(stip_length_type n,
                    piece blocker_type,
                    unsigned int blocker_index,
                    square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TracePiece(blocker_type);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (!officer_uninterceptably_attacks_king(Black,to_be_blocked,blocker_type)
      && intelligent_reserve_officer_moves_from_to(white[blocker_index].diagram_square,
                                                   to_be_blocked,
                                                   blocker_type))
  {
    unsigned int const nr_checks_to_white = 0;
    SetPiece(blocker_type,to_be_blocked,white[blocker_index].flags);
    continue_after_block(n,to_be_blocked,blocker_type,nr_checks_to_white);
    intelligent_unreserve();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_white_block(stip_length_type n, square to_be_blocked)
{
  unsigned int blocker_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
  {
    white[index_of_king].usage = piece_blocks;
    white_king(n,to_be_blocked);
    white[index_of_king].usage = piece_is_unused;
  }

  if (intelligent_reserve_masses(White,1))
  {
    for (blocker_index = 1; blocker_index<MaxPiece[White]; ++blocker_index)
      if (white[blocker_index].usage==piece_is_unused)
      {
        piece const blocker_type = white[blocker_index].type;

        white[blocker_index].usage = piece_blocks;

        if (blocker_type==pb)
        {
          promoted_pawn(n,blocker_index,to_be_blocked);
          if (to_be_blocked>=square_a2 && to_be_blocked<=square_h7)
            unpromoted_pawn(n,blocker_index,to_be_blocked);
        }
        else
          officer(n,blocker_type,blocker_index,to_be_blocked);

        white[blocker_index].usage = piece_is_unused;
      }

    intelligent_unreserve();
  }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
