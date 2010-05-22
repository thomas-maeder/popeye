#if !defined(STIPULATION_BATTLE_PLAY_VARIATION_H)
#define STIPULATION_BATTLE_PLAY_VARIATION_H

#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with variations
 */

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
