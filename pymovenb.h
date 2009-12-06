#if !defined(PYMOVENB_H)
#define PYMOVENB_H

/* Implementation of everything related to move number output and the
 * restart option
 * This includes the STRestartGuard slice - stops solutions that start
 * with key moves with numbers lower than the restart number entered
 * by the user
 */

#include "pyhelp.h"
#include "pydirect.h"

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
 */
void stip_insert_restart_guards(void);

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @param max_number_refutations maximum number of refutations to deliver
 * @return slack_length_direct:           key solved next slice
 *         slack_length_direct+1..length: key solved this slice in so
 *                                        many moves
 *         length+2:                      key allows refutations
 *         length+4:                      key reached deadend (e.g.
 *                                        self check)
 */
stip_length_type restart_guard_root_defend(table refutations,
                                           slice_index si,
                                           unsigned int max_number_refutations);

/* Solve threats after an attacker's move
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type restart_guard_solve_threats_in_n(table threats,
                                                  slice_index si,
                                                  stip_length_type n);

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param si slice index
 * @param n maximum length of variations to be solved
 * @return true iff >= 1 variation was found
 */
boolean restart_guard_solve_variations_in_n(table threats,
                                            stip_length_type len_threat,
                                            slice_index si,
                                            stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean restart_guard_help_solve_in_n(slice_index si, stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean restart_guard_series_solve_in_n(slice_index si, stip_length_type n);

#endif
