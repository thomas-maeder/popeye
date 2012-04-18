#if !defined(OUTPUT_PLAINTEXT_ILLEGAL_SELFCHECK_WRITER_H)
#define OUTPUT_PLAINTEXT_ILLEGAL_SELFCHECK_WRITER_H

#include "stipulation/battle_play/attack_play.h"

/* Allocate a STIllegalSelfcheckWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_illegal_selfcheck_writer_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type illegal_selfcheck_writer_attack(slice_index si,
                                                 stip_length_type n);

#endif
