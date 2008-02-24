/******************** MODIFICATIONS to pyint.h **************************
**
** Date       Who  What
** 
** 1997/04/04 TLi  Original
** 
**************************** End of List ******************************/ 

#if !defined(_PYINT_H)
#define _PYINT_H

boolean Redundant(void);
boolean MatePossible(void);
boolean SolAlreadyFound(void);
void StoreSol(void);
boolean Intelligent(smallint whmoves, smallint blmoves,
                    boolean (*proc)(couleur, smallint, boolean),
                    couleur camp, smallint length);

#endif
