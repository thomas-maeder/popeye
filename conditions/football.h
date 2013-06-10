#if !defined(SOLVING_CONDITIONS_FOOTBALL_H)
#define SOLVING_CONDITIONS_FOOTBALL_H

#include "solving/solve.h"

/* This module implements Football Chess */

extern PieNam current_football_substitution[maxply+1];

/* Initialise the substitutes' bench for the current twin
 */
void init_football_substitutes(void);

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
stip_length_type football_chess_substitutor_solve(slice_index si,
                                                   stip_length_type n);

/* Instrument slices with promotee markers
 */
void stip_insert_football_chess(slice_index si);

#endif
