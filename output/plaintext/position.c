#include "output/plaintext/position.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/message.h"
#include "output/output.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/stipulation.h"
#include "options/maxthreatlength.h"
#include "options/maxflightsquares.h"
#include "options/nontrivial.h"
#include "conditions/grid.h"
#include "conditions/imitator.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/hunters.h"
#include "position/position.h"
#include "solving/castling.h"
#include "solving/move_generator.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

enum
{
  fileWidth = 4
};

static void CenterLine(char const *s)
{
  /* TODO move into one module per platform */
#if defined(ATARI)
#   if defined(__TURBOC__)
  sprintf(GlobalStr, "%s\n", s);
#   else    /* not __TURBOC__ */
  sprintf(GlobalStr, "%*s\n", (36+strlen(s))/2, s);
#   endif   /* __TURBOC__ */
#else   /* not ATARI */
  /* sprintf(GlobalStr, "%*s\n", (36+(int)strlen(s))/2, s); */
  sprintf(GlobalStr, "%*s\n", (38+(int)strlen(s))/2, s);
#endif  /* ATARI */
  StdString(GlobalStr);
}

static void MultiCenter(char *s) {
  char *p;

  while ((p=strchr(s,'\n'))) {
    *p= '\0';
    CenterLine(s);
    *p= '\n';
    s= p + 1;
  }
}

static boolean is_square_occupied_by_imitator(position const *pos, square s)
{
  boolean result = false;
  unsigned int imi_idx;

  for (imi_idx = 0; imi_idx<pos->number_of_imitators; ++imi_idx)
    if (s==pos->isquare[imi_idx])
    {
      result = true;
      break;
    }

  return result;
}

static void WriteCondition(char const CondLine[], boolean is_first)
{
  CenterLine(CondLine);
}

static void WriteCastlingMutuallyExclusive(void)
{
  /* no need to test in [Black] - information is redundant */
  if (castling_mutual_exclusive[White][queenside_castling-min_castling]!=0
      || castling_mutual_exclusive[White][kingside_castling-min_castling]!=0)
  {
    StdString(OptString[UserLanguage][mutuallyexclusivecastling]);

    if ((castling_mutual_exclusive[White][queenside_castling-min_castling]
         &ra_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_a1);
      WriteSquare(square_a8);
    }

    if ((castling_mutual_exclusive[White][queenside_castling-min_castling]
         &rh_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_a1);
      WriteSquare(square_h8);
    }

    if ((castling_mutual_exclusive[White][kingside_castling-min_castling]
         &ra_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_h1);
      WriteSquare(square_a8);
    }

    if ((castling_mutual_exclusive[White][kingside_castling-min_castling]
         &rh_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_h1);
      WriteSquare(square_h8);
    }

    StdChar('\n');
  }
}

static void WriteGrid(void)
{
  square square, square_a;
  int row, file;
  char    HLine[40];

  static char BorderL[]="+---a---b---c---d---e---f---g---h---+\n";
  static char HorizL[]="%c                                   %c\n";
  static char BlankL[]="|                                   |\n";

  StdChar('\n');
  StdString(BorderL);
  StdString(BlankL);

  for (row=0, square_a = square_a8;
       row<nr_rows_on_board;
       row++, square_a += dir_down) {
    char const *digits="87654321";
    sprintf(HLine, HorizL, digits[row], digits[row]);

    for (file=0, square= square_a;
         file<nr_files_on_board;
         file++, square += dir_right)
    {
      char g = (GridNum(square))%100;
      HLine[4*file+3]= g>9 ? (g/10)+'0' : ' ';
      HLine[4*file+4]= (g%10)+'0';
    }

    StdString(HLine);
    StdString(BlankL);
  }

  StdString(BorderL);
}

static void CollectPiecesWithAttribute(position const *pos,
                                       char ListSpec[256],
                                       piece_flag_type sp)
{
  square square_a = square_a8;
  unsigned int row;

  strcpy(ListSpec,PieSpString[UserLanguage][sp-nr_sides]);

  for (row = 1; row<=nr_rows_on_board; ++row, square_a += dir_down)
  {
    unsigned int file;
    square square = square_a;

    for (file = 1; file <= nr_files_on_board; ++file, square += dir_right)
      if (TSTFLAG(pos->spec[square],sp))
        AppendSquare(ListSpec,square);
  }
}

