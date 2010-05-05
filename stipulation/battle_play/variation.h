#if !defined(STIPULATION_BATTLE_PLAY_VARIATION_H)
#define STIPULATION_BATTLE_PLAY_VARIATION_H

#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with variations
 */

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean variation_writer_are_threats_refuted_in_n(table threats,
                                                  stip_length_type len_threat,
                                                  slice_index si,
                                                  stip_length_type n);

/* Determine whether a branch slice has a solution
 * @param si slice index
 * @param n maximal number of moves
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type variation_writer_has_solution_in_n(slice_index si,
                                                    stip_length_type n,
                                                    stip_length_type n_min);

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_battle)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type variation_writer_solve_threats_in_n(table threats,
                                                     slice_index si,
                                                     stip_length_type n,
                                                     stip_length_type n_min);

/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of solution found and written, i.e.:
 *            n_min-2 defense has turned out to be illegal
 *            n_min..n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type variation_writer_solve_in_n(slice_index si,
                                             stip_length_type n,
                                             stip_length_type n_min);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type variation_writer_solve(slice_index si);

/* Instrument the stipulation representation so that it can deal with
 * variations
 */
void stip_insert_variation_handlers(void);

#endif
