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
    unsigned int const nr_captures_required = abs(blocks_from%onerow
                                                  - to_be_blocked%onerow);
    unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                                      blocks_from,
                                                                                      to_be_blocked);
    if (Nr_unused_black_masses>=nr_captures_required
        && time<=Nr_remaining_white_moves)
    {
      Nr_unused_black_masses -= nr_captures_required;
      Nr_remaining_white_moves -= time;
      TraceValue("%u",Nr_unused_black_masses);
      TraceValue("%u\n",Nr_remaining_white_moves);
      SetPiece(pb,to_be_blocked,white[blocker_index].flags);
      intelligent_stalemate_test_target_position(n);
      Nr_remaining_white_moves += time;
      Nr_unused_black_masses += nr_captures_required;
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
  if (Nr_remaining_white_moves>=nr_moves_required)
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!officer_uninterceptably_attacks_king(Black,to_be_blocked,pp))
      {
        square const comes_from = white[blocker_index].diagram_square;
        unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(comes_from,
                                                                                       pp,
                                                                                       to_be_blocked);
        if (time<=Nr_remaining_white_moves)
        {
          unsigned int diffcol = 0;
          if (pp==fb)
          {
            unsigned int const comes_from_file = comes_from%nr_files_on_board;
            square const promotion_square_on_same_file = square_a8+comes_from_file;
            if (SquareCol(to_be_blocked)!=SquareCol(promotion_square_on_same_file))
              diffcol = 1;
          }
          if (diffcol<=Nr_unused_black_masses)
          {
            unsigned int const nr_checks_to_white = 0;
            Nr_remaining_white_moves -= time;
            Nr_unused_black_masses -= diffcol;
            TraceValue("%u",Nr_remaining_white_moves);
            TraceValue("%u\n",Nr_unused_black_masses);
            SetPiece(pp,to_be_blocked,white[blocker_index].flags);
            intelligent_stalemate_continue_after_block(n,
                                                       Black,
                                                       to_be_blocked,
                                                       pp,
                                                       nr_checks_to_white);
            Nr_unused_black_masses += diffcol;
            Nr_remaining_white_moves += time;
          }
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
      {
        unsigned int const nr_of_checks_to_black = 0;
        intelligent_stalemate_intercept_checks(n,nr_of_checks_to_black,White);
      }
      else
        intelligent_stalemate_test_target_position(n);

      king_square[White] = initsquare;
      Nr_remaining_white_moves += time;
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
    if (time<=Nr_remaining_white_moves)
    {
      unsigned int const nr_checks_to_white = 0;
      Nr_remaining_white_moves -= time;
      TraceValue("%u\n",Nr_remaining_white_moves);
      SetPiece(blocker_type,to_be_blocked,white[blocker_index].flags);
      intelligent_stalemate_continue_after_block(n,
                                                 Black,
                                                 to_be_blocked,
                                                 blocker_type,
                                                 nr_checks_to_white);
      Nr_remaining_white_moves += time;
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

  if (Nr_unused_white_masses>=1)
  {
    --Nr_unused_white_masses;
    TraceValue("%u\n",Nr_unused_white_masses);

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

    ++Nr_unused_white_masses;
  }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
