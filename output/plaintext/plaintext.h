#if !defined(OUTPUT_PLAINTEXT_PLAINTEXT_H)
#define OUTPUT_PLAINTEXT_PLAINTEXT_H

#include "stipulation/stipulation.h"
#include "output/output.h"

extern output_engine_type const output_plaintext_engine;
extern output_symbol_table_type const output_plaintext_symbol_table;

extern boolean output_plaintext_check_indication_disabled;


/* Write the current move
 */
void output_plaintext_write_move(output_engine_type const *engine,
                                 FILE *file,
                                 output_symbol_table_type const *symbol_table);

/* Write the effects of a dummy move (e.g. if the black "any" move has some
 * effects such as removal of white lost pieces
 */
void output_plaintext_write_dummy_move_effects(output_engine_type const *engine,
                                               FILE *file,
                                               output_symbol_table_type const *symbol_table);

/* Instrument the solving machinery with slices that write the solution in
 * plain text
 */
void output_plaintext_instrument_solving(slice_index si);

void output_plaintext_instrument_solving_builder_solve(slice_index si);

void output_plaintext_writer_solve(slice_index si);

slice_index output_plaintext_alloc_writer(FILE *file);

#endif
