#include "input/plaintext/token.h"
#include "output/output.h"
#include "output/plaintext/message.h"
#include "output/plaintext/protocol.h"
#include "utilities/boolean.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char SpaceChar[] = " \t\n\r;.,";
static char LastChar = ' ';

char TokenLine[LINESIZE];    /* This array contains the lowercase input */

static char TokenChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ#=+-%>!.<()~/&|:[]{}";
/* Steingewinn ! */
/* introductory move */
/* h##! */
/* dia3.5 */
/* a1<-->h1  */
/* reci-h(=)#n */
/* h~2  do ANY helpmove */

static char SepraChar[] = "\n\r;.,";

static char Sep[] = "\n";

char const **TokenTab; /* set according to language */

char const *TokenString[LanguageCount][TokenCount] =
{
  { /* francais */
    /* 0*/  "DebutProbleme",
    /* 1*/  "FinProbleme",
    /* 2*/  "asuivre",
    /* 3*/  "enonce",
    /* 4*/  "senonce",
    /* 5*/  "auteur",
    /* 6*/  "source",
    /* 7*/  "pieces",
    /* 8*/  "condition",
    /* 9*/  "option",
    /*10*/  "remarque",
    /*11*/  "protocol",
    /*12*/  Sep,
    /*13*/  "titre",
    /*14*/  "jumeau",
    /*15*/  "zeroposition",
    /*16*/  "LaTeX",
    /*17*/  "PiecesLaTeX",
    /*18*/  "prix",
    /*19*/  "PositionInitialPartie",
    /*20*/  "Forsyth"
  },
  { /* Deutsch */
    /* 0*/  "AnfangProblem",
    /* 1*/  "EndeProblem",
    /* 2*/  "WeiteresProblem",
    /* 3*/  "Forderung",
    /* 4*/  "sForderung",
    /* 5*/  "Autor",
    /* 6*/  "Quelle",
    /* 7*/  "Steine",
    /* 8*/  "Bedingung",
    /* 9*/  "Option",
    /*10*/  "Bemerkung",
    /*11*/  "Protokoll",
    /*12*/  Sep,
    /*13*/  "Titel",
    /*14*/  "Zwilling",
    /*15*/  "NullStellung",
    /*16*/  "LaTeX",
    /*17*/  "LaTeXSteine",
    /*18*/  "Auszeichnung",
    /*19*/  "PartieAnfangsStellung",
    /*20*/  "Forsyth"
  },
  { /* english */
    /* 0*/  "beginproblem",
    /* 1*/  "endproblem",
    /* 2*/  "nextproblem",
    /* 3*/  "stipulation",
    /* 4*/  "sstipulation",
    /* 5*/  "author",
    /* 6*/  "origin",
    /* 7*/  "pieces",
    /* 8*/  "condition",
    /* 9*/  "option",
    /*10*/  "remark",
    /*11*/  "protocol",
    /*12*/  Sep,
    /*13*/  "title",
    /*14*/  "twin",
    /*15*/  "zeroposition",
    /*16*/  "LaTeX",
    /*17*/  "LaTeXPieces",
    /*18*/  "award",
    /*19*/  "InitialGameArray",
    /*20*/  "Forsyth"
  }
};

static FILE *Input;
static FILE *InputOriginal;

static FILE *InputMirror;
static fpos_t mirrorEnd;

char InputLine[LINESIZE];    /* This array contains the input as is */

static char LineSpaceChar[] = " \t;.,";

boolean OpenInput(char const *s)
{
  if (strlen(s)==0)
    Input = stdin;
  else
    Input = fopen(s,"r");

  if (Input==NULL)
    return false;
  else
  {
    InputOriginal = Input;
    InputMirror = tmpfile();
    return true;
  }
}

void CloseInput(void)
{
  fclose(Input);
  fclose(InputMirror);
}

/* advance LastChar to the next1 input character */
static void NextChar(void)
{
  int const ch = getc(Input);
  if (ch==EOF)
    LastChar= ' ';
  else
  {
    LastChar = ch;
    if (InputMirror!=Input)
      fputc(ch,InputMirror);
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
  Input = InputMirror;
  fgetpos(InputMirror,&mirrorEnd);
  fsetpos(Input,&pos);
}

void InputEndReplay(void)
{

  Input = InputOriginal;
  fsetpos(InputMirror,&mirrorEnd);
}

char *ReadNextCaseSensitiveTokStr(void)
{
  while (strchr(" \t\n\r;,:",LastChar))  /* SpaceChar minus '.' which can be first char of extended Forsyth */
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
      FtlMsg(InpLineOverflow);

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
    IoErrorMsg(WrongChar,LastChar);
    LastChar = TokenLine[0]= ' ';
    TokenLine[1] = '\0';
    return TokenLine;
  }
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
      *t++ = (isupper((int)LastChar) ? tolower((int)LastChar) : LastChar);
      /* *t++= (isupper(ch)?tolower(ch):ch);  */
      /* EBCDIC support ! HD */
      NextChar();
    } while (strchr(TokenChar,LastChar));

    if (p >= (InputLine+sizeof(InputLine)))
      FtlMsg(InpLineOverflow);

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
    IoErrorMsg(WrongChar,LastChar);
    LastChar = TokenLine[0]= ' ';
    TokenLine[1] = '\0';
    return TokenLine;
  }
}

static boolean sncmp(char const *a, char const *b)
{
  while (*b)
  {
    if ((isupper((int const)*a) ? tolower((int const)*a) : *a) == *b++)
      /* EBCDIC support ! HD */
      ++a;
    else
      return false;
  }

  return true;
}

static unsigned int GetIndex(unsigned int index, unsigned int limit,
                             char const * const *list, char const *tok)
{
  while (index<limit)
    if (sncmp(list[index],tok))
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

Token StringToToken(char const *tok)
{
  return GetUniqIndex(TokenCount,TokenTab,tok);
}

/* read into InputLine until the next1 end of line */
void ReadToEndOfLine(void)
{
  char *p = InputLine;

  do
  {
    NextChar();
  } while (strchr(LineSpaceChar,LastChar));

  while (LastChar!='\n')
  {
    *p++ = LastChar;
    NextChar();
  }

  if (p >= (InputLine + sizeof(InputLine)))
    FtlMsg(InpLineOverflow);

  *p = '\0';
}

void ReadRemark(void)
{
  if (LastChar != '\n')
  {
    ReadToEndOfLine();
    protocol_fprintf(stdout,"%s",InputLine);
    Message(NewLine);
    protocol_fflush(stdout);
  }
}
