#if !defined(OPTIMISATION_KILLER_MOVE_COLLECTOR_H)
#define OPTIMISATION_KILLER_MOVE_COLLECTOR_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* This module provides functionality dealing with the attacking side
 * in STKillerMoveCollector stipulation slices.
 */

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximal number of moves
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
killer_move_collector_has_solution_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min,
                                        stip_length_type n_max_unsolvable);

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
stip_length_type
killer_move_collector_solve_in_n(slice_index si,
                                 stip_length_type n,
                                 stip_length_type n_max_unsolvable);

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
stip_length_type
killer_move_collector_defend_in_n(slice_index si,
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
killer_move_collector_can_defend_in_n(slice_index si,
                                      stip_length_type n,
                                      stip_length_type n_max_unsolvable,
                                      unsigned int max_nr_refutations);

/* Instrument stipulation with STKillerMoveCollector slices
 * @param si identifies slice where to start
 */
void stip_insert_killer_move_collectors(slice_index si);

#endif
