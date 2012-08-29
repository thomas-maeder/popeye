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

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type move_counter_attack(slice_index si, stip_length_type n)
{
  INCREMENT_COUNTER(play_move);
  return attack(slices[si].next1,n);
}

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
stip_length_type move_counter_defend(slice_index si, stip_length_type n)
{
  INCREMENT_COUNTER(play_move);
  return defend(slices[si].next1,n);
}

/* Instrument slices with move counters
 */
void stip_insert_move_counters(slice_index si)
{
  stip_instrument_moves(si,STMoveCounter);
}

#endif
