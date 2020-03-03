#if !defined(OPTION_MOVENUMBER_H)
#define OPTION_MOVENUMBER_H

/* Implementation of everything related to move number output and the
 * restart option
 * This includes the STRestartGuard slice - stops solutions that start
 * with key moves with numbers lower than the restart number entered
 * by the user, and writes move numbers
 */

#include "solving/machinery/solve.h"
#include "solving/ply.h"

void move_numbers_write_history(ply top_ply);

/* Reset the restart number setting.
 */
void reset_restart_number(void);

/* Retrieve the current restart number
 */
unsigned int get_restart_number(void);

/* Interpret restart option
 * @param optionValue value of option restart
 * @return true iff optionValue points to a valid value
 */
boolean read_restart_number(char const *optionValue);

/* Instrument stipulation with STRestartGuard slices
 * @param si identifies slice where to start
 */
void solving_insert_restart_guards(slice_index si);

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
void restart_guard_solve(slice_index si);
void restart_guard_nested_solve(slice_index si);

#endif
