#if !defined(STIPULATION_TESTING_PIPE_H)
#define STIPULATION_TESTING_PIPE_H

/* Functionality related to "testing pipe slices"; i.e. pipe slices that may
 * switch to testing mode (i.e. look for a solution or a defense without
 * producing output).
 */

#include "stipulation/pipe.h"
#include "stipulation/structure_traversal.h"

/* Allocate a new testing pipe and make an existing pipe its successor
 * @param type which slice type
 * @return newly allocated slice
 */
slice_index alloc_testing_pipe(slice_type type);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a testing pipe slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_testing_pipe(slice_index si,
                                        stip_structure_traversal *st);

#endif
