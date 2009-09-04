#if !defined(PYFLIGHT_H)
#define PYFLIGHT_H

/* Implementation of the "max flight squares" optimisation: Solving
 * stops if an attacker's move grants the defender too many flight
 * squares
 */

#include "pyslice.h"

/* Reset the max flights setting to off
 */
void reset_max_flights(void);

/* Read the requested max flight setting from a text token entered by
 * the user
 * @param textToken text token from which to read
 * @return true iff max flight setting was successfully read
 */
boolean read_max_flights(const char *textToken);

/* Retrieve the current max flights setting
 * @return current max flights setting
 *         UINT_MAX if max flights option is not active
 */
unsigned int get_max_flights(void);

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_maxflight_guards(void);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defender can successfully defend
 */
boolean maxflight_guard_root_defend(slice_index si);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean maxflight_guard_defend_in_n(slice_index si,
                                    stip_length_type n,
                                    unsigned int curr_max_nr_nontrivial);

/* Determine whether there is a defense after an attempted key move at
 * non-root level 
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean maxflight_guard_can_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        unsigned int curr_max_nr_nontrivial);

#endif
