#include "output/plaintext/position.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/pieces.h"
#include "output/output.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/stipulation.h"
#include "options/maxthreatlength.h"
#include "options/maxflightsquares.h"
#include "options/nontrivial.h"
#include "conditions/grid.h"
#include "conditions/imitator.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/hunters.h"
#include "position/position.h"
#include "solving/castling.h"
#include "solving/move_generator.h"
#include "pymsg.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

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

static boolean is_square_occupied_by_imitator(square s)
{
  boolean result = false;
  unsigned int imi_idx;

  for (imi_idx = 0; imi_idx<number_of_imitators; ++imi_idx)
    if (s==isquare[imi_idx])
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

void WritePosition()
{
  int nBlack, nWhite, nNeutr;
  square square, square_a;
  int row, file;
  char    HLine1[40];
  char    HLine2[40];
  char    PieCnts[20];
  char    StipOptStr[300];
  PieSpec sp;
  char    ListSpec[PieSpCount-nr_sides][256];
  unsigned int SpecCount[PieSpCount-nr_sides] = { 0 };

  static char BorderL[]="+---a---b---c---d---e---f---g---h---+\n";
  static char HorizL[]="%c   .   .   .   .   .   .   .   .   %c\n";
  static char BlankL[]="|                                   |\n";

  unsigned int const fileWidth = 4;

  for (sp = nr_sides; sp<PieSpCount; ++sp)
    strcpy(ListSpec[sp-nr_sides], PieSpString[UserLanguage][sp-nr_sides]);

  StdChar('\n');
  MultiCenter(ActAuthor);
  MultiCenter(ActOrigin);
  MultiCenter(ActAward);
  MultiCenter(ActTitle);

  nBlack= nWhite= nNeutr= 0;
  StdChar('\n');
  StdString(BorderL);
  StdString(BlankL);

  for (row=1, square_a = square_a8;
       row<=nr_rows_on_board;
       row++, square_a += dir_down)
  {
    char const *digits="87654321";
    sprintf(HLine1, HorizL, digits[row-1], digits[row-1]);

    strcpy(HLine2,BlankL);

    for (file= 1, square= square_a;
         file <= nr_files_on_board;
         file++, square += dir_right)
    {
      char *h1= HLine1 + fileWidth*file;

      if (CondFlag[gridchess] && !OptFlag[suppressgrid])
      {
        if (file < nr_files_on_board
            && GridLegal(square, square+dir_right))
          HLine1[fileWidth*file+2] = '|';

        if (row < nr_rows_on_board
            && GridLegal(square, square+dir_down))
        {
          HLine2[fileWidth*file-1] = '-';
          HLine2[fileWidth*file] = '-';
          HLine2[fileWidth*file+1] = '-';
        }
      }

      if (is_square_occupied_by_imitator(square))
        *h1= 'I';
      else if (is_square_blocked(square))
        /* this is a hole ! */
        *h1= ' ';
      else if (is_square_empty(square))
      {
        /* nothing */
      }
      else
      {
        PieNam const pp = get_walk_of_piece_on_square(square);
        for (sp= nr_sides; sp<PieSpCount; ++sp)
          if (TSTFLAG(spec[square],sp)
              && !(sp==Royal && (pp==King || pp==Poseidon)))
          {
            AppendSquare(ListSpec[sp-nr_sides], square);
            ++SpecCount[sp-nr_sides];
          }

        if (pp<Hunter0 || pp>=Hunter0+maxnrhuntertypes)
        {
          if ((*h1= PieceTab[pp][1]) != ' ')
          {
            *h1= toupper(*h1);
            h1--;
          }
          *h1--= toupper(PieceTab[pp][0]);
        }
        else
        {
          char *n1 = HLine2 + (h1-HLine1); /* current position on next1 line */

          unsigned int const hunterIndex = pp-Hunter0;
          assert(hunterIndex<maxnrhuntertypes);

          *h1-- = '/';
          if ((*h1= PieceTab[huntertypes[hunterIndex].away][1]) != ' ')
          {
            *h1= toupper(*h1);
            h1--;
          }
          *h1--= toupper(PieceTab[huntertypes[hunterIndex].away][0]);

          --n1;   /* leave pos. below '/' empty */
          if ((*n1= PieceTab[huntertypes[hunterIndex].home][1]) != ' ')
            *n1= toupper(*n1);
          *n1 = toupper(PieceTab[huntertypes[hunterIndex].home][0]);
        }

        if (is_piece_neutral(spec[square]))
        {
          nNeutr++;
          *h1= '=';
        }
        else if (TSTFLAG(spec[square],Black))
        {
          nBlack++;
          *h1= '-';
        }
        else
          nWhite++;
      }
    }

    StdString(HLine1);
    StdString(HLine2);
  }

  StdString(BorderL);
  if (nNeutr>0)
    sprintf(PieCnts, "%d + %d + %dn", nWhite, nBlack, nNeutr);
  else
    sprintf(PieCnts, "%d + %d", nWhite, nBlack);

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
    sprintf(GlobalStr, "  %s%*s\n", StipOptStr, pieceCntWidth, PieCnts);
    StdString(GlobalStr);
  }

  if (SpecCount[Royal-nr_sides]>0)
    CenterLine(ListSpec[Royal-nr_sides]);

  for (sp = nr_sides; sp<PieSpCount; sp++)
    if (TSTFLAG(some_pieces_flags,sp))
      if (sp!=Royal
          && !(sp==Patrol && CondFlag[patrouille])
          && !(sp==Volage && CondFlag[volage])
          && !(sp==Beamtet && CondFlag[beamten]))
        CenterLine(ListSpec[sp-nr_sides]);

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
