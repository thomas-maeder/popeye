#if !defined(OPTIMISATION_GOALS_MATE_NEUTRAL_RETRACTABLE_H)
#define OPTIMISATION_GOALS_MATE_NEUTRAL_RETRACTABLE_H

#include "solving/solve.h"

/* This module optimises generation of moves that are supposed to reach mate.
 * It prevents moves by neutral pieces from being played if the defending side
 * can simply take them back.
 */

/* is optimisation possible with the current fairy elements? */
extern boolean optim_neutralretractable;

/* Optimise move generation by inserting orthodox mating move generators
 * @param si identifies the root slice of the stipulation
 */
void stip_optimise_by_omitting_retractable_neutral_moves(slice_index si);

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
stip_length_type mate_remove_retractable_solve(slice_index si,
                                               stip_length_type n);

#endif
