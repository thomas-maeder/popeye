#include "output/latex/latex.h"
#include "output/latex/twinning.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
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
#include "conditions/grid.h"
#include "platform/maxtime.h"

#include "debugging/assert.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

boolean LaTeXout;

static char filename[LINESIZE];    /* This array contains the input as is */

FILE *LaTeXFile;
FILE *TextualSolutionBuffer;

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

void LaTeXStr(char const *line)
{
  while (*line) {
    switch (*line) {
    case '#':
      fprintf(LaTeXFile, "\\%c", *line);
      break;
    case '&':
      fprintf(LaTeXFile, "\\%c", *line);
      break;
    case '%':
      if (*(line+1) == '%') {
        /* it's introducing a comment */
        fprintf(LaTeXFile, "%%");
        line++;
      }
      else {
        fprintf(LaTeXFile, "\\%%");
      }
      break;
    case '0':
      if (strncmp(line, "0-0-0", 5) == 0) {
        fprintf(LaTeXFile, "{\\OOO}");
        line += 4;
      }
      else if (strncmp(line, "0-0", 3) == 0) {
        fprintf(LaTeXFile, "{\\OO}");
        line += 2;
      }
      else {
        fprintf(LaTeXFile, "%c", *line);
      }
      break;
    case '-':
      if (*(line+1) == '>') {   /* convert -> to \ra   FCO */
        fprintf(LaTeXFile, "{\\ra}");
        line++;
      } else {  /* ordinary minus */
        fprintf(LaTeXFile, "%c", *line);
      }
      break;
    case '<':
      if (*(line+1)=='-' && *(line+2)=='-' && *(line+3)=='>')   /* convert -> to \lra  */
      {
        fprintf(LaTeXFile, "{\\lra}");
        line += 3;
      } else {  /* ordinary less than */
        fprintf(LaTeXFile, "%c", *line);
      }
      break;

    default:
      fprintf(LaTeXFile, "%c", *line);
      fflush(LaTeXFile);         /* non-buffered output  FCO */
      break;
    }
    line++;
  }
}

static void WriteIntro(void)
{
  fprintf(LaTeXFile, "\\documentclass{article}%%");
  if (!flag_regression)
    fprintf(LaTeXFile, "%s", versionString);
  fprintf(LaTeXFile, "\n");
  fprintf(LaTeXFile, "\\usepackage{diagram}\n");
  if (UserLanguage == German) {
    fprintf(LaTeXFile, "\\usepackage{german}\n");
  }
  fprintf(LaTeXFile, "\n\\begin{document}\n\n");
}

boolean LaTeXSetup(void)
{
  LaTeXFile = fopen(InputLine,"w");
  if (LaTeXFile==NULL)
  {
    IoErrorMsg(WrOpenError,0);
    return false;
  }
  else
  {
    strcpy(filename,InputLine);
    WriteIntro();
    fclose(LaTeXFile);
    LaTeXFile = NULL;
    return true;
  }
}

void LaTeXShutdown(void)
{
  LaTeXFile = fopen(filename,"a");

  if (LaTeXFile!=0)
  {
    fprintf(LaTeXFile, "\n\\putsol\n\n\\end{document}\n");
    fclose(LaTeXFile);
    LaTeXFile = NULL;
  }
}

static boolean Open(void)
{
  assert(LaTeXFile==NULL);

  if (filename[0]==0)
    return false;
  else
  {
    LaTeXFile = fopen(filename,"a");
    if (LaTeXFile==NULL)
    {
      IoErrorMsg(WrOpenError,0);
      return false;
    }
    else
      return true;
  }
}

static void Close(void)
{
  assert(LaTeXFile!=NULL);
  fclose(LaTeXFile);
  LaTeXFile = NULL;
}

void LaTexOpenSolution(void)
{
  if (LaTeXFile!=0)
  {
    fprintf(LaTeXFile, " \\solution{%%\n");

    assert(TextualSolutionBuffer==NULL);
    TextualSolutionBuffer = tmpfile();
  }
}

void LaTexCloseSolutionBuffer(void)
{
  if (TextualSolutionBuffer!=NULL)
  {
    fclose(TextualSolutionBuffer);
    TextualSolutionBuffer = NULL;
  }
}

