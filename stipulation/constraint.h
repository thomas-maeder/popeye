#if !defined(PYREFLXG_H)
#define PYREFLXG_H

/* In reflex stipulations, guard against tries that would allow the
 * wrong side to reach the goal.
 */

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_attacker_filter_root_solve(slice_index si);

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_defender_filter_root_solve(slice_index si);

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_help_filter_root_solve(slice_index si);

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_battle)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type reflex_attacker_filter_solve_in_n(slice_index si,
                                                   stip_length_type n,
                                                   stip_length_type n_min);

/* Solve a slice - adapter for direct slices
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_attacker_filter_solve(slice_index si);

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
 *         goal (in which case n_min<slack_length_battle and we return
 *         n_min)
 */
stip_length_type
reflex_attacker_filter_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min);

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_battle)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type
reflex_attacker_filter_direct_solve_threats_in_n(table threats,
                                                 slice_index si,
                                                 stip_length_type n,
                                                 stip_length_type n_min);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defending side can successfully defend
 */
boolean reflex_defender_filter_root_defend(slice_index si,
                                           stip_length_type n_min);

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defender can defend
 */
boolean reflex_defender_filter_defend_in_n(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_min);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_nr_refutations how many refutations should we look for
 * @return n+4 refuted - >max_nr_refutations refutations found
           n+2 refuted - <=max_nr_refutations refutations found
           <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
 */
stip_length_type
reflex_defender_filter_can_defend_in_n(slice_index si,
                                       stip_length_type n,
                                       unsigned int max_nr_refutations);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_help_filter_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean reflex_help_filter_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void reflex_help_filter_solve_threats_in_n(table threats,
                                           slice_index si,
                                           stip_length_type n);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean
reflex_attacker_filter_are_threats_refuted_in_n(table threats,
                                                stip_length_type len_threat,
                                                slice_index si,
                                                stip_length_type n);

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean reflex_series_filter_root_solve(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean reflex_series_filter_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean reflex_series_filter_has_solution_in_n(slice_index si,
                                               stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void reflex_series_filter_solve_threats_in_n(table threats,
                                             slice_index si,
                                             stip_length_type n);

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 */
void reflex_attacker_filter_make_setplay_slice(slice_index si,
                                               stip_structure_traversal *st);

/* Spin off a set play slice at root level
 * @param si slice index
 * @param st state of traversal
 */
void reflex_defender_filter_make_setplay_slice(slice_index si,
                                               stip_structure_traversal *st);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void reflex_attacker_filter_reduce_to_postkey_play(slice_index si,
                                                   stip_structure_traversal *st);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void reflex_defender_filter_reduce_to_postkey_play(slice_index si,
                                                   stip_structure_traversal *st);

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 */
void reflex_filter_impose_starter(slice_index si, stip_structure_traversal *st);

/* Instrument a branch with STReflexGuard slices for a reflex
 * stipulation 
 * @param si root of branch to be instrumented
 * @param proxy_to_avoided identifies what branch needs to be guarded from
 * @return identifier of branch entry slice after insertion
 */
slice_index slice_insert_reflex_filters(slice_index si,
                                        slice_index proxy_to_avoided);

/* Instrument a branch with STReflexGuard slices for a semi-reflex
 * stipulation 
 * @param si root of branch to be instrumented
 * @param proxy_to_avoided identifies what branch needs to be guarded from
 * @return identifier of branch entry slice after insertion
 */
slice_index slice_insert_reflex_filters_semi(slice_index si,
                                             slice_index proxy_to_avoided);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reflex_attacker_filter_insert_root(slice_index si,
                                        stip_structure_traversal *st);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reflex_defender_filter_insert_root(slice_index si,
                                        stip_structure_traversal *st);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reflex_help_filter_insert_root(slice_index si,
                                    stip_structure_traversal *st);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void reflex_series_filter_insert_root(slice_index si,
                                      stip_structure_traversal *st);

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void reflex_filter_resolve_proxies(slice_index si, stip_structure_traversal *st);

#endif
