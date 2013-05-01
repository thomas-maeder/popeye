#if !defined(STIPULATION_MOVE_H)
#define STIPULATION_MOVE_H

#include "stipulation/slice_type.h"
#include "stipulation/structure_traversal.h"

/* This module plays deals with moves
 */

/* Start inserting according to the slice type order for move execution
 * @param si identifies starting point of insertion
 * @param st insertion traversal where we come from and will return to
 * @param end_of_factored_order slice type where to return to insertion defined
 *                              by st
 */
void start_insertion_according_to_move_order(slice_index si,
                                             stip_structure_traversal *st,
                                             slice_type end_of_temporary);

/* Instrument moves with a slice type
 * @param identifies where to start instrumentation
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_moves(slice_index si, slice_type type);

/* Instrument dummy moves with a slice type
 * @param identifies where to start instrumentation
 * @param type type of slice with which to instrument moves
 */
void stip_instrument_dummy_moves(slice_index si, slice_type type);

#endif
