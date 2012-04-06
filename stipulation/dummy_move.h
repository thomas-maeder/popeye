#if !defined(STIPULATION_DUMMY_MOVE_H)
#define STIPULATION_DUMMY_MOVE_H

#include "stipulation/battle_play/defense_play.h"
#include "stipulation/battle_play/attack_play.h"

/* Slices of type STDummyMove are used to represent the "dummy move" not
 * played by the side not playing the series.
 */

/* Allocate a STDummyMove slice.
 * @return index of allocated slice
 */
slice_index alloc_dummy_move_slice(void);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type dummy_move_defend(slice_index si, stip_length_type n);

#endif
