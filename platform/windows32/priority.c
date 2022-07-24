#include "platform/priority.h"
#include <windows.h>

/* This is strange:
 * http://msdn.microsoft.com/en-us/library/ms686219(VS.85).aspx
 * documents BELOW_NORMAL_PRIORITY_CLASS and tells us to #include
 * <windows.h>, but BELOW_NORMAL_PRIORITY_CLASS is nowhere to be
 * found in the MSVC6 headers. */

/* C++BuilderX headers have it, though ... */
#undef BELOW_NORMAL_PRIORITY_CLASS

enum dummy
{
  BELOW_NORMAL_PRIORITY_CLASS = 0x4000
};

void platform_set_nice_priority(void)
{
  SetPriorityClass(GetCurrentProcess(),
                   BELOW_NORMAL_PRIORITY_CLASS);
}

