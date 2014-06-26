#include "output/plaintext/message.h"
#include "output/plaintext/language_dependant.h"
#include "input/plaintext/language.h"

static char const * const *ActualMsgTab = MessageTabs[LanguageDefault];

/* This is used to record an argument which is used
 * in a message-string. There are only message strings
 * that contain at most one format specifier.  Therefore
 * one pointer is sufficient.
 * Three small routines are provided to assign a value:
 */

void InitMsgTab(Language l)
{
  ActualMsgTab = MessageTabs[l];
}

char const *output_message_get(message_id_t id)
{
  return ActualMsgTab[id];
}
