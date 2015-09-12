#include "input/plaintext/plaintext.h"
#include "output/plaintext/language_dependant.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

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

void input_plaintext_iterate_problems(slice_index si)
{
  boolean halt = false;

  do
  {
    pipe_solve_delegate(si);

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

void input_plaintext_detect_user_language(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  UserLanguage = detect_user_language(ReadNextTokStr());

  if (UserLanguage==LanguageCount)
    output_plaintext_input_error_message(NoBegOfProblem, 0);
  else
  {
    output_plaintext_select_language(UserLanguage);
    output_message_initialise_language(UserLanguage);

    pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
