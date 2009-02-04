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

/* Verify the current position as suitable as a starting position
 */
boolean ProofVerifie(void);

/* a=>b: save the current piece places for the start position
 */
void ProofAtoBSaveStartPieces(void);

/* a=>b: save the current "royal information" for the start position
 */
void ProofAtoBSaveStartRoyal(void);

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

/* write position (1 resp. 2 diagrams)
 */
void ProofWritePosition(void);

/* Inform proof games module about goal to be reached
 * @param goal goal to be reached (one of goal_proof and goal_atob)
 */
void ProofSetGoal(Goal goal);

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

#endif
