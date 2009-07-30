#if !defined(PYREFLXG_H)
#define PYREFLXG_H

/* In reflex stipulations, guard against tries that would allow the
 * wrong side to reach the goal.
 */

#include "pyhelp.h"
#include "pyslice.h"

/* Insert a STReflexGuard slice in front (and at the place)
 * of an existing slice.
 * @param si identifies slice to be superseded by a STReflexGuard slice
 * @param to_be_avoided prototype of slice that must be solvable
 */
void insert_reflex_guard_slice(slice_index si, slice_index to_be_avoided);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_guard_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean reflex_guard_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void reflex_guard_solve_continuations_in_n(table continuations,
                                           slice_index si,
                                           stip_length_type n);

/* Is there no chance left for reaching the solution?
 * E.g. did the help side just allow a mate in 1 in a hr#N?
 * Tests may rely on the current position being hash-encoded.
 * @param si slice index
 * @param just_moved side that has just moved
 * @return true iff no chance is left
 */
boolean reflex_guard_must_starter_resign_hashed(slice_index si);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean reflex_guard_impose_starter(slice_index si, slice_traversal *st);

/* Is there no chance left for the starting side at the move to win?
 * E.g. did the defender just capture that attacker's last potential
 * mating piece?
 * @param si slice index
 * @return true iff starter must resign
 */
boolean reflex_guard_must_starter_resign(slice_index si);

#endif
