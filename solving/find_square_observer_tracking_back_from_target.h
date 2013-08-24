#if !defined(SOLVING_FIND_SQUARE_OBSERVER_TRACKING_BACK_FROM_TARGET_H)
#define SOLVING_FIND_SQUARE_OBSERVER_TRACKING_BACK_FROM_TARGET_H

#include "solving/observation.h"
#include "solving/ply.h"
#include "utilities/boolean.h"
#include "pieces/pieces.h"

extern PieNam observing_walk[maxply+1];

typedef boolean (checkfunction_t)(evalfunction_t *);
extern  checkfunction_t *checkfunctions[PieceCount];

extern PieNam checkpieces[PieceCount-Leo+1]; /* only fairies ! */

boolean find_square_observer_tracking_back_from_target_king(slice_index si,
                                                            evalfunction_t *evaluate);

boolean find_square_observer_tracking_back_from_target_non_king(slice_index si,
                                                                evalfunction_t *evaluate);

boolean find_square_observer_tracking_back_from_target_fairy(slice_index si,
                                                             evalfunction_t *evaluate);

#endif
