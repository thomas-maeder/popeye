#include "input/plaintext/plaintext.h"
#include "optimisations/hash.h"
#include "solving/moves_traversal.h"
#include "optimisations/orthodox_check_directions.h"
#include "input/plaintext/problem.h"
#include "input/plaintext/memory.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/protocol.h"
#include "output/latex/latex.h"
#include "platform/platform.h"
#include "platform/maxtime.h"
#include "platform/maxmem.h"
#include "platform/pytime.h"
#include "platform/priority.h"
#include "stipulation/pipe.h"
#include "debugging/trace.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* iterate until we detect an input token that identifies the user's language
 * @return the detected language
 */
static Language detect_user_language(char *tok)
{
  Language result = LanguageCount;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  {
    Language candidate;
    for (candidate = 0; candidate<LanguageCount; ++candidate)
    {
      TraceValue("%u",candidate);TraceEOL();
      if (GetUniqIndex(GlobalTokenCount,GlobalTokenString[candidate],tok)==BeginProblem)
      {
        result = candidate;
        break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void input_plaintext_iterate_problems(slice_index si)
{
  boolean halt = false;

  do
  {
    input_plaintext_problem_handle(si);

    switch (GetUniqIndex(ProblemTokenCount,ProblemTokenTab,TokenLine))
    {
      case ProblemTokenCount:
      case EndProblem:
        halt = true;
        break;

      case NextProblem:
        break;

      default:
        output_plaintext_input_error_message(ComNotUniq,0);
        break;
    }
  } while (!halt);
}

/* Start dealing with plaintext input
 */
void input_plaintext_start(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  UserLanguage = detect_user_language(ReadNextTokStr());

  if (UserLanguage==LanguageCount)
    output_plaintext_input_error_message(NoBegOfProblem, 0);
  else
  {
    slice_index const problems_iterator = alloc_pipe(STInputPlainTextProblemsIterator);

    output_plaintext_select_language(UserLanguage);
    output_message_initialise_language(UserLanguage);

    input_plaintext_iterate_problems(problems_iterator);

    dealloc_slices(problems_iterator);
  }

  assert_no_leaked_slices();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
