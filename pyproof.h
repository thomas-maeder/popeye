/******************** MODIFICATIONS to pyproof.h **************************
**
** Date       Who  What
**
** 1995/10/31 TLi  Original
**
**************************** End of List ******************************/

#if !defined(PYPROOF_H)
#define PYPROOF_H

#include "pyhash.h"

/* Encode the hash key for the current position
 */
void ProofEncode(stip_length_type min_length);

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

/* a=>b: swap pieces' colors in the starting position
 */
void ProofStartSwapColors(void);

/* a=>b: reflect starting position at the horizontal center line
 */
void ProofStartReflectboard(void);

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

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_proofgame_help(slice_index si,
                                               stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_proofgame_can_help(slice_index si,
                                                        stip_length_type n);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_proofgame_fairy_help(slice_index si,
                                                     stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+4 the move leading to the current position has turned out
 *             to be illegal
 *         n+2 no solution found
 *         n   solution found
 */
stip_length_type goalreachable_guard_proofgame_fairy_can_help(slice_index si,
                                                              stip_length_type n);

extern int const ProofKnightMoves[];

#endif
