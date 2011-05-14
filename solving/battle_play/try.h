#if !defined(STIPULATION_BATTLE_PLAY_TRY_H)
#define STIPULATION_BATTLE_PLAY_TRY_H

#include "pytable.h"
#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with writing tries.
 * This functionality is a superset of that provided by solution_writer
 */

/* Table where refutations are collected
 * Exposed for read-only access only */
extern table refutations;

/* Read the maximum number of refutations that the user is interested
 * to see
 * @param tok input token from which to read the number
 * @return true iff the number could be successfully read
 */
boolean read_max_nr_refutations(char const *tok);

/* Set the maximum number of refutations that the user is interested
 * to see to some value
 * @param mnr maximum number of refutations that the user is
 *            interested to see
 */
void set_max_nr_refutations(unsigned int mnr);

/* Retrieve the maximum number of refutations that the user is interested
 * to see to some value
 * @return maximum number of refutations that the user is interested to see
 */
unsigned int get_max_nr_refutations(void);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type refutations_allocator_defend(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_max_unsolvable);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
refutations_allocator_can_defend(slice_index si,
                                 stip_length_type n,
                                 stip_length_type n_max_unsolvable);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type try_solver_defend(slice_index si,
                                   stip_length_type n,
                                   stip_length_type n_max_unsolvable);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type try_solver_can_defend(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable);

/* Allocate a STRefutationsSolver defender slice.
 * @return index of allocated slice
 */
slice_index alloc_refutations_solver(void);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type refutations_solver_defend(slice_index si,
                                           stip_length_type n,
                                           stip_length_type n_max_unsolvable);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
refutations_solver_can_defend(slice_index si,
                              stip_length_type n,
                              stip_length_type n_max_unsolvable);

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximal number of moves
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
refutations_collector_can_attack(slice_index si,
                                 stip_length_type n,
                                 stip_length_type n_max_unsolvable);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutations_collector_attack(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_max_unsolvable);

/* Instrument a branch with try solving slices
 * @param adapter adapter slice leading into the branch
 * @param max_nr_refutations maximum number of refutations per try
 */
void branch_insert_try_solvers(slice_index adapter,
                               unsigned int max_nr_refutations);

#endif
