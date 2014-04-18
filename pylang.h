/******************** MODIFICATIONS to pylang.h **************************
**
** Date       Who  What
** 
** 1992            Original
** 
**************************** End of List ******************************/ 

#if !defined(PYLANG_H)
#define PYLANG_H

#include "input/plaintext/language.h"

/* Get name of a language
 * @param lang language identifier
 * @return 0 terminated string containing the name of the language;
 *         the string is owned by the language module and maybe
 *         overwritten by a subsequent call to a function from this
 *         module.
 */
char const *GetLanguageName(Language lang);

/* Get name of the file containing the strings for a language
 * @param lang language identifier
 * @return 0 terminated string containing the file name;
 *         the string is owned by the language module and maybe
 *         overwritten by a subsequent call to a function from this
 *         module.
 */
char const *MkStrFileName(Language lang);

/* Get name of the file containing the messages for a language
 * @param lang language identifier
 * @return 0 terminated string containing the file name;
 *         the string is owned by the language module and maybe
 *         overwritten by a subsequent call to a function from this
 *         module.
 */
char const *MkMsgFileName(Language lang);

#endif
