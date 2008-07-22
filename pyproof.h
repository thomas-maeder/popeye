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
boolean ProofSol(couleur camp,
                 int n,
                 boolean restartenabled,
                 slice_index si);
boolean SeriesProofSol(int n, boolean restartenabled, slice_index si);
void    ProofInitialiseKingMoves(square ProofRB, square ProofRN);
boolean ProofVerifie(void);
void    ProofInitialise(void);

#endif
