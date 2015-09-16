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

void LaTeXSetup(slice_index start);

void LaTeXBeginDiagram(FILE *file);
void LaTeXEndDiagram(FILE *file);

void LaTeXMeta(FILE *file);

void LaTeXConditions(FILE *file);
void LaTeXStipulation(FILE *file);
void LaTeXWritePieces(FILE *file);
void LaTeXOptions(void);
void LaTeXCo(FILE *file);
void LaTeXHfill(FILE *file);

void LaTexOpenSolution(FILE *file);
void LaTexCloseSolution(FILE *file);

char *ParseLaTeXPieces(void);

void LaTeXStr(FILE *file, char const *line);
char *LaTeXWalk(piece_walk_type walk);

void WriteUserInputElement(FILE *file, char const *name, char const *value);

/* Instrument the solving machinery with slices that write the solution in
 * LaTeX
 */
void output_latex_instrument_solving(slice_index si);

void output_latex_instrument_solving_builder_solve(slice_index si);

/* Instrument the solving machinery with slices that write the solution in
 * LaTeX
 */
void output_latex_instrument_setup(slice_index si);

/* Instrument the solving machinery with slices that write the twinning in
 * LaTeX
 */
void output_latex_twinning_writer_builder_solve(slice_index si);

void output_latex_closer_solve(slice_index si);

/* Allocate a LaTeX writer slice.
 * @param type slice type
 * @param file_owner identifies the slice that owns the output file
 * @return index of allocated slice
 */
slice_index alloc_output_latex_writer(slice_type type, slice_index file_owner);

#endif
