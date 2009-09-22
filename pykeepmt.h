#if !defined(PYKEEPMT_H)
#define PYKEEPMT_H

/* Implementation of the "keep mating piece" optimisation:
 * Solving stops once the last piece of the mating side that could
 * deliver mate has been captured.
 */

#include "pyhelp.h"
#include "pydirect.h"
#include "pyslice.h"

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_keepmating_guards(void);

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @return success of key move
 */
attack_result_type keepmating_guard_root_defend(table refutations,
                                                slice_index si);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_direct and we return
 *         n_min)
 */
stip_length_type
keepmating_guard_direct_has_solution_in_n(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_min);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return success of key move
 */
attack_result_type keepmating_guard_defend_in_n(slice_index si,
                                                stip_length_type n);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_result how many refutations should we look for
 * @return number of refutations found (0..max_result+1)
 */
unsigned int keepmating_guard_can_defend_in_n(slice_index si,
                                              stip_length_type n,
                                              unsigned int max_result);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean keepmating_guard_are_threats_refuted_in_n(table threats,
                                                  stip_length_type len_threat,
                                                  slice_index si,
                                                  stip_length_type n);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return number of half moves effectively used
 *         n+2 if no continuation was found
 */
stip_length_type
keepmating_guard_direct_solve_continuations_in_n(table continuations,
                                                 slice_index si,
                                                 stip_length_type n);

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type keepmating_guard_direct_solve_threats_in_n(table threats,
                                                            slice_index si,
                                                            stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean keepmating_guard_help_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean keepmating_guard_help_has_solution_in_n(slice_index si,
                                            stip_length_type n);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void keepmating_guard_help_solve_continuations_in_n(table continuations,
                                                    slice_index si,
                                                    stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean keepmating_guard_series_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean keepmating_guard_series_has_solution_in_n(slice_index si,
                                                  stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void keepmating_guard_series_solve_threats_in_n(table threats,
                                                slice_index si,
                                                stip_length_type n);

/* Solve threats after an attacker's move
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type
keepmating_guard_defender_solve_threats_in_n(table threats,
                                             slice_index si,
                                             stip_length_type n);

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param si slice index
 * @param n maximum length of variations to be solved
 * @return true iff >= 1 variation was found
 */
boolean keepmating_guard_solve_variations_in_n(table threats,
                                               stip_length_type len_threat,
                                               slice_index si,
                                               stip_length_type n);

/* Solve variations after the move that has just been played at root level
 * @param threats table containing threats
 * @param len_threat length of threats
 * @param refutations table containing refutations to move just played
 * @param si slice index
 */
void keepmating_guard_root_solve_variations(table threats,
                                            stip_length_type len_threat,
                                            table refutations,
                                            slice_index si);

#endif
