/******************** MODIFICATIONS to py1.h **************************
**
** Date       Who  What
** 
** 2001       TLi  Original
** 
**************************** End of List ******************************/ 
 
#ifndef PY1_INCLUDED
#define PY1_INCLUDED

boolean fffriendcheck(
  square        i,
  piece         pfr,
  piece         p,
  boolean       (*evaluate)(square,square,square));

void InitCheckDir(void);
void InitBoard(void);
void InitOpt(void);
void InitAlways(void);

#endif /*PY1_INCLUDED*/
