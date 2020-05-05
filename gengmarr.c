#include "position/position.h"

#include <stdio.h>
#include <stdlib.h>

/* Standalone program that writes the initializer for the position
 * object containing the game array.
 */


/* Write information to the output file about how the output file was
 * generated.
 * @param dest destination stream (output file)
 * @param argv0 name of executable produced from this file (i.e. that
 *              produces the outfile)
 */
static void write_generation_info(FILE *dest, char const *argv0)
{
  fputs("/******** IMPORTANT INFORMATION ****************\n",dest);
  fprintf(dest,
          "This file is generated using the program %s -- DON'T CHANGE.\n",
          argv0);
  fprintf(dest,"%s was compiled from %s.\n",argv0,__FILE__);
  fputs("***********************************************/\n",dest);
}

/* Write board initialiser to output file
 * @param dest destination stream (output file)
 * @param board initialized board to dump
 */
static void dump_board_initialiser_to_stream(FILE *dest, echiquier const board)
{
  unsigned int i;
  size_t const nr_squares = maxsquare+4;
  unsigned int column = 0;

  fputs("  { /* board */\n    ",dest);
  for (i = 0; i+1<nr_squares; ++i)
  {
    fprintf(dest,"(piece_walk_type) %2u,",(unsigned int)board[i]);
    ++column;
    if (column==onerow)
    {
      fputs("\n    ",dest);
      column = 0;
    }
  }
  fprintf(dest,"(piece_walk_type) %2u\n",(unsigned int)board[i]);
  fputs("  }",dest);
}

/* Write piece specs initialiser to output file
 * @param dest destination stream (output file)
 * @param spec initialized specs to dump
 */
static void dump_spec_initialiser_to_stream(FILE *dest, Flags const spec[])
{
  unsigned int i;
  size_t const nr_squares = maxsquare+4;
  unsigned int column = 0;

  fputs("  { /* spec */\n    ",dest);
  for (i = 0; i+1<nr_squares; ++i)
  {
    fprintf(dest,"%lu,",spec[i]);
    ++column;
    if (column==onerow)
    {
      fputs("\n    ",dest);
      column = 0;
    }
  }
  fprintf(dest,"%lu\n",spec[i]);
  fputs("  }",dest);
}

/* Write king square initialisers to output file
 * @param dest destination stream (output file)
 * @param rb position of white king to dump
 * @param rn position of black king to dump
 */
static void dump_royal_initialisers_to_stream(FILE *dest, square rb, square rn)
{
  fprintf(dest,"  { %d,%d } /* king positions */",rb,rn);
}

/* Write imitator initialiser to output file
 * @param dest destination stream (output file)
 * @param inum initial number of imitators
 * @param isquare initial position of imitators
 */
static void dump_imitator_initialisers_to_stream(FILE *dest,
                                                 unsigned int inum,
                                                 imarr const isquare)
{
  unsigned int i;
  fprintf(dest,"  %u, ",inum);
  fputs("{ ",dest);
  for (i = 0; i+1<maxinum; ++i)
    fprintf(dest,"%d,",isquare[i]);
  fprintf(dest,"%d } /* imitators */",isquare[i]);
}

/* Write piece count initialiser to output file
 * @param dest destination stream (output file)
 * @param nr_piece array containing initial piece counts
 */
static void dump_nr_piece_initialisers_to_stream(FILE *dest, position const *pos)
{
  piece_walk_type p;
  unsigned int column = 0;

  fputs("  { /* numbers of pieces */\n",dest);
  fputs("    { /* White */\n      ",dest);
  for (p = Empty; p<nr_piece_walks-1; ++p)
  {
    fprintf(dest,"%u,",pos->number_of_pieces[White][p]);
    ++column;
    if (column==20)
    {
      fputs("\n      ",dest);
      column = 0;
    }
  }
  fprintf(dest,"%u\n",pos->number_of_pieces[White][p]);
  fputs("    },\n",dest);
  column = 0;
  fputs("    { /* Black */\n      ",dest);
  for (p = Empty; p<nr_piece_walks-1; ++p)
  {
    fprintf(dest,"%u,",pos->number_of_pieces[Black][p]);
    ++column;
    if (column==20)
    {
      fputs("\n      ",dest);
      column = 0;
    }
  }
  fprintf(dest,"%u\n",pos->number_of_pieces[Black][p]);
  fputs("    }\n",dest);
  fputs("  }",dest);
}

static void dump_castling_rights_initialiser_to_stream(FILE *dest, position const *pos)
{
  fprintf(dest,"  (castling_rights_type) 0x%x /* castling_rights */\n",(unsigned int)pos->castling_rights);
}

/* Write position initialiser to output file
 * @param dest destination stream (output file)
 * @param pos array of position object containing initial position
 */
static void dump_position_initialiser_to_stream(FILE *dest, position const *pos)
{
  fputs("#include \"position/position.h\"\n",dest);
  fputs("position const game_array =\n",dest);
  fputs("{\n",dest);
  dump_board_initialiser_to_stream(dest,pos->board);
  fputs(",\n",dest);
  dump_spec_initialiser_to_stream(dest,pos->spec);
  fputs(",\n",dest);
  dump_royal_initialisers_to_stream(dest,pos->king_square[White],pos->king_square[Black]);
  fputs(",\n",dest);
  dump_imitator_initialisers_to_stream(dest,pos->number_of_imitators,pos->isquare);
  fputs(",\n",dest);
  dump_nr_piece_initialisers_to_stream(dest,pos);
  fputs(",\n",dest);
  dump_castling_rights_initialiser_to_stream(dest,pos);
  fputs("  , 0 /* currPieceId */\n",dest);
  fputs("};\n",dest);
}

/* Write position initialiser to output file (yet to be opened);
 * produce error message if output file can't be opened.
 * @param pos array of position object containing initial position
 * @param argv0 first element of parameter argv (i.e. name of executable)
 */
static void dump_position_initialiser(position const *pos,
                                      char const *argv0,
                                      FILE *dest)
{
  write_generation_info(dest,argv0);
  dump_position_initialiser_to_stream(dest,pos);
}

int main(int argc, char *argv[])
{
  FILE * const output_stream = argc<=1 ? stdout : fopen(argv[1],"w");
  if (output_stream==NULL)
  {
    perror("Error opening output file");
    return EXIT_FAILURE;
  }
  else
  {
    position game_array;
    initialise_game_array(&game_array);

    dump_position_initialiser(&game_array,argv[0],output_stream);

    if (fclose(output_stream)==EOF)
    {
      perror("Error closing output file");
      return EXIT_FAILURE;
    }
    else
      return EXIT_SUCCESS;
  }
}
