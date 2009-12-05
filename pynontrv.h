#if !defined(PYNONTRV_H)
#define PYNONTRV_H

/* Implementation of the "max threat length" optimisation: Solving
 * stops if an attacker's move does not reach the goal nor delivers
 * check nor threatens to reach the goal in a maximum number of moves
 */

#include "pydirect.h"

/* NOTE: exposed for performance reasons only - DON'T WRITE TO THIS
 * VARIABLE!!
 */
extern unsigned int max_nr_nontrivial;


/* Reset the non-trivial optimisation setting to off
 */
void reset_nontrivial_settings(void);

/* Read the requested non-trivial optimisation settings from user input
 * @param tok text token from which to read maximum number of
 *            acceptable non-trivial variations (apart from main variation)
 * @return true iff setting was successfully read
 */
boolean read_max_nr_nontrivial(char const *tok);

/* Read the requested non-trivial optimisation settings from user input
 * @param tok text token from which to read minimimal length of what
 *            is to be considered a non-trivial variation
 * @return true iff setting was successfully read
 */
boolean read_min_length_nontrivial(char const *tok);

/* Retrieve the current minimum length (in full moves) of what is to
 * be considered a non-trivial variation
 * @return maximum acceptable number of non-trivial variations
 */
stip_length_type get_min_length_nontrivial(void);

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_max_nr_nontrivial_guards(void);

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @return slack_length_direct:           key solved next slice
 *         slack_length_direct+1..length: key solved this slice in so
 *                                        many moves
 *         length+2:                      key allows refutations
 *         length+4:                      key reached deadend (e.g.
 *                                        self check)
 */
stip_length_type max_nr_nontrivial_guard_root_defend(table refutations,
                                                     slice_index si);

/* Try to defend after an attempted key move at non-root level
 * @return true iff the defender can defend
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return true iff the defender can defend
 */
boolean max_nr_nontrivial_guard_defend_in_n(slice_index si, stip_length_type n);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int max_nr_nontrivial_guard_can_defend_in_n(slice_index si,
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
stip_length_type max_nr_nontrivial_guard_solve_threats_in_n(table threats,
                                                            slice_index si,
                                                            stip_length_type n);

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param si slice index
 * @param n maximum length of variations to be solved
 * @return true iff >= 1 variation was found
 */
boolean
max_nr_nontrivial_guard_solve_variations_in_n(table threats,
                                              stip_length_type len_threat,
                                              slice_index si,
                                              stip_length_type n);

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param refutations table containing refutations to move just played
 * @param si slice index
 */
void max_nr_nontrivial_guard_root_solve_variations(table threats,
                                                   stip_length_type len_threat,
                                                   table refutations,
                                                   slice_index si);

#endif
