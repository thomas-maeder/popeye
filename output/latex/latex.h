#if !defined(OUTPUT_LATEX_H)
#define OUTPUT_LATEX_H

#include "position/position.h"
#include "input/plaintext/twin.h"
#include "output/plaintext/plaintext.h"
#include "utilities/boolean.h"

#include <stdio.h>

extern FILE *LaTeXFile;

boolean LaTeXSetup(void);
void LaTeXShutdown(void);

void LaTeXBeginDiagram(FILE *file);
void LaTeXEndDiagram(FILE *file);

void LaTexOpenSolution(FILE *file);
void LaTexCloseSolution(FILE *file);

char *ParseLaTeXPieces(char *tok);

void LaTeXStr(FILE *file, char const *line);
char *LaTeXWalk(piece_walk_type walk);

void output_latex_write_piece_exchange(output_plaintext_move_context_type *context,
                                       move_effect_journal_index_type curr);
void output_latex_write_castling(FILE *file,
                                 move_effect_journal_index_type movement);
void output_latex_write_move(FILE *file);

void WriteUserInputElement(FILE *file, char const *name, char const *value);

/* Instrument the solving machinery with slices that write the solution in
 * LaTeX
 */
void output_latex_instrument_solving(slice_index si, FILE *file);

#endif
