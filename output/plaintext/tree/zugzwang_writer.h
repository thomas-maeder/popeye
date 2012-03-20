#if !defined(OUTPUT_PLAINTEXT_TREE_ZUGZWANG_WRITER_H)
#define OUTPUT_PLAINTEXT_TREE_ZUGZWANG_WRITER_H

#include "boolean.h"
#include "pystip.h"
#include "stipulation/battle_play/attack_play.h"

/* Allocate a STZugzwangWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_zugzwang_writer_slice(void);

/* Solve a slice, by trying n_min, n_min+2 ... n half-moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type zugzwang_writer_attack(slice_index si, stip_length_type n);

#endif
