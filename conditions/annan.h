#if !defined(CONDITIONS_ANNAN_H)
#define CONDITIONS_ANNAN_H

#include "py.h"
#include "position/board.h"
#include "utilities/boolean.h"
#include "pyproc.h"

/* This module implements the condition Annan Chess */

typedef enum
{
  annan_type_A,
  annan_type_B,
  annan_type_C,
  annan_type_D
} annan_type_type;

extern annan_type_type annan_type;

/* Determine whether a square is observed in Annan Chess
* @param si identifies tester slice
* @return true iff sq_target is observed
*/
boolean annan_is_square_observed(slice_index si, evalfunction_t *evaluate);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void annan_generate_moves_for_piece(slice_index si,
                                    square sq_departure,
                                    PieNam p);

/* Inialise the solving machinery with Annan Chess
 * @param si identifies root slice of solving machinery
 */
void annan_initialise_solving(slice_index si);

#endif
