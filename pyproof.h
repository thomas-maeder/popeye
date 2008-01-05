/******************** MODIFICATIONS to pyproof.h **************************
**
** Date       Who  What
** 
** 1995/10/31 TLi  Original
** 
**************************** End of List ******************************/ 

#ifndef _PYPROOF_H
#define _PYPROOF_H

BCMemValue *ProofEncode(void);
void    ProofInitilise(void);
boolean ProofSol(couleur camp, smallint n, boolean restartenabled);
boolean SeriesProofSol(smallint n, boolean restartenabled);
void    ProofInitialiseKingMoves(square ProofRB, square ProofRN);
boolean ProofVerifie(void);
void    ProofInitialise(void);

#endif
