#if !defined(CONDITIONS_OHNESCHACH_IMMOBILILTY_TESTS_HELP_H)
#define CONDITIONS_OHNESCHACH_IMMOBILILTY_TESTS_HELP_H

/* This module provides an optimisation for the condition Ohneschach:
 * it's not necessary to test for immobility in the middle of help play
 */

#include "solving/solve.h"

/* Optimise away redundant immobility tests
 * @param si identifies root slice of stipulation
 */
void ohneschach_optimise_away_immobility_tests_help(slice_index si);

#endif
