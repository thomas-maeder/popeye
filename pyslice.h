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

/* Find refutations after a move of the attacking side at root level.
 * @param si slice index
 * @param maximum number of refutations to be reported
 * @return slack_length_direct:   key solved
 *         slack_length_direct+2: key allows refutations
 *         slack_length_direct+4: key reached deadend (e.g. self check)
 */
stip_length_type
slice_root_find_refutations(slice_index si,
                            unsigned int max_number_refutations);

/* Spin off a set play slice at root level
 * @param si slice index
 * @return set play slice spun off; no_slice if not applicable
 */
slice_index slice_root_make_setplay_slice(slice_index si);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @return index of first postkey slice; no_slice if postkey play not
 *         applicable
 */
slice_index slice_root_reduce_to_postkey_play(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_solve(slice_index si);

/* As slice_solve(), but the key move has just been played.
 * I.e. determine whether a slice has been solved with the move just
 * played; if yes, write the solution including the move just played.
 * @param si slice identifier
 * @return true iff the slice is solved
 */
boolean slice_solved(slice_index si);

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_root_solve(slice_index si);

/* Solve a slice in exactly n moves at root level
 * @param si slice index
 * @param n exact number of moves
 */
void slice_root_solve_in_n(slice_index si, stip_length_type n);

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

/* Determine whether a side has reached the goal
 * @param just_moved side that has just moved
 * @param si slice index
 * @return true iff just_moved has reached the goal
 */
boolean slice_is_goal_reached(Side just_moved, slice_index si);

/* Find and write post key play
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_solve_postkey(slice_index si);

typedef enum
{
  dont_know_who_decides_on_starter,
  leaf_decides_on_starter
} who_decides_on_starter;

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter slice_detect_starter(slice_index si,
                                            boolean same_side_as_root);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean slice_defend(slice_index si);

#endif
