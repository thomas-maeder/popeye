#if !defined(OPTION_MOVENUMBER_H)
#define OPTION_MOVENUMBER_H

/* Implementation of everything related to move number output and the
 * restart option
 * This includes the STRestartGuard slice - stops solutions that start
 * with key moves with numbers lower than the restart number entered
 * by the user, and writes move numbers
 */

#include "solving/solve.h"

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
void stip_insert_restart_guards(slice_index si);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type restart_guard_solve(slice_index si, stip_length_type n);

#endif
