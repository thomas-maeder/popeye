#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_CAPTURE_BY_INVISIBLE_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_CAPTURE_BY_INVISIBLE_H

#include "utilities/boolean.h"
#include "solving/ply.h"
#include "position/board.h"

void flesh_out_capture_by_invisible(void);

square need_existing_invisible_as_victim_for_capture_by_pawn(ply ply_capture);

#endif
