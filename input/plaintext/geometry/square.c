#include "input/plaintext/geometry/square.h"
#include "debugging/trace.h"

#include <ctype.h>

  enum
  {
    chars_per_square = 2
  };

/* Parse a square from two characters
 * @param tok where to start parsing
 * @param s where to store the square
 * @return first unparsed element in the token
 */
char *ParseSquare(char *tok, square *s)
{
  char *result = tok;
  board_label_type const char_file = (board_label_type)tolower((unsigned char)tok[0]);

  *s = initsquare;

  unsigned int const fileIndex = getBoardFileIndex(char_file);
  if (fileIndex < nr_files_on_board)
  {
    /* only know that we know that tok[0] is not the end of the string, or
     * we might read past the end of a buffer!
     */
    board_label_type const char_row = (board_label_type)tok[1];
    unsigned int const rowIndex = getBoardRowIndex(char_row);
    if (rowIndex < nr_rows_on_board)
    {
      *s = square_a1 + (int)fileIndex*dir_right +(int)rowIndex*dir_up;
      result += chars_per_square;
    }
  }

  return result;
}

/* Parse a list of squares from an input token
 * @param tok the input token
 * @param handleSquare callback function invoked for each square parsed
 * @param param parameter passed to handleSquare
 * @return first unparsed element in the token
 */
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
