/******************** MODIFICATIONS to pyproof.h **************************
**
** Date       Who  What
** 
** 1995/10/31 TLi  Original
** 
**************************** End of List ******************************/ 

#if !defined(PYPROOF_H)
#define PYPROOF_H

union HashBuffer;

void ProofEncode(HashBuffer *);
void    ProofInitialiseKingMoves(square ProofRB, square ProofRN);
boolean ProofVerifie(void);

void ProofRestoreTargetPosition(void);
void ProofSaveStartPosition(void);
void ProofAtoBWriteStartPosition(void);

void    ProofInitialise(void);
void    ProofInitialiseIntelligent(void);
boolean ProofIdentical(void);


/* Functions determining that going on from a certain position will
 * not lead to the required position. */
typedef boolean (*ProofImpossible_fct_t)(void);

ProofImpossible_fct_t alternateImpossible; /* TODO */

#endif
