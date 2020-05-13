#if !defined(DEBUGGING_MEASURE_H)
#define DEBUGGING_MEASURE_H

/* some functions for very simple measurements
 * - write useful output to standard out
 * - activate by #defining DOMEASURE
 * - no overhead if not active
 */

#if defined(DOMEASURE)

#include "stipulation/stipulation.h"
#include "output/plaintext/protocol.h"

#define COUNTER_PREFIX COUNTER_

#define COUNTER_TYPE unsigned long

/* Define a counter. To be put into the global namespace.
 */
#define DEFINE_COUNTER(name) COUNTER_TYPE COUNTER_##name

/* Declare a counter.
 */
#define DECLARE_COUNTER(name) extern COUNTER_TYPE COUNTER_##name

/* Increment a counter defined elsewhere
 */
#define INCREMENT_COUNTER(name) ++COUNTER_##name

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void move_counter_solve(slice_index si);

/* Instrument slices with move tracers
 */
void solving_insert_move_counters(slice_index si);

void counters_writer_solve(slice_index si);

#else

#define DEFINE_COUNTER(name)

#define DECLARE_COUNTER(name)

#define INCREMENT_COUNTER(name)

#endif

#endif
