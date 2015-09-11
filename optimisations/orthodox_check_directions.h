#if !defined(OPTIMISATIONS_ORTHODOX_CHECK_DIRECTIONS_H)
#define OPTIMISATIONS_ORTHODOX_CHECK_DIRECTIONS_H

#include "stipulation/stipulation.h"
#include "pieces/walks/vectors.h"

extern numvec const * const * const CheckDir;

void check_dir_initialiser_solve(slice_index si);

#endif
