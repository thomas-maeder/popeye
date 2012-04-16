#if !defined(UTILITIES_BOOLEAN_H)
#define UTILITIES_BOOLEAN_H

#if defined(_WIN32) || defined(__CYGWIN__)

/* avoid clash with type boolean defined in: */
#include <wtypes.h>

enum
{
  false,
  true
};

#else

typedef enum
{
  false,
  true
} boolean;

#endif

#endif
