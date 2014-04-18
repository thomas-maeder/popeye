/******************** MODIFICATIONS to pylang.c **************************

**
** Date       Who  What
** 
** 1992            Original
** 
**************************** End of List ******************************/ 

#include <stdio.h>
#include "input/plaintext/language.h"
#include "pylang.h"

static char const * const LanguageName[LanguageCount] =
{
  "fran",
  "deut",
  "engl"
};

/* Buffer holding the return values of the language functions.
 */
static char Name[30];

/* Get name of a language
 * @param lang language identifier
 * @return 0 terminated string containing the name of the language;
 *         the string is owned by the language module and maybe
 *         overwritten by a subsequent call to a function from this
 *         module.
 */
char const *GetLanguageName(Language lang)
{
  return LanguageName[lang];
}

/* Get name of the file containing the strings for a language
 * @param lang language identifier
 * @return 0 terminated string containing the file name;
 *         the string is owned by the language module and maybe
 *         overwritten by a subsequent call to a function from this
 *         module.
 */
char const *MkStrFileName(Language lang)
{
  sprintf(Name, "py-%s.str", LanguageName[lang]);
  return Name;
}

/* Get name of the file containing the messages for a language
 * @param lang language identifier
 * @return 0 terminated string containing the file name;
 *         the string is owned by the language module and maybe
 *         overwritten by a subsequent call to a function from this
 *         module.
 */
char const *MkMsgFileName(Language lang)
{
  sprintf(Name, "py-%s.msg", LanguageName[lang]);
  return Name;
}
