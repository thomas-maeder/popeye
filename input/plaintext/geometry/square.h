#if !defined(INPUT_PLAINTEXT_GEOMETRY_SQUARE_H)
#define INPUT_PLAINTEXT_GEOMETRY_SQUARE_H

#include "position/board.h"

#include <limits.h>

/* Parse a square from two characters
 * @param tok where to start parsing
 * @param s where to store the square
 * @return first unparsed element in the token
 */
char *ParseSquare(char *tok, square *s);

typedef void (*parsed_square_handler)(square s, void *param);

/* Parse a list of squares from an input token
 * @param tok the input token
 * @param handleSquare callback function invoked for each square parsed
 * @param param parameter passed to handleSquare
 * @return first unparsed element in the token
 */
char *ParseSquareList(char *tok,
                      parsed_square_handler handleSquare,
                      void *param);

#endif
