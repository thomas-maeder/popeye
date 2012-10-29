#if !defined(STIPULATION_END_OF_BRANCH_TESTER_H)
#define STIPULATION_END_OF_BRANCH_TESTER_H

/* Test whether the end of a branch has been reached before going on. We only
 * do this in branches that don't end in a position where the side at the move
 * is immobile (e.g. branches that end in mate or stalemate).
 */

#include "py.h"

/* Instrument STEndOfBranch* slices in help play in order to
 * - avoid writing short solutions when looking for longer ones
 * - avoid going on solving if a non-immobilising goal has been reached
 * @param root_slice identifes root slice of stipulation
 */
void stip_instrument_help_ends_of_branches(slice_index root_slice);

#endif
