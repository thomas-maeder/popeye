#if !defined(OUTPUT_PLAINTEXT_TREE_BATTLE_PLAY_SOLUTION_WRITER_H)
#define OUTPUT_PLAINTEXT_TREE_BATTLE_PLAY_SOLUTION_WRITER_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* Allocate a STBattlePlaySolutionWriter defender slice.
 * @return index of allocated slice
 */
slice_index alloc_battle_play_solution_writer(void);

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
battle_play_solution_writer_can_defend_in_n(slice_index si,
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
battle_play_solution_writer_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min,
                                        stip_length_type n_max_unsolvable);

#endif
