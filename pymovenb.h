#if !defined(PYMOVENB_H)
#define PYMOVENB_H

/* Implementation of everything related to move number output and the
 * restart option
 * This includes the STRestartGuard slice - stops solutions that start
 * with key moves with numbers lower than the restart number entered
 * by the user
 */

#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"

/* Reset the restart number setting.
 */
void reset_restart_number(void);

/* Retrieve the current restart number
 */
unsigned int get_restart_number(void);

/* Interpret maxmem command line parameter value
 * @param commandLineValue value of -maxmem command line parameter
 * @return true iff optionValue points to a valid value
 */
boolean read_restart_number(char const *optionValue);

/* Instrument stipulation with STRestartGuard slices
 * @param si identifies slice where to start
 */
void stip_insert_restart_guards(slice_index si);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type restart_guard_defend_in_n(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_max_unsolvable);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type restart_guard_help_solve_in_n(slice_index si,
                                               stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type restart_guard_series_solve_in_n(slice_index si,
                                                 stip_length_type n);

#endif
