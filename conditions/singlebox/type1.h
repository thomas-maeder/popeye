#if !defined(CONDITIONS_SINGLEBOX_TYPE1_H)
#define CONDITIONS_SINGLEBOX_TYPE1_H

/* Implementation of condition Singlebox Type 1
 */

#include "solving/solve.h"

/* Determine the next legal single box promotee type
 * @param p type of previous promotee (vide if the first promotee type is to be
 *          found)
 * @param c side of promotee type to be found
 * @return next promotee type; vide if there is none
 */
PieNam next_singlebox_prom(PieNam p, Side c);

/* Determine whether the move just played is legal according to Singlebox Type 1
 * @return true iff the move is legal
 */
boolean singlebox_type1_illegal(void);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_singlebox_type1(slice_index si);

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
stip_length_type singlebox_type1_legality_tester_solve(slice_index si,
                                                        stip_length_type n);

#endif
