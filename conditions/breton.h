#if !defined(CONDITIONS_BRETON_H)
#define CONDITIONS_BRETON_H

#include "solving/machinery/solve.h"

/* This module implements the condition Breton including its variants */

typedef enum
{
  breton_propre,
  breton_adverse,

  nr_breton_modes
} breton_mode_type;

typedef enum
{
  breton_nonchromatic,
  breton_chromatic
} breton_chromaticity_type;

extern breton_mode_type breton_mode;
extern breton_chromaticity_type breton_chromaticity;

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
void breton_remover_solve(slice_index si);

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
void breton_adverse_remover_solve(slice_index si);

/* Instrument slices with move tracers
 */
void solving_insert_breton(slice_index si);

#endif