static void WriteNonRoyalAttributedPieces(position const *pos)
{
  piece_flag_type sp;

  for (sp = Royal+1; sp<nr_piece_flags; ++sp)
    if (TSTFLAG(some_pieces_flags,sp))
    {
      if (!(sp==Patrol && CondFlag[patrouille])
          && !(sp==Volage && CondFlag[volage])
          && !(sp==Beamtet && CondFlag[beamten]))
      {
        char ListSpec[256];
        CollectPiecesWithAttribute(pos,ListSpec,sp);
        CenterLine(ListSpec);
      }
    }
}

static unsigned int CollectRoyalPiecePositions(position const *pos,
                                               char ListSpec[256])
{
  unsigned int result = 0;

  square square_a = square_a8;
  unsigned int row;

  strcpy(ListSpec,PieSpString[UserLanguage][Royal-nr_sides]);

  for (row = 0; row!=nr_rows_on_board; ++row, square_a += dir_down)
  {
    unsigned int file;
    square square = square_a;

    for (file = 0; file!=nr_files_on_board; ++file, square += dir_right)
      if (TSTFLAG(pos->spec[square],Royal)
          && !is_king(pos->board[square]))
      {
        AppendSquare(ListSpec,square);
        ++result;
      }
  }

  return result;
}

static void WriteRoyalPiecePositions(position const *pos)
{
  char ListSpec[256];

  if (CollectRoyalPiecePositions(pos,ListSpec)>0)
    CenterLine(ListSpec);
}

static void WriteStipulationOptionsPieceCounts(position const *pos)
{
  unsigned int nBlack = 0;
  unsigned int nWhite = 0;
  unsigned int nNeutr = 0;
  square square_a = square_a8;
  unsigned int row;
  char StipOptStr[300];

  for (row = 0; row!=nr_rows_on_board; ++row, square_a += dir_down)
  {
    unsigned int file;
    square square = square_a;

    for (file = 0; file!=nr_files_on_board; ++file, square += dir_right)
    {
      if (is_piece_neutral(pos->spec[square]))
        ++nNeutr;
      else if (TSTFLAG(pos->spec[square],Black))
        ++nBlack;
      else if (TSTFLAG(pos->spec[square],White))
        ++nWhite;
    }
  }

  strcpy(StipOptStr, AlphaStip);

  if (OptFlag[solmenaces])
  {
    sprintf(StipOptStr+strlen(StipOptStr), "/%u", get_max_threat_length());
    if (OptFlag[solflights])
      sprintf(StipOptStr+strlen(StipOptStr), "/%d", get_max_flights());
  }
  else if (OptFlag[solflights])
    sprintf(StipOptStr+strlen(StipOptStr), "//%d", get_max_flights());

  if (OptFlag[nontrivial])
    sprintf(StipOptStr+strlen(StipOptStr),
            ";%d,%u",
            max_nr_nontrivial,get_min_length_nontrivial());

  {
    size_t const stipOptLength = strlen(StipOptStr);
    int const pieceCntWidth = (stipOptLength>nr_files_on_board*fileWidth
                               ? 1
                               : nr_files_on_board*fileWidth-stipOptLength+1);
    char PieCnts[20];
    if (nNeutr>0)
      sprintf(PieCnts, "%d + %d + %dn", nWhite, nBlack, nNeutr);
    else
      sprintf(PieCnts, "%d + %d", nWhite, nBlack);
    sprintf(GlobalStr, "  %s%*s\n", StipOptStr, pieceCntWidth, PieCnts);
    StdString(GlobalStr);
  }
}

static char *WriteWalkRtoL(char *pos, piece_walk_type walk)
{
  pos[0] = PieceTab[walk][1];
  if (pos[0]!=' ')
  {
    pos[0] = toupper(pos[0]);
    --pos;
  }

  pos[0] = toupper(PieceTab[walk][0]);
  --pos;

  return pos;
}

