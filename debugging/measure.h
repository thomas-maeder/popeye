#if !defined(DEBUGGING_MEASURE_H)
#define DEBUGGING_MEASURE_H

/* some functions for very simple measurements
 * - write useful output to standard out
 * - activate by #defining DOMEASURE
 * - no overhead if not active
 */

#if defined(DOMEASURE)

#include "stipulation/stipulation.h"

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
    snprintf(buf,bufsize,"%30s:%12lu\n",#name,counter##name);   \
    StdString(buf);                                             \
    counter##name = 0;                                          \
  }

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type move_counter_solve(slice_index si, stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_move_counters(slice_index si);

#else

#define DEFINE_COUNTER(name)

#define INCREMENT_COUNTER(name)

#define WRITE_COUNTER(name)

#endif

#endif
