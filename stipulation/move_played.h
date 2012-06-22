#if !defined(STIPULATION_MOVE_PLAYED_H)
#define STIPULATION_MOVE_PLAYED_H

#include "stipulation/structure_traversal.h"
#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* This module provides functionality dealing with the attacking side
 * in STMovePlayed stipulation slices.
 */

/* Allocate a STMovePlayed slice.
 * @return index of allocated slice
 */
slice_index alloc_move_played_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_played_attack(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type move_played_defend(slice_index si, stip_length_type n);


/* Allocate a STHelpMovePlayed slice.
 * @return index of allocated slice
 */
slice_index alloc_help_move_played_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type help_move_played_attack(slice_index si, stip_length_type n);

/* Detect starter field with the starting side if possible.
 * @param si identifies slice being traversed
 * @param st status of traversal
 */
void move_played_detect_starter(slice_index si, stip_structure_traversal *st);

#endif
