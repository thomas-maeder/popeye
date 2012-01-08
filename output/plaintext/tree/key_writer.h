#if !defined(OUTPUT_PLAINTEXT_TREE_KEY_WRITER_H)
#define OUTPUT_PLAINTEXT_TREE_KEY_WRITER_H

#include "stipulation/battle_play/defense_play.h"

/* Allocate a STKeyWriter defender slice.
 * @return index of allocated slice
 */
slice_index alloc_key_writer(void);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type key_writer_can_defend(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type key_writer_defend(slice_index si, stip_length_type n);

#endif
