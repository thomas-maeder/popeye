#include "output/latex/latex.h"
#include "output/latex/problem.h"
#include "output/latex/twinning.h"
#include "output/latex/line/line.h"
#include "output/latex/tree/tree.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/stipulation.h"
#include "output/plaintext/sstipulation.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/pieces.h"
#include "input/plaintext/token.h"
#include "input/plaintext/language.h"
#include "input/plaintext/stipulation.h"
#include "pieces/walks/classification.h"
#include "pieces/attributes/neutral/neutral.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/pipe.h"
#include "conditions/bgl.h"
#include "conditions/grid.h"
#include "conditions/singlebox/type1.h"
#include "solving/incomplete.h"
#include "platform/tmpfile.h"
#include "debugging/assert.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *LaTeXPiecesAbbr[nr_piece_walks];
static char *LaTeXPiecesFull[nr_piece_walks];
char const *LaTeXStdPie[8] = { NULL, "C", "K", "B", "D", "S", "T", "L"};

static char const CharChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

output_engine_type const output_latex_engine =
{
    &fputc,
    &vfprintf,
    &fprintf,
    &fflush
};

output_symbol_table_type const output_latex_symbol_table =
{
    "{\\ra}",
    "{\\lra}",
    "{\\000}",
    "{\\00}"
};

char const *LaTeXWalk(piece_walk_type walk)
{
  if (walk > Bishop)
  {
    if (LaTeXPiecesAbbr[walk] == NULL)
    {
      output_plaintext_error_message(UndefLatexPiece);
      return "??";
    } else
      return LaTeXPiecesAbbr[walk];
  } else
    return LaTeXStdPie[walk];
}

