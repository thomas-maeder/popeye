#if !defined(STIPULATION_BATTLE_PLAY_CONTINUATION_H)
#define STIPULATION_BATTLE_PLAY_CONTINUATION_H

#include "stipulation/battle_play/defense_play.h"

/* This module provides functionality dealing with continuations.
 */

/* Allocate a STContinuationWriter defender slice.
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of allocated slice
 */
slice_index alloc_continuation_writer_slice(stip_length_type length,
                                            stip_length_type min_length);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
continuation_writer_root_defend(slice_index si,
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
boolean continuation_writer_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        stip_length_type n_min);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @param max_nr_refutations how many refutations should we look for
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=max_nr_refutations refutations found
 *         n+4 refuted - >max_nr_refutations refutations found
 */
stip_length_type
continuation_writer_can_defend_in_n(slice_index si,
                                    stip_length_type n,
                                    stip_length_type n_min,
                                    unsigned int max_nr_refutations);

/* Insert root slices
 * @param si identifies (non-root) slice
 * @param st address of structure representing traversal
 */
void continuation_writer_insert_root(slice_index si,
                                     stip_structure_traversal *st);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void continuation_writer_reduce_to_postkey_play(slice_index si,
                                                stip_structure_traversal *st);

#endif
