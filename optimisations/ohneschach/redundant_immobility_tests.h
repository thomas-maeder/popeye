#if !defined(CONDITIONS_OHNESCHACH_REDUNDANT_IMMOBILILTY_TESTS_H)
#define CONDITIONS_OHNESCHACH_REDUNDANT_IMMOBILILTY_TESTS_H

/* This module provides an optimisation for the condition Ohneschach:
 * it's not necessary to test for immobility if we have already done so or if
 * immobility doesn't matter.
 */

#include "py.h"

/* Optimise away redundant immobility tests
 * @param si identifies root slice of stipulation
 */
void ohneschach_optimise_away_redundant_immobility_tests(slice_index si);

#endif
