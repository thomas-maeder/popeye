#if !defined(SOLVING_H)
#define SOLVING_H

#include "py.h"

/* Spin off slices for testing whethere there is a solution
 * @param si root slice of the stipulation
 * @note the state information passed to the slice visitors is of type boolean
 *       and indicates whether spinning off testers has started
 */
void stip_spin_off_testers(slice_index si);

/* Instrument a stipulation with move generator slices
 * @param si root of branch to be instrumented
 */
void stip_insert_move_generators(slice_index si);

#endif
