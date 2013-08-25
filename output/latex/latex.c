#include "output/latex/latex.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/pieces.h"
#include "input/plaintext/token.h"
#include "input/plaintext/line.h"
#include "options/options.h"
#include "options/maxsolutions/maxsolutions.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "pieces/attributes/neutral/neutral.h"
#include "conditions/grid.h"
#include "platform/maxtime.h"
#include "pylang.h"
#include "pymsg.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

boolean LaTeXout;

char ActTwinning[1532];

static FILE *LaTeXFile;

static    char *LaTeXPiecesAbbr[PieceCount];
static    char *LaTeXPiecesFull[PieceCount];
char *LaTeXStdPie[8] = { NULL, "C", "K", "B", "D", "S", "T", "L"};

static char const CharChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char *LaTeXPiece(PieNam Name)
{
  if (Name > Bishop) {
    if (LaTeXPiecesAbbr[Name] == NULL) {
      ErrorMsg(UndefLatexPiece);
      return "??";
    } else
      return LaTeXPiecesAbbr[Name];
  } else
    return LaTeXStdPie[Name];
} /* LaTeXPiece */

void LaTeXEchoAddedPiece(Flags Spec, PieNam Name, square Square)
{
  sprintf(GlobalStr,
          "%s\\%c%s %c%c",
          is_square_empty(Square) ? "+" : "",
          is_piece_neutral(Spec) ? 'n' : (TSTFLAG(Spec, White) ? 'w' : 's'),
          LaTeXPiece(Name),
          'a'-nr_of_slack_files_left_of_board+Square%onerow,
          '1'-nr_of_slack_rows_below_board+Square/onerow);
  strcat(ActTwinning, GlobalStr);
}

void LaTeXEchoRemovedPiece(Flags Spec, PieNam Name, square Square)
{
  strcat(ActTwinning, " --");
  strcat(ActTwinning,
         is_piece_neutral(Spec) ? "\\n" : (TSTFLAG(Spec, White) ? "\\w" : "\\s"));
  strcat(ActTwinning,LaTeXPiece(Name));
  sprintf(GlobalStr, " %c%c",
          'a'-nr_files_on_board+Square%onerow,
          '1'-nr_rows_on_board+Square/onerow);
  strcat(ActTwinning,GlobalStr);
}

void LaTeXEchoMovedPiece(Flags Spec, PieNam Name, square FromSquare, square ToSquare)
{
  sprintf(GlobalStr,
          "\\%c%s %c%c",
          is_piece_neutral(Spec) ? 'n' : (TSTFLAG(Spec, White) ? 'w' : 's'),
          LaTeXPiece(Name),
          'a'-nr_of_slack_files_left_of_board+FromSquare%onerow,
          '1'-nr_of_slack_rows_below_board+FromSquare/onerow);
  strcat(ActTwinning, GlobalStr);

  strcat(ActTwinning, "{\\ra}");

  sprintf(GlobalStr, "%c%c",
          'a'-nr_files_on_board+ToSquare%onerow,
          '1'-nr_rows_on_board+ToSquare/onerow);
  strcat(ActTwinning, GlobalStr);
}

void LaTeXEchoExchangedPiece(Flags Spec1, PieNam Name1, square Square1,
                             Flags Spec2, PieNam Name2, square Square2)
{
  sprintf(GlobalStr,
          "\\%c%s %c%c",
          is_piece_neutral(Spec1) ? 'n' : (TSTFLAG(Spec1, White) ? 'w' : 's'),
          LaTeXPiece(Name1),
          'a'-nr_of_slack_files_left_of_board+Square1%onerow,
          '1'-nr_of_slack_rows_below_board+Square1/onerow);
  strcat(ActTwinning, GlobalStr);

  strcat(ActTwinning, "{\\lra}");

  sprintf(GlobalStr, "\\%c%s ",
          is_piece_neutral(Spec2) ? 'n' : (TSTFLAG(Spec2, White) ? 'w' : 's'),
          LaTeXPiece(Name2));
  strcat(ActTwinning, GlobalStr);
  sprintf(GlobalStr, "%c%c",
          'a'-nr_files_on_board+Square2%onerow,
          '1'-nr_rows_on_board+Square2/onerow);
  strcat(ActTwinning, GlobalStr);
}

void LaTeXEchoSubstitutedPiece(PieNam from, PieNam to)
{
  sprintf(GlobalStr,"{\\w%s} $\\Rightarrow$ \\w%s",
          LaTeXPiece(from),LaTeXPiece(to));
  strcat(ActTwinning,GlobalStr);
}

