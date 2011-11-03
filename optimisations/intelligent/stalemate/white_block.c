#include "optimisations/intelligent/stalemate/white_block.h"
#include "pyint.h"
#include "pydata.h"
#include "optimisations/intelligent/place_white_piece.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_white_king.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "trace.h"

#include <assert.h>
#include <stdlib.h>

/* Block a black pawn with a white piece
 * @param to_be_blocked where to block the pawn
 */
void intelligent_stalemate_white_block(square to_be_blocked)
{
  unsigned int blocker_index;

  TraceFunctionEntry(__func__);
  TraceSquare(to_be_blocked);
  TraceFunctionParamListEnd();

  if (white[index_of_king].usage==piece_is_unused)
  {
    white[index_of_king].usage = piece_blocks;
    intelligent_place_white_king(to_be_blocked,
                                 &intelligent_stalemate_test_target_position);
    white[index_of_king].usage = piece_is_unused;
  }

  if (intelligent_reserve_masses(White,1))
  {
    for (blocker_index = 1; blocker_index<MaxPiece[White]; ++blocker_index)
      if (white[blocker_index].usage==piece_is_unused)
      {
        white[blocker_index].usage = piece_blocks;

        switch (white[blocker_index].type)
        {
          case db:
          case tb:
          case fb:
            intelligent_place_white_rider(blocker_index,
                                          to_be_blocked,
                                          &intelligent_stalemate_test_target_position);
            break;

          case cb:
            intelligent_place_white_knight(blocker_index,
                                           to_be_blocked,
                                           &intelligent_stalemate_test_target_position);
            break;

          case pb:
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
