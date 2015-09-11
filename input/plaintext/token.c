#include "input/plaintext/token.h"
#include "output/output.h"
#include "output/plaintext/message.h"
#include "output/plaintext/protocol.h"
#include "utilities/boolean.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "platform/tmpfile.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char SpaceChar[] = " \t\n\r;,";
static char LastChar = ' ';

char TokenLine[LINESIZE];    /* This array contains the lowercase input */

static char savedTokenLine[LINESIZE];

static char TokenChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ#=+-%>!.<()~/&|:[]{}?";
/* Steingewinn ! */
/* introductory move */
/* h##! */
/* dia3.5 */
/* a1<-->h1  */
/* reci-h(=)#n */
/* h~2  do ANY helpmove */

static char SepraChar[] = "\n\r;.,";

static char Sep[] = "\n";

static FILE *Input;
static FILE *InputOriginal;

static FILE *InputMirror;
static fpos_t mirrorEnd;

char InputLine[LINESIZE];    /* This array contains the input as is */

static char LineSpaceChar[] = " \t;.,";

static boolean OpenInput(char const *s)
{
  if (strlen(s)==0)
  {
    Input = stdin;
    InputOriginal = Input;
    InputMirror = platform_tmpfile();
    if (InputMirror==0)
      perror("error opening temporary input mirror file");
    else
      return true;
  }
  else
  {
    /* b is essential for f[gs]etpos when s is a file with Linux EOL sequence
     * tested on Windows! */
    Input = fopen(s,"rb");

    if (Input==NULL)
      perror("error opening input file");
    else
    {
      InputMirror = Input;
      InputOriginal = Input;
      return true;
    }
  }

  return false;
}

static void CloseInput(void)
{
  fclose(Input);

  if (InputMirror!=0 && InputMirror!=Input)
    fclose(InputMirror);
}

void input_plaintext_opener_solve(slice_index si)
{
  char const *name = SLICE_U(si).input_opener.name;

  if (OpenInput(name))
  {
    pipe_solve_delegate(si);
    CloseInput();
  }
}

slice_index input_plaintext_alloc_opener(char const *name)
{
  slice_index const result = alloc_pipe(STInputPlainTextOpener);
  SLICE_U(result).input_opener.name = name;
  return result;
}

/* advance LastChar to the next input character */
static void NextChar(void)
{
  if (feof(Input))
    /* premature EOF - bail out */
    exit(1);
  else
  {
    int const ch = getc(Input);
    if (ch==EOF)
      /* causes end of token to be recognised if input lacks final \n */
      LastChar = ' ';
    else
    {
      LastChar = ch;
      if (InputMirror!=Input)
        fputc(ch,InputMirror);
    }
  }
}

fpos_t InputGetPosition(void)
{
  fpos_t result;
  fgetpos(InputMirror,&result);
  return result;
}

void InputStartReplay(fpos_t pos)
{
  memcpy(savedTokenLine, TokenLine, sizeof TokenLine);
  Input = InputMirror;
  fgetpos(InputMirror,&mirrorEnd);
  fsetpos(Input,&pos);
}

void InputEndReplay(void)
{
  Input = InputOriginal;
  fsetpos(InputMirror,&mirrorEnd);
  memcpy(TokenLine, savedTokenLine, sizeof TokenLine);
}

char *ReadNextTokStr(void)
{
  while (strchr(SpaceChar,LastChar))
    NextChar();

  if (strchr(TokenChar,LastChar))
  {
    char *p = InputLine;
    char *t = TokenLine;

    do {
      *p++ = LastChar;
      *t++ = LastChar;
      /* *t++= (isupper(ch)?tolower(ch):ch);  */
      /* EBCDIC support ! HD */
      NextChar();
    } while (strchr(TokenChar,LastChar));

    if (p >= (InputLine+sizeof(InputLine)))
      output_plaintext_fatal_message(InpLineOverflow);

    *t = '\0';
    *p = '\0';

    return TokenLine;
  }
  else if (strchr(SepraChar,LastChar))
  {
    do
    {
      NextChar();
    } while (strchr(SepraChar,LastChar));
    return Sep;
  }
  else
  {
    output_plaintext_input_error_message(WrongChar,LastChar);
    LastChar = TokenLine[0]= ' ';
    TokenLine[1] = '\0';
    return TokenLine;
  }
}

/* Test whether the next input token starts with a string
 * @param start start to be matched
 * @param token input token
 * @return true iff the entire start is found at the start of token
 * @note the comparison is done character-wise case-insensitively
 */
boolean token_starts_with(char const *start, char const *token)
{
  while (*start)
    if (tolower(*start++)!=tolower(*token++))
      return false;

  return true;
}

/* This works just like token_starts_with() apart from the different order of
 * arguments.
 * Keeping the two functions separate hopefully clarifies things a bit:
 * - sometimes, the token can be an abbreviation (provided it's unique)
 * - sometimes, the token consists of different elements (e.g. h#2)
 */
static boolean token_matches_start_of_keyword(char const *keyword,
                                              char const *token)
{
  while (*token)
    if (tolower(*keyword++)!=tolower(*token++))
      return false;

  return true;
}

static unsigned int GetIndex(unsigned int index, unsigned int limit,
                             char const * const *list, char const *tok)
{
  while (index<limit)
    if (token_matches_start_of_keyword(list[index],tok))
      return index;
    else
      ++index;

  return limit;
}

unsigned int GetUniqIndex(unsigned int limit, char const * const *list, char const *tok)
{
  unsigned int const index = GetIndex(0,limit,list,tok);
  if (index==limit)
    return index;
  else
  {
    if (strlen(tok)==strlen(list[index]))
      return index;
    else if (GetIndex(index+1,limit,list,tok)==limit)
      return index;
    else
      return limit+1;
  }
}

/* read into InputLine until the next1 end of line
 * @return true iff something has actually been read, i.e. we have not already
 *         been at the end of line
 */
boolean ReadToEndOfLine(void)
{
  if (LastChar=='\r' || LastChar=='\n')
    return false;
  else
  {
    char *p = InputLine;

    do
    {
      NextChar();
    } while (strchr(LineSpaceChar,LastChar));

    while (LastChar!='\r' && LastChar!='\n')
    {
      *p++ = LastChar;
      NextChar();
    }

    if (p >= (InputLine + sizeof(InputLine)))
      output_plaintext_fatal_message(InpLineOverflow);

    *p = '\0';

    return true;
  }
}
