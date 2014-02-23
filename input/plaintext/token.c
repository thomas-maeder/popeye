#include "input/plaintext/token.h"
#include "input/plaintext/input_stack.h"
#include "output/output.h"
#include "utilities/boolean.h"
#include "pymsg.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char SpaceChar[] = " \t\n\r;.,";
char LastChar;

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
    /* 3*/  "senonce",
    /* 4*/  "auteur",
    /* 5*/  "source",
    /* 6*/  "pieces",
    /* 7*/  "condition",
    /* 8*/  "option",
    /* 9*/  "remarque",
    /*10*/  "protocol",
    /*11*/  "entree",
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
    /* 3*/  "sForderung",
    /* 4*/  "Autor",
    /* 5*/  "Quelle",
    /* 6*/  "Steine",
    /* 7*/  "Bedingung",
    /* 8*/  "Option",
    /* 9*/  "Bemerkung",
    /*10*/  "Protokoll",
    /*11*/  "Eingabe",
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
    /* 3*/  "sstipulation",
    /* 4*/  "author",
    /* 5*/  "origin",
    /* 6*/  "pieces",
    /* 7*/  "condition",
    /* 8*/  "option",
    /* 9*/  "remark",
    /*10*/  "protocol",
    /*11*/  "input",
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

/* advance LastChar to the next1 input character */
void NextChar(void)
{
  static boolean eof = false;
  int const ch = getc(InputStack[NestLevel]);
  if (ch==-1)
  {
    if (eof)
    {
      if (PopInput()<0)
        FtlMsg(EoFbeforeEoP);
      NextChar();
    }
    eof = true;
    LastChar= ' ';
  }
  else
    LastChar = ch;
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
