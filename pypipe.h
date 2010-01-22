#if !defined(PYPIPE_H)
#define PYPIPE_H

/* Functionality related to "pipe slices"; i.e. slices that have a
 * next member and whose functions normally delegate to those of the
 * next slice.
 */

#include "pystip.h"
#include "pyslice.h"

/* Allocate a new non-proxy pipe
 * @param type which slice type
 * @return newly allocated slice
 */
slice_index alloc_pipe(SliceType type);

/* Make a slice the predecessor of a pipe
 * @param pipe identifies the pipe
 * @param pred slice to be made the predecessor of pipe
 */
void pipe_set_predecessor(slice_index pipe, slice_index pred);

/* Make a slice the successor of a pipe
 * @param pipe identifies the pipe
 * @param succ slice to be made the successor of pipe
 */
void pipe_set_successor(slice_index pipe, slice_index succ);

/* Detect starter field with the starting side if possible.
 * @param pipe identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean pipe_detect_starter(slice_index pipe, slice_traversal *st);

/* Impose the starting side on a stipulation
 * @param pipe identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean pipe_impose_starter(slice_index pipe, slice_traversal *st);

/* Impose the starting side on a stipulation. Impose the inverted
 * starter on the slice's successor. 
 * @param pipe identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean pipe_impose_inverted_starter(slice_index pipe, slice_traversal *st);

/* Traverse the sub-graph starting at the successor slice of a pipe
 * slice (but don't traverse possible other children of the pipe
 * slice)
 * @param pipe identifies pipe slice
 * @return true iff the sub-graph has been successfully traversed
 */
boolean pipe_traverse_next(slice_index pipe, slice_traversal *st);

/* Determine whether a slice has a solution
 * @param pipe slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type pipe_has_solution(slice_index pipe);

/* Determine whether a slice has a solution
 * @param pipe slice index
 * @param n exact number of half moves until end state has to be reached
 * @return true iff slice si has a solution
 */
boolean pipe_series_solve_in_n(slice_index pipe, stip_length_type n);

#endif
