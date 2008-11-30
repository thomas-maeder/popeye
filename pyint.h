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

extern boolean isIntelligentModeActive;

boolean isGoalReachable(void);
boolean SolAlreadyFound(void);
void StoreSol(void);
boolean Intelligent(boolean looking_for_short_solutions, stip_length_type n);

#endif
