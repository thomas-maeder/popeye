#if !defined(SOLVING_PLAY_SUPPRESSOR_H)
#define SOLVING_PLAY_SUPPRESSOR_H

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* Allocate a STPlaySuppressor defender slice.
 * @return index of allocated slice
 */
slice_index alloc_play_suppressor_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type play_suppressor_attack(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <=n solved  - return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 no solution found
 */
stip_length_type play_suppressor_defend(slice_index si, stip_length_type n);

/* Instrument the stipulation representation so that postkey play is suppressed
 * @param si identifies slice where to start
 */
void stip_insert_play_suppressors(slice_index si);

#endif
