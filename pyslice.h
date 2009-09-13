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

/* Write a priori unsolvability (if any) of a slice (e.g. forced
 * reflex mates).
 * @param si slice index
 */
void slice_write_unsolvability(slice_index si);

/* Determine and write continuations of a slice
 * @param table table where to store continuing moves (i.e. threats)
 * @param si index of slice
 */
void slice_solve_continuations(table continuations, slice_index si);

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean slice_root_solve_postkey(table refutations, slice_index si);

#define ENUMERATION_TYPENAME quantity_of_refutations_type
#define ENUMERATORS \
  ENUMERATOR(attacker_has_solved_next_slice), \
  ENUMERATOR(found_no_refutation), \
  ENUMERATOR(found_refutations), \
  ENUMERATOR(attacker_has_reached_deadend)

#define ENUMERATION_DECLARE

#include "pyenum.h"

/* Find refutations after a move of the attacking side at root level.
 * @param refutations table where to store refutations
 * @param si slice index
 * @return attacker_has_reached_deadend if we are in a situation where
 *              the position after the attacking move is to be
 *              considered hopeless for the attacker, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *         attacker_has_solved_next_slice if the attacking move has
 *            solved the branch
 *         found_refutations if refutations contains some refutations
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type slice_root_find_refutations(table refutations,
                                                         slice_index si);

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
 * @param si slice index
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean slice_are_threats_refuted(table threats, slice_index si);

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

#define ENUMERATION_TYPENAME has_starter_won_result_type
#define ENUMERATORS                             \
  ENUMERATOR(starter_has_not_won),              \
    ENUMERATOR(starter_has_not_won_selfcheck),  \
    ENUMERATOR(starter_has_won)

#define ENUMERATION_DECLARE

#include "pyenum.h"

/* Determine whether the attacker has won with his move just played.
 * @param si slice identifier
 * @return whether the starter has won
 */
has_starter_won_result_type slice_has_starter_won(slice_index si);

/* Determine whether the attacker has reached slice si's goal with his
 * move just played.
 * @param si slice identifier
 * @return true iff the starter reached the goal
 */
boolean slice_has_starter_reached_goal(slice_index si);

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

/* Write that the non-starter has solved (i.e. in a self stipulation)
 * @param si slice index
 */
void slice_write_non_starter_has_solved(slice_index si);

/* Determine whether the defender wins after a move by the attacker
 * @param si slice index
 * @return true iff defender wins
 */
boolean slice_does_defender_win(slice_index si);

#endif
