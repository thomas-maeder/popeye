#if !defined(SOLVING_SLACK_LENGTH_h)
#define SOLVING_SLACK_LENGTH_h

#include "stipulation/stipulation.h"

/* Number of slack half-moves in stipulation length indications:
 */
extern stip_length_type slack_length;

void adjust_slack_length(slice_index si);

#endif
