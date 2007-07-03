/******************** MODIFICATIONS to pylang.c **************************

**
** Date       Who  What
** 
** 1992            Original
** 
**************************** End of List ******************************/ 

#include <stdio.h>                 /* H.D. 10.02.93 */
#include "pylang.h"

static char *LangName[LangCount] = {
	"fran",
	"deut",
	"engl"
};
static char Name[30];

char *GetLangName(l)
Lang l;
{
	return LangName[l];
}

char *MkStrFileName(l)
Lang l;
{
	sprintf(Name, "py-%s.str", LangName[l]);
	return Name;
}

char *MkMsgFileName(l)
Lang l;
{
	sprintf(Name, "py-%s.msg", LangName[l]);
	return Name;
}
