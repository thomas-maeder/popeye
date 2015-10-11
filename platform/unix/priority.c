#include "platform/priority.h"

void platform_set_nice_priority()
{
  /* Nothing to be done here. Everything is nice on Unix, and if not,
   * users know how to change that, even if there's no such thing as a
   * Task Manager ...
   */
}
