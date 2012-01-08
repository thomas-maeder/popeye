#if !defined(OUTPUT_PLAINTEXT_TREE_REFUTATION_WRITER_H)
#define OUTPUT_PLAINTEXT_TREE_REFUTATION_WRITER_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* Allocate a STRefutationWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_refutation_writer_slice(void);

/* Determine whether there is a solution in n half moves.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutation_writer_can_attack(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutation_writer_attack(slice_index si, stip_length_type n);

#endif
