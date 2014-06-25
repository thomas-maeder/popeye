#include "output/latex/latex.h"
#include "output/latex/twinning.h"
#include "output/latex/line/line.h"
#include "output/latex/tree/tree.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/message.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/condition.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/pieces.h"
#include "input/plaintext/token.h"
#include "input/plaintext/language.h"
#include "input/plaintext/stipulation.h"
#include "options/maxsolutions/maxsolutions.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "pieces/walks/classification.h"
#include "pieces/attributes/neutral/neutral.h"
#include "stipulation/pipe.h"
#include "conditions/bgl.h"
#include "conditions/grid.h"
#include "conditions/singlebox/type1.h"
#include "platform/maxtime.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char filename[LINESIZE];    /* This array contains the input as is */

FILE *LaTeXFile;

static char *LaTeXPiecesAbbr[nr_piece_walks];
static char *LaTeXPiecesFull[nr_piece_walks];
char *LaTeXStdPie[8] = { NULL, "C", "K", "B", "D", "S", "T", "L"};

static char const CharChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

char *LaTeXWalk(piece_walk_type walk)
{
  if (walk > Bishop)
  {
    if (LaTeXPiecesAbbr[walk] == NULL)
    {
      ErrorMsg(UndefLatexPiece);
      return "??";
    } else
      return LaTeXPiecesAbbr[walk];
  } else
    return LaTeXStdPie[walk];
}

char *ParseLaTeXPieces(char *tok)
{
  piece_walk_type Name;
  int i;

  /* don't delete all this. Since both arrays are declared static,
   * they are initialized to NULL.
   * Simply allow overwriting these definitions within the LaTeX clause
   * and let it be initialized from pie-<lang>.dat
   *
   for (Name= 1; Name < PieceCount; Name++) {
   if (LaTeXPiecesAbbr[Name]) {
   free(LaTeXPiecesAbbr[Name]);
   free(LaTeXPiecesFull[Name]);
   }
   LaTeXPiecesAbbr[Name]= NULL;
   LaTeXPiecesFull[Name]= NULL;
   }
  */

  if (strlen(tok) < 3) {
    while (true)
    {
      Name= GetPieNamIndex(*tok, strlen(tok) == 1 ? ' ' : tok[1]);

      if (Name < King) {
        return tok;
      }

      if (LaTeXPiecesAbbr[Name])
      {
        free(LaTeXPiecesAbbr[Name]);
        free(LaTeXPiecesFull[Name]);
      }

      tok = ReadNextTokStr();
      LaTeXPiecesAbbr[Name]= (char *)malloc(sizeof(char)*(strlen(tok)+1));
      i= 0;
      while (tok[i]) {
        /* to avoid compiler warnings below made "better readable" */
        /*      LaTeXPiecesAbbr[Name][i]= tok[i++]+ 'A' - 'a';          */
        LaTeXPiecesAbbr[Name][i]= tok[i] + 'A' - 'a';
        i++;
      }
      LaTeXPiecesAbbr[Name][i]= tok[i];

      ReadToEndOfLine();
      tok = InputLine;
      LaTeXPiecesFull[Name]= (char *)malloc(sizeof(char)*(strlen(tok)+1));
      strcpy(LaTeXPiecesFull[Name], tok);

      tok = ReadNextTokStr();
    }
  }

  return tok;
}

static void WriteFixElement(FILE *file,
                            char const *name, char const *value,
                            unsigned int indentation)
{
  fprintf(file,"%*c%s{%s}%%\n",indentation+1,'\\',name,value);
}

void WriteUserInputElement(FILE *file, char const *name, char const *value)
{
  fprintf(file," \\%s{",name);
  LaTeXStr(file,value);
  fputs("}%\n",file);
}

static void WriteUserInputSubElement(FILE *file,
                                     char const *name,
                                     unsigned value_length, char const *value)
{
  fprintf(file," \\%s{%.*s}%%\n",name,value_length,value);
}

static void WriteGeneratedElement(FILE *file,
                                  char const *name, char const *value)
{
  fprintf(file," \\%s{%s}%%\n",name,value);
}

static void OpenGeneratedElementOneLine(FILE *file, char const *name)
{
  fprintf(file," \\%s{",name);
}

static void CloseElementOneLine(FILE *file)
{
  fputs("}",file);
}

static void CloseElement(FILE *file)
{
  fputs(" }%\n",file);
}

static void WriteCommand(FILE *file, char const *name)
{
  fprintf(file,"\\%s%%\n",name);
}

static void WriteInBraces(FILE *file, char const *name)
{
  fprintf(file,"{\\%s}",name);
}

void LaTeXStr(FILE *file, char const *line)
{
  while (*line)
  {
    switch (*line)
    {
    case '#':
    case '&':
    case '%':
      fprintf(file,"\\%c", *line);
      break;

    case '-':
      if (*(line+1) == '>')   /* convert -> to \ra   FCO */
      {
        WriteInBraces(file,"ra");
        ++line;
      }
      else   /* ordinary minus */
        fputc(*line,file);
      break;

    case '<':
      if (*(line+1)=='-' && *(line+2)=='-' && *(line+3)=='>')   /* convert -> to \lra  */
      {
        WriteInBraces(file,"lra");
        line += 3;
      }
      else  /* ordinary less than */
        fputc(*line,file);
      break;

    default:
      fputc(*line,file);
      fflush(file);         /* non-buffered output  FCO */
      break;
    }

    ++line;
  }
}

