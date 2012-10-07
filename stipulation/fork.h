#if !defined(STIPULATION_FORK_H)
#define STIPULATION_FORK_H

/* Branch fork - branch decides that when to continue play in branch
 * and when to change to slice representing subsequent play
 */

#include "stipulation/structure_traversal.h"
#include "solving/solve.h"

/* Allocate a new branch fork slice
 * @param type which slice type
 * @param fork identifies proxy slice that leads towards goal from the branch
 * @return newly allocated slice
 */
slice_index alloc_fork_slice(slice_type type, slice_index fork);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void fork_detect_starter(slice_index si, stip_structure_traversal *st);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a fork slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_fork(slice_index si, stip_structure_traversal *st);

#endif
