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
#include "conditions/grid.h"
#include "conditions/singlebox/type1.h"
#include "platform/maxtime.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

boolean LaTeXout;

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
        fprintf(file,"%s", "{\\ra}");
        ++line;
      }
      else   /* ordinary minus */
        fprintf(file, "%c", *line);
      break;

    case '<':
      if (*(line+1)=='-' && *(line+2)=='-' && *(line+3)=='>')   /* convert -> to \lra  */
      {
        fprintf(file,"%s", "{\\lra}");
        line += 3;
      }
      else  /* ordinary less than */
        fprintf(file, "%c", *line);
      break;

    default:
      fprintf(file, "%c", *line);
      fflush(file);         /* non-buffered output  FCO */
      break;
    }

    ++line;
  }
}

static void WriteIntro(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  fprintf(LaTeXFile, "\\documentclass{article}%%");
  if (!flag_regression)
    fprintf(LaTeXFile, "%s", versionString);
  fprintf(LaTeXFile, "\n");
  fprintf(LaTeXFile, "\\usepackage{diagram}\n");
  if (UserLanguage == German) {
    fprintf(LaTeXFile, "\\usepackage{german}\n");
  }
  fprintf(LaTeXFile, "\n\\begin{document}\n\n");

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
    WriteIntro();
    fclose(LaTeXFile);
    LaTeXFile = NULL;
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

  LaTeXFile = fopen(filename,"a");

  if (LaTeXFile!=0)
  {
    fprintf(LaTeXFile, "\n\\putsol\n\n\\end{document}\n");
    fclose(LaTeXFile);
    LaTeXFile = NULL;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean Open(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(LaTeXFile==NULL);

  if (filename[0]==0)
    result = false;
  else
  {
    LaTeXFile = fopen(filename,"a");
    if (LaTeXFile==NULL)
    {
      IoErrorMsg(WrOpenError,0);
      result = false;
    }
    else
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void Close(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(LaTeXFile!=NULL);

  fclose(LaTeXFile);
  LaTeXFile = NULL;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTexOpenSolution(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(LaTeXFile!=NULL);

  fprintf(LaTeXFile, " \\solution{%%\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTexCloseSolution(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(LaTeXFile!=NULL);

  fprintf(LaTeXFile, " }%%\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXEndDiagram(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(LaTeXFile!=NULL);

  if (!(OptFlag[solmenaces]
        || OptFlag[solflights]
        || OptFlag[nontrivial]
        || max_solutions_reached()
        || was_max_nr_solutions_per_target_position_reached()
        || has_short_solution_been_found_in_problem()
        || hasMaxtimeElapsed()))
  {
    fprintf(LaTeXFile, " \\Co+%%");
    if (!flag_regression)
      fprintf(LaTeXFile, "%s", versionString);
    fprintf(LaTeXFile, "\n");
  }

  fprintf(LaTeXFile, "\\end{diagram}\n\\hfill\n");

  Close();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteCondition(FILE *file, char const CondLine[], boolean is_first)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (is_first)
    fprintf(file,"%s", " \\condition{");
  else
    fprintf(file,"%s", "{\\newline}\n   ");
  LaTeXStr(file,CondLine);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteAuthor(void)
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

      sprintf(GlobalStr," \\author{%s}", TeXAuthor);
      LaTeXStr(LaTeXFile,GlobalStr);
      fprintf(LaTeXFile,"%s","%\n");

      if (endcp!=0)
        *endcp = '\n';
      while ((cp=strchr(TeXAuthor, ';')))
        *cp= '\n';
    }
    else
    {
      /* reverse first and surnames */
      char *cp1, *cp2, *cp3;

      fprintf(LaTeXFile, " \\author{");
      cp1= TeXAuthor;
      while ((cp2=strchr(cp1, '\n'))) {
        *cp2= '\0';
        if (cp1 != TeXAuthor)
          fprintf(LaTeXFile, "; ");
        cp3= cp2;
        while (cp3 > cp1 && *cp3 != ' ')
          cp3--;
        /* wrong LaTeX output if the authors surname only given */
        if (cp3 == cp1) {
          /* we got only the surname ! */
          sprintf(GlobalStr, "%s, ", cp3);
        } else {
          /* we got firstname and surname */
          *cp3= '\0';
          sprintf(GlobalStr, "%s, %s", cp3+1, cp1);
        }
        LaTeXStr(LaTeXFile,GlobalStr);
        *cp3= *cp2= '\n';

        cp1= cp2+1;
      }
      fprintf(LaTeXFile,"%s","}%\n");
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteSource(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* format: [diagram number,] source [issue number,] [date] */
  if (ActOrigin[0] != '\0') {
    char *source= ActOrigin;
    char *date, *eol, *tmp;

    /* diagram number */
    while (strchr(CharChar, *source))
      source++;

    if (*source == ',') {
      *source= '\0';
      fprintf(LaTeXFile, " \\sourcenr{%s}%%\n", ActOrigin);
      *source= ',';
      while (*(++source) == ' ')
        ;
    }
    else {
      source= ActOrigin;
    }

    /* date */
    /* supported formats: year
    **            month/year
    **            month-month/year
    **            day. month. year
    **            day.-day. month. year
    */
    /* year */
    eol= date= strchr(source, '\n');
    *eol= '\0';

    while (strchr("0123456789-", *(date-1))) {
      date--;
    }

    if (date != eol) {
      /* sucessfully parsed a year */
      fprintf(LaTeXFile, " \\year{%s}%%\n", date);

      /* parse month(s) now */
      /* while (*(date-1) == ' ') date--; */
      switch (*(date-1)) {
      case '/':
        /* format is either month/year or month-month/year */
        date--;
        while (*(date-1) == ' ')
          date--;
        tmp= date;
        while (strchr("0123456789-", *(date-1)))
          date--;
        fprintf(LaTeXFile, " \\month%s{%.*s}%%\n",
                strchr(date, '-') ? "s" : "", (int)(tmp-date), date);
        break;

      case '.':
        /* format is either
           day. month. year or day.-day. month. year
        */
        date--;
        tmp= date;
        while (strchr("0123456789", *(date-1)))
          date--;
        fprintf(LaTeXFile, " \\month{%.*s}%%\n",
                (int)(tmp-date), date);
        /* now parse day(s) */
        while (*(--date) == ' ');
        tmp= date;
        while (strchr("0123456789-.", *(date-1)))
          date--;
        fprintf(LaTeXFile, " \\day{%.*s}%%\n",
                (int)(tmp-date), date);
        break;
      }
    } /* month(s), day(s) */

    /* issue number */
    while (*(date-1) == ' ')
      date--;
    if (*(date-1) == ',') {
      /* issue number found */
      tmp= --date;
      while (*(date-1) != ' ')
        date--;
      fprintf(LaTeXFile, " \\issue{%.*s}%%\n",
              (int)(tmp-date), date);
    } /* issue */

    /* default */
    /* source name or complete source if not interpretable */
    while (*(date-1) == ' ')
      date--;
    sprintf(GlobalStr," \\source{%.*s}",(int)(date-source),source);
    LaTeXStr(LaTeXFile,GlobalStr);
    fprintf(LaTeXFile,"%s","%\n");

    *eol= '\n';
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteAward(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (ActAward[0] != '\0')
  {
    char *tour= strchr(ActAward, ',');
    char *eol= strchr(ActAward, '\n');
    *eol= '\0';
    if (tour)
    {
      fprintf(LaTeXFile,
              " \\award{%.*s}%%\n", (int)(tour-ActAward), ActAward);
      while (*(++tour) == ' ');
      fprintf(LaTeXFile, " \\tournament{%s}%%\n", tour);
    } else
      fprintf(LaTeXFile, " \\award{%s}%%\n", ActAward);
    *eol= '\n';
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteDedication(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (ActTitle[0] != '\0')
  {
    sprintf(GlobalStr, "\\dedication{%s}", ActTitle);
    LaTeXStr(LaTeXFile,GlobalStr);
    fprintf(LaTeXFile,"%s","%\n");
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WritePieces(void)
{
  square const *bnp;
  boolean piece_found = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  fprintf(LaTeXFile, " \\pieces{");

  for (bnp = boardnum; *bnp; bnp++)
  {
    if (!is_square_blocked(*bnp) && !is_square_empty(*bnp))
    {
      piece_walk_type const p = get_walk_of_piece_on_square(*bnp);

      if (piece_found)
        fprintf(LaTeXFile, ", ");
      else
        piece_found = true;

      fprintf(LaTeXFile,"%c%s%c%c",
              is_piece_neutral(being_solved.spec[*bnp]) ? 'n' : TSTFLAG(being_solved.spec[*bnp],White) ? 'w' : 's',
              LaTeXWalk(p),
              *bnp%onerow-200%onerow+'a',
              *bnp/onerow-200/onerow+'1');
    }
  }

  fprintf(LaTeXFile, "}%%\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean FindFairyWalks(boolean side_has_piece[nr_piece_walks][nr_sides+1])
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    piece_walk_type p;
    for (p = Empty; p < nr_piece_walks; ++p)
    {
      side_has_piece[p][White] = false;
      side_has_piece[p][Black] = false;
      side_has_piece[p][nr_sides] = false;
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
            side_has_piece[p][nr_sides] = true;
          else if (TSTFLAG(being_solved.spec[*bnp], White))
            side_has_piece[p][White] = true;
          else
            side_has_piece[p][Black] = true;
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
                                   char ListSpec[nr_piece_flags-nr_sides][256])
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

static boolean FindHoles(char HolesSqList[256])
{
  boolean result = false;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
  {
    if (is_square_blocked(*bnp))
    {
      if (result)
        strcat(HolesSqList, ", ");
      else
        result = true;
      AppendSquare(HolesSqList,*bnp);
    }
  }

  if (result)
  {
    fprintf(LaTeXFile," \\nofields{%s}%%\n",HolesSqList);
    fprintf(LaTeXFile," \\fieldframe{%s}%%\n",HolesSqList);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void WriteFairyWalks(boolean side_has_walk[nr_piece_walks][nr_sides+1])
{
  boolean fairy_walk_written = false;
  piece_walk_type p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (p = Bishop+1; p<nr_piece_walks; ++p)
    if (side_has_walk[p][White] || side_has_walk[p][Black] || side_has_walk[p][nr_sides])
    {
      piece_walk_type q;
      for (q = Bishop+1; q<p; q++)
        if ((side_has_walk[q][White]
             || side_has_walk[q][Black]
             || side_has_walk[q][nr_sides])
            && LaTeXPiecesAbbr[p][0]==LaTeXPiecesAbbr[q][0]
            && LaTeXPiecesAbbr[p][1]==LaTeXPiecesAbbr[q][1])
          fprintf(stderr,
                  "+++ Warning: double representation '%s' for %s and %s\n",
                  LaTeXPiecesAbbr[q],
                  LaTeXPiecesFull[p],
                  LaTeXPiecesFull[q]);

      if (fairy_walk_written)
        fprintf(LaTeXFile, "{\\newline}\n    ");
      fprintf(LaTeXFile, "\\mbox{");
      if (side_has_walk[p][White])
        fprintf(LaTeXFile, "\\w%s ",LaTeXPiecesAbbr[p]);
      if (side_has_walk[p][ Black])
        fprintf(LaTeXFile, "\\s%s ",LaTeXPiecesAbbr[p]);
      if (side_has_walk[p][nr_sides])
        fprintf(LaTeXFile, "\\n%s ",LaTeXPiecesAbbr[p]);
      fprintf(LaTeXFile, "=%s}", LaTeXPiecesFull[p]);
      fairy_walk_written = true;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WritePiecesWithSpecs(boolean remark_written,
                                 unsigned int const SpecCount[nr_piece_flags-nr_sides],
                                 char ListSpec[nr_piece_flags-nr_sides][256])
{
  piece_flag_type sp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (sp = nr_sides; sp<nr_piece_flags; ++sp)
    if (SpecCount[sp-nr_sides]>0)
    {
      if (remark_written)
        fprintf(LaTeXFile, "{\\newline}\n    ");
      fprintf(LaTeXFile, "%s\n", ListSpec[sp-nr_sides]);
      remark_written = true;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteHoles(boolean remark_written,
                       char const HolesSqList[256])
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (remark_written)
    fprintf(LaTeXFile,"{\\newline}\n    ");
  fprintf(LaTeXFile,"%s %s%%\n",CondString[UserLanguage][holes],HolesSqList);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteFairyPieces(void)
{
  boolean side_has_walk[nr_piece_walks][nr_sides+1];
  boolean const fairy_walk_found = FindFairyWalks(side_has_walk);

  unsigned int SpecCount[nr_piece_flags-nr_sides] = { 0 };
  char ListSpec[nr_piece_flags-nr_sides][256];
  boolean const piece_with_specs_found = FindPiecesWithSpecs(SpecCount,ListSpec);

  char HolesSqList[256] = "";
  boolean const hole_found = FindHoles(HolesSqList);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (fairy_walk_found || hole_found || piece_with_specs_found)
  {
    fprintf(LaTeXFile," \\remark{");

    if (fairy_walk_found)
      WriteFairyWalks(side_has_walk);

    if (piece_with_specs_found)
      WritePiecesWithSpecs(fairy_walk_found,SpecCount,ListSpec);

    if (hole_found)
      WriteHoles(fairy_walk_found || piece_with_specs_found, HolesSqList);

    fprintf(LaTeXFile,"}%%\n");
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteStipulation(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  fprintf(LaTeXFile, " \\stipulation{");
  LaTeXStr(LaTeXFile,ActStip);
  if (OptFlag[solapparent])
    fprintf(LaTeXFile, "*");
  if (OptFlag[whitetoplay])
    fprintf(LaTeXFile," %c{\\ra}", tolower(*PieSpString[UserLanguage][White]));
  fprintf(LaTeXFile,"%s","}%\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteGridOrthogonalLines(void)
{
  unsigned int i;
  boolean line_written = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 1; i<nr_files_on_board; i++)
    if (GridNum(square_a1+dir_right*(i-1))!=GridNum(square_a1+dir_right*i))
    {
      if (line_written)
        fprintf(LaTeXFile, ", ");
      else
      {
        fprintf(LaTeXFile, " \\gridlines{");
        line_written = true;
      }
      fprintf(LaTeXFile, " v%u08", i);
    }

  for (i = 1; i<nr_rows_on_board; i++)
    if (GridNum(square_a1+dir_up*(i-1))!=GridNum(square_a1+dir_up*i))
    {
      if (line_written)
        fprintf(LaTeXFile, ", ");
      else
      {
        fprintf(LaTeXFile, " \\gridlines{");
        line_written = true;
      }
      fprintf(LaTeXFile, " h0%u8", i);
    }

  if (line_written)
    fprintf(LaTeXFile, "}%%\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteGridIrregular(void)
{
  boolean line_written = false;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; bnp++)
  {
    unsigned int const file = *bnp%onerow - nr_of_slack_files_left_of_board;
    unsigned int const row = *bnp/onerow - nr_of_slack_rows_below_board;

    if (file>0 && GridLegal(*bnp+dir_left,*bnp))
    {
      if (line_written)
        fprintf(LaTeXFile, ", ");
      else
      {
        fprintf(LaTeXFile, " \\gridlines{");
        line_written = true;
      }
      fprintf(LaTeXFile," v%d%d1",file,row);
    }

    if (row>0 && GridLegal(*bnp+dir_down,*bnp))
    {
      if (line_written)
        fprintf(LaTeXFile, ", ");
      else
      {
        fprintf(LaTeXFile, " \\gridlines{");
        line_written = true;
      }
      fprintf(LaTeXFile," h%d%d1",file,row);
    }
  }

  if (line_written)
    fprintf(LaTeXFile, "}%%\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void WriteGrid(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (CondFlag[gridchess] && !OptFlag[suppressgrid])
    switch (grid_type)
    {
      case grid_normal:
        fprintf(LaTeXFile," \\stdgrid%%\n");
        break;

      case grid_vertical_shift:
        fprintf(LaTeXFile," \\gridlines{h018, h038, h058, h078, v208, v408, v608}%%\n");
        break;

      case grid_horizontal_shift:
        fprintf(LaTeXFile, " \\gridlines{h028, h048, h068, v108, v308, v508, v708}%%\n");
        break;

      case grid_diagonal_shift:
        fprintf(LaTeXFile," \\gridlines{h018, h038, h058, h078, v108, v308, v508, v708}%%\n");
        break;

      case grid_orthogonal_lines:
        WriteGridOrthogonalLines();
        break;

      /* of course, WriteGridIrregular() could cover the above cases as well */
      case grid_irregular:
        WriteGridIrregular();
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void WriteSquareFrames(void)
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

    fprintf(LaTeXFile," \\fieldframe{%s}%%\n", MagicSqList);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXBeginDiagram(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (Open())
  {
    fprintf(LaTeXFile, "\\begin{diagram}%%\n");

    WriteAuthor();
    WriteSource();
    WriteAward();
    WriteDedication();
    LaTeXWriteOptions();
    WritePieces();
    WriteFairyPieces();
    WriteStipulation();
    WriteGrid();

    if (WriteConditions(LaTeXFile,&WriteCondition))
      fprintf(LaTeXFile,"}%%\n");

    WriteSquareFrames();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void write_transfer(output_plaintext_move_context_type *context,
                           move_effect_journal_index_type removal,
                           move_effect_journal_index_type addition)
{
  output_plaintext_next_context(context,removal,"[","]");

  output_plaintext_write_complete_piece(context->file,
                                        move_effect_journal[removal].u.piece_removal.flags,
                                        move_effect_journal[removal].u.piece_removal.walk,
                                        move_effect_journal[removal].u.piece_removal.on);

  fprintf(context->file,"{\\ra}");

  if (move_effect_journal[removal].u.piece_removal.flags
      !=move_effect_journal[addition].u.piece_addition.flags
      || (TSTFLAG(move_effect_journal[addition].u.piece_addition.flags,Royal)
          && is_king(move_effect_journal[removal].u.piece_removal.walk)
          && !is_king(move_effect_journal[addition].u.piece_addition.walk)))
  {
    WriteSpec(context->file,
              move_effect_journal[addition].u.piece_addition.flags,
              move_effect_journal[addition].u.piece_addition.walk,
              false);
    WritePiece(context->file,move_effect_journal[addition].u.piece_addition.walk);
  }
  else if (move_effect_journal[removal].u.piece_removal.walk
           !=move_effect_journal[addition].u.piece_addition.walk)
    WritePiece(context->file,move_effect_journal[addition].u.piece_addition.walk);

  WriteSquare(context->file,move_effect_journal[addition].u.piece_addition.on);
}

void output_latex_write_piece_readdition(output_plaintext_move_context_type *context,
                                         move_effect_journal_index_type curr)
{
  if (move_effect_journal[curr].reason==move_effect_reason_volcanic_remember)
    fprintf(context->file,"->v");
  else
  {
    PieceIdType const id_added = GetPieceId(move_effect_journal[curr].u.piece_addition.flags);
    move_effect_journal_index_type const removal = output_plaintext_find_piece_removal(context,
                                                                                       curr,
                                                                                       id_added);
    if (removal==move_effect_journal_index_null)
      output_plaintext_write_piece_creation(context,curr);
    else
      write_transfer(context,removal,curr);
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
      fprintf(file,"%s","{\\OO}");
    else
      fprintf(file,"%s","{\\OOO}");
  }
}

static void write_exchange(FILE *file, move_effect_journal_index_type movement)
{
  WritePiece(file,get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.from));
  WriteSquare(file,move_effect_journal[movement].u.piece_exchange.to);
  fprintf(file,"%s","{\\lra}");
  WritePiece(file,get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.to));
  WriteSquare(file,move_effect_journal[movement].u.piece_exchange.from);
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

  output_plaintext_context_open(file,context,base,"","");

  if (capture_type==move_effect_piece_removal)
  {
    assert(move_effect_journal[movement].type==move_effect_piece_movement);
    assert(move_effect_journal[movement].reason==move_effect_reason_moving_piece_movement);
    output_plaintext_write_capture(file,context,capture,movement);
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
        output_plaintext_write_no_capture(file,context,movement);
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

void output_latex_write_piece_exchange(output_plaintext_move_context_type *context,
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
      output_plaintext_next_context(context,curr,"[","]");
      WritePiece(context->file,
                 get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.from));
      WriteSquare(context->file,move_effect_journal[curr].u.piece_exchange.to);
      fprintf(context->file,"\\lra");
      WritePiece(context->file,
                 get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.to));
      WriteSquare(context->file,move_effect_journal[curr].u.piece_exchange.from);
      break;

    default:
      write_exchange(context->file,curr);
      break;
  }
}

static void write_other_effects(FILE *FILE, output_plaintext_move_context_type *context)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr = move_effect_journal_base[nbply];

  for (curr += move_effect_journal_index_offset_other_effects; curr!=top; ++curr)
  {
    switch (move_effect_journal[curr].type)
    {
      case move_effect_flags_change:
        output_plaintext_write_flags_change(context,curr);
        break;

      case move_effect_side_change:
        output_plaintext_write_side_change(context,curr);
        break;

      case move_effect_piece_change:
        output_plaintext_write_piece_change(context,curr);
        break;

      case move_effect_piece_movement:
        output_plaintext_write_piece_movement(context,curr);
        break;

      case move_effect_piece_creation:
        output_plaintext_write_piece_creation(context,curr);
        break;

      case move_effect_piece_readdition:
        output_latex_write_piece_readdition(context,curr);
        break;

      case move_effect_piece_removal:
        output_plaintext_write_piece_removal(context,curr);
        break;

      case move_effect_piece_exchange:
        output_latex_write_piece_exchange(context,curr);
        break;

      case move_effect_imitator_addition:
        output_plaintext_write_imitator_addition(context);
        break;

      case move_effect_imitator_movement:
        output_plaintext_write_imitator_movement(context,curr);
        break;

      case move_effect_half_neutral_deneutralisation:
        output_plaintext_write_half_neutral_deneutralisation(context,curr);
        break;

      case move_effect_half_neutral_neutralisation:
        output_plaintext_write_half_neutral_neutralisation(context,curr);
        break;

      case move_effect_bgl_adjustment:
        output_plaintext_write_bgl_status(context,curr);
        break;

      default:
        break;
    }
  }
}

void output_latex_write_move(void)
{
  output_plaintext_move_context_type context;

  if (CondFlag[singlebox] && SingleBoxType==ConditionType3)
    output_plaintext_write_singlebox_type3_promotion(LaTeXFile);

  output_latex_write_regular_move(LaTeXFile,&context);
  write_other_effects(LaTeXFile,&context);
  output_plaintext_context_close(&context);
}

static void visit_output_mode_selector(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.output_mode_selector.mode==output_mode_line)
    solving_insert_output_latex_line_slices(si);
  else
  {
    boolean const is_setplay = st->level==structure_traversal_level_setplay;
    solving_insert_output_latex_tree_slices(si,is_setplay);
  }

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STOutputLaTeXTwinningWriter)
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
void output_latex_instrument_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STOutputModeSelector,
                                           &visit_output_mode_selector);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
