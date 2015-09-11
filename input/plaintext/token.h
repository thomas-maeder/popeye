#if !defined(INPUT_PLAINTEXT_TOKEN_H)
#define INPUT_PLAINTEXT_TOKEN_H

#include "input/plaintext/language.h"
#include "stipulation/stipulation.h"
#include "utilities/boolean.h"

#include <stdio.h>

enum
{
  LINESIZE = 256
};

extern char InputLine[LINESIZE];    /* This array contains the input as is */

extern char TokenLine[LINESIZE];

void input_plaintext_opener_solve(slice_index si);

slice_index input_plaintext_alloc_opener(char const *name);

char *ReadNextTokStr(void);

/* Test whether the next input token starts with a string
 * @param start start to be matched
 * @param token input token
 * @return true iff the entire start is found at the start of token
 * @note the comparison is done character-wise case-insensitively
 */
boolean token_starts_with(char const *start, char const *token);

unsigned int GetUniqIndex(unsigned int limit, char const * const *list, char const *tok);

/* read into InputLine until the next1 end of line
 * @return true iff something has actually been read, i.e. we have not already
 *         been at the end of line
 */
boolean ReadToEndOfLine(void);

fpos_t InputGetPosition(void);
void InputStartReplay(fpos_t pos);
void InputEndReplay(void);

#endif
