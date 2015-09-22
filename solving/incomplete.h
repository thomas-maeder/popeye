#if !defined(SOLVING_INCOMPLETE_H)
#define SOLVING_INCOMPLETE_H

/* This module allows various options to report whether they have caused solving
 * to be incomplete. And to the output modules to report that the written
 * solution may not be complete for that reason.
 * Solving is incomplete per phase - the information that information is
 * incomplete is automatically propagated to the problem.
 */

#include "stipulation/stipulation.h"

typedef enum
{
  solving_complete,
  solving_partial,
  solving_interrupted
} solving_completeness_type;

/* Reset our state before delegating, then be ready to report our state
 * @param si identifies the STProblemSolvingIncomplete slice
 */
void problem_solving_incomplete_solve(slice_index si);

/* Report whether solving has been interrupted
 * @param si identifies the STProblemSolvingIncomplete slice
 * @return completeness of solution
 */
solving_completeness_type problem_solving_completeness(slice_index si);

/* Allocate a STPhaseSolvingIncomplete slice
 * @param base base for searching for the STProblemSolvingIncomplete slice
 *             that the result will propagate the information about
 *             interruptions to.
 * @return identiifer of the allocates slice
 */
slice_index alloc_phase_solving_incomplete(slice_index base);

/* Reset our state before delegating, then be ready to report our state
 * @param si identifies the STProblemSolvingIncomplete slice
 */
void phase_solving_incomplete_solve(slice_index si);

/* Remember that solving has been interrupted
 * @param si identifies the STProblemSolvingIncomplete slice
 * @param c completeness of phase
 */
void phase_solving_remember_incompleteness(slice_index si,
                                           solving_completeness_type c);

/* Report whether solving has been interrupted
 * @param si identifies the STProblemSolvingIncomplete slice
 * @return completeness of solution
 */
solving_completeness_type phase_solving_completeness(slice_index si);

#endif
