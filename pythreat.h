#if !defined(PYTHREAT_H)
#define PYTHREAT_H

/* Implementation of the "max threat length" optimisation: Solving
 * stops if an attacker's move does not reach the goal nor delivers
 * check nor threatens to reach the goal in a maximum number of moves
 */

#include "stipulation/battle_play/defense_play.h"

/* Reset the max threats setting to off
 */
void reset_max_threat_length(void);

/* Read the requested max threat length setting from a text token
 * entered by the user
 * @param textToken text token from which to read
 * @return true iff max threat length setting was successfully read
 */
boolean read_max_threat_length(const char *textToken);

/* Retrieve the current max threat length setting
 * @return current max threat length setting
 *         no_stip_length if max threats option is not active
 */
stip_length_type get_max_threat_length(void);

/* Instrument stipulation with STMaxThreatLength slices
 * @return true iff the stipulation could be instrumented
 */
boolean stip_insert_maxthreatlength_guards(void);

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found and written
 */
boolean maxthreatlength_guard_root_solve(slice_index si);

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
maxthreatlength_guard_defend_in_n(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_min,
                                  stip_length_type n_max_unsolvable);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
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
maxthreatlength_guard_can_defend_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable,
                                      unsigned int max_nr_refutations);

#endif
