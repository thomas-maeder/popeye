#if !defined(PYHELP_H)
#define PYHELP_H

/* Interface for dynamically dispatching help play functions to slices
 * depending on the slice type
 */

#include "py.h"
#include "pystip.h"
#include "pytable.h"
#include "pyslice.h"

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean help_root_solve(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean help_solve_in_n(slice_index si, stip_length_type n);

/* Solve a branch slice at non-root level.
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean help_solve(slice_index si);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean help_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void help_solve_threats_in_n(table threats,
                             slice_index si,
                             stip_length_type n);

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void help_solve_threats(table threats, slice_index si);

/* Determine whether a branch slice.has just been solved with the
 * just played move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean help_has_non_starter_solved(slice_index si);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean help_are_threats_refuted(table threats, slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return true iff slice si has a solution
 */
has_solution_type help_has_solution(slice_index si);

#endif
