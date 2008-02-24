/******************** MODIFICATIONS to pylang.h **************************
**
** Date       Who  What
** 
** 1992            Original
** 
**************************** End of List ******************************/ 

#if !defined(PYLANG_H)
#define PYLANG_H

/*--- Start of typedef enum {---*/
#define	French		0
#define	German		1
#define	English		2
#define	LangCount	3
typedef int Lang;
/*--- End of } Lang;---*/

char *GetLangName(Lang l);
char *MkStrFileName(Lang l);
char *MkMsgFileName(Lang l);

#endif	/* not PYLANG_H */
