#if !defined(PYPIPE_H)
#define PYPIPE_H

/* Functionality related to "pipe slices"; i.e. slices that have a
 * next member and whose functions normally delegate to those of the
 * next slice.
 */

#include "pystip.h"
#include "pyslice.h"

/* Allocate a new pipe and make an existing pipe its successor
 * @param successor successor of slice to be allocated
 * @return newly allocated slice
 */
slice_index alloc_pipe(slice_index successor);

/* Installs a pipe slice before pipe slice si. I.e. every link that
 * currently leads to slice si will now lead to the new slice.
 * @param si identifies pipe slice before which to insert a new pipe slice
 */
void pipe_insert_before(slice_index si);

/* Installs a pipe slice between pipe slice si and its current
 * successor slice. 
 * @param si identifies pipe slice after which to insert a new pipe slice
 */
void pipe_insert_after(slice_index si);

/* Removes a pipe slice after pipe slice si. This is the inverse
 * operation to pipe_insert_after(); if another slice references
 * slices[si].u.pipe.next, that reference will be dangling.  
 * @param si identifies pipe slice after which to insert a new pipe slice
 */
void pipe_remove_after(slice_index si);

/* Detect starter field with the starting side if possible. 
 * @param si identifies slice
 * @param same_side_as_root does si start with the same side as root?
 * @return does the leaf decide on the starter?
 */
who_decides_on_starter pipe_detect_starter(slice_index si,
                                           boolean same_side_as_root);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean pipe_impose_starter(slice_index si, slice_traversal *st);

/* Impose the starting side on a stipulation. Impose the inverted
 * starter on the slice's successor. 
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean pipe_impose_inverted_starter(slice_index si, slice_traversal *st);

/* Traverse the sub-graph starting at the successor slice of a pipe
 * slice (but don't traverse possible other children of the pipe
 * slice)
 * @param pipe identifies pipe slice
 * @return true iff the sub-graph has been successfully traversed
 */
boolean pipe_traverse_next(slice_index pipe, slice_traversal *st);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type pipe_has_solution(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @param n exact number of half moves until end state has to be reached
 * @return true iff slice si has a solution
 */
boolean pipe_series_solve_in_n(slice_index si, stip_length_type n);

#endif
