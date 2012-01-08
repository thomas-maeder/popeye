#if !defined(OUTPUT_PLAINTEXT_END_OF_SOLUTION_WRITER_H)
#define OUTPUT_PLAINTEXT_END_OF_SOLUTION_WRITER_H

#include "boolean.h"
#include "pystip.h"
#include "pyslice.h"

/* Allocate a STEndOfSolutionWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_end_of_solution_writer_slice(void);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - acceptable number of refutations found
 *         n+4 refuted - more refutations found than acceptable
 */
stip_length_type end_of_solution_writer_defend(slice_index si, stip_length_type n);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type
end_of_solution_writer_can_defend(slice_index si, stip_length_type n);

#endif
