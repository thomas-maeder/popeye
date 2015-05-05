#include "input/plaintext/geometry/square.h"
#include "utilities/boolean.h"
#include "debugging/trace.h"

/* Parse a square from two characters
 * @return the parsed quare
 *         initsquare if a square can't be parsed form the characters */
square ParseSquare(char const *tok)
{
  char const char_file = tok[0];
  char const char_row = tok[1];

  if ('a'<=char_file && char_file<='h' && '1'<=char_row && char_row<='8')
    return square_a1 + (char_file-'a')*dir_right +(char_row-'1')*dir_up;
  else
    return initsquare;
}

unsigned int ParseSquareList(char *tok,
                             parsed_square_handler handleSquare,
                             void *param)
{
  enum
  {
    chars_per_square = 2
  };

  unsigned int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  while (tok[0]!=0)
  {
    square const sq = ParseSquare(tok);
    if (tok[0]!=0 && tok[1]!=0 && sq!=initsquare)
    {
      handleSquare(sq,param);
      ++result;
      tok += chars_per_square;
    }
    else
    {
      if (result>0)
        result = UINT_MAX;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
