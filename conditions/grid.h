#if !defined(CONDITIONS_GRID_H)
#define CONDITIONS_GRID_H

#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

/* This module implements the condition Grid Chess */

typedef enum
{
  grid_normal,
  grid_vertical_shift,
  grid_horizontal_shift,
  grid_diagonal_shift,
  grid_orthogonal_lines,
  grid_irregular
} grid_type_type;

extern grid_type_type grid_type;

boolean CrossesGridLines(square sq_departure, square sq_arrival);

void IntialiseIrregularGridLines(void);

typedef enum
{
  gridline_horizonal,
  gridline_vertical
} gridline_direction;

boolean PushIrregularGridLine(unsigned int file,
                              unsigned int row,
                              unsigned int length,
                              gridline_direction dir);

#define GridLegal(sq1,sq2) \
    (GridNum(sq1)!=GridNum(sq2) \
     || (grid_type==grid_irregular && CrossesGridLines((sq1), (sq2))))

/* Validate the geometry of observation according to Grid Chess
 * @return true iff the observation is valid
 */
boolean grid_validate_observation_geometry(slice_index si);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void grid_remove_illegal_moves_solve(slice_index si);

/* Inialise solving in Grid Chess
 * @param si identifies the root slice of the stipulation
 */
void grid_initialise_solving(slice_index si);

#endif
