#include "pieces/pieces.h"
#include "utilities/bitmask.h"

Flags some_pieces_flags;
Flags all_pieces_flags;
Flags all_royals_flags;

boolean piece_walk_exists[nr_piece_walks];
boolean piece_walk_may_exist[nr_piece_walks];
boolean piece_walk_may_exist_fairy;
