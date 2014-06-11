#if !defined(OUTPUT_OUTPUT_H)
#define OUTPUT_OUTPUT_H

#include "output/mode.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

extern boolean flag_regression;

/* Allocate an STOutputModeSelector slice
 * @param mode output mode to be selected by the allocated slice
 * @return identifier of the allocated slice
 */
slice_index alloc_output_mode_selector(output_mode mode);

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
void output_end_of_intro(slice_index si);

/* Instrument the stipulation structure with slices that implement
 * the selected output mode.
 * @param si identifies slice where to start
 */
void solving_insert_output_slices(slice_index si);

boolean WriteConditions(void (*WriteCondition)(char const CondLine[], boolean is_first));

#endif
