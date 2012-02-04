#if !defined(OUTPUT_PLAINTEXT_LINE_LINE_WRITER_H)
#define OUTPUT_PLAINTEXT_LINE_LINE_WRITER_H

#include "pyslice.h"

/* This module provides the STOutputPlaintextLineLineWriter slice type.
 * Slices of this type write lines in line mode.
 */

/* identifies a slice whose starter is the nominal starter of the stipulation
 * before any move inversions are applied
 * (e.g. in a h#N.5, this slice's starter is Black)
 */
extern slice_index output_plaintext_slice_determining_starter;

/* Allocate a STOutputPlaintextLineLineWriter slice.
 * @param goal goal to be reached at end of line
 * @return index of allocated slice
 */
slice_index alloc_line_writer_slice(Goal goal);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type line_writer_solve(slice_index si);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length_battle - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type line_writer_defend(slice_index si, stip_length_type n);

#endif
