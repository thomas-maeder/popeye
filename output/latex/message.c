#include "output/latex/message.h"

#include <stdarg.h>

#if defined(DEBUG)
#       define  DBG(x) fprintf x
#else
#       define DBG(x)
#endif

void output_latex_message(FILE *file, message_id_t id, ...)
{
  va_list args;
  DBG((stderr, "Mesage(%d) = %s\n", id, output_message_get(id)));
  va_start(args,id);
  if (id<MsgCount)
    vfprintf(file,output_message_get(id),args);
  else
    fprintf(file,output_message_get(InternalError),id);
  va_end(args);
}
