#if !defined(CONDITIONS_ANNAN_H)
#define CONDITIONS_ANNAN_H

#include "conditions/conditions.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"
#include "position/board.h"
#include "pieces/pieces.h"

/* This module implements the conditions Annan Chess and Nanna Chess */

extern ConditionLetteredVariantType annan_type;

/* Make sure that the observer has the expected walk - annanised or originally
 * @return true iff the observation is valid
 */
boolean annan_enforce_observer_walk(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void annan_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with Annan Chess
 * @param si identifies root slice of solving machinery
 */
void annan_initialise_solving(slice_index si);

/* Make sure that the observer has the expected walk - nannaised or originally
 * @return true iff the observation is valid
 */
boolean nanna_enforce_observer_walk(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void nanna_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with Annan Chess
 * @param si identifies root slice of solving machinery
 */
void nanna_initialise_solving(slice_index si);

#endif
