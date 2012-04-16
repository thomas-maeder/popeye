#if !defined(DEBUGGING_MEASURE_H)
#define DEBUGGING_MEASURE_H

/* some functions for very simple measurements
 * - write useful output to standard out
 * - activate by #defining DOMEASURE
 * - no overhead if not active
 */

#if defined(DOMEASURE)

#include "pyproc.h"

#include <stdio.h>

#define COUNTER_PREFIX COUNTER_

#define COUNTER_TYPE unsigned long

/* Define a counter. To be put into the global namespace.
 */
#define DEFINE_COUNTER(name) COUNTER_TYPE counter##name;

/* Increment a counter defined elsewhere
 */
#define INCREMENT_COUNTER(name)         \
  {                                     \
    extern COUNTER_TYPE counter##name;  \
    ++counter##name;                    \
  }

/* Write the value of a counter defined elsewhere
 */
#define WRITE_COUNTER(name)                       \
  {                                               \
    extern COUNTER_TYPE counter##name;            \
    enum { bufsize = 50 };                        \
    char buf[bufsize];                                          \
    snprintf(buf,bufsize,"%20s:%12lu\n",#name,counter##name);   \
    StdString(buf);                                             \
    counter##name = 0;                                          \
  }

#else

#define DEFINE_COUNTER(name)

#define INCREMENT_COUNTER(name)

#define WRITE_COUNTER(name)

#endif

#endif