void LaTeXFlushSolution(void)
{
  if (LaTeXFile!=0)
  {
    char line[256];

    /* solution */
    rewind(TextualSolutionBuffer);
    while (fgets(line, (sizeof line)-1, TextualSolutionBuffer))
    {
      if (strlen(line)>1 && line[1]==')')
        /* twin */
        fprintf(LaTeXFile, "%c)", line[0]);
      else if (strlen(line)>2 && line[2]==')')
      {
        if (line[0] == '+')        /* twin (continued) */
          fprintf(LaTeXFile, "%c)", line[1]);
        else
          fprintf(LaTeXFile, "%c%c)", line[0], line[1]);
      }
      else if (strlen(line)>3 && line[3]==')')
        /* continued twinning and >z */
        fprintf(LaTeXFile, "%c%c)", line[1], line[2]);

      if (strchr(line, '.'))   /* line containing a move */
        LaTeXStr(line);
    }

    fprintf(LaTeXFile, "\n }%%\n");

    fclose(TextualSolutionBuffer);
    TextualSolutionBuffer = NULL;
  }
}

void LaTeXEndDiagram(void)
{
  if (LaTeXFile!=0)
  {
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
  }
}

static void WriteCondition(char const CondLine[], boolean is_first)
{
  if (is_first)
    fprintf(LaTeXFile, " \\condition{");
  else
    fprintf(LaTeXFile, "{\\newline}\n   ");
  LaTeXStr(CondLine);
}

static void WriteAuthor(void)
{
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

      sprintf(GlobalStr, " \\author{%s}%%%%\n", TeXAuthor);
      LaTeXStr(GlobalStr);

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
        LaTeXStr(GlobalStr);
        *cp3= *cp2= '\n';

        cp1= cp2+1;
      }
      fprintf(LaTeXFile, "}%%\n");
    }
  }
}

static void WriteSource(void)
{
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
    sprintf(GlobalStr,
            " \\source{%.*s}%%%%\n", (int)(date-source), source);
    LaTeXStr(GlobalStr);

    *eol= '\n';
  }
}

static void WriteAward(void)
{
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
}

static void WriteDedication(void)
{
  if (ActTitle[0] != '\0')
  {
    sprintf(GlobalStr, "\\dedication{%s}%%%%\n", ActTitle);
    LaTeXStr(GlobalStr);
  }
}

static void WritePieces(void)
{
  square const *bnp;
  boolean piece_found = false;

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
}

static boolean FindFairyWalks(boolean side_has_piece[nr_piece_walks][nr_sides+1])
{
  boolean result = false;

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

  return result;
}

static boolean FindPiecesWithSpecs(unsigned int SpecCount[nr_piece_flags-nr_sides],
                                   char ListSpec[nr_piece_flags-nr_sides][256])
{
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
        return true;

    return false;
  }
}

static boolean FindHoles(char HolesSqList[256])
{
  boolean result = false;

  square const *bnp;
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

  return result;
}

static void WriteFairyWalks(boolean side_has_walk[nr_piece_walks][nr_sides+1])
{
  boolean fairy_walk_written = false;

  piece_walk_type p;
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
}

static void WritePiecesWithSpecs(boolean remark_written,
                                 unsigned int const SpecCount[nr_piece_flags-nr_sides],
                                 char ListSpec[nr_piece_flags-nr_sides][256])
{
  piece_flag_type sp;
  for (sp = nr_sides; sp<nr_piece_flags; ++sp)
    if (SpecCount[sp-nr_sides]>0)
    {
      if (remark_written)
        fprintf(LaTeXFile, "{\\newline}\n    ");
      fprintf(LaTeXFile, "%s\n", ListSpec[sp-nr_sides]);
      remark_written = true;
    }
}

static void WriteHoles(boolean remark_written,
                       char const HolesSqList[256])
{
  if (remark_written)
    fprintf(LaTeXFile,"{\\newline}\n    ");
  fprintf(LaTeXFile,"%s %s%%\n",CondString[UserLanguage][holes],HolesSqList);
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
}

static void WriteStipulation(void)
{
  fprintf(LaTeXFile, " \\stipulation{");
  LaTeXStr(ActStip);
  if (OptFlag[solapparent])
    fprintf(LaTeXFile, "*");
  if (OptFlag[whitetoplay])
    fprintf(LaTeXFile," %c{\\ra}", tolower(*PieSpString[UserLanguage][White]));
  fprintf(LaTeXFile, "}%%\n");
}

static void WriteGridOrthogonalLines(void)
{
  unsigned int i;
  boolean line_written = false;

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
}

static void WriteGridIrregular(void)
{
  boolean line_written = false;

  square const *bnp;
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
}

static void WriteGrid(void)
{
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
}

void WriteSquareFrames(void)
{
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
}

void LaTeXBeginDiagram(void)
{
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

    if (WriteConditions(&WriteCondition))
      fprintf(LaTeXFile, "}%%\n");

    WriteSquareFrames();
  }
}
