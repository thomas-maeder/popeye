#if !defined(OUTPUT_PLAINTEXT_TREE_CHECK_DETECTOR_H)
#define OUTPUT_PLAINTEXT_TREE_CHECK_DETECTOR_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* Allocate a STOutputPlaintextTreeCheckWriterAttackerFilter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index
alloc_output_plaintext_tree_check_writer_attacker_filter_slice(stip_length_type length,
                                                               stip_length_type min_length);

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
output_plaintext_tree_check_writer_has_solution_in_n(slice_index si,
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
output_plaintext_tree_check_writer_solve_in_n(slice_index si,
                                              stip_length_type n,
                                              stip_length_type n_max_unsolvable);

/* Allocate a STOutputPlaintextTreeCheckWriterDefenderFilter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index
alloc_output_plaintext_tree_check_writer_defender_filter_slice(stip_length_type length,
                                                               stip_length_type min_length);

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
output_plaintext_tree_check_writer_can_defend_in_n(slice_index si,
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
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
output_plaintext_tree_check_writer_defend_in_n(slice_index si,
                                               stip_length_type n,
                                               stip_length_type n_max_unsolvable);

#endif
