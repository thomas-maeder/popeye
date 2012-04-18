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
