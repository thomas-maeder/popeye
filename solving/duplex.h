#if !defined(SOLVING_DUPLEX_H)
#define SOLVING_DUPLEX_H

#include "stipulation/stipulation.h"

typedef enum
{
  twin_no_duplex,
  twin_has_duplex,
  twin_is_duplex
} twin_duplex_type_type;
extern twin_duplex_type_type twin_duplex_type;

/* Solve a "half-duplex" problem
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
void half_duplex_solve(slice_index si);

/* Solve a duplex problem
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
void duplex_solve(slice_index si);

/* Instrument the input machinery with a dpulex type
 * @param start start slice of input machinery
 * @param type duplex type to instrument input machinery with
 */
void input_instrument_duplex(slice_index start, slice_type type);

boolean input_is_instrumented_with_duplex(slice_index start);

#endif
