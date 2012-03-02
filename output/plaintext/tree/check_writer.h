#if !defined(OUTPUT_PLAINTEXT_TREE_CHECK_WRITER_H)
#define OUTPUT_PLAINTEXT_TREE_CHECK_WRITER_H

#include "stipulation/battle_play/attack_play.h"
#include "stipulation/battle_play/defense_play.h"

/* Allocate a STOutputPlaintextTreeCheckWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_output_plaintext_tree_check_writer_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
output_plaintext_tree_check_writer_attack(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type
output_plaintext_tree_check_writer_defend(slice_index si, stip_length_type n);

#endif
