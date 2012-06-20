#if !defined(SOLVING_H)
#define SOLVING_H

#include "stipulation/structure_traversal.h"
#include "utilities/boolean.h"

/* Instrument the stipulation structure with solving slices
 * @param root_slice root slice of the stipulation
 */
void stip_insert_solvers(slice_index root_slice);

/* Callback to stip_spin_off_testers
 * Copy a slice to the testers, remove it from the solvers
 * @param si identifies the slice
 * @param st address of structure representing traversal
 */
void spin_off_testers_move_pipe_to_testers(slice_index si,
                                           stip_structure_traversal *st);

/* Spin off slices for testing whethere there is a solution
 * @param si root slice of the stipulation
 * @note the state information passed to the slice visitors is of type boolean
 *       and indicates whether spinning off testers has started
 */
void stip_spin_off_testers(slice_index si);

#endif
