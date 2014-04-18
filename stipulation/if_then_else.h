#if !defined(STIPULATION_IF_THEN_ELSE_H)
#define STIPULATION_IF_THEN_ELSE_H

/* Slice type STIfThenElse - select the sucessor based on a condition
 */

#include "stipulation/structure_traversal.h"

/* Allocate a STIfThenElse slice.
 * @param normal identifies "normal" (else) successor
 * @param exceptional identifies "exceptional" (if) successor
 * @param condition identifies condition on which to take exceptional path
 * @return index of allocated slice
 */
slice_index alloc_if_then_else_slice(slice_index normal,
                                     slice_index exceptional,
                                     slice_index condition);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off a binary slice
 * @param si identifies the testing pipe slice
 * @param st address of structure representing traversal
 */
void stip_spin_off_testers_if_then_else(slice_index si,
                                        stip_structure_traversal *st);

#endif
