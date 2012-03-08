#if !defined(STIPULATION_CHECK_ZIGZAG_JUMP_H)
#define STIPULATION_CHECK_ZIGZAG_JUMP_H

/* Slice type STCheckZigzagJump - take a shortcut if the side at the move is
 * not in check
 */

#include "stipulation/battle_play/defense_play.h"
#include "stipulation/battle_play/attack_play.h"

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
stip_length_type check_zigzag_jump_defend(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type check_zigzag_jump_attack(slice_index si, stip_length_type n);

/* Instrument a battle branch with a STCheckZigzagJump slice providing a
 * shortcut for the defense moe
 * @param adapter identifies adapter slice into the battle branch
 */
void battle_branch_insert_defense_check_zigzag(slice_index adapter);

/* Instrument a help branch with a STCheckZigzagJump slice
 * @param adapter identifies adapter slice into the help branch
 */
void help_branch_insert_check_zigzag(slice_index adapter);

#endif
