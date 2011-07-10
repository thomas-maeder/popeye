#if !defined(STIPULATION_TEMPORARY_HACKS_H)
#define STIPULATION_TEMPORARY_HACKS_H

#include "pystip.h"

/* interface to some slices inserted as temporary hacks */

/* fork slice into mate tester */
extern slice_index temporary_hack_mate_tester[nr_sides];

/* fork slice into Ohneschach immobility tester */
extern slice_index temporary_hack_immobility_tester[nr_sides];

/* fork slice into mating move counter */
extern slice_index temporary_hack_exclusive_mating_move_counter[nr_sides];

/* Initialise temporary hack slices
 * @param root_slice identifies root slice of stipulation
 */
void insert_temporary_hacks(slice_index root_slice);

void temporary_hacks_swap_colors(void);

#endif
