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
 * @param si identifies slice where to start
 * @return true iff the stipulation could be instrumented
 */
boolean stip_insert_maxthreatlength_guards(slice_index si);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type maxthreatlength_guard_defend(slice_index si,
                                              stip_length_type n);

#endif
