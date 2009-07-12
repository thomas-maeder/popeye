#if !defined(PYHELPHA_H)
#define PYHELPHA_H

/* Hashed implementation of help play functions
 */

#include "pyhelp.h"
#include "pyslice.h"

/* Allocate a STHelpHashed slice for a STBranchHelp slice
 * @param base identifies STBranchHelp slice
 * @return index of allocated slice
 */
slice_index alloc_help_hashed_slice(slice_index base);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at the move
 * @return true iff >=1 solution was found
 */
boolean help_hashed_solve_in_n(slice_index si,
                               stip_length_type n,
                               Side side_at_move);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 * @return true iff >= 1 solution has been found
 */
boolean help_hashed_has_solution_in_n(slice_index si,
                                      stip_length_type n,
                                      Side side_at_move);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @param side_at_move side at move
 */
void help_hashed_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n,
                                          Side side_at_move);

/* Determine the starter in a help stipulation in n half-moves
 * @param si identifies slice
 * @param n number of half-moves
 * @param return starting side
 */
Side help_hashed_starter_in_n(slice_index si, stip_length_type n);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean help_hashed_impose_starter(slice_index si, slice_traversal *st);

#endif
