#if !defined(PYMOVEIN_H)
#define PYMOVEIN_H

#include "py.h"
#include "pyslice.h"
#include "boolean.h"

/* This module provides functionality dealing with move inverter slice
 */

/* Allocate a move inverter slice.
 * @param next next slice
 * @return index of allocated slice
 */
slice_index alloc_move_inverter_slice(slice_index next);

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 * @return true iff this slice has been sucessfully traversed
 */
boolean move_inverter_root_make_setplay_slice(slice_index si,
                                              struct slice_traversal *st);

/* Solve a move inverter at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean move_inverter_root_solve(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean move_inverter_solve(slice_index si);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 * @return true iff slice has been successfully traversed
 */
boolean move_inverter_detect_starter(slice_index si, slice_traversal *st);

/* Retrieve the starting side of a slice
 * @param si slice index
 * @return current starting side of slice si
 */
Side move_inverter_get_starter(slice_index si);

#endif
