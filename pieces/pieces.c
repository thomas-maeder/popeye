#include "pieces/pieces.h"
#include "utilities/bitmask.h"

Flags some_pieces_flags;
Flags all_pieces_flags;
Flags all_royals_flags;

boolean piece_walk_exists[nr_piece_walks];
boolean piece_walk_may_exist[nr_piece_walks];
boolean piece_walk_may_exist_fairy;

/* reset the pieces module for solving a new problem */
void pieces_reset(void)
{
  CLEARFL(all_pieces_flags);
  CLEARFL(all_royals_flags);
  CLEARFL(some_pieces_flags);
}
