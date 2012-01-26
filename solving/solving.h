#if !defined(SOLVING_H)
#define SOLVING_H

#include "pystip.h"

/* Instrument the stipulation structure with solving slices
 * @param root_slice root slice of the stipulation
 */
void stip_insert_solvers(slice_index root_slice);

/* state stip_spin_off_testers() traversal */
typedef struct
{
    boolean spinning_off;
    slice_index spun_off[max_nr_slices];
} spin_off_tester_state_type;

/* Spin off slices for testing whethere there is a solution
 * @param si root slice of the stipulation
 */
void stip_spin_off_testers(slice_index si);

#endif
