#if !defined(OUTPUT_PLAINTEXT_TREE_CHECK_DETECTOR_H)
#define OUTPUT_PLAINTEXT_TREE_CHECK_DETECTOR_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* Reset the pending check state
 */
void reset_pending_check(void);

/* Write a possible pending check
 */
void flush_pending_check(ply move_ply);

/* Allocate a STOutputPlaintextTreeCheckDetectorAttackerFilter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index
alloc_output_plaintext_tree_check_detector_attacker_filter_slice(stip_length_type length,
                                                                 stip_length_type min_length);

/* Determine whether there is a solution in n half moves, by trying
 * n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
output_plaintext_tree_check_detector_has_solution_in_n(slice_index si,
                                                       stip_length_type n,
                                                       stip_length_type n_min,
                                                       stip_length_type n_max_unsolvable);

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
output_plaintext_tree_check_detector_solve_in_n(slice_index si,
                                                stip_length_type n,
                                                stip_length_type n_min,
                                                stip_length_type n_max_unsolvable);

/* Allocate a STOutputPlaintextTreeCheckDetectorDefenderFilter slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index
alloc_output_plaintext_tree_check_detector_defender_filter_slice(stip_length_type length,
                                                                 stip_length_type min_length);

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
output_plaintext_tree_check_detector_can_defend_in_n(slice_index si,
                                                     stip_length_type n,
                                                     stip_length_type n_max_unsolvable,
                                                     unsigned int max_nr_refutations);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type
output_plaintext_tree_check_detector_defend_in_n(slice_index si,
                                                 stip_length_type n,
                                                 stip_length_type n_min,
                                                 stip_length_type n_max_unsolvable);

#endif
