#include "platform/platform.h"
#include "solving/pipe.h"

#include <limits.h>

/* Guess the "bitness" of the platform
 * @return 32 if we run on a 32bit platform etc.
 */
unsigned int platform_guess_bitness(void)
{
#if defined(__unix) || (__APPLE__ & __MACH__)
#  if defined(ULONG_MAX) && ULONG_MAX==18446744073709551615U
  return 64;
#  else
  return 32;
#  endif
#elif defined(_WIN64)
  return 64;
#elif defined(_WIN32)
  return 32;
#else
  return (sizeof(void *) * CHAR_BIT); /* Seems like a reasonable guess.  It's only used in an output statement anyway. */
#endif
}

void platform_initialiser_solve(slice_index si)
{
  platform_init();
  pipe_solve_delegate(si);
}
