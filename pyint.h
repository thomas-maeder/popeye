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


Goal goal_to_be_reached;

boolean isGoalReachable(void);
boolean SolAlreadyFound(void);
void StoreSol(void);
boolean Intelligent(stip_length_type n);

/* How well does the stipulation support intelligent mode?
 */
typedef enum
{
  intelligent_not_supported,
  intelligent_not_active_by_default,
  intelligent_active_by_default
} support_for_intelligent_mode;

support_for_intelligent_mode stip_supports_intelligent(void);

#endif
