#if !defined(SOLVING_FIND_SQUARE_OBSERVER_TRACKING_BACK_FROM_TARGET_H)
#define SOLVING_FIND_SQUARE_OBSERVER_TRACKING_BACK_FROM_TARGET_H

#include "solving/observation.h"
#include "solving/ply.h"
#include "utilities/boolean.h"
#include "pieces/pieces.h"

extern PieNam observing_walk[maxply+1];

typedef boolean (checkfunction_t)(validator_id );
extern checkfunction_t *checkfunctions[PieceCount];

extern PieNam checkpieces[PieceCount-Leo+1]; /* only fairies ! */

boolean track_back_from_target_according_to_observer_walk(slice_index si,
                                                          validator_id evaluate);

boolean determine_observer_walk(slice_index si, validator_id evaluate);

#endif
