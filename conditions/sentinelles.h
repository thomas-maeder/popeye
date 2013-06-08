#if !defined(CONDITION_SENTINELLES_H)
#define CONDITION_SENTINELLES_H

#include "solving/solve.h"

/* This module implements Echecs Sentinelles.
 */

extern unsigned int sentinelles_max_nr_pawns[nr_sides];
extern unsigned int sentinelles_max_nr_pawns_total;

extern PieNam sentinelle;

extern boolean SentPionAdverse;
extern boolean SentPionNeutral;
extern boolean flagparasent;

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
stip_length_type sentinelles_inserter_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_sentinelles_inserters(slice_index si);

#endif
