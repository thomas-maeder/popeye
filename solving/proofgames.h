/******************** MODIFICATIONS to pyproof.h **************************
**
** Date       Who  What
**
** 1995/10/31 TLi  Original
**
**************************** End of List ******************************/

#if !defined(PYPROOF_H)
#define PYPROOF_H

#include "stipulation/stipulation.h"
#include "position/position.h"

extern boolean change_moving_piece;

extern position proofgames_start_position;
extern position proofgames_target_position;

/* Initialise the proof module for the current target position
 */
void ProofInitialise(void);

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

/* Compare two positions
 * @return true iff the current position is equal to the target
 *              position
 */
boolean ProofIdentical(void);

#endif
