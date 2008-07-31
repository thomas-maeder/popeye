/******************** MODIFICATIONS to pyint.h **************************
**
** Date       Who  What
** 
** 1997/04/04 TLi  Original
** 
**************************** End of List ******************************/ 

#if !defined(PYINT_H)
#define PYINT_H

extern int WhMovesLeft, BlMovesLeft;

boolean Redundant(void);
boolean MatePossible(void);
boolean SolAlreadyFound(void);
void StoreSol(void);
boolean Intelligent(int whmoves, int blmoves,
                    boolean (*proc)(Side, int, boolean, slice_index),
                    Side camp, int length);

#endif
