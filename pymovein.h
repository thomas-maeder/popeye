#if !defined(PYMOVEIN_H)
#define PYMOVEIN_H

#include "py.h"
#include "pyslice.h"
#include "boolean.h"

/* This module provides functionality dealing with move inverter slice
 */

/* Allocate a STMoveInverterRootSolvableFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_move_inverter_root_solvable_filter(void);

/* Allocate a STMoveInverterSolvableFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_move_inverter_solvable_filter(void);

/* Allocate a STMoveInverterSeriesFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_move_inverter_series_filter(void);

/* Spin off set play
 * @param si slice index
 * @param st state of traversal
 */
void move_inverter_apply_setplay(slice_index si, stip_structure_traversal *st);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void move_inverter_insert_root(slice_index si, stip_structure_traversal *st);

/* Solve a move inverter at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean move_inverter_root_solve(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type move_inverter_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type move_inverter_has_solution(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean move_inverter_series_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean move_inverter_series_has_solution_in_n(slice_index si,
                                               stip_length_type n);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void move_inverter_detect_starter(slice_index si, stip_structure_traversal *st);

/* Retrieve the starting side of a slice
 * @param si slice index
 * @return current starting side of slice si
 */
Side move_inverter_get_starter(slice_index si);

#endif
