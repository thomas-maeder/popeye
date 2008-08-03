/******************** MODIFICATIONS to pyproof.h **************************
**
** Date       Who  What
** 
** 1995/10/31 TLi  Original
** 
**************************** End of List ******************************/ 

#if !defined(PYPROOF_H)
#define PYPROOF_H

extern Flags ProofSpec[nr_squares_on_board];
extern Flags SpecA[nr_squares_on_board];

union HashBuffer;

void ProofEncode(HashBuffer *);
void    ProofInitialiseKingMoves(square ProofRB, square ProofRN);
boolean ProofVerifie(void);
void    ProofInitialise(void);
void    ProofInitialiseIntelligent(void);
boolean ProofIdentical(void);


/* Functions determining that going on from a certain position will
 * not lead to the required position. */
typedef boolean (*ProofImpossible_fct_t)(void);

ProofImpossible_fct_t alternateImpossible; /* TODO */

#endif
