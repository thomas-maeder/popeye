#if !defined(HEARTBEAT_H)
#define HEARTBEAT_H

#include "utilities/boolean.h"
#include "stipulation/stipulation.h"

typedef unsigned int heartbeat_type;

enum
{
  no_rate_set = 0ul,
  heartbeat_default_rate = 50000ul
};

/* Inform the heartbeat module about the value of the -heartbeat command
 * line parameter
 * @param commandlineValue value of the -heartbeat command line parameter
 */
void platform_set_commandline_heartbeat(heartbeat_type commandlineValue);

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
void heartbeat_writer_solve(slice_index si);

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
void heartbeat_set(slice_index si);

/* Instrument the solving machinery
 * @param si identifies the slice where to start instrumenting
 */
void heartbeat_problem_instrumenter_solve(slice_index si);

/* Instrument the solving machinery with option maxsolutions
 * @param si identifies the slice where to start instrumenting
 * @param heartbeat
 */
void heartbeat_instrument_solving(slice_index si);

#endif
