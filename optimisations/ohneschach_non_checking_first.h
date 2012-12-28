#if !defined(OPTIMISATIONS_OHNESCHACH_NON_CHECKING_FIRST_H)
#define OPTIMISATIONS_OHNESCHACH_NON_CHECKING_FIRST_H

#include "solving/solve.h"

/* This module provides an optimisation for the condition Ohneschach:
 * when testing the (im)mobility of a side, it is more efficient to first
 * test non-checking moves only, and checking (i.e. potentially mating) moves
 * later
 */

/* Optimise Ohneschach immobility tester slices
 * @param si where to start (entry slice into stipulation)
 */
void ohneschach_optimise_immobility_testers(slice_index si);

#endif
