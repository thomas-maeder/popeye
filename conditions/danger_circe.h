#if !defined(CONDITIONS_DANGER_CIRCE_H)
#define CONDITIONS_DANGER_CIRCE_H

#include "pieces/pieces.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* This module implements the condition Danger Circe */

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void danger_circe_generate_moves_for_piece(slice_index si);

/* Inialise the solving machinery with Danger Circe
 * @param si identifies root slice of solving machinery
 */
void danger_circe_initialise_solving(slice_index si);

#endif