static void WriteIntro(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  WriteFixElement(file,"documentclass","article",0);
  WriteFixElement(file,"usepackage","diagram",0);
  if (UserLanguage==German)
    WriteFixElement(file,"usepackage","german",0);
  fputs("\n",file);
  WriteFixElement(file,"begin","document",0);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean LaTeXSetup(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  LaTeXFile = fopen(InputLine,"w");
  if (LaTeXFile==NULL)
  {
    IoErrorMsg(WrOpenError,0);
    result = false;
  }
  else
  {
    strcpy(filename,InputLine);
    WriteIntro(LaTeXFile);
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void LaTeXShutdown(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (LaTeXFile!=0)
  {
    WriteCommand(LaTeXFile,"putsol");
    WriteFixElement(LaTeXFile,"end","document",0);
    fclose(LaTeXFile);
    LaTeXFile = NULL;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTexOpenSolution(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  OpenGeneratedElementOneLine(file,"solution");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTexCloseSolution(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  fputs(" }%\n",file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXEndDiagram(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!(OptFlag[solmenaces]
        || OptFlag[solflights]
        || OptFlag[nontrivial]
        || max_solutions_reached()
        || was_max_nr_solutions_per_target_position_reached()
        || has_short_solution_been_found_in_problem()
        || hasMaxtimeElapsed()))
  {
    fputs(" \\Co+%",file);
    if (!flag_regression)
      fputs(versionString,file);
    fputs("\n",file);
  }

  WriteFixElement(file,"end","diagram",0);
  WriteCommand(file,"hfill");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteCondition(FILE *file, char const CondLine[], condition_rank rank)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (rank)
  {
    case condition_first:
      OpenGeneratedElementOneLine(file,"condition");
      LaTeXStr(file,CondLine);
      break;

    case condition_subsequent:
      WriteInBraces(file,"newline");
      LaTeXStr(file,CondLine);
      break;

    case condition_end:
      CloseElement(file);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteAuthor(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (strlen(ActAuthor)>0)
  {
    char TeXAuthor[256];
    strcpy(TeXAuthor,ActAuthor);

    if (strchr(TeXAuthor, ','))
    {
      /* , --> correct format */
      char *cp = strchr(TeXAuthor,'\n');
      char *endcp = 0;

      while (cp!=0)
      {
        *cp= ';';
        endcp = cp;
        cp = strchr(TeXAuthor,'\n');
      }

      if (endcp!=0)
        *endcp = '\0';

      WriteUserInputElement(file,"author",TeXAuthor);
    }
    else
    {
      /* reverse first and surnames */
      char *cp1 = TeXAuthor;
      char *cp2 = strchr(cp1,'\n');

      OpenGeneratedElementOneLine(file,"author");

      while (cp2)
      {
        char *cp3 = cp2;
        *cp2= '\0';

        if (cp1!=TeXAuthor)
          fputs("; ",file);

        while (cp3>cp1 && *cp3!=' ')
          --cp3;

        /* wrong LaTeX output if the authors surname only given */
        if (cp3 == cp1)
        {
          /* we got only the surname ! */
          LaTeXStr(file,cp3);
          fputs(", ",file);
        }
        else
        {
          /* we got firstname and surname */
          *cp3 = '\0';
          LaTeXStr(file,cp3+1);
          fputs(", ",file);
          LaTeXStr(file,cp1);
        }

        *cp3 = '\n';
        *cp2 = '\n';

        cp1= cp2+1;
        cp2 = strchr(cp1,'\n');
      }

      CloseElement(file);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteSource(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* format: [diagram number,] source [issue number,] [date] */
  if (ActOrigin[0] != '\0')
  {
    char *source= ActOrigin;
    char *date, *eol, *tmp;

    /* diagram number */
    while (strchr(CharChar, *source))
      source++;

    if (*source == ',')
    {
      *source= '\0';
      WriteUserInputElement(file,"sourcenr",ActOrigin);
      *source = ',';
      do
      {
        ++source;
      } while (*source==' ');
    }
    else
      source = ActOrigin;

    /* date */
    /* supported formats: year
    **            month/year
    **            month-month/year
    **            day. month. year
    **            day.-day. month. year
    */
    /* year */
    eol = date= strchr(source, '\n');
    *eol = '\0';

    while (strchr("0123456789-", *(date-1)))
      date--;

    if (date != eol)
    {
      /* sucessfully parsed a year */
      WriteUserInputElement(file,"year",date);

      /* parse month(s) now */
      /* while (*(date-1) == ' ') date--; */
      switch (*(date-1))
      {
      case '/':
        /* format is either month/year or month-month/year */
        --date;
        while (*(date-1) == ' ')
          date--;
        tmp = date;
        while (strchr("0123456789-", *(date-1)))
          date--;

        if (strchr(date,'-'))
          fprintf(file, " \\months{%.*s}%%\n",(int)(tmp-date), date);
        else
          fprintf(file, " \\month{%.*s}%%\n",(int)(tmp-date), date);

        break;

      case '.':
        /* format is either
           day. month. year or day.-day. month. year
        */
        date--;
        tmp= date;
        while (strchr("0123456789", *(date-1)))
          date--;
        fprintf(file, " \\month{%.*s}%%\n", (int)(tmp-date), date);
        /* now parse day(s) */
        while (*(--date) == ' ');
        tmp= date;
        while (strchr("0123456789-.", *(date-1)))
          date--;
        fprintf(file, " \\day{%.*s}%%\n", (int)(tmp-date), date);
        break;
      }
    } /* month(s), day(s) */

    /* issue number */
    while (*(date-1) == ' ')
      date--;
    if (*(date-1) == ',')
    {
      /* issue number found */
      tmp= --date;
      while (*(date-1) != ' ')
        date--;
      fprintf(file, " \\issue{%.*s}%%\n", (int)(tmp-date), date);
    } /* issue */

    /* default */
    /* source name or complete source if not interpretable */
    while (*(date-1) == ' ')
      date--;

    fputs(" \\source{",file);
    {
      char const save = *date;
      *date = '\0';
      LaTeXStr(file,source);
      *date = save;
    }
    fputs("}%\n",file);

    *eol= '\n';
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteAward(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (ActAward[0] != '\0')
  {
    char *tour = strchr(ActAward, ',');
    char *eol = strchr(ActAward, '\n');
    *eol = '\0';
    if (tour)
    {
      WriteUserInputSubElement(file,"award",tour-ActAward,ActAward);
      do
      {
        ++tour;
      } while (*tour==' ');
      WriteUserInputElement(file,"tournament",tour);
    } else
      WriteUserInputElement(file,"award",ActAward);
    *eol = '\n';
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteDedication(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (ActTitle[0] != '\0')
    WriteUserInputElement(file,"dedication",ActTitle);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WritePieces(FILE *file)
{
  square const *bnp;
  boolean piece_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  OpenGeneratedElementOneLine(file,"pieces");

  for (bnp = boardnum; *bnp; bnp++)
  {
    if (!is_square_blocked(*bnp) && !is_square_empty(*bnp))
    {
      piece_walk_type const p = get_walk_of_piece_on_square(*bnp);

      if (piece_found)
        fputs(", ",file);
      else
        piece_found = true;

      fprintf(file,"%c%s%c%c",
              is_piece_neutral(being_solved.spec[*bnp]) ? 'n' : TSTFLAG(being_solved.spec[*bnp],White) ? 'w' : 's',
              LaTeXWalk(p),
              *bnp%onerow-200%onerow+'a',
              *bnp/onerow-200/onerow+'1');
    }
  }

  CloseElement(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean FindFairyWalks(boolean colour_has_piece[nr_piece_walks][nr_colours])
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    piece_walk_type p;
    for (p = Empty; p < nr_piece_walks; ++p)
    {
      colour_has_piece[p][colour_white] = false;
      colour_has_piece[p][colour_black] = false;
      colour_has_piece[p][colour_neutral] = false;
    }
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
    {
      if (!is_square_blocked(*bnp) && !is_square_empty(*bnp))
      {
        piece_walk_type const p = get_walk_of_piece_on_square(*bnp);
        if (p>Bishop && LaTeXPiecesAbbr[p]!=NULL)
        {
          result = true;

          if (is_piece_neutral(being_solved.spec[*bnp]))
            colour_has_piece[p][colour_neutral] = true;
          else if (TSTFLAG(being_solved.spec[*bnp], White))
            colour_has_piece[p][colour_white] = true;
          else
            colour_has_piece[p][colour_black] = true;
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean FindPiecesWithSpecs(unsigned int SpecCount[nr_piece_flags-nr_sides],
                                   char ListSpec[nr_piece_flags-nr_sides][4*nr_files_on_board*nr_rows_on_board])
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    piece_flag_type sp;
    for (sp= nr_sides; sp<nr_piece_flags; ++sp)
      strcpy(ListSpec[sp-nr_sides], PieSpString[UserLanguage][sp-nr_sides]);
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
      if (!is_square_blocked(*bnp) && !is_square_empty(*bnp))
      {
        piece_walk_type const p = get_walk_of_piece_on_square(*bnp);

        piece_flag_type sp;
        for (sp= nr_sides; sp<nr_piece_flags; ++sp)
          if (TSTFLAG(being_solved.spec[*bnp], sp) && !(sp==Royal && is_king(p)))
          {
            AppendSquare(ListSpec[sp-nr_sides],*bnp);
            ++SpecCount[sp-nr_sides];
          }
      }
  }

  {
    piece_flag_type sp;
    for (sp= nr_sides; sp<nr_piece_flags; ++sp)
      if (SpecCount[sp-nr_sides]>0
          && !(sp==Patrol && CondFlag[patrouille])
          && !(sp==Volage && CondFlag[volage])
          && !(sp==Beamtet && CondFlag[beamten]))
      {
        result = true;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean FindHoles(char HolesSqList[4*nr_files_on_board*nr_rows_on_board])
{
  boolean result = false;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
    if (is_square_blocked(*bnp))
    {
      if (result)
        strcat(HolesSqList, ", ");
      else
        result = true;
      AppendSquare(HolesSqList,*bnp);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void WriteFairyWalk(FILE *file, Colour colour, piece_walk_type w)
{
  char const colour_short[] = { 'w', 'b', 'n' };
  fprintf(file,
          "\\%c%s =%s",
          colour_short[colour],
          LaTeXPiecesAbbr[w],
          LaTeXPiecesFull[w]);
}

static void WriteFairyWalks(FILE *file,
                            boolean side_has_walk[nr_piece_walks][nr_colours])
{
  boolean fairy_walk_written = false;
  piece_walk_type p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (p = Bishop+1; p<nr_piece_walks; ++p)
    if (side_has_walk[p][colour_white]
        || side_has_walk[p][colour_black]
        || side_has_walk[p][colour_neutral])
    {
      piece_walk_type q;
      for (q = Bishop+1; q<p; q++)
        if ((side_has_walk[q][colour_white]
             || side_has_walk[q][colour_black]
             || side_has_walk[q][colour_neutral])
            && LaTeXPiecesAbbr[p][0]==LaTeXPiecesAbbr[q][0]
            && LaTeXPiecesAbbr[p][1]==LaTeXPiecesAbbr[q][1])
          fprintf(stderr,
                  "+++ Warning: double representation '%s' for %s and %s\n",
                  LaTeXPiecesAbbr[q],
                  LaTeXPiecesFull[p],
                  LaTeXPiecesFull[q]);

      if (fairy_walk_written)
        WriteInBraces(file,"newline");
      OpenGeneratedElementOneLine(file,"mbox");
      {
        Colour c;
        for (c = 0; c!=nr_colours; ++c)
          if (side_has_walk[p][c])
            WriteFairyWalk(file,c,p);
      }
      CloseElementOneLine(file);
      fairy_walk_written = true;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WritePiecesWithSpecs(FILE *file,
                                 boolean remark_written,
                                 unsigned int const SpecCount[nr_piece_flags-nr_sides],
                                 char ListSpec[nr_piece_flags-nr_sides][4*nr_files_on_board*nr_rows_on_board])
{
  piece_flag_type sp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (sp = nr_sides; sp<nr_piece_flags; ++sp)
    if (SpecCount[sp-nr_sides]>0)
    {
      if (remark_written)
        fputs("{\\newline}\n    ",file);
      fprintf(file, "%s\n", ListSpec[sp-nr_sides]);
      remark_written = true;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteHoles(FILE *file,
                       boolean remark_written,
                       char const HolesSqList[256])
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (remark_written)
    fputs("{\\newline}\n    ",file);
  fprintf(file,"%s %s%%\n",CondString[UserLanguage][holes],HolesSqList);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteFairyPieces(FILE *file)
{
  boolean colour_has_walk[nr_piece_walks][nr_colours];
  boolean const fairy_walk_found = FindFairyWalks(colour_has_walk);

  unsigned int SpecCount[nr_piece_flags-nr_sides] = { 0 };
  char ListSpec[nr_piece_flags-nr_sides][4*nr_files_on_board*nr_rows_on_board];
  boolean const piece_with_specs_found = FindPiecesWithSpecs(SpecCount,ListSpec);

  char HolesSqList[4*nr_files_on_board*nr_rows_on_board] = "";
  boolean const hole_found = FindHoles(HolesSqList);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (hole_found)
  {
    WriteGeneratedElement(file,"nofields",HolesSqList);
    WriteGeneratedElement(file,"fieldframe",HolesSqList);
  }

  if (fairy_walk_found || hole_found || piece_with_specs_found)
  {
    OpenGeneratedElementOneLine(file,"remark");

    if (fairy_walk_found)
      WriteFairyWalks(file,colour_has_walk);

    if (piece_with_specs_found)
      WritePiecesWithSpecs(file,fairy_walk_found,SpecCount,ListSpec);

    if (hole_found)
      WriteHoles(file, fairy_walk_found || piece_with_specs_found, HolesSqList);

    CloseElement(file);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteStipulation(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  OpenGeneratedElementOneLine(file,"stipulation");
  LaTeXStr(file,ActStip);
  if (OptFlag[solapparent])
    fputs("*",file);
  if (OptFlag[whitetoplay])
    fprintf(file," %c{\\ra}", tolower(*PieSpString[UserLanguage][White]));
  CloseElement(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteGridOrthogonalLines(FILE *file)
{
  unsigned int i;
  boolean line_written = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 1; i<nr_files_on_board; i++)
    if (GridNum(square_a1+dir_right*(i-1))!=GridNum(square_a1+dir_right*i))
    {
      if (line_written)
        fputs(", ",file);
      else
      {
        OpenGeneratedElementOneLine(file,"gridlines");
        line_written = true;
      }
      fprintf(file, " v%u08", i);
    }

  for (i = 1; i<nr_rows_on_board; i++)
    if (GridNum(square_a1+dir_up*(i-1))!=GridNum(square_a1+dir_up*i))
    {
      if (line_written)
        fputs(", ",file);
      else
      {
        OpenGeneratedElementOneLine(file,"gridlines");
        line_written = true;
      }
      fprintf(file, " h0%u8", i);
    }

  if (line_written)
    CloseElement(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteGridIrregular(FILE *file)
{
  boolean line_written = false;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
  {
    unsigned int const column = *bnp%onerow - nr_of_slack_files_left_of_board;
    unsigned int const row = *bnp/onerow - nr_of_slack_rows_below_board;

    if (column>0 && GridLegal(*bnp+dir_left,*bnp))
    {
      if (line_written)
        fputs(", ",file);
      else
      {
        OpenGeneratedElementOneLine(file,"gridlines");
        line_written = true;
      }
      fprintf(file," v%d%d1",column,row);
    }

    if (row>0 && GridLegal(*bnp+dir_down,*bnp))
    {
      if (line_written)
        fputs(", ",file);
      else
      {
        OpenGeneratedElementOneLine(file,"gridlines");
        line_written = true;
      }
      fprintf(file," h%d%d1",column,row);
    }
  }

  if (line_written)
    CloseElement(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteGrid(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[gridchess] && !OptFlag[suppressgrid])
    switch (grid_type)
    {
      case grid_normal:
        WriteCommand(file,"stdgrid");
        break;

      case grid_vertical_shift:
        WriteGeneratedElement(file,"gridlines","h018, h038, h058, h078, v208, v408, v608");
        break;

      case grid_horizontal_shift:
        WriteGeneratedElement(file,"gridlines","h028, h048, h068, v108, v308, v508, v708");
        break;

      case grid_diagonal_shift:
        WriteGeneratedElement(file,"gridlines","h018, h038, h058, h078, v108, v308, v508, v708");
        break;

      case grid_orthogonal_lines:
        WriteGridOrthogonalLines(file);
        break;

      /* of course, WriteGridIrregular() could cover the above cases as well */
      case grid_irregular:
        WriteGridIrregular(file);
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void WriteSquareFrames(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[magicsquare])
  {
    char MagicSqList[256] = "";
    boolean magic_piece_found = false;

    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
      if (TSTFLAG(sq_spec[*bnp], MagicSq))
      {
        if (magic_piece_found)
          strcat(MagicSqList, ", ");
        else
          magic_piece_found = true;
        AppendSquare(MagicSqList,*bnp);
      }

    WriteGeneratedElement(file,"fieldframe",MagicSqList);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXBeginDiagram(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  WriteFixElement(file,"begin","diagram",0);
  WriteAuthor(file);
  WriteSource(file);
  WriteAward(file);
  WriteDedication(file);
  LaTeXWriteOptions();
  WritePieces(file);
  WriteFairyPieces(file);
  WriteStipulation(file);
  WriteGrid(file);
  WriteConditions(LaTeXFile,&WriteCondition);

  WriteSquareFrames(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void context_open(FILE *file,
                         output_plaintext_move_context_type *context,
                         move_effect_journal_index_type start,
                         char const *opening_sequence,
                         char const *closing_sequence)
{
  fprintf(file,"%s",opening_sequence);

  context->start = start;
  context->closing_sequence = closing_sequence;
}

static void context_close(FILE *file,
                          output_plaintext_move_context_type *context)
{
  fprintf(file,"%s",context->closing_sequence);
  context->start = move_effect_journal_index_null;
}

void next_context(FILE *file,
                  output_plaintext_move_context_type *context,
                  move_effect_journal_index_type start,
                  char const *opening_sequence,
                  char const *closing_sequence)
{
  context_close(file,context);
  context_open(file,context,start,opening_sequence,closing_sequence);
}

static void write_complete_piece(FILE *file,
                                 Flags spec,
                                 piece_walk_type piece,
                                 square on)
{
  WriteSpec(file,spec,piece,true);
  WritePiece(file,piece);
  WriteSquare(file,on);
}

static void write_transfer(FILE *file,
                           output_plaintext_move_context_type *context,
                           move_effect_journal_index_type removal,
                           move_effect_journal_index_type addition)
{
  next_context(file,context,removal,"[","]");

  write_complete_piece(file,
                       move_effect_journal[removal].u.piece_removal.flags,
                       move_effect_journal[removal].u.piece_removal.walk,
                       move_effect_journal[removal].u.piece_removal.on);

  fputs("{\\ra}",file);

  if (move_effect_journal[removal].u.piece_removal.flags
      !=move_effect_journal[addition].u.piece_addition.flags
      || (TSTFLAG(move_effect_journal[addition].u.piece_addition.flags,Royal)
          && is_king(move_effect_journal[removal].u.piece_removal.walk)
          && !is_king(move_effect_journal[addition].u.piece_addition.walk)))
  {
    WriteSpec(file,
              move_effect_journal[addition].u.piece_addition.flags,
              move_effect_journal[addition].u.piece_addition.walk,
              false);
    WritePiece(file,move_effect_journal[addition].u.piece_addition.walk);
  }
  else if (move_effect_journal[removal].u.piece_removal.walk
           !=move_effect_journal[addition].u.piece_addition.walk)
    WritePiece(file,move_effect_journal[addition].u.piece_addition.walk);

  WriteSquare(file,move_effect_journal[addition].u.piece_addition.on);
}

static void write_piece_creation(FILE *file,
                                 output_plaintext_move_context_type *context,
                                 move_effect_journal_index_type curr)
{
  next_context(file,context,curr,"[+","]");
  write_complete_piece(file,
                       move_effect_journal[curr].u.piece_addition.flags,
                       move_effect_journal[curr].u.piece_addition.walk,
                       move_effect_journal[curr].u.piece_addition.on);
}

static
move_effect_journal_index_type
find_piece_removal(output_plaintext_move_context_type const *context,
                   move_effect_journal_index_type curr,
                   PieceIdType id_added)
{
  move_effect_journal_index_type m;

  for (m = curr-1; m>=context->start; --m)
    if (move_effect_journal[m].type==move_effect_piece_removal
        && move_effect_journal[m].reason!=move_effect_reason_regular_capture
        && move_effect_journal[m].reason!=move_effect_reason_ep_capture
        && GetPieceId(move_effect_journal[m].u.piece_removal.flags)==id_added)
      return m;

  return move_effect_journal_index_null;
}

static void write_piece_readdition(FILE *file,
                                   output_plaintext_move_context_type *context,
                                   move_effect_journal_index_type curr)
{
  if (move_effect_journal[curr].reason==move_effect_reason_volcanic_remember)
    fputs("->v",file);
  else
  {
    PieceIdType const id_added = GetPieceId(move_effect_journal[curr].u.piece_addition.flags);
    move_effect_journal_index_type const removal = find_piece_removal(context,
                                                                      curr,
                                                                      id_added);
    if (removal==move_effect_journal_index_null)
      write_piece_creation(file,context,curr);
    else
      write_transfer(file,context,removal,curr);
  }
}

void output_latex_write_castling(FILE *file,
                                 move_effect_journal_index_type movement)
{
  if (CondFlag[castlingchess])
  {
    WritePiece(file,move_effect_journal[movement].u.piece_movement.moving);
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.from);
    fputc('-',file);
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.to);
  }
  else
  {
    square const to = move_effect_journal[movement].u.piece_movement.to;
    if (to==square_g1 || to==square_g8)
      fputs("{\\OO}",file);
    else
      fputs("{\\OOO}",file);
  }
}

static void write_exchange(FILE *file, move_effect_journal_index_type movement)
{
  WritePiece(file,get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.from));
  WriteSquare(file,move_effect_journal[movement].u.piece_exchange.to);
  fputs("{\\lra}",file);
  WritePiece(file,get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.to));
  WriteSquare(file,move_effect_journal[movement].u.piece_exchange.from);
}

static move_effect_journal_index_type find_pre_move_effect(move_effect_type type,
                                                           move_effect_reason_type reason)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type i;

  for (i = base; i!=capture; ++i)
    if (move_effect_journal[i].type==type
        && move_effect_journal[i].reason==reason)
      return i;

  return move_effect_journal_index_null;
}

static void write_departing_piece(FILE *file,
                                  move_effect_journal_index_type movement)
{
  if (WriteSpec(file,
                move_effect_journal[movement].u.piece_movement.movingspec,
                move_effect_journal[movement].u.piece_movement.moving,
                false)
      || move_effect_journal[movement].u.piece_movement.moving!=Pawn)
    WritePiece(file,move_effect_journal[movement].u.piece_movement.moving);

  WriteSquare(file,move_effect_journal[movement].u.piece_movement.from);
}

static void write_departure(FILE *file,
                            move_effect_journal_index_type movement)
{
  move_effect_journal_index_type const phantom_movement = find_pre_move_effect(move_effect_piece_movement,
                                                                               move_effect_reason_phantom_movement);

  if (phantom_movement==move_effect_journal_index_null)
    write_departing_piece(file,movement);
  else
  {
    write_departing_piece(file,phantom_movement);
    fputc('-',file);
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.from);
  }
}

static void write_capture(FILE *file,
                          output_plaintext_move_context_type *context,
                          move_effect_journal_index_type capture,
                          move_effect_journal_index_type movement)
{
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;

  write_departure(file,movement);
  fputc('*',file);
  if (sq_capture==move_effect_journal[movement].u.piece_movement.to)
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.to);
  else if (move_effect_journal[capture].reason==move_effect_reason_ep_capture)
  {
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.to);
    fprintf(file,"%s"," ep.");
  }
  else
  {
    WriteSquare(file,sq_capture);
    fputc('-',file);
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.to);
  }
}

static void write_no_capture(FILE *file,
                             output_plaintext_move_context_type *context,
                             move_effect_journal_index_type movement)
{
  write_departure(file,movement);
  fputc('-',file);
  WriteSquare(file,move_effect_journal[movement].u.piece_movement.to);
}

void output_latex_write_regular_move(FILE *file,
                                     output_plaintext_move_context_type *context)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_type const capture_type = move_effect_journal[capture].type;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  move_effect_type const movement_type = move_effect_journal[movement].type;

  assert(capture_type==move_effect_no_piece_removal
         || capture_type==move_effect_piece_removal
         || capture_type==move_effect_none);

  context_open(file,context,base,"","");

  if (capture_type==move_effect_piece_removal)
  {
    assert(move_effect_journal[movement].type==move_effect_piece_movement);
    assert(move_effect_journal[movement].reason==move_effect_reason_moving_piece_movement);
    write_capture(file,context,capture,movement);
  }
  else if (capture_type==move_effect_no_piece_removal)
  {
    assert(movement_type==move_effect_piece_movement
           || movement_type==move_effect_piece_exchange
           || movement_type==move_effect_none);
    if (movement_type==move_effect_piece_movement)
    {
      move_effect_reason_type const movement_reason = move_effect_journal[movement].reason;

      assert(movement_reason==move_effect_reason_moving_piece_movement
             || movement_reason==move_effect_reason_castling_king_movement);

      if (movement_reason==move_effect_reason_moving_piece_movement)
        write_no_capture(file,context,movement);
      else
        output_latex_write_castling(file,movement);
    }
    else if (movement_type==move_effect_piece_exchange)
    {
      assert(move_effect_journal[movement].reason==move_effect_reason_exchange_castling_exchange
             || move_effect_journal[movement].reason==move_effect_reason_messigny_exchange);
      write_exchange(file,movement);
    }
  }
}

static void write_piece_exchange(FILE *file,
                                 output_plaintext_move_context_type *context,
                                 move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_exchange_castling_exchange:
    case move_effect_reason_messigny_exchange:
      /* already dealt with */
      assert(0);
      break;

    case move_effect_reason_oscillating_kings:
      next_context(file,context,curr,"[","]");
      WritePiece(file,
                 get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.from));
      WriteSquare(file,move_effect_journal[curr].u.piece_exchange.to);
      fputs("\\lra",file);
      WritePiece(file,
                 get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.to));
      WriteSquare(file,move_effect_journal[curr].u.piece_exchange.from);
      break;

    default:
      write_exchange(file,curr);
      break;
  }
}

static Flags find_piece_walk(output_plaintext_move_context_type const *context,
                             move_effect_journal_index_type curr,
                             square on)
{
  move_effect_journal_index_type m;

  for (m = curr-1; m>=context->start; --m)
    switch (move_effect_journal[m].type)
    {
      case move_effect_piece_movement:
        if (move_effect_journal[m].u.piece_movement.to==on)
          return move_effect_journal[m].u.piece_movement.moving;
        else
          break;

      case move_effect_piece_readdition:
        if (move_effect_journal[m].u.piece_addition.on==on)
          return move_effect_journal[m].u.piece_addition.walk;
        else
          break;

      default:break;
    }

  assert(0);
  return 0;
}

static void write_flags_change(FILE *file,
                               output_plaintext_move_context_type *context,
                               move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_pawn_promotion:
      fputc('=',file);
      WriteSpec(file,
                move_effect_journal[curr].u.flags_change.to,
                find_piece_walk(context,curr,move_effect_journal[curr].u.flags_change.on),
                false);
      break;

    case move_effect_reason_kobul_king:
      if (move_effect_journal[curr-1].type!=move_effect_piece_change
          || move_effect_journal[curr-1].reason!=move_effect_reason_kobul_king)
        /* otherwise the flags are written with the changed piece */
      {
        next_context(file,context,curr,"[","]");
        WriteSquare(file,move_effect_journal[curr].u.flags_change.on);
        fputc('=',file);
        WriteSpec(file,
                  move_effect_journal[curr].u.flags_change.to,
                  being_solved.board[move_effect_journal[curr].u.flags_change.on],
                  false);
      }
      break;

    default:
      break;
  }
}

static void write_side_change(FILE *file,
                              output_plaintext_move_context_type *context,
                              move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_andernach_chess:
    case move_effect_reason_volage_side_change:
    case move_effect_reason_magic_square:
    case move_effect_reason_circe_turncoats:
      fputc('=',file);
      fputc(tolower(ColourTab[move_effect_journal[curr].u.side_change.to][0]),file);
      break;

    case move_effect_reason_magic_piece:
    case move_effect_reason_masand:
    case move_effect_reason_hurdle_colour_changing:
      next_context(file,context,curr,"[","]");
      WriteSquare(file,move_effect_journal[curr].u.side_change.on);
      fputc('=',file);
      fputc(tolower(ColourTab[move_effect_journal[curr].u.side_change.to][0]),file);
      break;

    default:
      break;
  }
}

static Flags find_piece_flags(output_plaintext_move_context_type const *context,
                              move_effect_journal_index_type curr,
                              square on)
{
  move_effect_journal_index_type m;

  for (m = curr-1; m>=context->start; --m)
    switch (move_effect_journal[m].type)
    {
      case move_effect_piece_movement:
        if (move_effect_journal[m].u.piece_movement.to==on)
          return move_effect_journal[m].u.piece_movement.movingspec;
        else
          break;

      case move_effect_piece_readdition:
        if (move_effect_journal[m].u.piece_addition.on==on)
          return move_effect_journal[m].u.piece_addition.flags;
        else
          break;

      default:break;
    }

  assert(0);
  return 0;
}

static void write_singlebox_promotion(FILE *file,
                                      move_effect_journal_index_type curr)
{
  WriteSquare(file,move_effect_journal[curr].u.piece_change.on);
  fputc('=',file);
  WritePiece(file,move_effect_journal[curr].u.piece_change.to);
}

static void write_piece_change(FILE *file,
                               output_plaintext_move_context_type *context,
                               move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_pawn_promotion:
    case move_effect_reason_protean_adjustment:
    case move_effect_reason_chameleon_movement:
    case move_effect_reason_degradierung:
    case move_effect_reason_norsk_chess:
    case move_effect_reason_promotion_of_reborn:
      if (/* regular promotion doesn't test whether the "promotion" is
           * into pawn (e.g. in SingleBox); it's more efficient to test here */
          move_effect_journal[curr].u.piece_change.to
          !=move_effect_journal[curr].u.piece_change.from)
      {
        square const on = move_effect_journal[curr].u.piece_change.on;
        Flags const flags = find_piece_flags(context,curr,on);
        fputc('=',file);
        WriteSpec(file,flags,move_effect_journal[curr].u.piece_change.to,false);
        WritePiece(file,move_effect_journal[curr].u.piece_change.to);
      }
      break;

    case move_effect_reason_singlebox_promotion:
      /* type 3 is already dealt with, so this is type 2 */
      next_context(file,context,curr,"[","]");
      write_singlebox_promotion(file,curr);
      break;

    case move_effect_reason_kobul_king:
      next_context(file,context,curr,"[","]");

      WriteSquare(file,move_effect_journal[curr].u.piece_change.on);
      fputc('=',file);

      {
        Flags flags;

        if (move_effect_journal[curr+1].type==move_effect_flags_change
            && move_effect_journal[curr+1].reason==move_effect_reason_kobul_king)
          flags = move_effect_journal[curr+1].u.flags_change.to;
        else
          flags = BIT(Royal);

        WriteSpec(file,flags,move_effect_journal[curr].u.piece_change.to,false);
      }

      WritePiece(file,move_effect_journal[curr].u.piece_change.to);
      break;

    case move_effect_reason_einstein_chess:
    case move_effect_reason_football_chess_substitution:
    case move_effect_reason_king_transmutation:
      fputc('=',file);
      WritePiece(file,move_effect_journal[curr].u.piece_change.to);
      break;

    default:
      break;
  }
}

static void write_piece_movement(FILE *file,
                                 output_plaintext_move_context_type *context,
                                 move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_moving_piece_movement:
      /* write_capture() and write_no_capture() have dealt with this */
      assert(0);
      break;

    case move_effect_reason_castling_king_movement:
      /* output_plaintext_write_castling() has dealt with this */
      assert(0);
      break;

    case move_effect_reason_castling_partner_movement:
      if (CondFlag[castlingchess])
      {
        fputc('/',file);
        write_complete_piece(file,
                             move_effect_journal[curr].u.piece_movement.movingspec,
                             move_effect_journal[curr].u.piece_movement.moving,
                             move_effect_journal[curr].u.piece_movement.from);
        fputc('-',file);
        WriteSquare(file,move_effect_journal[curr].u.piece_movement.to);
      }
      else
      {
        /* implicitly written in castling symbols */
      }
      break;

    default:
      break;
  }
}

static void write_piece_removal(FILE *file,
                                output_plaintext_move_context_type *context,
                                move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_no_reason:
    case move_effect_reason_regular_capture:
    case move_effect_reason_ep_capture:
      /* write_capture() has dealt with these */
      assert(0);
      break;

    case move_effect_reason_transfer_no_choice:
    case move_effect_reason_transfer_choice:
      /* dealt with at the end of the transfer */
      break;

    case move_effect_reason_kamikaze_capturer:
      next_context(file,context,curr,"[-","]");
      write_complete_piece(file,
                           move_effect_journal[curr].u.piece_removal.flags,
                           move_effect_journal[curr].u.piece_removal.walk,
                           move_effect_journal[curr].u.piece_removal.on);
      break;

    case move_effect_reason_assassin_circe_rebirth:
      /* no output for the removal of an assassinated piece ... */
    case move_effect_reason_pawn_promotion:
      /* ... nor for the removal of a pawn promoted to imitator ... */
    case move_effect_reason_volcanic_remember:
      break;

    default:
      assert(0);
      break;
  }
}

static void write_imitator_addition(FILE *file,
                                    output_plaintext_move_context_type *context)
{
  fprintf(file,"=I");
}

static void write_imitator_movement(FILE *file,
                                    output_plaintext_move_context_type *context,
                                    move_effect_journal_index_type curr)
{
  unsigned int const nr_moved = move_effect_journal[curr].u.imitator_movement.nr_moved;
  unsigned int icount;

  fprintf(file,"[I");

  for (icount = 0; icount<nr_moved; ++icount)
  {
    WriteSquare(file,being_solved.isquare[icount]);
    if (icount+1<nr_moved)
      fputc(',',file);
  }

  fputc(']',file);
}

static void write_half_neutral_deneutralisation(FILE *file,
                                                output_plaintext_move_context_type *context,
                                                move_effect_journal_index_type curr)
{
  fputc('=',file);
  fputc(tolower(ColourTab[move_effect_journal[curr].u.half_neutral_phase_change.side][0]),file);
  fputc('h',file);
}

static void write_half_neutral_neutralisation(FILE *file,
                                              output_plaintext_move_context_type *context,
                                              move_effect_journal_index_type curr)
{
  fprintf(file,"%s","=nh");
}

static void write_bgl_status(FILE *file,
                             output_plaintext_move_context_type *context,
                             move_effect_journal_index_type curr)
{
  char buf[12];

  if (BGL_global)
  {
    if (move_effect_journal[curr].u.bgl_adjustment.side==White)
    {
      next_context(file,context,curr," (",")");
      WriteBGLNumber(buf,BGL_values[White]);
      fprintf(file,"%s",buf);
    }
  }
  else
  {
    next_context(file,context,curr," (",")");
    WriteBGLNumber(buf,BGL_values[White]);
    fprintf(file,"%s",buf);
    fputc('/',file);
    WriteBGLNumber(buf,BGL_values[Black]);
    fprintf(file,"%s",buf);
  }
}

static void write_other_effects(FILE *file,
                                output_plaintext_move_context_type *context)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr = move_effect_journal_base[nbply];

  for (curr += move_effect_journal_index_offset_other_effects; curr!=top; ++curr)
  {
    switch (move_effect_journal[curr].type)
    {
      case move_effect_flags_change:
        write_flags_change(file,context,curr);
        break;

      case move_effect_side_change:
        write_side_change(file,context,curr);
        break;

      case move_effect_piece_change:
        write_piece_change(file,context,curr);
        break;

      case move_effect_piece_movement:
        write_piece_movement(file,context,curr);
        break;

      case move_effect_piece_creation:
        write_piece_creation(file,context,curr);
        break;

      case move_effect_piece_readdition:
        write_piece_readdition(file,context,curr);
        break;

      case move_effect_piece_removal:
        write_piece_removal(file,context,curr);
        break;

      case move_effect_piece_exchange:
        write_piece_exchange(file,context,curr);
        break;

      case move_effect_imitator_addition:
        write_imitator_addition(file,context);
        break;

      case move_effect_imitator_movement:
        write_imitator_movement(file,context,curr);
        break;

      case move_effect_half_neutral_deneutralisation:
        write_half_neutral_deneutralisation(file,context,curr);
        break;

      case move_effect_half_neutral_neutralisation:
        write_half_neutral_neutralisation(file,context,curr);
        break;

      case move_effect_bgl_adjustment:
        write_bgl_status(file,context,curr);
        break;

      default:
        break;
    }
  }
}

static void write_singlebox_type3_promotion(FILE *file)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const sb3_prom = find_pre_move_effect(move_effect_piece_change,
                                                                       move_effect_reason_singlebox_promotion);

  if (sb3_prom!=move_effect_journal_index_null)
  {
    output_plaintext_move_context_type context;
    context_open(file,&context,base,"[","]");
    write_singlebox_promotion(file,sb3_prom);
    context_close(file,&context);
  }
}

void output_latex_write_move(FILE *file)
{
  output_plaintext_move_context_type context;

  if (CondFlag[singlebox] && SingleBoxType==ConditionType3)
    write_singlebox_type3_promotion(file);

  output_latex_write_regular_move(file,&context);
  write_other_effects(file,&context);
  context_close(file,&context);
}

static void visit_output_mode_selector(slice_index si, stip_structure_traversal *st)
{
  FILE * const file = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.output_mode_selector.mode==output_mode_line)
    solving_insert_output_latex_line_slices(si,file);
  else
  {
    boolean const is_setplay = st->level==structure_traversal_level_setplay;
    solving_insert_output_latex_tree_slices(si,is_setplay,file);
  }

  {
    slice_index const prototypes[] =
    {
        alloc_output_latex_twinning_writer(file)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with slices that write the solution in
 * LaTeX
 */
void output_latex_instrument_solving(slice_index si, FILE *file)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,file);
  stip_structure_traversal_override_single(&st,
                                           STOutputModeSelector,
                                           &visit_output_mode_selector);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
