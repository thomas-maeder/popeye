#if !defined(PIECES_WALKS_UBIUBI_H)
#define PIECES_WALKS_UBIUBI_H

/* This module implements the fairy piece UbiUbi */

#include "solving/observation.h"

void ubiubi_generate_moves(void);

boolean ubiubi_check(evalfunction_t *evaluate);

#endif
