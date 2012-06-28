#if !defined(PIECES_NEUTRAL_INITIALISER_H)
#define PIECES_NEUTRAL_INITIALISER_H

#include "solving/battle_play/attack_play.h"
#include "solving/battle_play/defense_play.h"

/* This module provides slice type STPiecesNeutralInitialiser - initialises
 * neutral pieces to the appropriate side so that the subsequent move
 * generation can generate moves capturing neutral pieces
 */

/* Side that the neutral pieces currently belong to
 */
extern Side neutral_side;

/* Change the side of the piece on a specific square
 * @param pos position of piece whose side to change
 */
void change_side(square s);

/* Initialise the neutral pieces to belong to the side to be captured in the
 * subsequent move
 * @param captured_side side of pieces to be captured
 */
void initialise_neutrals(Side captured_side);

/* Allocate a STPiecesParalysingMateFilter slice.
 * @param starter_or_adversary is the starter mated or its adversary?
 * @return index of allocated slice
 */
slice_index alloc_neutral_initialiser_slice(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type neutral_initialiser_attack(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found */
stip_length_type neutral_initialiser_defend(slice_index si, stip_length_type n);

#endif