char *ParseLaTeXPieces(void)
{
  char *tok;
  piece_walk_type walk;
  int i;

  /* don't delete all this. Since both arrays are declared static,
   * they are initialized to NULL.
   * Simply allow overwriting these definitions within the LaTeX clause
   * and let it be initialized from pie-<lang>.dat
   *
   for (walk= 1; walk < PieceCount; walk++) {
   if (LaTeXPiecesAbbr[walk]) {
   free(LaTeXPiecesAbbr[walk]);
   free(LaTeXPiecesFull[walk]);
   }
   LaTeXPiecesAbbr[walk]= NULL;
   LaTeXPiecesFull[walk]= NULL;
   }
  */

  tok = ReadNextTokStr();

  if (strlen(tok) < 3)
    while (true)
    {
      walk = GetPieNamIndex(tolower(tok[0]), strlen(tok) == 1 ? ' ' : tolower(tok[1]));

      if (walk==nr_piece_walks)
        return tok;

      if (LaTeXPiecesAbbr[walk])
      {
        free(LaTeXPiecesAbbr[walk]);
        free(LaTeXPiecesFull[walk]);
      }

      tok = ReadNextTokStr();
      LaTeXPiecesAbbr[walk]= (char *)malloc(sizeof(char)*(strlen(tok)+1));
      i= 0;
      while (tok[i]) {
        /* to avoid compiler warnings below made "better readable" */
        /*      LaTeXPiecesAbbr[walk][i]= tok[i++]+ 'A' - 'a';          */
        LaTeXPiecesAbbr[walk][i]= tolower(tok[i]) + 'A' - 'a';
        i++;
      }
      LaTeXPiecesAbbr[walk][i]= tolower(tok[i]);

      if (ReadToEndOfLine())
      {
        tok = InputLine;
        LaTeXPiecesFull[walk]= (char *)malloc(sizeof(char)*(strlen(tok)+1));
        strcpy(LaTeXPiecesFull[walk], tok);
      }

      tok = ReadNextTokStr();
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
  /* do *not* add a blank before the } here */
  fputs("}%\n",file);
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

void LaTeXCopyFile(FILE *src, FILE *dest, int size)
{
  char * const buffer = malloc(size+1);
  if (buffer!=0)
  {
    if (fgets(buffer,size+1,src))
      LaTeXStr(dest,buffer);
    free(buffer);
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

void LaTeXSetup(slice_index start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    slice_index const writer = branch_find_slice(STOutputLaTeXProblemWriter,
                                                 start,
                                                 stip_traversal_context_intro);
    slice_index const file_owner = SLICE_NEXT2(writer);

    assert(writer!=no_slice);

    SLICE_U(file_owner).writer.file = fopen(InputLine,"w");
    if (SLICE_U(file_owner).writer.file==NULL)
      output_plaintext_input_error_message(WrOpenError);
    else
      WriteIntro(SLICE_U(file_owner).writer.file);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void LaTeXShutdown(slice_index file_owner)
{
  FILE * const file = SLICE_U(file_owner).writer.file;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (file!=0)
  {
    WriteCommand(file,"putsol");
    WriteFixElement(file,"end","document",0);
    fclose(file);
    SLICE_U(file_owner).writer.file = NULL;

    LaTeXShutdownTwinning();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void output_latex_closer_solve(slice_index si)
{
  slice_index const proto = alloc_output_latex_writer(STOutputLaTeXProblemWriter,
                                                      si);
  SLICE_U(proto).writer.file = 0;
  slice_insertion_insert(si,&proto,1);

  pipe_solve_delegate(si);

  LaTeXShutdown(si);
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

void LaTeXCo(slice_index si, FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const interruption = branch_find_slice(STProblemSolvingIncomplete,
                                                       si,
                                                       stip_traversal_context_intro);
    assert(interruption!=no_slice);
    if (!(OptFlag[solmenaces]
          || OptFlag[solflights]
          || OptFlag[nontrivial]
          || problem_solving_completeness(interruption)!=solving_complete))
    {
      fputs(" \\Co+%",file);
      output_plaintext_print_version_info(file);
      fputs("\n",file);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXEndDiagram(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  WriteFixElement(file,"end","diagram",0);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXHfill(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

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
      strcpy(ListSpec[sp-nr_sides],PieSpTab[sp-nr_sides]);
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
  fprintf(file,"%s %s%%\n",CondTab[holes],HolesSqList);

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
      fprintf(file," v%u%u1",column,row);
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
      fprintf(file," h%u%u1",column,row);
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXMeta(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  WriteAuthor(file);
  WriteSource(file);
  WriteAward(file);
  WriteDedication(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXOptions(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  LaTeXWriteOptions();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXWritePieces(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  WritePieces(file);
  WriteFairyPieces(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXStipulation(FILE *file, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  OpenGeneratedElementOneLine(file,"stipulation");

  {
    FILE *tmp = platform_open_tmpfile();
    if (tmp==0)
    {
      perror("error opening tmpfile for initial LaTeX stipulation");
      fprintf(file,"%s","??");
    }
    else
    {
      int const length = WriteStipulation(tmp,si);
      rewind(tmp);
      LaTeXCopyFile(tmp,file,length);
      platform_close_tmpfile(tmp);
    }
  }

  if (OptFlag[solapparent])
    fputs("*",file);

  if (OptFlag[whitetoplay])
    fprintf(file," %c{\\ra}", tolower(*PieSpTab[White]));

  CloseElement(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXSStipulation(FILE *file, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  OpenGeneratedElementOneLine(file,"stipulation");

  {
    FILE *tmp = platform_open_tmpfile();
    int const length = WriteSStipulation(tmp,si);
    rewind(tmp);
    LaTeXCopyFile(tmp,file,length);
    platform_close_tmpfile(tmp);
  }

  CloseElement(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void LaTeXConditions(FILE *file)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  WriteGrid(file);
  WriteConditions(file,&WriteCondition);
  WriteSquareFrames(file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void visit_output_mode_selector(slice_index si, stip_structure_traversal *st)
{
  FILE * const file = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_U(si).output_mode_selector.mode==output_mode_line)
    solving_insert_output_latex_line_slices(si,file);
  else
    solving_insert_output_latex_tree_slices(si,file);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with slices that write the solution in
 * LaTeX
 */
void output_latex_instrument_solving(slice_index si)
{
  slice_index const file_owner = SLICE_NEXT2(si);
  FILE * const file = SLICE_U(file_owner).writer.file;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;

    TraceStipulation(si);

    stip_structure_traversal_init(&st,file);
    stip_structure_traversal_override_single(&st,
                                             STOutputModeSelector,
                                             &visit_output_mode_selector);
    stip_traverse_structure(si,&st);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void output_latex_instrument_solving_builder_solve(slice_index si)
{
  slice_index const file_owner = SLICE_NEXT2(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] = {
        alloc_output_latex_writer(STOutputLaTeXInstrumentSolvers,file_owner)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a LaTeX writer slice.
 * @param type slice type
 * @param file_owner identifies the slice that owns the output file
 * @return index of allocated slice
 */
slice_index alloc_output_latex_writer(slice_type type, slice_index file_owner)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(type);
  SLICE_NEXT2(result) = file_owner;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
