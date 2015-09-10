#if !defined(OUTPUT_LATEX_H)
#define OUTPUT_LATEX_H

#include "position/position.h"
#include "input/plaintext/twin.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "utilities/boolean.h"

#include <stdio.h>

extern output_engine_type const output_latex_engine;
extern output_symbol_table_type const output_latex_symbol_table;

boolean LaTeXSetup(void);
void LaTeXShutdown(void);

void LaTeXBeginDiagram(FILE *file);
void LaTeXEndDiagram(FILE *file);

void LaTexOpenSolution(FILE *file);
void LaTexCloseSolution(FILE *file);

char *ParseLaTeXPieces(char *tok);

void LaTeXStr(FILE *file, char const *line);
char *LaTeXWalk(piece_walk_type walk);

void WriteUserInputElement(FILE *file, char const *name, char const *value);

/* Instrument the solving machinery with slices that write the solution in
 * LaTeX
 */
void output_latex_instrument_solving(slice_index si);

/* Instrument the solving machinery with slices that write the solution in
 * LaTeX
 */
void output_latex_instrument_setup(slice_index si);


/* Instrument the solving machinery with slices that write the solution in
 * LaTeX
 */
void output_latex_diagram_writer_builder_solve(slice_index si);

#endif
