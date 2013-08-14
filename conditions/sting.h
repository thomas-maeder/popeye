#if !defined(CONDITIONS_STING_H)
#define CONDITIONS_STING_H

/* This module implements the fairy condition Vaulting Kings */

#include "position/position.h"
#include "py.h"
#include "solving/observation.h"

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void sting_generate_moves_for_piece(slice_index si, PieNam p);

/* Determine whether a square is observed in Sting Chess
* @param si identifies tester slice
* @return true iff sq_target is observed
*/
boolean sting_is_square_observed(slice_index si, evalfunction_t *evaluate);

/* Initialise the solving machinery with Sting Chess
 * @param si root slice of the solving machinery
 */
void sting_initalise_solving(slice_index si);

#endif
