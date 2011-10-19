#include "optimisations/intelligent/stalemate/white_block.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/intercept_checks.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void unpromoted_pawn(stip_length_type n,
                            unsigned int blocker_index,
                            square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (!white_pawn_attacks_king(to_be_blocked))
  {
    square const blocks_from = white[blocker_index].diagram_square;
    unsigned int const save_nr_remaining_moves = Nr_remaining_moves[White];
    unsigned int const save_nr_unused_masses = Nr_unused_masses[Black];
    if (intelligent_reserve_white_pawn_moves_from_to_no_promotion(blocks_from,
                                                                  to_be_blocked))
    {
      SetPiece(pb,to_be_blocked,white[blocker_index].flags);
      intelligent_stalemate_test_target_position(n);

      Nr_unused_masses[Black] = save_nr_unused_masses;
      Nr_remaining_moves[White] = save_nr_remaining_moves;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void promoted_pawn(stip_length_type n,
                          unsigned int blocker_index,
                          square to_be_blocked)
{
  unsigned int const nr_moves_required = (to_be_blocked<=square_h7
                                          ? moves_to_white_prom[blocker_index]+1
                                          : moves_to_white_prom[blocker_index]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nr_moves_required);
  if (Nr_remaining_moves[White]>=nr_moves_required)
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!officer_uninterceptably_attacks_king(Black,to_be_blocked,pp))
      {
        square const comes_from = white[blocker_index].diagram_square;
        unsigned int const save_nr_remaining_moves = Nr_remaining_moves[White];
        unsigned int const save_nr_unused_masses = Nr_unused_masses[Black];
        if (intelligent_reserve_promoting_pawn_moves_from_to(comes_from,
                                                             pp,
                                                             to_be_blocked))
        {
          unsigned int const nr_checks_to_white = 0;
          SetPiece(pp,to_be_blocked,white[blocker_index].flags);
          intelligent_stalemate_continue_after_block(n,
                                                     Black,
                                                     to_be_blocked,
                                                     pp,
                                                     nr_checks_to_white);

          Nr_unused_masses[Black] = save_nr_unused_masses;
          Nr_remaining_moves[White] = save_nr_remaining_moves;
        }
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
      && !is_white_king_uninterceptably_attacked_by_non_king(to_be_blocked))
  {
    unsigned int const save_nr_remaining_moves = Nr_remaining_moves[White];
    if (intelligent_reserve_king_moves_from_to(White,
                                               white[index_of_king].diagram_square,
                                               to_be_blocked))
    {
      SetPiece(roib,to_be_blocked,white[index_of_king].flags);
      king_square[White] = to_be_blocked;

      if (is_white_king_interceptably_attacked())
      {
        unsigned int const nr_of_checks_to_black = 0;
        intelligent_stalemate_intercept_checks(n,nr_of_checks_to_black,White);
      }
      else
        intelligent_stalemate_test_target_position(n);

      king_square[White] = initsquare;
      Nr_remaining_moves[White] = save_nr_remaining_moves;
    }
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

  if (!officer_uninterceptably_attacks_king(Black,to_be_blocked,blocker_type))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(blocker_type,
                                                                             white[blocker_index].diagram_square,
                                                                             blocker_type,
                                                                             to_be_blocked);
    if (time<=Nr_remaining_moves[White])
    {
      unsigned int const nr_checks_to_white = 0;
      Nr_remaining_moves[White] -= time;
      TraceValue("%u\n",Nr_remaining_moves[White]);
      SetPiece(blocker_type,to_be_blocked,white[blocker_index].flags);
      intelligent_stalemate_continue_after_block(n,
                                                 Black,
                                                 to_be_blocked,
                                                 blocker_type,
                                                 nr_checks_to_white);
      Nr_remaining_moves[White] += time;
    }
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

  if (Nr_unused_masses[White]>=1)
  {
    --Nr_unused_masses[White];
    TraceValue("%u\n",Nr_unused_masses[White]);

    for (blocker_index = 1; blocker_index<MaxPiece[White]; ++blocker_index)
      if (white[blocker_index].usage==piece_is_unused)
      {
        piece const blocker_type = white[blocker_index].type;

        white[blocker_index].usage = piece_blocks;

        if (blocker_type==pb)
        {
          promoted_pawn(n,blocker_index,to_be_blocked);
          unpromoted_pawn(n,blocker_index,to_be_blocked);
        }
        else
          officer(n,blocker_type,blocker_index,to_be_blocked);

        white[blocker_index].usage = piece_is_unused;
      }

    ++Nr_unused_masses[White];
  }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
