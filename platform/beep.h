#if !defined(PLATFORM_BEEP_H)
#define PLATFORM_BEEP_H

#include "utilities/boolean.h"

/* Interpret beep command line parameter value
 * @param optionValue value of option beep
 */
boolean read_nr_beeps(char const *optionValue);

/* Produce a series of audible beep signals
 * @param nr_beeps number of beep signals to be produced
 */
void platform_produce_beep(void);

#endif
