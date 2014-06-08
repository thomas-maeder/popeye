#include "input/plaintext/problem.h"
#include "input/plaintext/token.h"
#include "input/plaintext/condition.h"
#include "input/plaintext/option.h"
#include "input/plaintext/twin.h"
#include "output/plaintext/message.h"
#include "output/plaintext/language_dependant.h"
#include "options/maxsolutions/maxsolutions.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "pieces/walks/hunters.h"
#include "solving/move_generator.h"
#include "platform/maxtime.h"
#include "debugging/assert.h"

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

  /* dummy squares for various purposes -- must be empty */
  empty_square(pawn_multistep);
  empty_square(messigny_exchange);
  empty_square(kingside_castling);
  empty_square(queenside_castling);
  empty_square(retro_capture_departure);

  for (bnp = boardnum; *bnp; bnp++)
    empty_square(*bnp);

  being_solved.king_square[White] = initsquare;
  being_solved.king_square[Black] = initsquare;
}

static void ReadBeginSpec(void)
{
  while (true)
  {
    char *tok = ReadNextTokStr();
    TokenTab = TokenString[0];
    for (UserLanguage= 0; UserLanguage<LanguageCount; UserLanguage++)
    {
      TokenTab= &(TokenString[UserLanguage][0]);
      if (GetUniqIndex(TokenCount,TokenTab,tok)==BeginProblem)
      {
        OptTab= &OptString[UserLanguage][0];
        CondTab= &CondString[UserLanguage][0];
        TwinningTab= &TwinningString[UserLanguage][0];
        TwinningMirrorTab= &TwinningMirrorString[UserLanguage][0];
        VaultingVariantTypeTab = &VaultingVariantTypeString[UserLanguage][0];
        ConditionLetteredVariantTypeTab = &ConditionLetteredVariantTypeString[UserLanguage][0];
        ConditionNumberedVariantTypeTab = &ConditionNumberedVariantTypeString[UserLanguage][0];
        AntiCirceVariantTypeTab = &AntiCirceVariantTypeString[UserLanguage][0];
        SentinellesVariantTypeTab = &SentinellesVariantTypeString[UserLanguage][0];
        GridVariantTypeTab = &GridVariantTypeString[UserLanguage][0];
        KobulVariantTypeTab = &KobulVariantTypeString[UserLanguage][0];
        KoekoVariantTypeTab = &KoekoVariantTypeString[UserLanguage][0];
        CirceVariantTypeTab = &CirceVariantTypeString[UserLanguage][0];
        ExtraCondTab= &ExtraCondString[UserLanguage][0];
        mummer_strictness_tab = &mummer_strictness_string[UserLanguage][0];
        PieceTab= PieNamString[UserLanguage];
        PieSpTab= PieSpString[UserLanguage];
        ColourTab= ColourString[UserLanguage];
        InitMsgTab(UserLanguage);
        return;
      }
    }

    IoErrorMsg(NoBegOfProblem, 0);
  }
}

/* Iterate over the problems read from standard input or the input
 * file indicated in the command line options
 */
void iterate_problems(void)
{
  Token prev_token = BeginProblem;

  ReadBeginSpec();

  do
  {
    nextply(no_side);
    assert(nbply==ply_diagram_setup);

    InitMetaData();
    InitBoard();
    InitCond();
    InitOpt();

    reset_max_solutions();
    reset_was_max_nr_solutions_per_target_position_reached();
    reset_short_solution_found_in_problem();

    hunters_reset();

    prev_token = iterate_twins();

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

    undo_move_effects();
    finply();
  } while (prev_token==NextProblem);
}
