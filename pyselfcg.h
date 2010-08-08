#if !defined(PYSELFCG_H)
#define PYSELFCG_H

/* STSelfCheckGuard slice - stops solutions with moves that expose the
 * own king
 */

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"
#include "stipulation/help_play/play.h"
#include "pyslice.h"

/* Instrument stipulation with STSelfCheckGuard slices
 */
void stip_insert_selfcheck_guards(void);

/* Allocate a STSelfCheckGuardAttackerFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_attacker_filter(stip_length_type length,
                                                  stip_length_type min_length);

/* Allocate a STSelfCheckGuardDefenderFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_defender_filter(stip_length_type length,
                                                  stip_length_type min_length);

/* Allocate a STSelfCheckGuardHelpFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_help_filter(stip_length_type length,
                                              stip_length_type min_length);

/* Allocate a STSelfCheckGuardSeriesFilter slice
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_series_filter(stip_length_type length,
                                                stip_length_type min_length);

/* Allocate a STSelfCheckGuardRootSolvableFilter slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_root_solvable_filter(void);

/* Allocate a STSelfCheckGuardSolvableFilter slice
 * @return allocated slice
 */
slice_index alloc_selfcheck_guard_solvable_filter(void);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void selfcheckguard_root_solvable_filter_reduce_to_postkey_play(slice_index si,
                                                                stip_structure_traversal *st);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type selfcheck_guard_root_solve(slice_index si);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void selfcheckguard_defender_filter_reduce_to_postkey_play(slice_index si,
                                                           stip_structure_traversal *st);

/* Solve a slice at
 * @param si slice index
 * @return true iff >=1 solution was found
 */
has_solution_type selfcheck_guard_solve(slice_index si);

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @return true if >=1 solution was found
 */
boolean selfcheck_guard_attacker_filter_root_solve_in_n(slice_index si);

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type selfcheck_guard_solve_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_max_unsolvable);
/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void selfcheck_guard_attacker_filter_make_root(slice_index si,
                                               stip_structure_traversal *st);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type selfcheck_guard_has_solution(slice_index si);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void selfcheck_guard_solvable_filter_make_root(slice_index si,
                                                 stip_structure_traversal *st);

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
stip_length_type selfcheck_guard_defend_in_n(slice_index si,
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
selfcheck_guard_can_defend_in_n(slice_index si,
                                stip_length_type n,
                                stip_length_type n_max_unsolvable,
                                unsigned int max_nr_refutations);

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void
selfcheck_guard_defender_filter_make_setplay_slice(slice_index si,
                                                   stip_structure_traversal *st);

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
selfcheck_guard_direct_has_solution_in_n(slice_index si,
                                         stip_length_type n,
                                         stip_length_type n_min,
                                         stip_length_type n_max_unsolvable);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type selfcheck_guard_help_solve_in_n(slice_index si,
                                                 stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type selfcheck_guard_help_has_solution_in_n(slice_index si,
                                                        stip_length_type n);

/* Produce slices representing set play
 * @param si slice index
 * @param st state of traversal
 */
void selfcheck_guard_help_make_setplay_slice(slice_index si,
                                             stip_structure_traversal *st);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void selfcheck_guard_help_make_root(slice_index si,
                                      stip_structure_traversal *st);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type selfcheck_guard_series_solve_in_n(slice_index si,
                                                   stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type selfcheck_guard_series_has_solution_in_n(slice_index si,
                                                          stip_length_type n);

/* Recursively make a sequence of root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void selfcheck_guard_series_make_root(slice_index si,
                                        stip_structure_traversal *st);

#endif
