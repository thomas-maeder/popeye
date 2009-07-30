#if !defined(PYHELP_H)
#define PYHELP_H

/* Interface for dynamically dispatching help play functions to slices
 * depending on the slice type
 */

#include "py.h"
#include "pystip.h"
#include "pytable.h"

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean help_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean help_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void help_solve_continuations_in_n(table continuations,
                                   slice_index si,
                                   stip_length_type n);

#endif
