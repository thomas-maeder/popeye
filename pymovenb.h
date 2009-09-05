#if !defined(PYMOVENB_H)
#define PYMOVENB_H

/* Implementation of everything related to move number output and the
 * restart option
 * This includes the STRestartGuard slice - stops solutions that start
 * with key moves with numbers lower than the restart number entered
 * by the user
 */

#include "pyhelp.h"
#include "pyslice.h"

/* Reset the restart number setting.
 */
void reset_restart_number(void);

/* Retrieve the current restart number
 */
unsigned int get_restart_number(void);

/* Interpret maxmem command line parameter value
 * @param commandLineValue value of -maxmem command line parameter
 * @return true iff optionValue points to a valid value
 */
boolean read_restart_number(char const *optionValue);

/* Instrument stipulation with STRestartGuard slices
 */
void stip_insert_restart_guards(void);

/* Try to defend after an attempted key move at root level
 * @param table table where to add refutations
 * @param si slice index
 * @return true iff the attacker has reached a deadend (e.g. by
 *         immobilising the defender in a non-stalemate stipulation)
 */
boolean restart_guard_root_defend(table refutations, slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean restart_guard_help_solve_in_n(slice_index si, stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean restart_guard_series_solve_in_n(slice_index si, stip_length_type n);

#endif
