#include "output/plaintext/message.h"
#include "output/plaintext/language_dependant.h"
#include "input/plaintext/language.h"
#include "debugging/assert.h"

static char const * const *ActualMsgTab = MessageTabs[LanguageDefault];

/* This is used to record an argument which is used
 * in a message-string. There are only message strings
 * that contain at most one format specifier.  Therefore
 * one pointer is sufficient.
 * Three small routines are provided to assign a value:
 */

/* Initialise message for a language
 * @param language the language
 */
void output_message_initialise_language(Language language)
{
  ActualMsgTab = MessageTabs[language];
}

/* Retrieve the message for a specific id in the current language
 * @param id identifies the message to be retrieved
 * @return the message
 */
char const *output_message_get(message_id_t id)
{
  assert(id<MsgCount);
  return ActualMsgTab[id];
}
