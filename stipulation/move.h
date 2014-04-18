#if !defined(STIPULATION_MOVE_H)
#define STIPULATION_MOVE_H

#include "stipulation/slice_type.h"
#include "stipulation/structure_traversal.h"

/* This module plays deals with moves
 */

/* Try to start slice insertion within the sequence of slices that deal with
 * move execution.
 * @param base_type type relevant for determining the position of the slices to
 *                  be inserted
 * @param si identifies the slice where to actually start the insertion traversal
 * @param st address of the structure representing the insertion traversal
 * @param end_of_move_slice_sequence type of slice which ends the move sequence
 * @return true iff base_type effectively is a type from the move slices sequence
 */
boolean move_start_insertion(slice_type base_type,
                             slice_index si,
                             stip_structure_traversal *st,
                             slice_type end_of_move_slice_sequence);

/* Initialise a structure traversal for inserting slices
 * into the move execution sequence
 * @param st address of structure representing the traversal
 */
void move_init_slice_insertion_traversal(stip_structure_traversal *st);

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
