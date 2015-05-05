#if !defined(INPUT_PLAINTEXT_GEOMETRY_SQUARE_H)
#define INPUT_PLAINTEXT_GEOMETRY_SQUARE_H

#include "position/board.h"

#include <limits.h>

/* Parse a square from two characters
 * @return the parsed quare
 *         initsquare if a square can't be parsed form the characters */
square ParseSquare(char const *tok);

typedef void (*parsed_square_handler)(square s, void *param);

/* Parse a list of squares from an input token
 * @param tok the input token
 * @param handleSquare callback function invoked for each square parsed
 * @param param parameter passed to handleSquare
 * @return number of squares parsed
 *         UINT_MAX if an error was detected after successful parsing of some squares
 */
unsigned int ParseSquareList(char *tok,
                             parsed_square_handler handleSquare,
                             void *param);

#endif
