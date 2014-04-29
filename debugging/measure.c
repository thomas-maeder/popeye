#include "debugging/measure.h"
#include "stipulation/stipulation.h"
#include "optimisations/hash.h"
#include "output/plaintext/plaintext.h"

#include "debugging/assert.h"
#include <ctype.h>
#include <limits.h>
#include <string.h>

#if defined(DOMEASURE)

#include "stipulation/move.h"

DEFINE_COUNTER(play_move)

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void move_counter_solve(slice_index si)
{
  INCREMENT_COUNTER(play_move);
  solve(slices[si].next1);
}

/* Instrument slices with move counters
 */
void solving_insert_move_counters(slice_index si)
{
  stip_instrument_moves(si,STMoveCounter);
}

#endif
