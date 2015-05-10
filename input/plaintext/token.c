#include "input/plaintext/token.h"
#include "output/output.h"
#include "output/plaintext/message.h"
#include "output/plaintext/protocol.h"
#include "utilities/boolean.h"
#include "platform/tmpfile.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char SpaceChar[] = " \t\n\r;.,";
static char LastChar = ' ';

char TokenLine[LINESIZE];    /* This array contains the lowercase input */

static char savedTokenLine[LINESIZE];

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

char const **GlobalTokenTab; /* set according to language */

char const *GlobalTokenString[LanguageCount][GlobalTokenCount] =
{
  { /* francais */
    /* 0*/  "DebutProbleme"
  },
  { /* Deutsch */
    /* 0*/  "AnfangProblem"
  },
  { /* english */
    /* 0*/  "beginproblem"
  }
};

char const **ProblemTokenTab; /* set according to language */

char const *ProblemTokenString[LanguageCount][ProblemTokenCount] =
{
  { /* francais */
    /* 0*/  "FinProbleme",
    /* 1*/  "asuivre"
  },
  { /* Deutsch */
    /* 0*/  "EndeProblem",
    /* 1*/  "WeiteresProblem"
  },
  { /* english */
    /* 0*/  "endproblem",
    /* 1*/  "nextproblem"
  }
};

char const **EndTwinTokenTab; /* set according to language */

char const *EndTwinTokenString[LanguageCount][EndTwinTokenCount] =
{
  { /* francais */
    /* 0*/  "jumeau",
    /* 1*/  "zeroposition"
  },
  { /* Deutsch */
    /* 0*/  "Zwilling",
    /* 1*/  "NullStellung"
  },
  { /* english */
    /* 0*/  "twin",
    /* 1*/  "zeroposition"
  }
};

char const **InitialTwinTokenTab; /* set according to language */

char const *InitialTwinTokenString[LanguageCount][InitialTwinTokenCount] =
{
  { /* francais */
    /* 0*/  "remarque",
    /* 1*/  "senonce",
    /* 2*/  "auteur",
    /* 3*/  "source",
    /* 4*/  "pieces",
    /* 5*/  "condition",
    /* 6*/  "option",
    /* 7*/  "enonce",
    /* 8*/  "protocol",
    /* 9*/  Sep,
    /*10*/  "titre",
    /*11*/  "LaTeX",
    /*12*/  "PiecesLaTeX",
    /*13*/  "prix",
    /*14*/  "PositionInitialPartie",
    /*15*/  "Forsyth"
  },
  { /* Deutsch */
    /* 0*/  "Bemerkung",
    /* 1*/  "sForderung",
    /* 2*/  "Autor",
    /* 3*/  "Quelle",
    /* 4*/  "Steine",
    /* 5*/  "Bedingung",
    /* 6*/  "Option",
    /* 7*/  "Forderung",
    /* 8*/  "Protokoll",
    /* 9*/  Sep,
    /*10*/  "Titel",
    /*11*/  "LaTeX",
    /*12*/  "LaTeXSteine",
    /*13*/  "Auszeichnung",
    /*14*/  "PartieAnfangsStellung",
    /*15*/  "Forsyth"
  },
  { /* english */
    /* 0*/  "remark",
    /* 1*/  "sstipulation",
    /* 2*/  "author",
    /* 3*/  "origin",
    /* 4*/  "pieces",
    /* 5*/  "condition",
    /* 6*/  "option",
    /* 7*/  "stipulation",
    /* 8*/  "protocol",
    /* 9*/  Sep,
    /*10*/  "title",
    /*11*/  "LaTeX",
    /*12*/  "LaTeXPieces",
    /*13*/  "award",
    /*14*/  "InitialGameArray",
    /*15*/  "Forsyth"
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
    Input = fopen(s,"r");

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

void CloseInput(void)
{
  fclose(Input);

  if (InputMirror!=0 && InputMirror!=Input)
    fclose(InputMirror);
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
    output_plaintext_fatal_message(InpLineOverflow);

  *p = '\0';
}

void ReadRemark(void)
{
  if (LastChar != '\n')
  {
    ReadToEndOfLine();
    protocol_fprintf(stdout,"%s",InputLine);
    output_plaintext_message(NewLine);
    protocol_fflush(stdout);
  }
}
