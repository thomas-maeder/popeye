#if !defined(PIECES_ATTRIBUTESNEUTRAL_NEUTRAL_H)
#define PIECES_ATTRIBUTESNEUTRAL_NEUTRAL_H

/* This module implements neutral pieces */

#include "position/position.h"

//#define is_piece_neutral(spec) TSTFLAG((spec),Neutral)
#define is_piece_neutral(spec) (TSTFLAGMASK((spec),BIT(White)|BIT(Black))==(BIT(White)|BIT(Black)))

#endif
