#if !defined(STIPULATION_BATTLE_PLAY_ATTACK_PLAY_H)
#define STIPULATION_BATTLE_PLAY_ATTACK_PLAY_H

/* Interface for dynamically dispatching attack operations to slices
 * depending on the slice type
 */

#include "pyslice.h"
#include "pydata.h"

/* maximum number of half-moves that we know have no solution
 */
extern stip_length_type max_unsolvable;

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack(slice_index si, stip_length_type n);

#endif
