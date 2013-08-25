/******************** MODIFICATIONS to pyio.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/01 SE   New Koeko conditions: GI-Koeko, AN-Koeko
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/05/17 SE   Changes to allow half-move specification for helpmates using 0.5 notation
 **                 Forsyth can be used as a twinning command
 **                 TakeMake name changed to Take&Make
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2007/01/28 NG   New stipulation: help-reflexmate (hr)
 **
 ** 2007/04/29 SE   Bugfix: not correctly parsing conditions listed after SAT
 **
 ** 2007/07/38 SE   Bugfix: Forsyth
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher
 **
 ** 2007/01/28 SE   New condition: Annan Chess
 **
 ** 2007/03/01 SE   Changed Twin Char to support larger no. of twins - old implementation
 **                 produced odd characters after a while; new just calls all twins after
 **                 z z1, z2, z3... etc. limited by sizeof int, I suppose
 **
 ** 2007/04/28 SE   Bugfix: parsing SAT followed by another condition
 **
 ** 2007/06/01 SE   Bug fixes: Forsyth entry (fairy piece on a8)
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/12/26 SE   New piece type: Protean man (invent A.H.Kniest?)
 **
 ** 2008/01/02 NG   New condition: Geneva Chess
 **
 ** 2008/01/11 SE   New variant: Special Grids
 **
 ** 2008/01/13 SE   New conditions: White/Black Vaulting Kings
 **
 ** 2008/01/24 SE   New variant: Gridlines
 **
 ** 2008/02/19 SE   New condition: AntiKoeko
 **
 ** 2009/06/14 SE   New option: LastCapture
 **
 ** 2012/01/27 NG   AlphabeticChess now possible for white or black only.
 **
 ** 2012/02/04 NG   New condition: Chess 8/1 (invented: Werner Keym, 5/2011)
 **
 **************************** End of List ******************************/

#if defined(macintosh)    /* is always defined on macintosh's  SB */
#   define SEGMIO
#   include "platform/unix/mac.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include "pyproc.h"
#include "pieces/pieces.h"
#include "options/options.h"
#include "conditions/conditions.h"
#include "pymsg.h"
#include "solving/solve.h"
#include "solving/castling.h"
#include "solving/proofgames.h"
#include "stipulation/pipe.h"
#include "output/output.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "output/latex/latex.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/token.h"
#include "input/plaintext/line.h"
#include "input/plaintext/pieces.h"
#include "input/plaintext/input_stack.h"
#include "input/plaintext/stipulation.h"
#include "pieces/walks/hunters.h"
#include "pieces/attributes/neutral/neutral.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/move_inverter.h"
#include "stipulation/if_then_else.h"
#include "stipulation/boolean/or.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/not.h"
#include "stipulation/boolean/true.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/mate/reached_tester.h"
#include "stipulation/goals/stalemate/reached_tester.h"
#include "stipulation/goals/doublestalemate/reached_tester.h"
#include "stipulation/goals/target/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/capture/reached_tester.h"
#include "stipulation/goals/steingewinn/reached_tester.h"
#include "stipulation/goals/enpassant/reached_tester.h"
#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/goals/countermate/reached_tester.h"
#include "stipulation/goals/castling/reached_tester.h"
#include "stipulation/goals/autostalemate/reached_tester.h"
#include "stipulation/goals/circuit/reached_tester.h"
#include "stipulation/goals/exchange/reached_tester.h"
#include "stipulation/goals/circuit_by_rebirth/reached_tester.h"
#include "stipulation/goals/exchange_by_rebirth/reached_tester.h"
#include "stipulation/goals/any/reached_tester.h"
#include "stipulation/goals/proofgame/reached_tester.h"
#include "stipulation/goals/atob/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/chess81/reached_tester.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/adapter.h"
#include "solving/find_shortest.h"
#include "solving/play_suppressor.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/try.h"
#include "solving/move_generator.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/move_effect_journal.h"
#include "conditions/isardam.h"
#include "conditions/synchronous.h"
#include "conditions/protean.h"
#include "conditions/madrasi.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/republican.h"
#include "conditions/bgl.h"
#include "conditions/grid.h"
#include "conditions/check_zigzag.h"
#include "conditions/patience.h"
#include "conditions/sat.h"
#include "conditions/oscillating_kings.h"
#include "conditions/kobul.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/april.h"
#include "conditions/circe/chameleon.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/cheylan.h"
#include "conditions/sentinelles.h"
#include "conditions/magic_square.h"
#include "conditions/mummer.h"
#include "conditions/transmuting_kings/super.h"
#include "conditions/immune.h"
#include "conditions/geneva.h"
#include "conditions/koeko/koeko.h"
#include "conditions/koeko/anti.h"
#include "conditions/phantom.h"
#include "conditions/annan.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/vaulting_kings.h"
#include "conditions/imitator.h"
#include "conditions/messigny.h"
#include "conditions/woozles.h"
#include "conditions/football.h"
#include "conditions/singlebox/type1.h"
#include "options/degenerate_tree.h"
#include "options/nontrivial.h"
#include "options/maxthreatlength.h"
#include "options/maxflightsquares.h"
#include "options/movenumbers.h"
#include "options/maxsolutions/maxsolutions.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "platform/beep.h"
#include "platform/maxtime.h"
#include "platform/maxmem.h"
#include "debugging/trace.h"

