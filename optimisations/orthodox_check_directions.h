#if !defined(OPTIMISATIONS_ORTHODOX_CHECK_DIRECTIONS_H)
#define OPTIMISATIONS_ORTHODOX_CHECK_DIRECTIONS_H

#include "stipulation/stipulation.h"
#include "pieces/walks/vectors.h"

numvec const * CheckDir(piece_walk_type t);

void check_dir_initialiser_solve(slice_index si);

#endif
