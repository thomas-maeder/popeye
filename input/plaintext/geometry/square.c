#include "input/plaintext/geometry/square.h"
#include "debugging/trace.h"

#include <ctype.h>

  enum
  {
    chars_per_square = 2
  };

  /* Parse a square from two characters
 * @return the parsed quare
 *         initsquare if a square can't be parsed form the characters */
char *ParseSquare(char *tok, square *s)
{
  char *result = tok;
  char const char_file = tolower(tok[0]);

  *s = initsquare;

  if ('a'<=char_file && char_file<='h')
  {
    /* only know that we know that tok[0] is not the end of the string, or
     * we might read past the end of a buffer!
     */
    char const char_row = tok[1];
    if ('1'<=char_row && char_row<='8')
    {
      *s = square_a1 + (char_file-'a')*dir_right +(char_row-'1')*dir_up;
      result += chars_per_square;
    }
  }

  return result;
}

char *ParseSquareList(char *tok,
                      parsed_square_handler handleSquare,
                      void *param)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  while (tok[0]!=0)
  {
    square sq;
    tok = ParseSquare(tok,&sq);
    if (sq==initsquare)
      break;
    else
      handleSquare(sq,param);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}
