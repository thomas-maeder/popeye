#if !defined(OUTPUT_PLAINTEXT_END_OF_PHASE_WRITER_H)
#define OUTPUT_PLAINTEXT_END_OF_PHASE_WRITER_H

#include "pystip.h"
#include "pyslice.h"

/* Allocate a STEndOfPhaseWriter slice.
 * @return index of allocated slice
 */
slice_index alloc_end_of_phase_writer_slice(void);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type end_of_phase_writer_has_solution(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type end_of_phase_writer_solve(slice_index si);

#endif
