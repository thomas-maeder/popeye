#if !defined(PYNONTRV_H)
#define PYNONTRV_H

/* Implementation of the "max threat length" optimisation: Solving
 * stops if an attacker's move does not reach the goal nor delivers
 * check nor threatens to reach the goal in a maximum number of moves
 */

#include "pyslice.h"

extern stip_length_type min_length_nontrivial;
extern unsigned int max_nr_nontrivial;


// /* Reset the max threats setting to off
//  */
// void reset_max_nr_nontrivial(void);

// /* Read the requested max threat length setting from a text token
//  * entered by the user
//  * @param textToken text token from which to read
//  * @return true iff max threat length setting was successfully read
//  */
// boolean read_max_nr_nontrivial(const char *textToken);

// /* Retrieve the current max threat length setting
//  * @return current max threat length setting
//  *         no_stip_length if max threats option is not active
//  */
// stip_length_type get_max_nr_nontrivial(void);

/* Instrument stipulation with STKeepMatingGuard slices
 */
void stip_insert_max_nr_nontrivial_guards(void);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @return true iff the defender can successfully defend
 */
boolean max_nr_nontrivial_guard_root_defend(slice_index si);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defender can successfully defend
 */
boolean max_nr_nontrivial_guard_defend_in_n(slice_index si,
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
boolean
max_nr_nontrivial_guard_can_defend_in_n(slice_index si,
                                        stip_length_type n,
                                        unsigned int curr_max_nr_nontrivial);

#endif
