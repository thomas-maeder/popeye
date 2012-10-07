#include "debugging/measure.h"
#include "pyproc.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "optimisations/hash.h"
#include "output/plaintext/plaintext.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#if defined(DOMEASURE)

#include "stipulation/move_player.h"

DEFINE_COUNTER(play_move)

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_counter_solve(slice_index si, stip_length_type n)
{
  INCREMENT_COUNTER(play_move);
  return solve(slices[si].next1,n);
}

/* Instrument slices with move counters
 */
void stip_insert_move_counters(slice_index si)
{
  stip_instrument_moves(si,STMoveCounter);
}

#endif
