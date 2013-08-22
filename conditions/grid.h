#if !defined(CONDITIONS_GRID_H)
#define CONDITIONS_GRID_H

#include "py.h"

/* This module implements the condition Grid Chess */

extern int gridvar;
extern int currentgridnum;
extern int gridlines[112][4];
extern int numgridlines;

boolean CrossesGridLines(square i, square j);

#define GridLegal(sq1, sq2) (GridNum(sq1) != GridNum(sq2) ||  \
  (numgridlines && CrossesGridLines((sq1), (sq2))))

/* Validate the geometry of observation according to Grid Chess
 * @return true iff the observation is valid
 */
boolean grid_validate_observation_geometry(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type grid_remove_illegal_moves_solve(slice_index si,
                                                 stip_length_type n);

/* Inialise solving in Grid Chess
 * @param si identifies the root slice of the stipulation
 */
void grid_initialise_solving(slice_index si);

#endif
