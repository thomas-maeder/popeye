#if !defined(SOLVING_BATTLE_PLAY_TRY_H)
#define SOLVING_BATTLE_PLAY_TRY_H

#include "utilities/table.h"
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

/* Allocate a STRefutationsAllocator defender slice.
 * @return index of allocated slice
 */
slice_index alloc_refutations_allocator(void);

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
stip_length_type refutations_allocator_defend(slice_index si, stip_length_type n);

/* Allocate a STRefutationsSolver defender slice.
 * @return index of allocated slice
 */
slice_index alloc_refutations_solver(void);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type refutations_solver_defend(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutations_collector_attack(slice_index si,
                                              stip_length_type n);

/* Allocate a STRefutationsAvoider slice.
 * @param max_nr_refutations maximum number of refutations to be allowed
 * @return index of allocated slice
 */
slice_index alloc_refutations_avoider_slice(unsigned int max_nr_refutations);

/* Spin a tester slice off a STHelpHashed slice
 * @param base_slice identifies the STHelpHashed slice
 * @return id of allocated slice
 */
void spin_off_testers_refutations_avoider(slice_index si,
                                          stip_structure_traversal *st);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutations_avoider_attack(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutations_filter_attack(slice_index si, stip_length_type n);

/* Instrument the stipulation structure with slices solving tries
 * @param root_slice root slice of the stipulation
 */
void stip_insert_try_solvers(slice_index si);

/* Spin a separate branch for solving refutations off the STRefutationsSolver
 * slice
 * @param si identifies entry branch into stipulation
 */
void stip_spin_off_refutation_solver_slices(slice_index si);

#endif