/* This is pyio.c
** It comprises a new io-Module for popeye.
** The old io was awkward, nonsystematic and I didn't like it.
** Therefore here a new one.
** Despite the remarks in the README file, I give here three languages
** for the input: english, french, german. It can easily be extended
** if necessary.
*/

#define UPCASE(c)   toupper(c)      /* (c+('A'-'a')) */
/* This is only correct, cause only lowercase letters are passed
   as arguments
*/

void AddSquare(char *List, square i)
{
  char    add[4];

  add[0]= ' ';
  add[1]= 'a' - nr_files_on_board + i%onerow;
  add[2]= '1' - nr_rows_on_board + i/onerow;
  add[3]= '\0';
  strcat(List, add);
}

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

/* These two arrays should have the same size */
#define LINESIZE    256

static void pyfputc(char c, FILE *f)
{
#if !defined(QUIET)
  fputc(c,f);
  fflush(f);
  if (TraceFile) {
    fputc(c,TraceFile);
    fflush(TraceFile);
  }
  if (SolFile) {
    fputc(c,SolFile);
    fflush(SolFile);
  }
#endif
}

void pyfputs(char const *s, FILE *f)
{
#if !defined(QUIET)
  fputs(s,f);
  fflush(f);
  if (TraceFile) {
    fputs(s,TraceFile);
    fflush(TraceFile);
  }
  if (SolFile) {
    fputs(s,SolFile);
    fflush(SolFile);
  }
#endif
}

void StdChar(char c)
{
  pyfputc(c, stdout);
}

static void ErrChar(char c)
{
  pyfputc(c, stderr);
}

void StdString(char const *s)
{
  pyfputs(s, stdout);
}

void ErrString(char const *s)
{
  pyfputs(s, stderr);
}

void IoErrorMsg(int n, int val)
{
  ErrorMsg(InputError);
  logIntArg(val);
  ErrorMsg(n);
  ErrChar('\n');
  logStrArg(InputLine);
  ErrorMsg(OffendingItem);
  ErrChar('\n');
}

/* All following Parse - Procedures return the next1 StringToken or
** the last token they couldn't interprete.
*/


void MultiCenter(char *s) {
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
  FILE    *OrigSolFile= SolFile;

  static char BorderL[]="+---a---b---c---d---e---f---g---h---+\n";
  static char HorizL[]="%c   .   .   .   .   .   .   .   .   %c\n";
  static char BlankL[]="|                                   |\n";

  unsigned int const fileWidth = 4;

  SolFile= NULL;

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
            AddSquare(ListSpec[sp-nr_sides], square);
            ++SpecCount[sp-nr_sides];
          }

        if (pp<Hunter0 || pp>=Hunter0+maxnrhuntertypes)
        {
          if ((*h1= PieceTab[pp][1]) != ' ')
          {
            *h1= UPCASE(*h1);
            h1--;
          }
          *h1--= UPCASE(PieceTab[pp][0]);
        }
        else
        {
          char *n1 = HLine2 + (h1-HLine1); /* current position on next1 line */

          unsigned int const hunterIndex = pp-Hunter0;
          assert(hunterIndex<maxnrhuntertypes);

          *h1-- = '/';
          if ((*h1= PieceTab[huntertypes[hunterIndex].away][1]) != ' ')
          {
            *h1= UPCASE(*h1);
            h1--;
          }
          *h1--= UPCASE(PieceTab[huntertypes[hunterIndex].away][0]);

          --n1;   /* leave pos. below '/' empty */
          if ((*n1= PieceTab[huntertypes[hunterIndex].home][1]) != ' ')
            *n1= UPCASE(*n1);
          *n1 = UPCASE(PieceTab[huntertypes[hunterIndex].home][0]);
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

  SolFile= OrigSolFile;
} /* WritePosition */