static void WriteRegularCells(position const *pos, square square_a)
{
  unsigned int file;
  square square;

  for (file = 0,  square = square_a;
       file!=nr_files_on_board;
       ++file, square += dir_right)
  {
    char cell[fileWidth+1];
    char *pos_in_cell = cell + (sizeof cell)/2;

    snprintf(cell, sizeof cell, "%*c", fileWidth, ' ');

    if (CondFlag[gridchess] && !OptFlag[suppressgrid])
    {
      if (is_on_board(square+dir_left)
          && GridLegal(square, square+dir_left))
        cell[0] = '|';
    }

    if (is_square_occupied_by_imitator(pos,square))
      pos_in_cell[0] = 'I';
    else if (pos->board[square]==Invalid)
      pos_in_cell[0] = ' ';
    else if (pos->board[square]==Empty)
      pos_in_cell[0] = '.';
    else
    {
      piece_walk_type const walk = pos->board[square];
      if (walk<Hunter0 || walk>=Hunter0+max_nr_hunter_walks)
        pos_in_cell = WriteWalkRtoL(pos_in_cell,walk);
      else
      {
        unsigned int const hunterIndex = walk-Hunter0;
        assert(hunterIndex<max_nr_hunter_walks);

        pos_in_cell[1] = '/';
        pos_in_cell = WriteWalkRtoL(pos_in_cell,huntertypes[hunterIndex].away);
      }

      if (is_piece_neutral(pos->spec[square]))
        pos_in_cell[0] = '=';
      else if (TSTFLAG(pos->spec[square],Black))
        pos_in_cell[0] = '-';
    }

    StdString(cell);
  }
}

static void WriteBaseCells(position const *pos, square square_a)
{
  unsigned int file;
  square square;

  for (file = 0, square = square_a;
       file!=nr_files_on_board;
       ++file, square += dir_right)
  {
    piece_walk_type const walk = pos->board[square];

    char cell[fileWidth+1];
    char *pos_in_cell = cell + (sizeof cell)/2;

    snprintf(cell, sizeof cell, "%*c", fileWidth, ' ');

    if (CondFlag[gridchess] && !OptFlag[suppressgrid])
    {
      if (is_on_board(square+dir_down)
          && GridLegal(square,square+dir_down))
      {
        pos_in_cell[-1] = '-';
        pos_in_cell[0] = '-';
        pos_in_cell[+1] = '-';
      }
    }

    if (Hunter0<=walk && walk<Hunter0+max_nr_hunter_walks)
    {
      unsigned int const hunterIndex = walk-Hunter0;
      WriteWalkRtoL(pos_in_cell,huntertypes[hunterIndex].home);
    }

    StdString(cell);
  }
}

static void WriteBorder(void)
{
  unsigned int file;
  char letter;

  assert(nr_files_on_board <= 'z'-'a');

  StdString("+--");

  for (file = 0, letter = 'a'; file!=nr_files_on_board; ++file, ++letter)
  {
    char cell[fileWidth+1];
    snprintf(cell, sizeof cell, "-%c--", letter);
    StdString(cell);
  }

  StdString("-+\n");
}

static void WriteBlankLine(void)
{
  unsigned int file;

  StdString("| ");
  StdString(" ");

  for (file = 0; file!=nr_files_on_board; ++file)
    StdString("    ");

  StdString(" |\n");
}

void WriteBoard(position const *pos)
{
  unsigned int row;
  square square_a;

  assert(nr_rows_on_board<10);

  StdChar('\n');
  WriteBorder();
  WriteBlankLine();

  for (row = 0, square_a = square_a8;
       row!=nr_rows_on_board;
       ++row, square_a += dir_down)
  {
    char border[4];

    snprintf(border, sizeof border, "%d ", nr_rows_on_board-row);
    StdString(border);

    WriteRegularCells(pos,square_a);

    snprintf(border, sizeof border, "  %d", nr_rows_on_board-row);
    StdString(border);
    StdChar('\n');

    StdString("| ");
    WriteBaseCells(pos,square_a);
    StdString("  |\n");
  }

  WriteBorder();
}

static void WriteMeta(void)
{
  StdChar('\n');
  MultiCenter(ActAuthor);
  MultiCenter(ActOrigin);
  MultiCenter(ActAward);
  MultiCenter(ActTitle);
}

void WritePosition(position const *pos)
{
  WriteMeta();
  WriteBoard(pos);
  WriteStipulationOptionsPieceCounts(pos);
  WriteRoyalPiecePositions(pos);
  WriteNonRoyalAttributedPieces(pos);
  WriteConditions(&WriteCondition);
  WriteCastlingMutuallyExclusive();

  if (OptFlag[halfduplex])
    CenterLine(OptString[UserLanguage][halfduplex]);
  else if (OptFlag[duplex])
    CenterLine(OptString[UserLanguage][duplex]);

  if (OptFlag[quodlibet])
    CenterLine(OptString[UserLanguage][quodlibet]);

  if (CondFlag[gridchess] && OptFlag[writegrid])
    WriteGrid();
}
