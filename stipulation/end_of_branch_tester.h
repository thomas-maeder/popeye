#if !defined(STIPULATION_END_OF_BRANCH_TESTER_H)
#define STIPULATION_END_OF_BRANCH_TESTER_H

/* Test whether the end of a branch has been reached before going on. We only
 * do this in branches that don't end in a position where the side at the move
 * is immobile (e.g. branches that end in mate or stalemate).
 */

#include "stipulation/help_play/play.h"

/* Instrument STEndOfBranchGoal (and STEndOfBranchForced) slices with the
 * necessary STEndOfBranchTester slices
 * @param root_slice identifes root slice of stipulation
 */
void stip_insert_end_of_branch_testers(slice_index root_slice);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off an end of a branch tester slice
 * @param si identifies the branch tester slice
 * @param st address of structure representing traversal
 */
void start_spinning_off_end_of_branch_tester(slice_index si,
                                             stip_structure_traversal *st);

/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type end_of_branch_tester_help(slice_index si, stip_length_type n);

#endif
