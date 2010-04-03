#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_PLAY_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_PLAY_H

/* Interface for dynamically dispatching defense operations to slices
 * depending on the slice type
 */

#include "pyslice.h"

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean defense_root_solve(slice_index si);

/* Try to defend after an attempted key move at root level
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defending side can successfully defend
 */
boolean defense_root_defend(slice_index si, stip_length_type n_min);

/* Try to defend after an attempted key move at non-root level.
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimum number of half-moves of interesting variations
 *              (slack_length_battle <= n_min <= slices[si].u.branch.length)
 * @return true iff the defender can defend
 */
boolean defense_defend_in_n(slice_index si,
                            stip_length_type n,
                            stip_length_type n_min);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param max_nr_refutations how many refutations should we look for
 * @return n+4 refuted - >max_nr_refutations refutations found
           n+2 refuted - <=max_nr_refutations refutations found
           <=n solved  - return value is maximum number of moves
                         (incl. defense) needed
 */
stip_length_type defense_can_defend_in_n(slice_index si,
                                         stip_length_type n,
                                         unsigned int max_nr_refutations);

/* Try to defend after an attempted key move at non-root level
 * @param si slice index
 * @return true iff the defender can defend
 */
boolean defense_defend(slice_index si);

/* Determine whether there are refutations after an attempted key move
 * at non-root level
 * @param si slice index
 * @return true iff there is >=1 refutation
 */
boolean defense_can_defend(slice_index si);

#endif
