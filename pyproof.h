/******************** MODIFICATIONS to pyproof.h **************************
**
** Date       Who  What
**
** 1995/10/31 TLi  Original
**
**************************** End of List ******************************/

#if !defined(PYPROOF_H)
#define PYPROOF_H

#include "py.h"
#include "stipulation/stipulation.h"

extern boolean change_moving_piece;

/* Encode the hash key for the current position
 */
void ProofEncode(stip_length_type min_length, stip_length_type validity_value);

/* Initialise the proof module for the current target position
 * @param si identifies start slice
 */
void ProofInitialise(slice_index si);

/* Proof game: initialize start position from PAS
 */
void ProofInitialiseStartPosition(void);

/* a=>b: save the current piece places for the start position
 */
void ProofSaveStartPosition(void);

/* Restore the start position
 */
void ProofRestoreStartPosition(void);

/* Save the current position (pieces + "royal information") for the
 * target position
 */
void ProofSaveTargetPosition(void);

/* Restore target position
 */
void ProofRestoreTargetPosition(void);

/* write start position (0 resp. 1 diagram)
 * @param start identifies start slice
 */
void ProofWriteStartPosition(slice_index start);

/* Initialise data structure for intelligent solving
 * @param length number of half-moves until goal has to be reached
 */
void ProofInitialiseIntelligent(stip_length_type length);

/* Compare two positions
 * @return true iff the current position is equal to the target
 *              position
 */
boolean ProofIdentical(void);

slice_type proof_make_goal_reachable_type(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type goalreachable_guard_proofgame_solve(slice_index si,
                                                     stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type goalreachable_guard_proofgame_fairy_solve(slice_index si,
                                                           stip_length_type n);

extern int const ProofKnightMoves[];

#endif
