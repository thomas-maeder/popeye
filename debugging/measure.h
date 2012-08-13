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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_counter_attack(slice_index si, stip_length_type n);

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type move_counter_defend(slice_index si, stip_length_type n);

/* Instrument slices with move tracers
 */
void stip_insert_move_counters(slice_index si);

#else

#define DEFINE_COUNTER(name)

#define INCREMENT_COUNTER(name)

#define WRITE_COUNTER(name)

#endif

#endif
