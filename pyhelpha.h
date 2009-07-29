#if !defined(PYHELPHA_H)
#define PYHELPHA_H

/* Hashed implementation of help play functions
 */

#include "pyhelp.h"
#include "pyslice.h"

/* Allocate a STHelpHashed slice for a STBranchHelp slice and insert
 * it at the STBranchHelp slice's position. 
 * The STHelpHashed takes the place of the STBranchHelp slice.
 * @param si identifies STBranchHelp slice
 */
void insert_help_hashed_slice(slice_index si);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean help_hashed_impose_starter(slice_index si, slice_traversal *st);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean help_hashed_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean help_hashed_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 */
void help_hashed_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n);

/* Is there no chance left for reaching the solution?
 * E.g. did the help side just allow a mate in 1 in a hr#N?
 * Tests may rely on the current position being hash-encoded.
 * @param si slice index
 * @param just_moved side that has just moved
 * @return true iff no chance is left
 */
boolean help_hashed_must_starter_resign_hashed(slice_index si);

#endif
