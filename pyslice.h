#if !defined(PYSLICE_H)
#define PYSLICE_H

#include "py.h"
#include "pystip.h"
#include "boolean.h"

/* Generic functionality about slices.
 * The functions typically determine the slice type and delegate to the
 * appropriate function of the slice type-specific module.
 */

///* Solve a slice
// * @param si slice index
// * @return whether there is a solution and (to some extent) why not
// */
//has_solution_type slice_solve(slice_index si);

#include "stipulation/battle_play/attack_play.h"
#define slice_solve(si) attack(si,length_unspecified)

#endif
