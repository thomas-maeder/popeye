#include "optimisations/intelligent/stalemate/white_block.h"
#include "pydata.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/place_white_piece.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

/* Block a black pawn with a white piece
 * @param to_be_blocked where to block the pawn
 */
void intelligent_stalemate_white_block(square to_be_blocked)
{
  TraceFunctionEntry(__func__);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (intelligent_reserve_masses(White,1))
  {
    unsigned int blocker_index;
    for (blocker_index = 0; blocker_index<MaxPiece[White]; ++blocker_index)
      if (white[blocker_index].usage==piece_is_unused)
      {
        white[blocker_index].usage = piece_blocks;

        switch (white[blocker_index].type)
        {
          case King:
            intelligent_place_white_king(to_be_blocked,
                                         &intelligent_stalemate_test_target_position);
            break;

          case Queen:
            intelligent_place_white_queen(blocker_index,
                                          to_be_blocked,
                                          &intelligent_stalemate_test_target_position);
            break;

          case Rook:
          case Bishop:
            intelligent_place_white_rider(blocker_index,
                                          to_be_blocked,
                                          &intelligent_stalemate_test_target_position);
            break;

          case Knight:
            intelligent_place_white_knight(blocker_index,
                                           to_be_blocked,
                                           &intelligent_stalemate_test_target_position);
            break;

          case Pawn:
            intelligent_place_promoted_white_pawn(blocker_index,
                                                  to_be_blocked,
                                                  &intelligent_stalemate_test_target_position);
            intelligent_place_unpromoted_white_pawn(blocker_index,
                                                    to_be_blocked,
                                                    &intelligent_stalemate_test_target_position);
            break;

          default:
            assert(0);
            break;
        }

        white[blocker_index].usage = piece_is_unused;
      }

    intelligent_unreserve();
  }

  e[to_be_blocked] = vide;
  spec[to_be_blocked] = EmptySpec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
