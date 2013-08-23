#if !defined(CONDITIONS_ANNAN_H)
#define CONDITIONS_ANNAN_H

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"
#include "position/board.h"

/* This module implements the condition Annan Chess */

typedef enum
{
  annan_type_A,
  annan_type_B,
  annan_type_C,
  annan_type_D
} annan_type_type;

extern annan_type_type annan_type;

/* Make sure that the observer has the expected walk - annanised or originally
 * @return true iff the observation is valid
 */
boolean annan_enforce_observer_walk(slice_index si);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void annan_generate_moves_for_piece(slice_index si, PieNam p);

/* Inialise the solving machinery with Annan Chess
 * @param si identifies root slice of solving machinery
 */
void annan_initialise_solving(slice_index si);

#endif
