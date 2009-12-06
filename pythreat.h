#if !defined(PYTHREAT_H)
#define PYTHREAT_H

/* Implementation of the "max threat length" optimisation: Solving
 * stops if an attacker's move does not reach the goal nor delivers
 * check nor threatens to reach the goal in a maximum number of moves
 */

#include "pydirect.h"

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

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_maxthreatlength_guards(void);

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found and written
 */
boolean maxthreatlength_guard_root_solve(slice_index si);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defending side can successfully defend
 */
boolean maxthreatlength_guard_root_defend(slice_index si);

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean maxthreatlength_guard_defend_in_n(slice_index si, stip_length_type n);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int maxthreatlength_guard_can_defend_in_n(slice_index si,
                                                   stip_length_type n,
                                                   unsigned int max_result);

#endif
