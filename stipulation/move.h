#if !defined(STIPULATION_MOVE_H)
#define STIPULATION_MOVE_H

#include "stipulation/slice_type.h"
#include "stipulation/structure_traversal.h"

/* This module plays deals with moves
 */

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

/* Insert slices into a move execution slices sequence.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by move_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param context initial context of the insertion traversal; typically the
 *                current context of a surrounding traversal that has arrived
 *                at slice si
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void move_insert_slices(slice_index si,
                        stip_traversal_context_type context,
                        slice_index const prototypes[],
                        unsigned int nr_prototypes);

#endif
