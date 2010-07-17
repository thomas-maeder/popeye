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

/* Make a slice the successor of a pipe
 * @param pipe identifies the pipe
 * @param succ slice to be made the successor of pipe
 */
void pipe_set_successor(slice_index pipe, slice_index succ);

/* Establish a link between a pipe slice and its successor
 * @param pipe identifies pipe slice
 * @param succ identifies slice to become the successor
 */
void pipe_link(slice_index pipe, slice_index succ);

/* Unlink a pipe and its successor
 * @param pipe identifies pipe slice
 */
void pipe_unlink(slice_index pipe);

/* Replace a slice by another. Links the substitute to the replaced
 * slice's predecessor and successor, but doesn't adjust the links
 * from other slices that may reference the replaced slice.
 * Deallocates the replaced slice.
 * @param replaced identifies the replaced slice
 * @param substitute identifies the substitute
 */
void pipe_replace(slice_index replaced, slice_index substitute);

/* Append a slice to another. Links the two slices and establishes the
 * same connection from the appended slice to the previous successor
 * that existed between the previously connected slices.
 * @param pos identifies where to append
 * @param appended identifies appended slice
 */
void pipe_append(slice_index pos, slice_index appended);

/* Detect starter field with the starting side if possible.
 * @param pipe identifies slice being traversed
 * @param st status of traversal
 */
void pipe_detect_starter(slice_index pipe, stip_structure_traversal *st);

/* Impose the starting side on a stipulation
 * @param pipe identifies pipe
 * @param st address of structure that holds the state of the traversal
 */
void pipe_impose_starter(slice_index pipe, stip_structure_traversal *st);

/* Impose the starting side on a stipulation. Impose the inverted
 * starter on the slice's successor. 
 * @param pipe identifies pipe
 * @param st address of structure that holds the state of the traversal
 */
void pipe_impose_inverted_starter(slice_index pipe, stip_structure_traversal *st);

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void pipe_resolve_proxies(slice_index si, stip_structure_traversal *st);

/* Determine whether a slice has a solution
 * @param pipe slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type pipe_has_solution(slice_index pipe);

/* Traverse a subtree
 * @param branch root slice of subtree
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_pipe(slice_index pipe,
                                  stip_structure_traversal *st);

/* Traversal of the moves of some pipe slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_pipe(slice_index si, stip_move_traversal *st);

#endif
