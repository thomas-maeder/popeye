/******************** MODIFICATIONS to pyproof.h **************************
**
** Date       Who  What
** 
** 1995/10/31 TLi  Original
** 
**************************** End of List ******************************/ 

#if !defined(_PYPROOF_H)
#define _PYPROOF_H

BCMemValue *ProofEncode(void);
void    ProofInitilise(void);
boolean ProofSol(couleur camp, int n, boolean restartenabled);
boolean SeriesProofSol(int n, boolean restartenabled);
void    ProofInitialiseKingMoves(square ProofRB, square ProofRN);
boolean ProofVerifie(void);
void    ProofInitialise(void);

#endif
