#include "output/latex/message.h"

#include <stdarg.h>

#if defined(DEBUG)
#       define  DBG(x) fprintf x
#else
#       define DBG(x)
#endif

/* Issue a message text
 * @param id identifies the message
 * @param ... additional parameters according the printf() like conversion
 *            specifiers in message id
 */
void output_latex_message(FILE *file, message_id_t id, ...)
{
  va_list args;
  DBG((stderr, "Mesage(%u) = %s\n", (unsigned int) id, output_message_get(id)));
  va_start(args,id);
  if (id<MsgCount)
    vfprintf(file,output_message_get(id),args);
  else
    fputs(output_message_get(InternalError), file);
  va_end(args);
}