char *ParseLaTeXPieces(char *tok)
{
  PieNam Name;
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

static void LaTeXStr(char const *line)
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

    default:
      fprintf(LaTeXFile, "%c", *line);
      fflush(LaTeXFile);         /* non-buffered output  FCO */
      break;
    }
    line++;
  }
}

boolean LaTeXOpen(void)
{
  /* open mode for protocol and/or TeX file; overwrite existing file(s)
   * if we are doing a regression test */
  char const *open_mode = flag_regression ? "w" : "a";

  LaTeXFile = fopen(InputLine,open_mode);
  if (LaTeXFile==NULL)
  {
    IoErrorMsg(WrOpenError,0);
    return false;
  }
  else
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
    return true;
  }
}

void LaTeXClose(void)
{
  if (LaTeXFile!=NULL)
  {
    fprintf(LaTeXFile, "\n\\putsol\n\n\\end{document}\n");
    fclose(LaTeXFile);
  }
}

void LaTeXEndDiagram(void)
{
  char line[256];

  /* twins */
  if (ActTwinning[0] != '\0')
  {
    fprintf(LaTeXFile, " \\twins{");
    /* remove the last "{\\newline} */
    ActTwinning[strlen(ActTwinning)-10]= '\0';
    LaTeXStr(ActTwinning);
    fprintf(LaTeXFile, "}%%\n");
  }

  /* solution */
  fprintf(LaTeXFile, " \\solution{%%\n");
  rewind(SolFile);
  while (fgets(line, 255, SolFile))
  {
    if (!strstr(line, GetMsgString(TimeString)))
    {
      if (strlen(line) > 1 && line[1] == ')') {
        /* twin */
        fprintf(LaTeXFile, "%c)", line[0]);
      }
      else if (strlen(line) > 2 && line[2] == ')') {
        if (line[0] == '+')        /* twin (continued) */
          fprintf(LaTeXFile, "%c)", line[1]);
        else
          fprintf(LaTeXFile, "%c%c)", line[0], line[1]);
      }
      else if (strlen(line) > 3 && line[3] == ')') {
        /* continued twinning and >z */
        fprintf(LaTeXFile, "%c%c)", line[1], line[2]);
      }
      if (strchr(line, '.')) {   /* line containing a move */
        LaTeXStr(line);
      }
    }
  }

  fprintf(LaTeXFile, " }%%\n");
  fclose(SolFile);

  SolFile= tmpfile();
  if (SolFile == NULL) {
    IoErrorMsg(WrOpenError,0);
  }

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
}

static void WriteCondition(char const CondLine[], boolean is_first)
{
  if (is_first)
    fprintf(LaTeXFile, " \\condition{");
  else
    fprintf(LaTeXFile, "{\\newline}\n   ");
  LaTeXStr(CondLine);
}

