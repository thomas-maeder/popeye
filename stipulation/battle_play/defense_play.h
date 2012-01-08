#if !defined(STIPULATION_BATTLE_PLAY_DEFENSE_PLAY_H)
#define STIPULATION_BATTLE_PLAY_DEFENSE_PLAY_H

/* Interface for dynamically dispatching defense operations to slices
 * depending on the slice type
 */

#include "pyslice.h"
#include "pydata.h"

/* maximum number of half-moves that we know have no solution
 * (n==max_unsolvable[nbply] means that we are solving refutations)
 */
extern stip_length_type max_unsolvable[maxply+1];

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type defend(slice_index si, stip_length_type n);

/* Determine whether there are defenses after an attacking move
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - <=acceptable number of refutations found
 *         n+4 refuted - >acceptable number of refutations found
 */
stip_length_type can_defend(slice_index si, stip_length_type n);

#endif
