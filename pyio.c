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