void LaTeXBeginDiagram(void)
{
  boolean firstpiece= true, fairypieces= false, holess= false,
    modifiedpieces=false;
  PieSpec sp;
  boolean is_piece_on_side[PieceCount][nr_sides+1];
  char ListSpec[PieSpCount-nr_sides][256];
  unsigned int SpecCount[PieSpCount-nr_sides] = { 0 };
  char    HolesSqList[256] = "";
  square const *bnp;

  for (sp= nr_sides; sp<PieSpCount; ++sp)
    strcpy(ListSpec[sp-nr_sides], PieSpString[UserLanguage][sp-nr_sides]);

  fprintf(LaTeXFile, "\\begin{diagram}%%\n");

  /* authors */
  if (ActAuthor[0] != '\0')
  {
    if (strchr(ActAuthor, ',')) {
      /* , --> correct format */
      char *cp, *endcp = 0;

      while ((cp=strchr(ActAuthor, '\n'))) {
        *cp= ';';
        endcp= cp;
      }
      if (endcp)
        *endcp= '\0';

      sprintf(GlobalStr, " \\author{%s}%%%%\n", ActAuthor);
      LaTeXStr(GlobalStr);

      if (endcp)
        *endcp= '\n';
      while ((cp=strchr(ActAuthor, ';')))
        *cp= '\n';
    }
    else {
      /* reverse first and surnames */
      char *cp1, *cp2, *cp3;

      fprintf(LaTeXFile, " \\author{");
      cp1= ActAuthor;
      while ((cp2=strchr(cp1, '\n'))) {
        *cp2= '\0';
        if (cp1 != ActAuthor)
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

  /* source */
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

  /* award */
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

  /* dedication */
  if (ActTitle[0] != '\0')
  {
    sprintf(GlobalStr, "\\dedication{%s}%%%%\n", ActTitle);
    LaTeXStr(GlobalStr);
  }

  /* pieces & twins */
  if (OptFlag[duplex])
  {
    strcat(ActTwinning, OptTab[duplex]);
    strcat(ActTwinning, "{\\newline}");
  }
  else if (OptFlag[halfduplex])
  {
    strcat(ActTwinning, OptTab[halfduplex]);
    strcat(ActTwinning, "{\\newline}");
  }
  if (OptFlag[quodlibet])
  {
    strcat(ActTwinning, OptTab[quodlibet]);
    strcat(ActTwinning, "{\\newline}");
  }

  fprintf(LaTeXFile, " \\pieces{");

  {
    PieNam p;
    for (p = Empty; p < PieceCount; ++p)
    {
      is_piece_on_side[p][White] = false;
      is_piece_on_side[p][Black] = false;
      is_piece_on_side[p][nr_sides] = false;
    }
  }

  for (bnp= boardnum; *bnp; bnp++) {
    if (is_square_blocked(*bnp))
    {
      /* holes */
      if (holess)
        strcat(HolesSqList, ", ");
      else
        holess= true;
      AddSquare(HolesSqList, *bnp);
    }
    else if (!is_square_empty(*bnp))
    {
      PieNam const p = get_walk_of_piece_on_square(*bnp);
      if (!firstpiece)
        fprintf(LaTeXFile, ", ");
      else
        firstpiece= false;

      fprintf(LaTeXFile, "%c%s%c%c",
              is_piece_neutral(spec[*bnp]) ? 'n' :
              TSTFLAG(spec[*bnp], White)   ? 'w' : 's',
              LaTeXPiece(p),
              *bnp%onerow-200%onerow+'a',
              *bnp/onerow-200/onerow+'1');

      if (p>Bishop && (LaTeXPiecesAbbr[p] != NULL))
      {
        fairypieces= true;

        if (is_piece_neutral(spec[*bnp])) {
          is_piece_on_side[p][nr_sides] = true;
        }
        else if (TSTFLAG(spec[*bnp], White)) {
          is_piece_on_side[p][White] = true;
        }
        else {
          is_piece_on_side[p][Black] = true;
        }
      }

      for (sp= nr_sides; sp<PieSpCount; ++sp)
      {
        if (TSTFLAG(spec[*bnp], sp)
            && !(sp==Royal && (p==King || p==Poseidon)))
        {
          AddSquare(ListSpec[sp-nr_sides], *bnp);
          ++SpecCount[sp-nr_sides];
        }
      }
    }
  }
  fprintf(LaTeXFile, "}%%\n");
  fflush(LaTeXFile);

  if (holess) {
    fprintf(LaTeXFile, " \\nofields{");
    fprintf(LaTeXFile, "%s}%%\n", HolesSqList);
    fprintf(LaTeXFile, " \\fieldframe{");
    fprintf(LaTeXFile, "%s}%%\n", HolesSqList);
  }

  for (sp= nr_sides; sp<PieSpCount; ++sp)
    if (SpecCount[sp-nr_sides]>0
        && !(sp==Patrol && CondFlag[patrouille])
        && !(sp==Volage && CondFlag[volage])
        && !(sp==Beamtet && CondFlag[beamten]))
      modifiedpieces =true;     /* to be used below */

  /* stipulation */
  fprintf(LaTeXFile, " \\stipulation{");
  LaTeXStr(ActStip);
  if (OptFlag[solapparent])
    fprintf(LaTeXFile, "*");
  if (OptFlag[whitetoplay])
    fprintf(LaTeXFile,
            " %c{\\ra}", tolower(*PieSpString[UserLanguage][White]));

  fprintf(LaTeXFile, "}%%\n");

  /* conditions */
  if (CondFlag[gridchess] && !OptFlag[suppressgrid]) {
    boolean entry=false;
    switch (grid_type)
    {
      case grid_normal:
        fprintf(LaTeXFile, " \\stdgrid%%\n");
        break;

      case grid_vertical_shift:
        fprintf(LaTeXFile, " \\gridlines{h018, h038, h058, h078, v208, v408, v608}%%\n");
        break;

      case grid_horizontal_shift:
        fprintf(LaTeXFile, " \\gridlines{h028, h048, h068, v108, v308, v508, v708}%%\n");
        break;

      case grid_diagonal_shift:
        fprintf(LaTeXFile, " \\gridlines{h018, h038, h058, h078, v108, v308, v508, v708}%%\n");
        break;

      case grid_orthogonal_lines:
      {
        unsigned int i;
        for (i=1; i<8; i++)
          if (GridNum(square_a1+i-1) != GridNum(square_a1+i))
          {
            if (!entry)
              fprintf(LaTeXFile, " \\gridlines{");
            else
              fprintf(LaTeXFile, ", ");
            entry= true;
            fprintf(LaTeXFile, " v%u08", i);
          }
        for (i=1; i<8; i++)
          if (GridNum(square_a1+24*(i-1)) != GridNum(square_a1+24*i))
          {
            if (!entry)
              fprintf(LaTeXFile, " \\gridlines{");
            else
              fprintf(LaTeXFile, ", ");
            entry= true;
            fprintf(LaTeXFile, " h0%u8", i);
          }
        if (entry)
          fprintf(LaTeXFile, "}%%\n");
        break;
      }

      /* of course, only the following block is necessary */
      case grid_irregular:
        for (bnp = boardnum; *bnp; bnp++)
        {
          int i= *bnp%24-8, j= *bnp/24-8;
          if (i && GridLegal((*bnp)-1, *bnp))
          {
            if (!entry)
              fprintf(LaTeXFile, " \\gridlines{");
            else
              fprintf(LaTeXFile, ", ");
            entry= true;
            fprintf(LaTeXFile, " v%d%d1", i, j);
          }
          if (j && GridLegal((*bnp)-24, *bnp))
          {
            if (!entry)
              fprintf(LaTeXFile, " \\gridlines{");
            else
              fprintf(LaTeXFile, ", ");
            entry= true;
            fprintf(LaTeXFile, " h%d%d1", i, j);
          }
        }
        if (entry)
          fprintf(LaTeXFile, "}%%\n");
        break;
    }
  }

  if (WriteConditions(&WriteCondition))
    fprintf(LaTeXFile, "}%%\n");

  /* magical squares with frame */
  if (CondFlag[magicsquare])
  {
    char    MagicSqList[256] = "";
    boolean first_magic_piece= true;
    square  i;

    fprintf(LaTeXFile, " \\fieldframe{");
    for (i= square_a1; i <= square_h8; i++)
      if (TSTFLAG(sq_spec[i], MagicSq))
      {
        if (!first_magic_piece)
          strcat(MagicSqList, ", ");
        else
          first_magic_piece= false;
        AddSquare(MagicSqList, i);
      }
    fprintf(LaTeXFile, "%s}%%\n", MagicSqList);
  }

  /* fairy pieces, modified pieces, holes */
  if (fairypieces || holess || modifiedpieces)
  {
    PieNam p;
    boolean firstline= true;

    fprintf(LaTeXFile, " \\remark{");
    for (p = Bishop+1; p < PieceCount; ++p)
    {
      PieNam q;
      if (is_piece_on_side[p][White] || is_piece_on_side[p][Black] || is_piece_on_side[p][nr_sides])
      {
        for (q = Bishop+1; q < p; q++) {
          if ((is_piece_on_side[q][White]
               || is_piece_on_side[q][Black]
               || is_piece_on_side[q][nr_sides])
              && LaTeXPiecesAbbr[p][0] == LaTeXPiecesAbbr[q][0]
              && LaTeXPiecesAbbr[p][1] == LaTeXPiecesAbbr[q][1])
          {
            fprintf(stderr, "+++ Warning: "
                    "double representation '%s' for %s and %s\n",
                    LaTeXPiecesAbbr[q],
                    LaTeXPiecesFull[p], LaTeXPiecesFull[q]);
          }
        }

        if (!firstline)
          fprintf(LaTeXFile, "{\\newline}\n    ");
        fprintf(LaTeXFile, "\\mbox{");
        if (is_piece_on_side[p][White])
          fprintf(LaTeXFile, "\\w%s ", LaTeXPiecesAbbr[p]);
        if (is_piece_on_side[p][ Black])
          fprintf(LaTeXFile, "\\s%s ", LaTeXPiecesAbbr[p]);
        if (is_piece_on_side[p][nr_sides])
          fprintf(LaTeXFile, "\\n%s ", LaTeXPiecesAbbr[p]);
        fprintf(LaTeXFile, "=%s}", LaTeXPiecesFull[p]);
        firstline= false;
      }
    }

    if (modifiedpieces)
    {
      for (sp = nr_sides; sp<PieSpCount; ++sp)
        if (SpecCount[sp-nr_sides]>0)
        {
          if (!firstline)
            fprintf(LaTeXFile, "{\\newline}\n    ");
          fprintf(LaTeXFile, "%s\n", ListSpec[sp-nr_sides]);
          firstline= false;
        }
    }

    if (holess) {
      if (!firstline)
        fprintf(LaTeXFile, "{\\newline}\n    ");
      fprintf(LaTeXFile, "%s %s%%\n",
              CondString[UserLanguage][holes], HolesSqList);
    }
    fprintf(LaTeXFile, "}%%\n");
  }
}
