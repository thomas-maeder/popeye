#if !defined(PYSLICE_H)
#define PYSLICE_H

#include "py.h"
#include "pystip.h"
#include "boolean.h"

/* Generic functionality about slices.
 * The functions typically determine the slice type and delegate to the
 * appropriate function of the slice type-specific module.
 */

#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                             \
  ENUMERATOR(opponent_self_check),              \
    ENUMERATOR(has_no_solution),                \
    ENUMERATOR(has_solution)                    \

#define ENUMERATION_DECLARE

#include "pyenum.h"

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type slice_solve(slice_index si);

/* Determine whether a composite slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type slice_has_solution(slice_index si);

#endif
