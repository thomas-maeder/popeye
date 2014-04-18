#if !defined(OUTPUT_PLAINTEXT_TREE_THREAT_WRITER_H)
#define OUTPUT_PLAINTEXT_TREE_THREAT_WRITER_H

#include "solving/machinery/solve.h"

/* Allocate a STThreatWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_threat_writer_slice(void);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
void threat_writer_solve(slice_index si);

#endif
