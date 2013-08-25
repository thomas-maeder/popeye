#include "input/plaintext/problem.h"
#include "input/plaintext/token.h"
#include "input/plaintext/condition.h"
#include "input/plaintext/option.h"
#include "input/plaintext/twin.h"
#include "output/latex/latex.h"
#include "options/maxsolutions/maxsolutions.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "pieces/walks/hunters.h"
#include "solving/move_generator.h"
#include "platform/maxtime.h"
#include "pyproc.h"
#include "pymsg.h"

char ActAuthor[256];
char ActOrigin[256];
char ActTitle[256];
char ActAward[256];
char ActStip[37];

static void InitMetaData(void)
{
  ActTitle[0] = '\0';
  ActAuthor[0] = '\0';
  ActOrigin[0] = '\0';
  ActAward[0] = '\0';
  ActStip[0] = '\0';
}

static void InitBoard(void)
{
  square i;
  square const *bnp;

  for (i= maxsquare-1; i>=0; i--)
  {
    empty_square(i);
    block_square(i);
  }

  /* dummy squares for Messigny chess and castling -- must be empty */
  empty_square(messigny_exchange);
  empty_square(kingside_castling);
  empty_square(queenside_castling);

  for (bnp = boardnum; *bnp; bnp++)
    empty_square(*bnp);

  king_square[White] = initsquare;
  king_square[Black] = initsquare;

  CLEARFL(all_pieces_flags);
  CLEARFL(all_royals_flags);
  CLEARFL(some_pieces_flags);

  nrhuntertypes = 0;
}

/* Iterate over the problems read from standard input or the input
 * file indicated in the command line options
 */
void iterate_problems(void)
{
  Token prev_token = BeginProblem;

  do
  {
    LatexResetTwinning();
    InitMetaData();
    InitBoard();
    InitCond();
    InitOpt();

    reset_max_solutions();
    reset_was_max_nr_solutions_per_target_position_reached();
    reset_short_solution_found_in_problem();

    prev_token = iterate_twins(prev_token);

    if (max_solutions_reached()
        || was_max_nr_solutions_per_target_position_reached()
        || has_short_solution_been_found_in_problem()
        || hasMaxtimeElapsed())
      StdString(GetMsgString(InterMessage));
    else
      StdString(GetMsgString(FinishProblem));

    StdString(" ");
    PrintTime();
    StdString("\n\n\n");

    if (LaTeXout)
      LaTeXEndDiagram();
  } while (prev_token==NextProblem);
}
