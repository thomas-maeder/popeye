#if !defined(CONDITIONS_OHNESCHACH_LEGALITY_TESTER_H)
#define CONDITIONS_OHNESCHACH_LEGALITY_TESTER_H

/* This module tests the legality of moves in condition Ohneschach
 */

#include "solving/solve.h"

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void ohneschach_insert_check_guards(slice_index si);

#endif
