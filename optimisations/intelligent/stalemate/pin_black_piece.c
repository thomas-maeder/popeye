#include "optimisations/intelligent/stalemate/pin_black_piece.h"
#include "pieces/pieces.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/pin_black_piece.h"
#include "optimisations/intelligent/stalemate/finish.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <stdlib.h>

/* Pin a mobile black piece
 * @param position_of_trouble_maker position of piece to be pinned
 */
void intelligent_stalemate_pin_black_piece(slice_index si,
                                           square position_of_trouble_maker)
{
  int const dir = intelligent_is_black_piece_pinnable(position_of_trouble_maker);
  piece_walk_type const pinned_type = get_walk_of_piece_on_square(position_of_trouble_maker);

  TraceFunctionEntry(__func__);
  TraceSquare(position_of_trouble_maker);
  TraceFunctionParamListEnd();

  if (dir!=0
      && pinned_type!=Queen /* queens cannot be pinned */
      /* bishops can only be pinned on rook lines and vice versa */
      && !(CheckDir[Bishop][dir]!=0 && pinned_type==Bishop)
      && !(CheckDir[Rook][dir]!=0 && pinned_type==Rook))
    intelligent_pin_pinnable_black_piece(si,
                                         position_of_trouble_maker,
                                         dir,
                                         &intelligent_stalemate_test_target_position);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
