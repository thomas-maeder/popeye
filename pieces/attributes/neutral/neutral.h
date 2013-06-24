#if !defined(PIECES_ATTRIBUTESNEUTRAL_NEUTRAL_H)
#define PIECES_ATTRIBUTESNEUTRAL_NEUTRAL_H

/* This module implements neutral pieces */

#include "position/position.h"

#define NeutralMask (BIT(White)|BIT(Black))
#define is_piece_neutral(spec) (TSTFLAGMASK((spec),NeutralMask)==NeutralMask)

#endif
