#if !defined(PYNONTRV_H)
#define PYNONTRV_H

/* Implementation of the "max threat length" optimisation: Solving
 * stops if an attacker's move does not reach the goal nor delivers
 * check nor threatens to reach the goal in a maximum number of moves
 */

#include "stipulation/battle_play/defense_play.h"

/* NOTE: exposed for performance reasons only - DON'T WRITE TO THIS
 * VARIABLE!!
 */
extern unsigned int max_nr_nontrivial;


/* Reset the non-trivial optimisation setting to off
 */
void reset_nontrivial_settings(void);

/* Read the requested non-trivial optimisation settings from user input
 * @param tok text token from which to read maximum number of
 *            acceptable non-trivial variations (apart from main variation)
 * @return true iff setting was successfully read
 */
boolean read_max_nr_nontrivial(char const *tok);

/* Read the requested non-trivial optimisation settings from user input
 * @param tok text token from which to read minimimal length of what
 *            is to be considered a non-trivial variation
 * @return true iff setting was successfully read
 */
boolean read_min_length_nontrivial(char const *tok);

/* Retrieve the current minimum length (in full moves) of what is to
 * be considered a non-trivial variation
 * @return maximum acceptable number of non-trivial variations
 */
stip_length_type get_min_length_nontrivial(void);

/* Instrument stipulation with STMaxNrNonTrivial slices
 * @param si identifies slice where to start
 */
void stip_insert_max_nr_nontrivial_guards(slice_index si);

/* Callback to stip_spin_off_testers
 * Spin a tester slice off an end of a STMaxNrNonTrivial slice
 * @param si identifies the STMaxNrNonTrivial slice
 * @param st address of structure representing traversal
 */
void spin_off_testers_max_nr_non_trivial(slice_index si,
                                         stip_structure_traversal *st);

/* Try to defend after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type
max_nr_nontrivial_guard_defend(slice_index si, stip_length_type n);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type
max_nr_nontrivial_guard_can_defend(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
max_nr_nontrivial_counter_can_attack(slice_index si, stip_length_type n);

#endif
