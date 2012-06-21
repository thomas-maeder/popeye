#if !defined(SOLVING_H)
#define SOLVING_H

#include "stipulation/structure_traversal.h"
#include "utilities/boolean.h"

/* Instrument the stipulation structure with solving slices
 * @param root_slice root slice of the stipulation
 */
void stip_insert_solvers(slice_index root_slice);

/* Register a call-back for the next run of stip_spin_off_testers()
 * @param type slice type for which to call back visitor
 * @param visitor address to function to invoke for each visited slice of type type
 */
void register_spin_off_testers_visitor(slice_type type,
                                       stip_structure_visitor visitor);

/* Spin off slices for testing whethere there is a solution
 * @param si root slice of the stipulation
 * @note the state information passed to the slice visitors is of type boolean
 *       and indicates whether spinning off testers has started
 */
void stip_spin_off_testers(slice_index si);

#endif
