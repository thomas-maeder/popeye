#if !defined(OUTPUT_PLAINTEXT_PLAINTEXT_H)
#define OUTPUT_PLAINTEXT_PLAINTEXT_H

#include "stipulation/stipulation.h"
#include "output/output.h"

extern output_engine_type const output_plaintext_engine;
extern output_symbol_table_type const output_plaintext_symbol_table;


/* Write the current move
 */
void output_plaintext_write_move(output_engine_type const *engine,
                                 FILE *file,
                                 output_symbol_table_type const *symbol_table);

/* Instrument the solving machinery with slices that write the solution in
 * plain text
 */
void output_plaintext_instrument_solving(slice_index si);

#endif
