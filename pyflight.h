#if !defined(PYFLIGHT_H)
#define PYFLIGHT_H

/* Implementation of the "max flight squares" optimisation: Solving
 * stops if an attacker's move grants the defender too many flight
 * squares
 */

#include "stipulation/battle_play/defense_play.h"

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
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type maxflight_guard_defend_in_n(slice_index si,
                                             stip_length_type n,
                                             stip_length_type n_max_unsolvable);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
maxflight_guard_can_defend_in_n(slice_index si,
                                stip_length_type n,
                                stip_length_type n_max_unsolvable,
                                unsigned int max_nr_refutations);

#endif
