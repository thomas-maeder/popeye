#if !defined(OUTPUT_OUTPUT_H)
#define OUTPUT_OUTPUT_H

#include "output/mode.h"
#include "stipulation/stipulation.h"
#include "stipulation/goals/goals.h"
#include "utilities/boolean.h"

#include <stdarg.h>
#include <stdio.h>

typedef struct
{
    int (*fputc)(int c, FILE *regular);
    int (*vfprintf)(FILE *regular, char const *format, va_list args);
    int (*fprintf)(FILE *regular, char const *format, ...);
    int (*fflush)(FILE *regular);
} output_engine_type;

typedef enum
{
  output_symbol_right_arrow,
  output_symbol_left_right_arrow,
  output_symbol_queen_side_castling,
  output_symbol_king_side_castling,

  output_nr_symbols
} output_symbol_type;

typedef char const *output_symbol_table_type[output_nr_symbols];

/* Determine whether the symbol for a reached goal preempts the symbol for a given
 * check (if not, both a possible check and the symbol for the reached goal
 * should be written).
 * @param goal goal written by goal writer
 * @return true iff the check writer should be replaced by the goal writer
 */
boolean output_goal_preempts_check(goal_type goal);

/* Allocate an STOutputModeSelector slice
 * @param mode output mode to be selected by the allocated slice
 * @return identifier of the allocated slice
 */
slice_index alloc_output_mode_selector(output_mode mode);

/* Notify the output machinery about a twinning event
 * @param si identifies the slice that detected the twinning (at the same time
 *           to be used as the starting point of any instrumentation)
 * @param stage the twinning event
 * @param continued is the twin continued?
 */
void output_notify_twinning(slice_index si,
                            twinning_event_type event,
                            boolean continued);

#endif
