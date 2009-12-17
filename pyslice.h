#if !defined(PYSLICE_H)
#define PYSLICE_H

#include "py.h"
#include "pystip.h"
#include "pytable.h"
#include "boolean.h"

/* Generic functionality about slices.
 * The functions typically determine the slice type and delegate to the
 * appropriate function of the slice type-specific module.
 */

/* Determine and write threats of a slice
 * @param threats table where to store threats
 * @param si index of branch slice
 */
void slice_solve_threats(table threats, slice_index si);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean slice_are_threats_refuted(table threats,
                                  stip_length_type len_threat,
                                  slice_index si);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean slice_root_defend(slice_index si, unsigned int max_number_refutations);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_solve(slice_index si);

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_root_solve(slice_index si);

#define ENUMERATION_TYPENAME has_solution_type
#define ENUMERATORS                             \
  ENUMERATOR(defender_self_check),              \
    ENUMERATOR(has_solution),                   \
    ENUMERATOR(has_no_solution)

#define ENUMERATION_DECLARE

#include "pyenum.h"

/* Determine whether a composite slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type slice_has_solution(slice_index si);

/* Determine whether a slice.has just been solved with the just played
 * move by the non-starter
 * @param si slice identifier
 * @return true iff the non-starting side has just solved
 */
boolean slice_has_non_starter_solved(slice_index si);

/* Determine whether there are refutations
 * @param leaf slice index
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int slice_count_refutations(slice_index si,
                                     unsigned int max_result);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean slice_defend(slice_index si);

#endif
