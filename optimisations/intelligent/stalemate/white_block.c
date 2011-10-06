#include "optimisations/intelligent/stalemate/white_block.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/stalemate/intercept_checks.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void unpromoted_pawn(unsigned int nr_remaining_black_moves,
                            unsigned int nr_remaining_white_moves,
                            unsigned int max_nr_allowed_captures_by_black_pieces,
                            unsigned int max_nr_allowed_captures_by_white_pieces,
                            stip_length_type n,
                            unsigned int blocker_index,
                            square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",blocker_index);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,to_be_blocked,pb))
  {
    square const blocks_from = white[blocker_index].diagram_square;
    unsigned int const nr_captures_required = abs(blocks_from%onerow
                                                  - to_be_blocked%onerow);
    unsigned int const time = count_nr_of_moves_from_to_pawn_no_promotion(pb,
                                                                          blocks_from,
                                                                          to_be_blocked);
    if (max_nr_allowed_captures_by_white_pieces>=nr_captures_required
        && time<=nr_remaining_white_moves)
    {
      SetPiece(pb,to_be_blocked,white[blocker_index].flags);
      intelligent_stalemate_test_target_position(nr_remaining_black_moves,
                                                 nr_remaining_white_moves-time,
                                                 max_nr_allowed_captures_by_black_pieces,
                                                 max_nr_allowed_captures_by_white_pieces-nr_captures_required,
                                                 n);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void promoted_pawn(unsigned int nr_remaining_black_moves,
                          unsigned int nr_remaining_white_moves,
                          unsigned int max_nr_allowed_captures_by_black_pieces,
                          unsigned int max_nr_allowed_captures_by_white_pieces,
                          stip_length_type n,
                          unsigned int blocker_index,
                          square to_be_blocked)
{
  unsigned int const nr_moves_required = (to_be_blocked<=square_h7
                                          ? moves_to_white_prom[blocker_index]+1
                                          : moves_to_white_prom[blocker_index]);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nr_moves_required);
  if (nr_remaining_white_moves>=nr_moves_required)
  {
    piece pp;
    for (pp = getprompiece[vide]; pp!=vide; pp = getprompiece[pp])
      if (!uninterceptably_attacks_king(Black,to_be_blocked,pp))
      {
        unsigned int const time = count_nr_of_moves_from_to_pawn_promotion(white[blocker_index].diagram_square,
                                                                           pp,
                                                                           to_be_blocked);
        if (time<=nr_remaining_white_moves)
        {
          unsigned int const nr_checks_to_white = 0;
          SetPiece(pp,to_be_blocked,white[blocker_index].flags);
          intelligent_stalemate_continue_after_block(nr_remaining_black_moves,
                                                     nr_remaining_white_moves-time,
                                                     max_nr_allowed_captures_by_black_pieces,
                                                     max_nr_allowed_captures_by_white_pieces,
                                                     n,
                                                     Black,
                                                     to_be_blocked,
                                                     pp,
                                                     nr_checks_to_white);
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void white_king(unsigned int nr_remaining_black_moves,
                       unsigned int nr_remaining_white_moves,
                       unsigned int max_nr_allowed_captures_by_black_pieces,
                       unsigned int max_nr_allowed_captures_by_white_pieces,
                       stip_length_type n,
                       square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (!would_white_king_guard_from(to_be_blocked)
      && !is_white_king_uninterceptably_attacked_by_non_king(to_be_blocked))
  {
    unsigned int const time = count_nr_of_moves_from_to_king(roib,
                                                             white[index_of_king].diagram_square,
                                                             to_be_blocked);
    if (time<=nr_remaining_white_moves)
    {
      SetPiece(roib,to_be_blocked,white[index_of_king].flags);
      king_square[White] = to_be_blocked;

      if (is_white_king_interceptably_attacked())
      {
        unsigned int const nr_of_checks_to_black = 0;
        intelligent_stalemate_intercept_checks(nr_remaining_black_moves,
                                               nr_remaining_white_moves-time,
                                               max_nr_allowed_captures_by_black_pieces,
                                               max_nr_allowed_captures_by_white_pieces,
                                               n,
                                               nr_of_checks_to_black,
                                               White);
      }
      else
        intelligent_stalemate_test_target_position(nr_remaining_black_moves,
                                                   nr_remaining_white_moves-time,
                                                   max_nr_allowed_captures_by_black_pieces,
                                                   max_nr_allowed_captures_by_white_pieces,
                                                   n);

      king_square[White] = initsquare;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void officer(unsigned int nr_remaining_black_moves,
                    unsigned int nr_remaining_white_moves,
                    unsigned int max_nr_allowed_captures_by_black_pieces,
                    unsigned int max_nr_allowed_captures_by_white_pieces,
                    stip_length_type n,
                    piece blocker_type,
                    unsigned int blocker_index,
                    square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TracePiece(blocker_type);
  TraceFunctionParam("%u",blocker_index);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(Black,to_be_blocked,blocker_type))
  {
    unsigned int const time = count_nr_of_moves_from_to_no_check(blocker_type,
                                                                 white[blocker_index].diagram_square,
                                                                 blocker_type,
                                                                 to_be_blocked);
    if (time<=nr_remaining_white_moves)
    {
      unsigned int const nr_checks_to_white = 0;
      SetPiece(blocker_type,to_be_blocked,white[blocker_index].flags);
      intelligent_stalemate_continue_after_block(nr_remaining_black_moves,
                                                 nr_remaining_white_moves-time,
                                                 max_nr_allowed_captures_by_black_pieces,
                                                 max_nr_allowed_captures_by_white_pieces,
                                                 n,
                                                 Black,
                                                 to_be_blocked,
                                                 blocker_type,
                                                 nr_checks_to_white);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_white_block(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n,
                                       square to_be_blocked)
{
  unsigned int blocker_index;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
  {
    white[index_of_king].usage = piece_blocks;
    white_king(nr_remaining_black_moves,
               nr_remaining_white_moves,
               max_nr_allowed_captures_by_black_pieces,
               max_nr_allowed_captures_by_white_pieces,
               n,
               to_be_blocked);
    white[index_of_king].usage = piece_is_unused;
  }

  if (max_nr_allowed_captures_by_black_pieces>=1)
  {
    --max_nr_allowed_captures_by_black_pieces;

    for (blocker_index = 1; blocker_index<MaxPiece[White]; ++blocker_index)
      if (white[blocker_index].usage==piece_is_unused)
      {
        piece const blocker_type = white[blocker_index].type;

        white[blocker_index].usage = piece_blocks;

        if (blocker_type==pb)
        {
          promoted_pawn(nr_remaining_black_moves,
                        nr_remaining_white_moves,
                        max_nr_allowed_captures_by_black_pieces,
                        max_nr_allowed_captures_by_white_pieces,
                        n,
                        blocker_index,
                        to_be_blocked);
          unpromoted_pawn(nr_remaining_black_moves,
                          nr_remaining_white_moves,
                          max_nr_allowed_captures_by_black_pieces,
                          max_nr_allowed_captures_by_white_pieces,
                          n,
                          blocker_index,
                          to_be_blocked);
        }
        else
          officer(nr_remaining_black_moves,
                  nr_remaining_white_moves,
                  max_nr_allowed_captures_by_black_pieces,
                  max_nr_allowed_captures_by_white_pieces,
                  n,
                  blocker_type,
                  blocker_index,
                  to_be_blocked);

        white[blocker_index].usage = piece_is_unused;
      }
  }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
