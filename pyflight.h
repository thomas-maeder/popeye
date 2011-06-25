#if !defined(PYFLIGHT_H)
#define PYFLIGHT_H

/* Implementation of the "max flight squares" optimisation: Solving
 * stops if an attacker's move grants the defender too many flight
 * squares
 */

#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"

/* Reset the max flights setting to off
 */
void reset_max_flights(void);

/* Read the requested max flight setting from a text token entered by
 * the user
 * @param textToken text token from which to read
 * @return true iff max flight setting was successfully read
 */
boolean read_max_flights(const char *textToken);

/* Retrieve the current max flights setting
 * @return current max flights setting
 *         UINT_MAX if max flights option is not active
 */
unsigned int get_max_flights(void);

/* Instrument stipulation with STMaxFlightsquares slices
 * @param si identifies slice where to start
 */
void stip_insert_maxflight_guards(slice_index si);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type maxflight_guard_defend(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_max_unsolvable);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type maxflight_guard_can_defend(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_max_unsolvable);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type flightsquares_counter_can_help(slice_index si,
                                                stip_length_type n);

#endif
