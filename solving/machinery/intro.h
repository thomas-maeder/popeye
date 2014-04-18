#if !defined(SOLVING_INTRO_H)
#define SOLVING_INTRO_H

/* Introductory slices of nested branches
 */

#include "stipulation/stipulation.h"

/* Wrap the slices representing the initial moves of nested slices
 * @param si identifies slice where to start
 */
void solving_insert_intro_slices(slice_index si);

#endif
