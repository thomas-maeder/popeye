#if !defined(CONDITIONS_STING_H)
#define CONDITIONS_STING_H

/* This module implements the fairy condition Vaulting Kings */

#include "position/position.h"
#include "py.h"

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void sting_generate_moves_for_piece(slice_index si,
                                    square sq_departure,
                                    PieNam p);

/* Initialise the solving machinery with Sting Chess
 * @param si root slice of the solving machinery
 */
void sting_initalise_solving(slice_index si);

#endif
