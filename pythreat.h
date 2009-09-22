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

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @return success of key move
 */
attack_result_type maxthreatlength_guard_root_defend(table refutations,
                                                     slice_index si);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return success of key move
 */
attack_result_type maxthreatlength_guard_defend_in_n(slice_index si,
                                                     stip_length_type n);

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

/* Solve threats after an attacker's move
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type maxthreatlength_guard_solve_threats_in_n(table threats,
                                                          slice_index si,
                                                          stip_length_type n);

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param si slice index
 * @param n maximum length of variations to be solved
 * @return true iff >= 1 variation was found
 */
boolean maxthreatlength_guard_solve_variations_in_n(table threats,
                                                    stip_length_type len_threat,
                                                    slice_index si,
                                                    stip_length_type n);

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param refutations table containing refutations to move just played
 * @param si slice index
 */
void maxthreatlength_guard_root_solve_variations(table threats,
                                                 stip_length_type len_threat,
                                                 table refutations,
                                                 slice_index si);

#endif