void WriteGrid(void)
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

void WritePiece(PieNam p) {
  if (p<Hunter0 || p>= (Hunter0 + maxnrhuntertypes))
  {
    char const p1 = PieceTab[p][1];
    StdChar(UPCASE(PieceTab[p][0]));
    if (p1!=' ')
      StdChar(UPCASE(p1));
  }
  else
  {
    unsigned int const i = p-Hunter0;
    assert(i<maxnrhuntertypes);
    WritePiece(huntertypes[i].away);
    StdChar('/');
    WritePiece(huntertypes[i].home);
  }
}

void WriteSquare(square i)
{
  StdChar('a' - nr_files_on_board + i%onerow);
  if (isBoardReflected)
    StdChar('8' + nr_rows_on_board - i/onerow);
  else
    StdChar('1' - nr_rows_on_board + i/onerow);
}

/* The input accepted by popeye is defined by the following grammar.
** If there is no space between two nonterminals, then there is also
** no other character allowed in the input. This holds for <SquareName>
** and <SquareList>. Other terminals must be seperated by any non
** alpha or non numeric character.
**
** <PopeyeInput>  ::= <BeginSpec> <PySpecList> <EndSpec>
**
** <BeginSpec>    ::= beginproblem
** <EndSpec>      ::= endproblem | nextproblem <PySpecList> <EndSpec>
**
** <PySpecList>   ::= <PySpec> | <PySpec> <PySpecList>
** <PySpec>   ::= <PieceSpec> | <StipSpec> | <CondSpec> | <AuthorSpec> |
**            <OrigSpec> | <KamikazeSpec> | <ImitatorSpec> |
**            <TitleSpec> | <RemarkSpec>
** <PieceSpec>    ::= pieces <PieceList>
** <KamikazeSpec> ::= kamikaze <PieceList>
** <ImitatorSpec> ::= imitator <SquareList>
** <PieceList>    ::= <MenSpec> <MenList> | <MenSpec> <MenList> <PieceList>
** <MenSpec>      ::= <ColorSpec> <AddSpecList>
** <ColorSpec>    ::= white | black | neutral
** <AddSpecList>  ::= <Empty> | <AddSpec>
** <AddSpec>      ::= royal
** <Empty>    ::=
**
** <MenList>      ::= <ManName> <SquareList> <Seperator> |
**            <ManName> <SquareList> <MenList>
** <ManName>      ::= as actually defined in POPEYE.
** <SquareList>    ::= <SquareName> | <SquareName><SquareList>
** <SquareName>    ::= <ColName><RowName>
** <ColName>      ::= a | b | c | d | e | f | g | h
** <RowName>      ::= 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8
**
** <StipSpec>     ::= stipulation <StipName> <Number>
** <StipName>     ::= as actually defined in popeye
** <Number>   ::= <Digit> | <Digit><Number>
** <Digit>    ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
**
** <CondSpec>     ::= condition <CondList>
** <CondList>     ::= <CondName> | <CondName> <CondList>
** <Condname>     ::= as actually defined in popeye
**
** <AuthorSpec>   ::= author <string> <eol>
** <OrigSpec>     ::= origin <string> <eol>
** <TitleSpec>    ::= title <string> <eol>
** <RemarkSpec>   ::= remark <string> <eol>
** <string>   ::= all characters except <eol>
** <eol>      ::= end of line
**
** Example:
** beginproblem
** author Erich Bartel
** origin Jugendschach 23, Nr.1234
** pieces
** white Ka1 KNf5,Ph2h3
** black Ka8 Qa6
** stipulation #2
** condition circe schwarzerrandzueger
** endproblem
**
** Every reserved identifier (i.E. token may be shortened in the input,
** as long as it is uniquely identified.
** There are some peculiarities about this rule:
** If you specify after pieces wh Ka1
** b Ka3
** with the intention to place the black king on a3, popeye will balk,
** due to the fact that he interpretes b as piece with name ... and ka3
** as squarelist. So you are encouraged to use at least four characters
** when switching from one color to the other.
** Therefore the above input could be shortened to
** beg
** au Erich Bartel
** or Jugendschach 23, Nr.1234
** pi
** wh Ka1 KNf5 PH2h3
** blac Ka8 Qa6
** st #2
** co cir schwarzerr
** end
**
*/
