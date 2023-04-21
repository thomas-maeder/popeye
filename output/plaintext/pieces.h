#if !defined(OUTPUT_PLAINTEXT_PIECES_H)
#define OUTPUT_PLAINTEXT_PIECES_H

#include "position/position.h"
#include "pieces/pieces.h"
#include "utilities/boolean.h"
#include "output/output.h"

#include <stdio.h>

void WriteFlag(output_engine_type const * engine, FILE *file,
               piece_flag_type spname);
boolean WriteSpec(output_engine_type const * engine, FILE *file,
                  Flags pspec, piece_walk_type p, boolean printcolours);
void WriteWalk(output_engine_type const * engine, FILE *file,
                piece_walk_type p);
void WriteSquare(output_engine_type const * engine, FILE *file,
                 square i);

void AppendSquare(char *List, square s);

#endif
