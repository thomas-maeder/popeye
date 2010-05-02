#if !defined(STIPULATION_BATTLE_PLAY_THREAT_H)
#define STIPULATION_BATTLE_PLAY_THREAT_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with threats
 */

/* Determine whether a branch slice has a solution
 * @param si slice index
 * @param n maximal number of moves
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            n_min-4 defense has turned out to be illegal
 *            n_min-2 defense has solved
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type threat_enforcer_has_solution_in_n(slice_index si,
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
stip_length_type threat_enforcer_solve_threats_in_n(table threats,
                                                    slice_index si,
                                                    stip_length_type n,
                                                    stip_length_type n_min);

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found and written, i.e.:
 *            n_min-4 defense has turned out to be illegal
 *            n_min-2 defense has solved
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type threat_enforcer_solve_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <slack_length_battle - stalemate
 *         <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type threat_writer_root_defend(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_min,
                                           unsigned int max_nr_refutations);

/* Try to defend after an attempted key move at non-root level
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defender can defend
 */
boolean threat_writer_defend_in_n(slice_index si,
                                  stip_length_type n,
                                  stip_length_type n_min);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <slack_length_battle - stalemate
           <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
           n+2 refuted - <=max_nr_refutations refutations found
           n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
threat_writer_can_defend_in_n(slice_index si,
                              stip_length_type n,
                              stip_length_type n_min,
                              unsigned int max_nr_refutations);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void threat_writer_reduce_to_postkey_play(slice_index si,
                                          stip_structure_traversal *st);

/* Substitute links to proxy slices by the proxy's target
 * @param si root of sub-tree where to resolve proxies
 * @param st address of structure representing the traversal
 */
void threat_writer_resolve_proxies(slice_index si,
                                   stip_structure_traversal *st);

/* Instrument the stipulation representation so that it can deal with
 * threats
 */
void stip_insert_threat_handlers(void);

#endif
