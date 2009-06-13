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
void ProofEncode(void);

/* Initialise the proof module for the current target position
 */
void ProofInitialise(void);

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
 */
void ProofWriteStartPosition(void);

/* Initialise data structure for intelligent solving
 */
void ProofInitialiseIntelligent(void);

/* Compare two positions
 * @return true iff the current position is equal to the target
 *              position
 */
boolean ProofIdentical(void);


/* Functions determining that going on from the current position will
 * not lead to the target position.
 */
typedef boolean (*ProofImpossible_fct_t)(void);

ProofImpossible_fct_t alternateImpossible; /* TODO */

extern int const ProofKnightMoves[];

#endif
