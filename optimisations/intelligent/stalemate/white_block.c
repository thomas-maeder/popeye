#include "optimisations/intelligent/stalemate/white_block.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/place_white_piece.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Block a black pawn with a white piece
 * @param to_be_blocked where to block the pawn
 */
void intelligent_stalemate_white_block(slice_index si, square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1,piece_blocks))
  {
    unsigned int blocker_index;
    for (blocker_index = 0; blocker_index<MaxPiece[White]; ++blocker_index)
      if (white[blocker_index].usage==piece_is_unused)
      {
        white[blocker_index].usage = piece_blocks;

        switch (white[blocker_index].type)
        {
          case King:
            intelligent_place_white_king(si,
                                         to_be_blocked,
                                         &intelligent_stalemate_test_target_position);
            break;

          case Queen:
            intelligent_place_white_queen(si,
                                          blocker_index,
                                          to_be_blocked,
                                          &intelligent_stalemate_test_target_position);
            break;

          case Rook:
          case Bishop:
            intelligent_place_white_rider(si,
                                          blocker_index,
                                          to_be_blocked,
                                          &intelligent_stalemate_test_target_position);
            break;

          case Knight:
            intelligent_place_white_knight(si,
                                           blocker_index,
                                           to_be_blocked,
                                           &intelligent_stalemate_test_target_position);
            break;

          case Pawn:
            intelligent_place_promoted_white_pawn(si,
                                                  blocker_index,
                                                  to_be_blocked,
                                                  &intelligent_stalemate_test_target_position);
            intelligent_place_unpromoted_white_pawn(si,
                                                    blocker_index,
                                                    to_be_blocked,
                                                    &intelligent_stalemate_test_target_position);
            break;

          case Dummy:
            break;

          default:
            assert(0);
            break;
        }

        white[blocker_index].usage = piece_is_unused;
      }

    intelligent_unreserve();
  }

  empty_square(to_be_blocked);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
