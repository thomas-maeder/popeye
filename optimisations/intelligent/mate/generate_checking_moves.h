#if !defined(OPTIMISATION_INTELLIGENT_MATE_GENERATE_CHECKING_MOVES_H)
#define OPTIMISATION_INTELLIGENT_MATE_GENERATE_CHECKING_MOVES_H

#include "py.h"

void remember_to_keep_checking_line_open(square from, square to,
                                         piece type, int delta);

void intelligent_mate_generate_checking_moves(void);

#endif
