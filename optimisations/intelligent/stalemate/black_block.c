#include "optimisations/intelligent/stalemate/black_block.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

static void promoted_pawn(unsigned int nr_remaining_black_moves,
                          unsigned int nr_remaining_white_moves,
                          unsigned int max_nr_allowed_captures_by_black_pieces,
                          unsigned int max_nr_allowed_captures_by_white_pieces,
                          stip_length_type n,
                          square to_be_blocked,
                          Flags blocker_flags,
                          square blocker_comes_from)
{
  /* A rough check whether it is worth thinking about promotions */
  unsigned int time = (blocker_comes_from>=square_a7
                       ? 5
                       : blocker_comes_from/onerow - nr_of_slack_rows_below_board);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  assert(time<=5);

  if (to_be_blocked>=square_a2)
    /* square is not on 1st rank -- 1 move necessary to get there */
    ++time;

  if (nr_remaining_black_moves>=time)
  {
    piece pp;
    for (pp = -getprompiece[vide]; pp!=vide; pp = -getprompiece[-pp])
      if (!uninterceptably_attacks_king(White,to_be_blocked,pp))
      {
        unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_promotion(blocker_comes_from,
                                                                                       pp,
                                                                                       to_be_blocked);
        if (time<=nr_remaining_black_moves)
        {
          unsigned int const nr_checks_to_black = 0;
          SetPiece(pp,to_be_blocked,blocker_flags);
          intelligent_stalemate_continue_after_block(nr_remaining_black_moves-time,
                                                     nr_remaining_white_moves,
                                                     max_nr_allowed_captures_by_black_pieces,
                                                     max_nr_allowed_captures_by_white_pieces,
                                                     n,
                                                     White,
                                                     to_be_blocked,
                                                     pp,
                                                     nr_checks_to_black);
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unpromoted_pawn(unsigned int nr_remaining_black_moves,
                            unsigned int nr_remaining_white_moves,
                            unsigned int max_nr_allowed_captures_by_black_pieces,
                            unsigned int max_nr_allowed_captures_by_white_pieces,
                            stip_length_type n,
                            square to_be_blocked,
                            Flags blocker_flags,
                            square blocker_comes_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,to_be_blocked,pn))
  {
    unsigned int const nr_required_captures = abs(blocker_comes_from%onerow
                                                  - to_be_blocked%onerow);
    unsigned int const time = intelligent_count_nr_of_moves_from_to_pawn_no_promotion(pn,
                                                                                      blocker_comes_from,
                                                                                      to_be_blocked);
    if (time<=nr_remaining_black_moves
        && nr_required_captures<=max_nr_allowed_captures_by_black_pieces)
    {
      SetPiece(pn,to_be_blocked,blocker_flags);
      intelligent_stalemate_test_target_position(nr_remaining_black_moves-time,
                                                 nr_remaining_white_moves,
                                                 max_nr_allowed_captures_by_black_pieces-nr_required_captures,
                                                 max_nr_allowed_captures_by_white_pieces,
                                                 n);
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
                    square to_be_blocked,
                    piece blocker_type,
                    Flags blocker_flags,
                    square blocker_comes_from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TracePiece(blocker_type);
  TraceSquare(blocker_comes_from);
  TraceFunctionParamListEnd();

  if (!uninterceptably_attacks_king(White,to_be_blocked,blocker_type))
  {
    unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(blocker_type,
                                                                             blocker_comes_from,
                                                                             blocker_type,
                                                                             to_be_blocked);
    if (time<=nr_remaining_black_moves)
    {
      unsigned int const nr_checks_to_black = 0;
      SetPiece(blocker_type,to_be_blocked,blocker_flags);
      intelligent_stalemate_continue_after_block(nr_remaining_black_moves-time,
                                                 nr_remaining_white_moves,
                                                 max_nr_allowed_captures_by_black_pieces,
                                                 max_nr_allowed_captures_by_white_pieces,
                                                 n,
                                                 White,
                                                 to_be_blocked,
                                                 blocker_type,
                                                 nr_checks_to_black);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void intelligent_stalemate_black_block(unsigned int nr_remaining_black_moves,
                                       unsigned int nr_remaining_white_moves,
                                       unsigned int max_nr_allowed_captures_by_black_pieces,
                                       unsigned int max_nr_allowed_captures_by_white_pieces,
                                       stip_length_type n,
                                       square to_be_blocked)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (max_nr_allowed_captures_by_white_pieces>=1)
  {
    --max_nr_allowed_captures_by_white_pieces;

    for (i = 1; i<MaxPiece[Black]; ++i)
      if (black[i].usage==piece_is_unused)
      {
        piece const blocker_type = black[i].type;
        Flags const blocker_flags = black[i].flags;
        square const blocker_comes_from = black[i].diagram_square;

        black[i].usage = piece_blocks;

        if (blocker_type==pn)
        {
          promoted_pawn(nr_remaining_black_moves,
                        nr_remaining_white_moves,
                        max_nr_allowed_captures_by_black_pieces,
                        max_nr_allowed_captures_by_white_pieces,
                        n,
                        to_be_blocked,
                        blocker_flags,
                        blocker_comes_from);
          if (to_be_blocked>=square_a2)
            unpromoted_pawn(nr_remaining_black_moves,
                            nr_remaining_white_moves,
                            max_nr_allowed_captures_by_black_pieces,
                            max_nr_allowed_captures_by_white_pieces,
                            n,
                            to_be_blocked,
                            blocker_flags,
                            blocker_comes_from);
        }
        else
          officer(nr_remaining_black_moves,
                  nr_remaining_white_moves,
                  max_nr_allowed_captures_by_black_pieces,
                  max_nr_allowed_captures_by_white_pieces,
                  n,
                  to_be_blocked,
                  blocker_type,
                  blocker_flags,
                  blocker_comes_from);

        black[i].usage = piece_is_unused;
      }

    e[to_be_blocked] = vide;
    spec[to_be_blocked] = EmptySpec;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
