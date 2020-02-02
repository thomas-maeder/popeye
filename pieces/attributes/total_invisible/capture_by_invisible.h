#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_CAPTURE_BY_INVISIBLE_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_CAPTURE_BY_INVISIBLE_H

#include "utilities/boolean.h"
#include "solving/ply.h"

void flesh_out_capture_by_invisible(void);

boolean need_existing_invisible_as_victim_for_capture_by_pawn(ply ply_capture);

void fake_capture_by_invisible(void);

#endif
