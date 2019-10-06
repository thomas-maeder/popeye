#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_CAPTURE_BY_INVISIBLE_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_CAPTURE_BY_INVISIBLE_H

#include "position/position.h"
#include "solving/ply.h"

void flesh_out_capture_by_invisible(square first_taboo_violation);

boolean is_capture_by_invisible_possible(ply ply);

void fake_capture_by_invisible(void);

#endif
