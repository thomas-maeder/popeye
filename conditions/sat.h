#if !defined(CONDITIONS_SAT_H)
#define CONDITIONS_SAT_H

#include "solving/solve.h"

extern boolean SATCheck;

extern boolean StrictSAT[nr_sides][maxply+1];

extern int SATFlights[nr_sides];

extern boolean satXY;

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type sat_flight_moves_generator_solve(slice_index si,
                                                   stip_length_type n);

/* Instrument the stipulation with SAT specific king flight move generators
 * @param root_slice root slice of stipulation
 */
void stip_substitute_sat_king_flight_generators(slice_index root_slice);

/* Determine whether a side is in SAT check
 * @param side side for which to test check
 * @return true iff side is in check
 */
boolean echecc_SAT(Side side);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type strict_sat_updater_solve(slice_index si,
                                           stip_length_type n);

/* Instrument a stipulation for strict SAT
 * @param si identifies root slice of stipulation
 */
void stip_insert_strict_sat(slice_index si);

#endif
