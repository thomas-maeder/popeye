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
void InitCheckDir(void);
void InitBoard(void);

#endif /*PY1_INCLUDED*/
