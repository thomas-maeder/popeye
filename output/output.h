#if !defined(OUTPUT_OUTPUT_H)
#define OUTPUT_OUTPUT_H

#include "output/mode.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

#include <stdarg.h>

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

/* Instrument the stipulation structure with slices that implement
 * the selected output mode.
 * @param si identifies slice where to start
 */
void solving_insert_output_slices(slice_index si);

#endif
