/******************** MODIFICATIONS to pyio.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/01 SE   New Koeko conditions: GI-Koeko, AN-Koeko
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/05/17 SE   Changes to allow half-move specification for helpmates using 0.5 notation
 **                 Forsyth can be used as a twinning command
 **                 TakeMake name changed to Take&Make
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2007/01/28 NG   New stipulation: help-reflexmate (hr)
 **
 ** 2007/04/29 SE   Bugfix: not correctly parsing conditions listed after SAT
 **
 ** 2007/07/38 SE   Bugfix: Forsyth
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher  
 **
 ** 2007/01/28 SE   New condition: Annan Chess 
 **
 ** 2007/03/01 SE   Changed Twin Char to support larger no. of twins - old implementation
 **                 produced odd characters after a while; new just calls all twins after
 **                 z z1, z2, z3... etc. limited by sizeof int, I suppose
 **
 ** 2007/04/28 SE   Bugfix: parsing SAT followed by another condition 
 **
 ** 2007/06/01 SE   Bug fixes: Forsyth entry (fairy piece on a8)
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/12/26 SE   New piece type: Protean man (invent A.H.Kniest?)
 **
 ** 2008/01/02 NG   New condition: Geneva Chess 
 **
 ** 2008/01/11 SE   New variant: Special Grids 
 **
 ** 2008/01/13 SE   New conditions: White/Black Vaulting Kings 
 **
 ** 2008/01/24 SE   New variant: Gridlines  
 **
 ** 2008/02/19 SE   New condition: AntiKoeko  
 **
 **************************** End of List ******************************/

#if defined(macintosh)    /* is always defined on macintosh's  SB */
#   define SEGMIO
#   include "pymac.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "platform/maxtime.h"

/* This is pyio.c
** It comprises a new io-Module for popeye.
** The old io was awkward, nonsystematic and I didn't like it.
** Therefore here a new one.
** Despite the remarks in the README file, I give here three languages
** for the input: english, french, german. It can easily be extended
** if necessary.
*/

#if !defined(DATABASE)
static
#endif
char AlphaStip[20];

#define MAXNEST 10
#define UPCASE(c)   toupper(c)      /* (c+('A'-'a')) */
/* This is only correct, cause only lowercase letters are passed
   as arguments
*/

static char Sep[] = "\n";
/* All entries in this table have to be in lower case */
static char *TokenString[LangCount][TokenCount] = {
  { /* francais */
    /* 0*/  "DebutProbleme",
    /* 1*/  "FinProbleme",
    /* 2*/  "asuivre",
    /* 3*/  "enonce",
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

#define WCcentered    0
#define WCleft        1
#define WCLaTeX       2

void WriteConditions(int alignment);

/***** twinning ***** */

#define TwinMove     0
#define TwinExchange     1
#define TwinStip     2
#define TwinAdd      3
#define TwinRemove   4
#define TwinContinued    5
#define TwinRotate   6
#define TwinCond     7
#define TwinPolish   8
#define TwinMirror   9
#define TwinMirra1h1    10
#define TwinMirra1a8    11
#define TwinMirra1h8    12
#define TwinMirra8h1    13
#define TwinShift   14
#define TwinSubstitute  15
#define TwinForsyth 16
#define TwinCount   17

char    *TwinString[LangCount][TwinCount] = {
  { /* francais */
    /* 0*/  "deplacement",
    /* 1*/  "echange",
    /* 2*/  "enonce",
    /* 3*/  "ajoute",
    /* 4*/  "ote",
    /* 5*/  "enplus",
    /* 6*/  "rotation",
    /* 7*/  "condition",
    /* 8*/  "CouleurEchange",
    /* 9*/  "miroir",
    /*10*/  "a1<-->h1",
    /*11*/  "a1<-->a8",
    /*12*/  "a1<-->h8",
    /*13*/  "a8<-->h1",
    /*14*/  "translation",
    /*15*/  "remplace",
    /*16*/  "forsyth"
  },
  { /* German */
    /* 0*/  "versetze",
    /* 1*/  "tausche",
    /* 2*/  "Forderung",
    /* 3*/  "hinzufuegen",
    /* 4*/  "entferne",
    /* 5*/  "ferner",
    /* 6*/  "Drehung",
    /* 7*/  "Bedingung",
    /* 8*/  "Farbwechsel",
    /* 9*/  "Spiegelung",
    /*10*/  "a1<-->h1",
    /*11*/  "a1<-->a8",
    /*12*/  "a1<-->h8",
    /*13*/  "a8<-->h1",
    /*14*/  "Verschiebung",
    /*15*/  "ersetze",
    /*16*/  "forsyth"
  },
  { /* English */
    /* 0*/  "move",
    /* 1*/  "exchange",
    /* 2*/  "stipulation",
    /* 3*/  "add",
    /* 4*/  "remove",
    /* 5*/  "continued",
    /* 6*/  "rotate",
    /* 7*/  "condition",
    /* 8*/  "PolishType",
    /* 9*/  "mirror",
    /*10*/  "a1<-->h1",
    /*11*/  "a1<-->a8",
    /*12*/  "a1<-->h8",
    /*13*/  "a8<-->h1",
    /*14*/  "shift",
    /*15*/  "substitute",
    /*16*/  "forsyth"
  }
};

/***** twinning ***** end */

static char **TokenTab; /* set according to language */
static char **OptTab;   /* set according to language */
static char **CondTab;  /* set according to language */

static char  **VariantTypeTab;
static char    **ExtraCondTab;
static char **TwinTab;
static char LastChar;

static  FILE    *TraceFile;
static  FILE    *InputStack[MAXNEST];

static  FILE    *LaTeXFile, *SolFile;
static    char *LaTeXPiecesAbbr[PieceCount];
static    char *LaTeXPiecesFull[PieceCount];
void LaTeXStr(char *line);
char *LaTeXStdPie[8] = { NULL, "C", "K", "B", "D", "S", "T", "L"};

static  int NestLevel=0;

extern echiquier ProofBoard, PosA;
extern square Proof_rb, Proof_rn, rbA, rnA;
extern Flags ProofSpec[64], SpecA[64];
extern imarr  isquareA;
boolean OscillatingKingsColour;  /* actually couleur but this is all a hack */
static nocontactfunc_t *nocontactfunc;

void    OpenInput(char *s)
{
  if((InputStack[0] = fopen(s,"r")) == NULL)
  {
    InputStack[0] = stdin;
  }
}

void    CloseInput(void)
{
  if(InputStack[0] != stdin)
  {
    fclose(InputStack[0]);
  }
}

/* These two arrays should have the same size */
#define LINESIZE    256

static char InputLine[LINESIZE];    /* This array contains the input as is */
static char TokenLine[LINESIZE];    /* This array contains the lowercase input */

static char SpaceChar[] = " \t\n\r;,.:";
static char TokenChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ#=+-%>!.<()~/&";
/* Steingewinn ! */
/* introductory move */
/* h##! */
/* dia3.5 */
/* a1<-->h1  */
/* reci-h(=)#n */
/* h~2  do ANY helpmove */

static char CharChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char SepraChar[] = "\n\r;,.:";

void pyfputc(char c, FILE *f)
{
#if !defined(QUIET)
  fputc(c,f);
  fflush(f);
  if (TraceFile) {
    fputc(c,TraceFile);
    fflush(TraceFile);
  }
  if (SolFile) {
    fputc(c,SolFile);
    fflush(SolFile);
  }
#endif
}

void pyfputs(char const *s, FILE *f)
{
#if !defined(QUIET)
  fputs(s,f);
  fflush(f);
  if (TraceFile) {
    fputs(s,TraceFile);
    fflush(TraceFile);
  }
  if (SolFile) {
    fputs(s,SolFile);
    fflush(SolFile);
  }
#endif
}

void StdChar(char c)
{
  pyfputc(c, stdout);
}

void ErrChar(char c)
{
  pyfputc(c, stderr);
}

void StdString(char const *s)
{
  pyfputs(s, stdout);
}

void ErrString(char const *s)
{
  pyfputs(s, stderr);
}

static void IoErrorMsg(int n, int val)
{
  ErrorMsg(InputError);
  logIntArg(val);
  ErrorMsg(n);
  ErrChar('\n');
  logStrArg(InputLine);
  ErrorMsg(OffendingItem);
  ErrChar('\n');
}

static int PushInput(char *FileName) {
  if ((NestLevel+1) < MAXNEST) {
    if ((InputStack[NestLevel + 1]= fopen(FileName,"r")) == NULL) {
      IoErrorMsg(RdOpenError,0);
      return -1;
    }
    else {
      NestLevel++;
      return 0;
    }
  }
  else {
    IoErrorMsg(TooManyInputs,0);
    return -1;
  }
}

static int PopInput() {
  fclose(InputStack[NestLevel]);
  if (NestLevel--)
    return 0;
  else
    return -1;
}

static char NextChar(void) {
  int ch;
  static boolean eof= False;

  if ((ch= getc(InputStack[NestLevel])) != -1)
    return LastChar=ch;
  else {
    if (eof) {
      if (PopInput() < 0)
        FtlMsg(EoFbeforeEoP);
      return NextChar();
    }
    eof= True;
    return LastChar= ' ';
  }
}

static char *ReadToEndOfLine(void) {
  char   ch,*p;

  while (strchr(SpaceChar,ch= NextChar()));
  *(p= InputLine)= ch;
  while ((*++p= NextChar()) != '\n');
  if (p >= (InputLine + sizeof(InputLine)))
    FtlMsg(InpLineOverflow);
  *p= '\0';
  return InputLine;
}

static char *ReadNextTokStr(void) {
  char ch,*p,*t;

  ch= LastChar;
  while (strchr(SpaceChar,ch))
    ch= NextChar();
  if (strchr(TokenChar,ch)) {
    p= InputLine;
    t= TokenLine;
    do {
      *p++= ch;
      /*           *t++= (isupper(ch)?tolower(ch):ch);  */
      *t++= (isupper((int)ch)?tolower((int)ch):ch);
      /* EBCDIC support ! HD */
    } while (strchr(TokenChar,ch= NextChar()));
    if (p > (InputLine+sizeof(InputLine)))
      FtlMsg(InpLineOverflow);
    *t= *p= '\0';
    return TokenLine;
  }
  if (strchr(SepraChar,ch)) {
    while (strchr(SepraChar,NextChar()));
    return Sep;
  }
  IoErrorMsg(WrongChar, ch);
  LastChar= TokenLine[0]= ' ';
  TokenLine[1]= '\0';
  return TokenLine;
}

static char *ReadNextCaseSensitiveTokStr(void) {
  char ch,*p,*t;

  ch= LastChar;
  while (strchr(" \t\n\r;,:",ch))  /* SpaceChar minus '.' which can be first char of extended Forsyth */
    ch= NextChar();
  if (strchr(TokenChar,ch)) {
    p= InputLine;
    t= TokenLine;
    do {
      *p++= ch;
      /*           *t++= (isupper(ch)?tolower(ch):ch);  */
      *t++= ch;
      /* EBCDIC support ! HD */
    } while (strchr(TokenChar,ch= NextChar()));
    if (p > (InputLine+sizeof(InputLine)))
      FtlMsg(InpLineOverflow);
    *t= *p= '\0';
    return TokenLine;
  }
  if (strchr(SepraChar,ch)) {
    while (strchr(SepraChar,NextChar()));
    return Sep;
  }
  IoErrorMsg(WrongChar, ch);
  LastChar= TokenLine[0]= ' ';
  TokenLine[1]= '\0';
  return TokenLine;
}

boolean sncmp(char *a, char *b) {
  while (*b) {
    /*  if ((isupper(*a)?tolower(*a):*a) != *b++) { */
    if ((isupper((int)*a)?tolower((int)*a):*a) != *b++) {
      /* EBCDIC support ! HD */
      return False;
    }
    a++;
  }
  return True;
}

int GetIndex(int indexx,int limit, char **list, char *tok) {
  while (indexx < limit) {
    if (sncmp(list[indexx],tok)) {
      return indexx;
    }
    indexx++;
  }
  return -1;
}

int GetUniqIndex(int limit, char **list, char *tok) {
  int indexx;

  if ((indexx= GetIndex(0,limit,list,tok)) >= 0) {
    if (   strlen(tok) == strlen(list[indexx])
           || GetIndex(indexx + 1,limit,list,tok) < 0)
    {
      return indexx;
    }
    else {
      return -1;
    }
  }
  else {
    return -2;
  }
}

Token StringToToken(char *tok)
{
  return GetUniqIndex(TokenCount,TokenTab,tok);
}

long int ReadBGLNumber(char* inptr, char** endptr)
{
  /* input must be of form - | {d}d(.|,(d(d))) where d=digit ()=0 or 1 {}=0 or more 
     in - and all other cases return infinity (no limit) */
  char buf[12];
  int res= BGL_infinity;
  int len;
  int dp;
  char* dpp;
  *endptr= inptr;
  while (**endptr && strchr("0123456789.,-", **endptr))
    /* isdigit(**endptr) || **endptr == '.' || **endptr == ',' || **endptr == '-')) */
    (*endptr)++;
  len= *endptr-inptr;
  if (len > 11)
    return res;
  strncpy(buf, inptr, len);
  buf[len]= '\0';
  if (len == 1 && buf[0] == '-')
    return res;
  for (dpp=buf; *dpp; dpp++)
    if (*dpp == ',')  /* allow 3,45 notation */
      *dpp= '.';
  for (dpp=buf; *dpp && *dpp != '.'; dpp++);
  dp= len-(int)(dpp-buf);
  if (!dp)
    return 100*(long int)atoi(buf);
  while (dpp-buf < len) {
    *dpp=*(dpp+1); 
    dpp++;
  }
  for (dpp=buf; *dpp; dpp++)
    if (*dpp == '.') 
      return res;  /* 2 d.p. characters */
  switch (dp) /* N.B> d.p. is part of count */
  {
  case 1 :
    return 100*(long int)atoi(buf);
  case 2 :
    return 10*(long int)atoi(buf);
  case 3 :
    return (long int)atoi(buf);
  default :
    return res;
  }
}

char* WriteBGLNumber(char* buf, long int num)
{
  if (num == BGL_infinity)
    sprintf(buf, "-");
  else if (num % 100 == 0)
    sprintf(buf, "%i", (int) (num / 100));
  else if (num % 10 == 0)
    sprintf(buf, "%i.%1i", (int) (num / 100), (int) ((num % 100) / 10));
  else
    sprintf(buf, "%i.%.2i", (int) (num / 100), (int) (num % 100));
  return buf;
}

static void ReadBeginSpec(void) {
  char *tok;

  while (True) {
    TokenTab= TokenString[0];
    tok= ReadNextTokStr();
    for (ActLang= 0; ActLang<LangCount; ActLang++) {
      TokenTab= &(TokenString[ActLang][0]);
      if (GetUniqIndex(TokenCount,TokenTab,tok) == BeginProblem) {
        OptTab= &(OptString[ActLang][0]);
        CondTab= &(CondString[ActLang][0]);
        TwinTab= &(TwinString[ActLang][0]);
        VariantTypeTab= &(VariantTypeString[ActLang][0]);
        ExtraCondTab= &(ExtraCondString[ActLang][0]);
        PieceTab= PieNamString[ActLang];
        PieSpTab= PieSpString[ActLang];
        InitMsgTab(ActLang, True);
        return;
      }
    }
    IoErrorMsg(NoBegOfProblem, 0);
  }
}

int GetPieNamIndex(char a,char b) {
  /* We search the array PieNam, for an index, where
     it matches the two characters a and b
  */
  int indexx;
  char *ch;

  ch= PieceTab[2];
  for (indexx= 2;
       indexx<PieceCount;
       indexx++,ch+= sizeof(PieceChar))
  {
    if (*ch == a && *(ch + 1) == b) {
      return indexx;
    }
  }
  return 0;
}

int SquareNum(char a,char b)
{
  if ('a' <= a && a <= 'h' && '1' <= b && b <= '8')
    return bas + a - 'a' + (b - '1') * onerow;
  else
    return 0;
}

/* All following Parse - Procedures return the next StringToken or
** the last token they couldn't interprete.
*/

static char *ParseLaTeXPieces(char *tok) {
  PieNam Name;
  int i;

  /* don't delete all this. Since both arrays are declared static,
   * they are initialized to NULL.
   * Simply allow overwriting these definitions within the LaTeX clause
   * and let it be initialized from pie-<lang>.dat
   *
   for (Name= 1; Name < PieceCount; Name++) {
   if (LaTeXPiecesAbbr[Name]) {
   free(LaTeXPiecesAbbr[Name]);
   free(LaTeXPiecesFull[Name]);
   }
   LaTeXPiecesAbbr[Name]= NULL;
   LaTeXPiecesFull[Name]= NULL;
   }
  */

  if (strlen(tok) < 3) {
    while (true) {
      Name= GetPieNamIndex(*tok, strlen(tok) == 1 ? ' ' : tok[1]);

      if (Name < King) {
        return tok;
      }

      if (LaTeXPiecesAbbr[Name]) {
        free(LaTeXPiecesAbbr[Name]);
        free(LaTeXPiecesFull[Name]);
      }

      tok= ReadNextTokStr();
      LaTeXPiecesAbbr[Name]= (char *)malloc(sizeof(char)*(strlen(tok)+1));
      i= 0;
      while (tok[i]) {
        /* to avoid compiler warnings below made "better readable" */
        /*      LaTeXPiecesAbbr[Name][i]= tok[i++]+ 'A' - 'a';          */
        LaTeXPiecesAbbr[Name][i]= tok[i] + 'A' - 'a';
        i++;
      }
      LaTeXPiecesAbbr[Name][i]= tok[i];

      tok= ReadToEndOfLine();
      LaTeXPiecesFull[Name]= (char *)malloc(sizeof(char)*(strlen(tok)+1));
      strcpy(LaTeXPiecesFull[Name], tok);

      tok= ReadNextTokStr();
    }
  }

  return tok;
}

char *LaTeXPiece(piece Name) {
  Name= abs(Name);

  if (Name > Bishop) {
    if (LaTeXPiecesAbbr[Name] == NULL) {
      ErrorMsg(UndefLatexPiece);
      return "??";
    } else
      return LaTeXPiecesAbbr[Name];
  } else
    return LaTeXStdPie[Name];
} /* LaTeXPiece */

static char *ParseSquareList(
  char  *tok,
  PieNam    Name,
  Flags Spec,
  char  echo)
{
  /* We interprete the tokenString as SquareList
     If we return always the next tokenstring
  */
  int     Square, SquareCnt= 0;

  while (True) {
    if (*tok && tok[1] && (Square=SquareNum(*tok,tok[1]))) {
      if (e[Square] != vide) {
        if (!echo) {
          WriteSquare(Square);
          StdChar(' ');
          Message(OverwritePiece);
        }
        if (Square == rb) {
          rb= initsquare;
        }
        if (Square == rn) {
          rn= initsquare;
        }
      }
      /* echo the piece if desired -- twinning */
      if (echo) {
        if (LaTeXout) {
          sprintf(GlobalStr,
                  "%s\\%c%s %c%c",
                  e[Square] == vide ? "+" : "",
                  TSTFLAG(Spec, Neutral)
                  ? 'n'
                  : TSTFLAG(Spec, White) ? 'w' : 's',
                  LaTeXPiece(Name),
                  'a'-nr_of_slack_files_left_of_board+Square%onerow,
                  '1'-nr_of_slack_rows_below_board+Square/onerow);
          strcat(ActTwin, GlobalStr);
        }
        if (e[Square] == vide) {
          StdChar(echo);
        }
        WriteSpec(Spec, Name!=vide);
        WritePiece(Name);
        WriteSquare(Square);
        StdChar(' ');
      }
      if (TSTFLAG(Spec, Neutral)) {
        Spec |= BIT(Black) + BIT(White);
      }
      spec[Square] = Spec;
      e[Square] = TSTFLAG(Spec,White) ? Name : -Name;
      tok+= 2;
      SquareCnt++;
      continue;
    }
    if (SquareCnt) {
      if (*tok) {
        ErrorMsg(WrongSquareList);
      }
      return ReadNextTokStr();
    }
    ErrorMsg(MissngSquareList);
    tok= ReadNextTokStr();
  }
}

static char *PrsPieShortcut(boolean onechar, char *tok, PieNam *pienam) {
  if (onechar) {
    *pienam= GetPieNamIndex(*tok,' ');
    tok++;
  }
  else {
    *pienam= GetPieNamIndex(*tok,tok[1]);
    tok+= 2;
  }

  return tok;
}

HunterType huntertypes[maxnrhuntertypes];
unsigned int nrhuntertypes;

static PieNam MakeHunterType(PieNam away, PieNam home) {
  unsigned int i;
  for (i = 0; i!=nrhuntertypes; ++i)
    if (huntertypes[i].away==away && huntertypes[i].home==home)
      return Hunter0+i;

  if (nrhuntertypes<maxnrhuntertypes) {
    PieNam const result = Hunter0+nrhuntertypes;
    HunterType * const huntertype = huntertypes+nrhuntertypes;
    huntertype->away = away;
    huntertype->home = home;
    ++nrhuntertypes;
    return result;
  }
  else {
    IoErrorMsg(HunterTypeLimitReached,maxnrhuntertypes);
    return Invalid;
  }
}

static char *PrsPieNam(char *tok, Flags Spec, char echo)
{
  /* We read from tok the name of the piece */
  int     l, NameCnt= 0;
  char    *btok;
  PieNam  Name;

  while (True) {
    char const * const hunterseppos = strchr(tok,hunterseparator);
    btok = tok; /* Save it, if we want to return it */
    if (hunterseppos!=0 && hunterseppos-tok<=2) {
      PieNam away, home;
      tok = PrsPieShortcut((hunterseppos-tok)&1,tok,&away);
      ++tok; /* skip slash */
      l= strlen(tok);
      tok = PrsPieShortcut(l&1,tok,&home);
      Name = MakeHunterType(away,home);
    }
    else {
      l= strlen(tok);
      tok = PrsPieShortcut(l&1,tok,&Name);
    }
    if (Name >= King) {
      if (l >= 3 && !strchr("12345678",tok[1]))
        return btok;
      /* We return here not the next tokenstring
      ** since this string is not a Men/Squarelist
      ** and therefore deserves processing by
      ** ParsePieSpec
      */
      NameCnt++;
      if (!*tok)
        tok= ReadNextTokStr();
      tok= ParseSquareList(tok, Name, Spec, echo);
      CLRFLAG(Spec, Royal);
    }
    else if (hunterseppos!=0)
      tok= ReadNextTokStr();
    else
      if (NameCnt > 0)
        return btok;
      else {
        IoErrorMsg(WrongPieceName,0);
        tok= ReadNextTokStr();
      }
  }
}

square NextSquare(square sq) {
  if (sq%onerow<nr_of_slack_files_left_of_board+nr_files_on_board-1)
    return sq+1;
  else if (sq>=square_a2 && sq<=haut)
    return sq - onerow - (nr_files_on_board-1);
  else
    return initsquare;
}

static square SetSquare(square sq, piece p, boolean bw, boolean *neut)
{
  e[sq]= bw ? -p : p;
  spec[sq]= bw ? BIT(Black) : BIT(White);
  if (*neut) {
    spec[sq]= BIT(Black) | BIT(White) | BIT(Neutral);
    e[sq] = p;  /* must be 'white' for neutral */
    SETFLAG(PieSpExFlags, Neutral);
  }
  *neut= false;
  return NextSquare(sq);
}

static char *ParseForsyth(boolean output) {
  piece pc;
  int num;
  square sq= square_a8;
  square *bnp;
  boolean NeutralFlag= false;
  char* tok= ReadNextCaseSensitiveTokStr();

  for (bnp= boardnum; *bnp; bnp++)
    e[*bnp]= vide;
  rb= rn= initsquare;

  sprintf(GlobalStr, "  %s  \n", tok);
  if (output)
  {
    StdString(tok);
  }
  while (sq && *tok) {
    if (isdigit((int)*tok)) {
      num= (*tok++) - '0';
      if (isdigit((int)*tok))
        num += num * 9 + (*tok++) - '0';
      for (;num && sq;num--) {
        e[sq]= vide;
        spec[sq]= BorderSpec;
        sq= NextSquare(sq);
      }
      NeutralFlag= false;
    }
    else if (isalpha((int)*tok)) {
      pc= GetPieNamIndex(tolower(*tok),' ');
      if (pc>=King) {
        sq= SetSquare(sq,
                      pc,
                      islower((int)InputLine[(tok++)-TokenLine]),
                      &NeutralFlag);
        if (NeutralFlag) 
          SETFLAG(PieSpExFlags,Neutral);
      }
      else {
        tok++;           /* error */
      }
    }
    else if (*tok == '.') {
      if (*(tok+1) == '=') {
        NeutralFlag= true;
        tok++;
      }
      pc= GetPieNamIndex(tolower(*(tok+1)), tolower(*(tok+2)));
      if (pc>=King) {
        sq= SetSquare(sq,
                      pc,
                      islower((int)InputLine[(tok+1-TokenLine)]),
                      &NeutralFlag);
        if (NeutralFlag) 
          SETFLAG(PieSpExFlags,Neutral);
      }
      tok += 3;
    }
    else if (*tok == '=') {
      NeutralFlag= true;
      tok++;
    }
    else
      tok++;
  }
  return ReadNextTokStr();
}

static char *ParsePieSpec(char echo) {
  /* We read the PieceSpecifikation.
  ** The first token we cannot interprete as PieceSpec
  ** When a color specification was suplied we try to
  ** parse the following MenList.
  ** If no ColorSpec is given, and we had already one, we return
  ** the token, to the caller.
  */
  PieSpec ps;
  Flags   PieSpFlags;
  int     SpecCnt= 0;
  char    *tok;
  Flags   ColorFlag= (BIT(White) | BIT(Black) | BIT(Neutral));

  tok= ReadNextTokStr();
  while (True) {
    CLEARFL(PieSpFlags);
    while ((ps= GetUniqIndex(PieSpCount,PieSpTab,tok)) >= -1) {
      if (ps == -1) {
        IoErrorMsg(PieSpecNotUniq,0);
      }
      else {
        Flags TmpFlg= PieSpFlags&ColorFlag;
        if (  TmpFlg
              && (TmpFlg & BIT(ps))
              && (TmpFlg != (Flags)BIT(ps)))
        {
          IoErrorMsg(WBNAllowed,0);
        }
        else {
          SETFLAG(PieSpFlags,ps);
          SETFLAG(PieSpExFlags,ps);
        }
      }
      tok= ReadNextTokStr();
    }
    if (PieSpFlags & ColorFlag) {
      tok= PrsPieNam(tok, PieSpFlags, echo);
      SpecCnt++;
    }
    else {
      if (SpecCnt) {
        return tok;
      }
      IoErrorMsg(NoColorSpec,0);
      tok= ReadNextTokStr();
    }
  }
}

static char *ParseFlow(char *tok) {
  /* seriesmovers with introducory moves */
  if (strstr(tok,"->")) {
    if ((introenonce= atoi(tok)) < 1) {
      IoErrorMsg(WrongInt, 0);
    }
    StipFlags |= FlowBit(Intro);
    tok= strstr(tok, "->")+2;
  }
  if (strncmp("exact-", tok, 6) == 0) {
    StipFlags|= FlowBit(Exact);
    OptFlag[nothreat] = True;
    tok+= 6;
  }
  if (strncmp("ser-", tok, 4) == 0) {
    StipFlags|= FlowBit(Series);
    tok+=4;
  }
  else {
    StipFlags|= FlowBit(Alternate);
  }
  if (strncmp("semi-", tok, 5) == 0) {
    StipFlags|= FlowBit(Semi);
    return tok+5;
  }
  if (strncmp("reci-", tok, 5) == 0) {
    StipFlags|= FlowBit(Reci);
    return tok+5;
  }
  /* proof games  V3.35  TLi */
  if (strncmp("dia", tok, 3) == 0) {
    StipFlags|= FlowBit(Exact);
    StipFlags|= SortBit(Proof);
    strcpy(stipSettings[nonreciprocal].alphaEnd, " dia");
    strcpy(currentStipSettings.alphaEnd," dia");
    return tok+3;
  }
#if !defined(DATABASE)
  /* transform position a into position b */
  if (strncmp("a=>b", tok, 4) == 0) {
    int i;
    StipFlags|= SortBit(Proof);
    strcpy(stipSettings[nonreciprocal].alphaEnd, " a=>b");
    strcpy(currentStipSettings.alphaEnd," a=>b");
    for (i=maxsquare-1; i>=0; i--) {
      PosA[i]=e[i];
    }
    for (i= 0; i< 64; i++) {
      SpecA[i]=spec[boardnum[i]];
      spec[i]= EmptySpec;
      e[boardnum[i]]= vide;
    }
    rnA=rn;
    rbA=rb;
    rn=rb=initsquare;
    for (i= 0; i < maxinum; i++) {
      isquareA[i]= isquare[i];
      isquare[i]= initsquare;
    }
    flag_atob= true;
    return tok+4;
  }
#endif
  return tok;
}

static char *ParseSort(char *tok)
{
  if (SortFlag(Proof))   /* proof gamesd */
    return tok;

  switch (*tok) {
  case 'h':
    StipFlags|= SortBit(Help);
    if (*(++tok) == 's') {
      StipFlags|= SortBit(Self);
      return tok+1;
    } else
      if (*tok == 'r') {
        StipFlags|= SortBit(Reflex);
        StipFlags|= SortBit(Self);
        return tok+1;
      } else
        return tok;
  case 'r':
    StipFlags|= SortBit(Reflex);
    StipFlags|= SortBit(Self);
    return tok+1;
  case 's':
    StipFlags|= SortBit(Self);
    return tok+1;
  default:
    StipFlags|= SortBit(Direct);
    return tok;
  }
}

/* map input strings to goals */
typedef struct
{
    char const *inputText;
    Stipulation goal;
    char const *outputText;
 } goalInputConfig_t;

/* make sure that input strings that are substrings of other strings
 * appear *after* them! */
static goalInputConfig_t const goalInputConfig[nr_stipulations] =
{
  { "##!", stip_countermate,   " ##!" },
  { "##",  stip_doublemate,    " ##"  },
  { "#=",  stip_mate_or_stale, " #="  },
  { "#",   stip_mate,          " #"   },
  { "==",  stip_dblstale,      " =="  },
  { "!=",  stip_autostale,     " !="  },
  { "=",   stip_stale,         " ="   },
  { "z",   stip_target,        " z"   },
  { "+",   stip_check,         " +"   },
  { "x",   stip_capture,       " x"   },
  { "%",   stip_steingewinn,   " %"   },
  { "ep",  stip_ep,            ""     },
  { "ctr", stip_circuitB,      ""     },
  { "ct",  stip_circuit,       ""     },
  { "<>r", stip_exchangeB,     ""     },
  { "<>",  stip_exchange,      ""     },
  { "00",  stip_castling,      ""     },
  { "~",   stip_any,           ""     }
};

static char *ParsPartialGoal(char *tok, stipSettings_t *settings) {
  goalInputConfig_t const *gic;
  for (gic = goalInputConfig; gic!=goalInputConfig+nr_stipulations; ++gic)
    if (strstr(tok,gic->inputText)==tok) {
      settings->stipulation = gic->goal;
      strcpy(settings->alphaEnd,gic->outputText);

      if (gic->goal==stip_target) {
        settings->targetSquare= SquareNum(tok[1],tok[2]);
        if (settings->targetSquare==0) {
          IoErrorMsg(MissngSquareList, 0);
          return 0;
        }
        else
          return tok+3;
      }
      else
        return tok+strlen(gic->inputText);
    }

  IoErrorMsg(UnrecStip, 0);
  return 0;
}

static char *ParseGoal(char *tok) {
  if (SortFlag(Proof))
    return tok;

  stipSettings[reciprocal].stipulation= no_stipulation;

  /* test for reciprocal help play with different ends for Black and
   * White; e.g. reci-h(=)#3 */
  if (FlowFlag(Reci) && *tok == '(') {
    char const *closingParenPos = strchr(tok,')');
    if (closingParenPos!=0) {
      tok = ParsPartialGoal(tok+1,&stipSettings[reciprocal]);
      if (tok==0)
        return 0;
      else if (tok==closingParenPos)
        ++tok;
      else {
        IoErrorMsg(UnrecStip, 0);
        return 0;
      }
    }
  }

  return ParsPartialGoal(tok,&stipSettings[nonreciprocal]);
}

static char *ParseStip(void) {
  char *tok;

  StipFlags= 0;
  tok= ReadNextTokStr();
  strcpy(AlphaStip,tok);
  tok= ParseGoal(ParseSort(ParseFlow(tok)));
  if (tok) {
    /* set defaults */
    currentStipSettings = stipSettings[nonreciprocal];

    /* set reci stip if not parsed */
    if (FlowFlag(Reci) && stipSettings[reciprocal].stipulation==no_stipulation)
      stipSettings[reciprocal] = stipSettings[nonreciprocal];

    if (!*tok) {
      tok= ReadNextTokStr();
      strcat(AlphaStip, tok);
    }
    if (!(SortFlag(Proof) && FlowFlag(Alternate))) {
      if ((enonce=atoi(tok)) < 1)
        IoErrorMsg(WrongInt, 0);
      if (SortFlag(Help) && FlowFlag(Alternate)) {
        while (*tok && '0' <= *tok && *tok <= '9')
          tok++;
        flag_appseul= (tok && *tok == '.' && (tok+1) && *(tok+1) == '5'); 
        if (flag_appseul)
          enonce++;
      }
    } else {
      if ((enonce=2*atoi(tok)) < 0)
        IoErrorMsg(WrongInt, 0);
      while (*tok && '0' <= *tok && *tok <= '9')
        tok++;
      if (tok && *tok == '.' && (tok+1) && *(tok+1) == '5')
        enonce++;
    }
  }
  if (enonce && ActStip[0] == '\0')
    strcpy(ActStip, AlphaStip);
  return ReadNextTokStr();
}

static char *ReadSquares(int which) {
  char     *tok = ReadNextTokStr();
  char *lastTok = tok;
  square   i;
  int    k, l;
  ply      n;
  int EpSquaresRead= 0;

  l=strlen(tok);
  if (l&1) {
    if (which != ReadFrischAuf && which != ReadGrid) {
      IoErrorMsg(WrongSquareList, 0);
    }
    currentgridnum = 0;
    return tok;
  }
  k= 0;
  while (*tok) {
    i= SquareNum(*tok, tok[1]);
    if (i != 0) {
      switch (which) {
      case ReadFrischAuf:
        k++;
        if (e[i] == vide || e[i] == obs || is_pawn(e[i])) {
          Message(NoFrischAufPromPiece);
        }
        else {
          SETFLAG(spec[i], FrischAuf);
        }
        break;

      case ReadImitators:
        isquare[k++]= i;
        break;

      case ReadHoles:
        e[i]= obs;
        break;

      case ReadEpSquares:
        switch (EpSquaresRead++) {
        case 0:
          ep[1]= i;
          break;
        case 1:
          ep2[1]= i;
          break;
        default:
          Message(ToManyEpKeySquares);
        }
        break;

      case ReadBlRoyalSq:
        bl_royal_sq= i;
        break;

      case ReadWhRoyalSq:
        wh_royal_sq= i;
        break;

      case ReadNoCastlingSquares:
        switch (i) {
        case square_a1:
          CLRFLAGMASK(no_castling,ra1_cancastle);
          break;
        case square_e1:
          CLRFLAGMASK(no_castling,ke1_cancastle);
          break;
        case square_h1:
          CLRFLAGMASK(no_castling,rh1_cancastle);
          break;
        case square_a8:
          CLRFLAGMASK(no_castling,ra8_cancastle);
          break;
        case square_e8:
          CLRFLAGMASK(no_castling,ke8_cancastle);
          break;
        case square_h8:
          CLRFLAGMASK(no_castling,rh8_cancastle);
          break;
        default:
          break;
        }
        break;

      case ReadGrid:
        k++;
        ClearGridNum(i);
        sq_spec[i]+= currentgridnum << Grid;
      break;  

      default:
        SETFLAG(sq_spec[i], which);
        break;
      }
      tok+= 2;
    }
    else {
      if (which == ReadGrid || k != 0) {
        currentgridnum=0;
        return lastTok;
      }
      if (which != ReadFrischAuf || k != 0) {
        IoErrorMsg(WrongSquareList, 0);
        return tok;
      }
    }
  }
  if (which == ReadImitators) {
    for (n= 1; n <= maxply; n++) {
      inum[n]= k;
    }
  }

  /* This is an ugly hack, but due to the new feature ReadFrischAuf,
     we need the returning of the token tok, and this leads to
     mistakes with other conditions that need reading
  */
  /* of a SquareList. */
  if (which == ReadFrischAuf) {
    tok = ReadNextTokStr();
  }
  return tok;
} /* ReadSquares */

static char *ParseRex(boolean *rex, Cond what) {
  char    *tok;
  tok= ReadNextTokStr();
  *rex= (what==GetUniqIndex(CondCount, CondTab, tok));
  if (*rex) {
    tok= ReadNextTokStr();
  }
  return tok;
}

typedef enum
{
  gpType,
  gpSentinelles,
  gpAntiCirce,
  gpKoeko,
  gpOsc,
  gpAnnan,
  gpGrid
} VariantGroup;

static char *ParseMaximumPawn(unsigned int *result,
                              unsigned int defaultVal,
                              unsigned int boundary)
{
  char *tok= ReadNextTokStr();

  char *end;
  unsigned long tmp = strtoul(tok,&end,10);
  if (tok==end || tmp>boundary)
    *result = defaultVal;
  else
    *result = tmp;

  return end;
}

static char *ParseVariant(boolean *type, VariantGroup group) {
  int       VariantType;
  char    *tok=ReadNextTokStr();

  if (type != NULL)
    *type= False;

  do {
    VariantType = GetUniqIndex(VariantTypeCount, VariantTypeTab, tok);

    if (VariantType < -1) {
      break;
    }

    if (VariantType == -1) {
      IoErrorMsg(CondNotUniq,0);
    }
    else if (VariantType==TypeB && group==gpType) {
      *type= True;
    }
    else if (VariantType==TypeB && group==gpOsc) {
      OscillatingKingsTypeB[OscillatingKingsColour]= True;
    }
    else if (VariantType==TypeC && group==gpOsc) {
      OscillatingKingsTypeC[OscillatingKingsColour]= True;
    }
    else if (VariantType==TypeB && group==gpAnnan) {
      annanvar= 1;
    }
    else if (VariantType==TypeC && group==gpAnnan) {
      annanvar= 2;
    }
    else if (VariantType==TypeD && group==gpAnnan) {
      annanvar= 3;
    }
    else if (VariantType==Type1 && group==gpType) {
      SingleBoxType = singlebox_type1;
    }
    else if (VariantType==Type2 && group==gpType) {
      SingleBoxType = singlebox_type2;
    }
    else if (VariantType==Type3 && group==gpType) {
      SingleBoxType = singlebox_type3;
    }
    else if (VariantType==PionAdverse && group==gpSentinelles) {
      *type= True;
    }
    else if (VariantType==PionNeutral && group==gpSentinelles) {
      SentPionNeutral= True;
    }
    else if (VariantType==PionNoirMaximum && group==gpSentinelles) {
      tok = ParseMaximumPawn(&max_pn,8,64);
    }
    else if (VariantType==PionBlancMaximum && group==gpSentinelles)
    {
      tok = ParseMaximumPawn(&max_pb,8,64);
    }
    else if (VariantType==PionTotalMaximum && group==gpSentinelles)
    {
      tok = ParseMaximumPawn(&max_pt,16,64);
    }
    else if (VariantType==ParaSent && group==gpSentinelles) {
      flagparasent= true;
    }
    else if (VariantType==SentBerolina && group==gpSentinelles) {
      sentinelb= pbb;
      sentineln= pbn;
    }
    else if (VariantType==AntiCirTypeCheylan && group==gpAntiCirce)
    {
      *type= True;
    }
    else if (VariantType==AntiCirTypeCalvet && group==gpAntiCirce)
    {
      *type= False;
    }
    else if (VariantType==Neighbour && group==gpKoeko)
    {
      piece tmp_piece;
      switch (strlen(tok= ReadNextTokStr())) {
      case 1:
        tmp_piece= GetPieNamIndex(*tok,' ');
        break;
      case 2:
        tmp_piece= GetPieNamIndex(*tok,tok[1]);
        break;
      default:
        IoErrorMsg(WrongPieceName,0);
        return tok;
      }
      switch (tmp_piece) {
      case roib:
        break;
      case cb:
        *nocontactfunc= noknightcontact;
        break;
      case vizirb:
        *nocontactfunc= nowazircontact;
        break;
      case fersb:
        *nocontactfunc= noferscontact;
        break;
      case chb:
        *nocontactfunc= nocamelcontact;
        break;
      case alfilb:
        *nocontactfunc= noalfilcontact;
        break;
      case zb:
        *nocontactfunc= nozebracontact;
        break;
      case dabb:
        *nocontactfunc= nodabbabacontact;
        break;
      case gib:
        *nocontactfunc= nogiraffecontact;
        break;
      case antilb:
        *nocontactfunc= noantelopecontact;
        break;
      default:
        IoErrorMsg(WrongPieceName,0);
      }
    }
    else if (VariantType==ShiftRank && group==gpGrid)
    {
      square * bnp;
      for (bnp= boardnum; *bnp; bnp++) 
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 8)/2)+4*((*bnp/24-7)/2)) << Grid;
      }
      gridvar = grid_vertical_shift;
    }
    else if (VariantType==ShiftFile && group==gpGrid)
    {
      square * bnp;
      for (bnp= boardnum; *bnp; bnp++) 
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 7)/2)+5*((*bnp/24-8)/2)) << Grid;
      }
      gridvar = grid_horizontal_shift;
    }
    else if (VariantType==ShiftRankFile && group==gpGrid)
    {
      square * bnp;
      for (bnp= boardnum; *bnp; bnp++) 
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 7)/2)+5*((*bnp/24-7)/2)) << Grid;
      }
      gridvar = grid_diagonal_shift;
    }
    else if (VariantType==Orthogonal && group==gpGrid)
    {
      square * bnp;
      int files[8], ranks[8];
      int filenum=1;
      int i;
      char c;
      tok = ReadNextTokStr();
      for (i=0; i<8; i++)
        files[i]=ranks[i]=0;
      while ((c=*tok++))
      {
        if (c >= '1' && c <= '8')
        {
          for (i=(c-'1')+1; i<8; i++)
            ranks[i]++;
        }
        else if (tolower(c) >= 'a' && tolower(c) <= 'h')
        {
          for (i=(tolower(c)-'a')+1; i<8; i++)
          files[i]++;
          filenum++;
        }
        else
        {
          IoErrorMsg(CondNotUniq, 0);
          return tok;
        }
      }
      for (bnp= boardnum; *bnp; bnp++) 
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (files[*bnp%24-8]+filenum*ranks[*bnp/24-8]) << Grid;
      }        
      gridvar = grid_orthogonal_lines;
    }
    else if (VariantType==Irregular && group==gpGrid)
    {
      square * bnp;
      for (bnp= boardnum; *bnp; bnp++) 
      ClearGridNum(*bnp);
      gridvar = grid_irregular;
      currentgridnum=1;
      do 
      {
        tok=ReadSquares(ReadGrid);
      }
      while (currentgridnum++);
      continue;
    }
    else if (VariantType==ExtraGridLines && group==gpGrid)
    {
      boolean parsed= true;
      numgridlines= 0;
      while (parsed && numgridlines < 100)
      {
        tok = ReadNextTokStr();
        if (strlen(tok) == 4)
        {
          int f=0, r=0, l=0;
          boolean horiz=false;
          char c= tok[0];
          if (tolower(c) == 'h')
            horiz= true;
          else if (tolower(c) == 'v')
            horiz= false;
          else
            parsed= false;
          c= tok[1];
          if (tolower(c) >= 'a' && tolower(c) <= 'h')
            f= (tolower(c)-'a');
          else
            parsed= false;
          c= tok[2];
          if (c >= '1' && c <= '8')
            r=(c-'1');
          else
            parsed= false;
          c= tok[3];
          if (c >= '1' && c <= '8')
            l=(c-'0');
          else
            parsed= false;
          if (parsed)
          {
            gridlines[numgridlines][0]=2*f;
            gridlines[numgridlines][1]=2*r;
            gridlines[numgridlines][2]=2*f+(horiz?2*l:0);
            gridlines[numgridlines][3]=2*r+(horiz?0:2*l);
            numgridlines++;
            gridvar= grid_irregular;
          }
        }
        else
        {
          break;
        }
      }
      continue;
    }
    else {
      return tok;
    }
    tok= ReadNextTokStr();
  } while (group==gpSentinelles || group==gpGrid);

  return tok;
}

static char *ParseExact(boolean *ex_flag, boolean *ul_flag)
{
  char    *tok;

  tok= ReadNextTokStr();
  *ul_flag= (ultra==GetUniqIndex(CondCount, CondTab, tok));
  if (*ul_flag) {
    *ex_flag= true;
    CondFlag[ultra]= true;
    tok= ReadNextTokStr();
  }
  else {
    *ex_flag= exact==GetUniqIndex(CondCount, CondTab, tok);
    if (*ex_flag) {
      CondFlag[exact]= true;
      tok= ReadNextTokStr();
    }
  }
  return tok;
} /* ParseExact */

int  AntiCirType;
char ChameleonSequence[256];

static char *ParseVaultingPieces(Flags fl)
{
  piece p;
  char  *tok;
  int tp = 0;
  boolean gotpiece;

  while (True) {
    gotpiece = false;
    switch (strlen(tok= ReadNextTokStr())) {
    case 1:
      p= GetPieNamIndex(*tok,' ');
      gotpiece = true;
      break;

    case 2:
      p= GetPieNamIndex(*tok,tok[1]);
      gotpiece = true;
      break;

    default:
      switch (GetUniqIndex(VariantTypeCount, VariantTypeTab, tok)) {
        case Transmuting:
          if (TSTFLAG(fl, blanc)) 
            calc_whtrans_king= true;
          if (TSTFLAG(fl, noir)) 
            calc_bltrans_king= true;
          break;

        default:
          return tok;
      }
    }
    if (gotpiece)
    {
      if (TSTFLAG(fl, blanc)) {
        whitetransmpieces[tp] = p;
      }
      if (TSTFLAG(fl, noir)) {
        blacktransmpieces[tp] = p;
      }
      tp++;
      if (TSTFLAG(fl, blanc)) {
        whitetransmpieces[tp] = vide;
      }
      if (TSTFLAG(fl, noir)) {
        blacktransmpieces[tp] = vide;
      }
    }
  }
  return tok;
}

char *ReadChameleonCirceSequence(void) {
  piece old_piece, new_piece;
  char  *tok, newpiece[3];

  old_piece= vide;

  for (new_piece= vide; new_piece < PieceCount; new_piece++) {
    NextChamCircePiece[new_piece]= new_piece;
  }
  ChameleonSequence[0]= '\0';

  while (True) {
    switch (strlen(tok= ReadNextTokStr())) {
    case 1:
      new_piece= GetPieNamIndex(*tok,' ');
      break;

    case 2:
      new_piece= GetPieNamIndex(*tok,tok[1]);
      break;

    default:
      return tok;
    }
    if (!new_piece) {
      IoErrorMsg(WrongPieceName,0);
      break;
    }
    else {
      InitChamCirce= false;
      if (old_piece != vide) {
        NextChamCircePiece[old_piece]= new_piece;
        strcat(ChameleonSequence, "->");
      }
      old_piece= new_piece;
      sprintf(newpiece,
              "%c%c", UPCASE(*tok), tok[1] == ' ' ? '\0' :
              UPCASE(tok[1]));
      strcat(ChameleonSequence, newpiece);
    }
  }
  return tok;
} /* ReadChameleonCirceSequence */

static char *ParseCond(void) {
  char    *tok, *ptr;
  int     indexx;
  int     CondCnt= 0;

  tok= ReadNextTokStr();
  while ((indexx= GetUniqIndex(CondCount,CondTab,tok)) >= -2) {

    if (indexx == -2) {
      indexx= GetUniqIndex(ExtraCondCount,ExtraCondTab,tok);
      if (indexx == -2) {
        break;
      }
      switch (indexx) {
      case maxi:
        flagmaxi= true;
        tok= ParseExact(&CondFlag[exact], &CondFlag[ultra]);
        CondCnt++;
        break;

      case ultraschachzwang:
        flagultraschachzwang= true;
        tok= ReadNextTokStr();
        CondCnt++;
        break;

      case -1:
        IoErrorMsg(CondNotUniq,0);
        tok= ReadNextTokStr();
        break;
      }
      continue;
    }

    if (indexx == -1) {
      IoErrorMsg(CondNotUniq,0);
      tok= ReadNextTokStr();
      continue;
    }
    CondFlag[indexx]= True;

    CondCnt++;
    switch (indexx) {
    case rexincl:
      if (CondFlag[exact])
        IoErrorMsg(NonSenseRexiExact, 0);
      break;
    case biheffalumps:
      CondFlag[heffalumps]= True;
      CondFlag[biwoozles]= True;
      CondFlag[woozles]= True;
      break;
    case heffalumps:
      CondFlag[woozles]= True;
      break;
    case biwoozles:
      CondFlag[woozles]= True;
      break;
    case hypervolage:
      CondFlag[volage]= True;
      break;
    case leofamily:
      CondFlag[chinoises]= True;
      break;
    case eiffel:
      flag_madrasi= true;
      break;
    case contactgrid:
      CondFlag[gridchess]=
        CondFlag[koeko]= True;
      break;
    case imitators:
      ReadSquares(ReadImitators);
      break;
    case blroyalsq:
      ReadSquares(ReadBlRoyalSq);
      break;
    case whroyalsq:
      ReadSquares(ReadWhRoyalSq);
      break;
    case magicsquare:
      ReadSquares(MagicSq);
      break;
    case dbltibet:
      CondFlag[tibet]= True;
      break;
    case holes:
      ReadSquares(ReadHoles);
      break;
    case blmax:
      black_length= len_max;
      flagblackmummer= true;
      break;
    case blmin:
      black_length= len_min;
      flagblackmummer= true;
      break;
    case blcapt:
      black_length= len_capt;
      flagblackmummer= true;
      break;
    case blfollow:
      black_length= len_follow;
      flagblackmummer= true;
      break;
    case whmax:
      white_length= len_max;
      flagwhitemummer= true;
      break;
    case whmin:
      white_length= len_min;
      flagwhitemummer= true;
      break;
    case whcapt:
      white_length= len_capt;
      flagwhitemummer= true;
      break;
    case whfollow:
      white_length= len_follow;
      flagwhitemummer= true;
      break;
    case duellist:
      white_length= len_whduell;
      black_length= len_blduell;
      flagwhitemummer= flagblackmummer= true;
      break;
    case alphabetic:
      white_length= len_alphabetic;
      black_length= len_alphabetic;
      flagwhitemummer= flagblackmummer= true;
      break;
    case blacksynchron:
      black_length= len_synchron;
      flagblackmummer= true;
      flag_synchron= true;
      break;
    case whitesynchron:
      white_length= len_synchron;
      flagwhitemummer= true;
      flag_synchron= true;
      break;
    case blackantisynchron:
      black_length= len_antisynchron;
      flagblackmummer= true;
      flag_synchron= true;
      break;
    case whiteantisynchron:
      white_length= len_antisynchron;
      flagwhitemummer= true;
      flag_synchron= true;
      break;
    case trans_king:
      calc_whtrans_king= true;
      calc_bltrans_king= true;
      calc_whrefl_king= true;
      calc_blrefl_king= true;
      break;
    case refl_king:
      calc_whrefl_king= true;
      calc_blrefl_king= true;
      break;
    case whrefl_king:
      calc_whrefl_king= true;
      break;
    case blrefl_king:
      calc_blrefl_king= true;
      break;
    case whtrans_king:
      calc_whtrans_king= true;
      calc_whrefl_king= true;
      break;
    case bltrans_king:
      calc_blrefl_king= true;
      calc_blrefl_king= true;
      break;
    case whvault_king:
      calc_whrefl_king= true;
      whitenormaltranspieces = false;
      whitetransmpieces[0]= equib;
      whitetransmpieces[1]= vide;
      break;
    case blvault_king:
      calc_blrefl_king= true;
      blacknormaltranspieces = false;
      blacktransmpieces[0]= equib;
      blacktransmpieces[1]= vide;
    case vault_king:
      calc_whrefl_king= true;
      calc_blrefl_king= true;
      whitenormaltranspieces = false;
      blacknormaltranspieces = false;
      whitetransmpieces[0]= equib;
      blacktransmpieces[0]= equib;
      whitetransmpieces[1]= vide;
      blacktransmpieces[1]= vide;
      break;
    case whsupertrans_king:
      calc_whrefl_king= true;
      flagwhitemummer= true;
      break;
    case blsupertrans_king:
      calc_bltrans_king= true;
      calc_blrefl_king= true;
      flagblackmummer= true;
      break;
    case antieinstein:
      CondFlag[einstein]= true;
      break;
    case reveinstein:
      CondFlag[einstein]= true;
      break;
    case whforsqu:
      ReadSquares(WhForcedSq);
      white_length= len_whforcedsquare;
      flagwhitemummer= true;
      break;
    case blforsqu:
      ReadSquares(BlForcedSq);
      black_length= len_blforcedsquare;
      flagblackmummer= true;
      break;
    case whconforsqu:
      ReadSquares(WhConsForcedSq);
      wh_ultra=
        wh_exact= true;
      white_length= len_whforcedsquare;
      flagwhitemummer= true;
      break;
    case blconforsqu:
      ReadSquares(BlConsForcedSq);
      bl_ultra=
        bl_exact= true;
      black_length= len_blforcedsquare;
      flagblackmummer= true;
      break;
    case schwarzschacher:
      flagblackmummer= true;
      black_length= len_schwarzschacher;
      nullgenre= true;
      blacknull= true;
      break;

      /* different types of circe */
    case couscousmirror:
      anycirprom= true;
      anycirce= true;
      CondFlag[couscous]= true;
      circerenai= renspiegel;
      break;
    case pwc:
      circerenai= renpwc;
      anycirprom= true;
      anycirce= true;
      break;
    case couscous:
      anycirprom= true;
      anycirce= true;
      break;
    case parrain:
      anycirprom= true;
      anycirce= true;
      break;
    case circediametral:
      circerenai= rendiametral;
      anycirce= true;
      break;
    case frischauf:
      anycirce= true;
      break;
    case circe:
      anycirce= true;
      break;
    case chamcirce:
      anycirce= true;
      break;
    case circemalefiquevertical:
      anycirce= true;
      break;
    case supercirce:
    case april:
      circerenai= rensuper;
      anycirprom= true;
      anycirce= true;
      break;
    case circeequipollents:
      circerenai= renequipollents;
      anycirce= true;
      anycirprom= true;
      break;
    case circemalefique:
      circerenai= renspiegel;
      anycirce= true;
      break;
    case circerank:
      circerenai= renrank;
      anycirce= true;
      break;
    case circefile:
      circerenai= renfile;
      anycirce= true;
      break;
    case circefilemalefique:
      circerenai= renspiegelfile;
      anycirce= true;
      break;
    case circediagramm:
      SetDiaRen(PieSpExFlags, haut);
      circerenai= rendiagramm;
      anycirce= true;
      break;
    case circesymmetry:
      circerenai= rensymmetrie;
      anycirce= true;
      anycirprom= true;
      break;
    case circeantipoden:
      circerenai= renantipoden;
      anycirce= true;
      anycirprom= true;
      break;
    case circeclonemalefique:
      circerenai= renspiegel;
      anycirce= true;
      anyclone= true;
      break;
    case circeclone:
      anycirce=
        anyclone= true;
      break;
    case circeassassin:
      anycirce= true;
      flagAssassin= true;
      circerenai= rennormal;
      break;

      /* different types of anticirce */
    case circeturncoats:
      anycirce= true;
      anytraitor= true;
      circerenai= rennormal;
      break;
    case circedoubleagents:
      anycirce= true;
      anytraitor= true;
      circerenai= renspiegel;
      break;
    case anti:
      anyanticirce= true;
      break;
    case antispiegel:
      antirenai= renspiegel;
      anyanticirce= true;
      break;
    case antidiagramm:
      SetDiaRen(PieSpExFlags, haut);
      antirenai= rendiagramm;
      anyanticirce= true;
      break;
    case antifile:
      antirenai= renfile;
      anyanticirce= true;
      break;
    case antisymmetrie:
      antirenai= rensymmetrie;
      anyanticirce= true;
      break;
    case antispiegelfile:
      antirenai= renspiegelfile;
      anyanticirce= true;
      break;
    case antiantipoden:
      antirenai= renantipoden;
      anyanticirce= true;
      break;
    case antiequipollents:
      antirenai= renequipollents_anti;
      anyanticirce= true;
      break;
    case antisuper:
      antirenai= rensuper;
      anyanticirce= true;
      break;

      /* different types of immunchess */
    case immun:
      anyimmun= true;
      break;
    case immunmalefique:
      immunrenai= renspiegel;
      anyimmun= true;
      break;
    case immunfile:
      immunrenai= renfile;
      anyimmun= true;
      break;
    case immundiagramm:
      SetDiaRen(PieSpExFlags, haut);
      immunrenai= rendiagramm;
      anyimmun= true;
      break;
    case immunspiegelfile:
      immunrenai= renspiegelfile;
      anyimmun= true;
      break;
    case immunsymmetry:
      immunrenai= rensymmetrie;
      anyimmun= true;
      break;
    case immunantipoden:
      immunrenai= renantipoden;
      anyimmun= true;
      break;
    case immunequipollents:
      immunrenai= renequipollents;
      anyimmun= true;
      break;

      /* different types of mars circe */
    case mars:
      marsrenai= rennormal;
      anymars= true;
      break;
    case marsmirror:
      marsrenai= renspiegel;
      anymars= true;
      break;
    case antimars:
      marsrenai= rennormal;
      anyantimars= true;
      break;
    case antimarsmirror:
      marsrenai= renspiegel;
      anyantimars= true;
      break;
    case antimarsantipodean:
      marsrenai= renantipoden;
      anyantimars= true;
      break;
    case phantom:
      marsrenai= rennormal;
      anymars= CondFlag[phantom]= true;
      break;
    case plus:
      marsrenai= renplus;
      anymars= true;
      break;
    case whprom_sq:
      ReadSquares(WhPromSq);
      break;
    case blprom_sq:
      ReadSquares(BlPromSq);
      break;

      /*****  different types of geneva chess   *****/
    case geneva:
      genevarenai= rennormal;
      anygeneva= true;
      break;
    }

    switch (indexx) {
    case frischauf:
      tok= ReadSquares(ReadFrischAuf);
      break;
    case messigny:
      tok= ParseRex(&rex_mess_ex, rexexcl);
      break;
    case woozles:
      tok= ParseRex(&rex_wooz_ex, rexexcl);
      break;
    case biwoozles:
      tok= ParseRex(&rex_wooz_ex, rexexcl);
      break;
    case immun:
      tok= ParseRex(&rex_immun, rexincl);
      break;
    case immunmalefique:
      tok= ParseRex(&rex_immun, rexincl);
      break;
    case immundiagramm:
      tok= ParseRex(&rex_immun, rexincl);
      break;
    case chamcirce:
      ReadChameleonCirceSequence();
      break;
    case circe:
      tok= ParseRex(&rex_circe, rexincl);
      break;
    case circemalefique:
      tok= ParseRex(&rex_circe, rexincl);
      break;
    case circediagramm:
      tok= ParseRex(&rex_circe, rexincl);
      break;
    case circeclone:
      tok= ParseRex(&rex_circe, rexincl);
      break;
    case circeclonemalefique:
      tok= ParseRex(&rex_circe, rexincl);
      break;
    case circemalefiquevertical:
      tok= ParseRex(&rex_circe, rexincl);
      break;
    case protean:
      tok= ParseRex(&rex_protean_ex, rexexcl);
      break;
    case phantom:
      tok= ParseRex(&rex_phan, rexincl);
      break;
    case madras:
      tok= ParseRex(&rex_mad, rexincl);
      flag_madrasi= true;
      break;
    case isardam:
      tok= ParseVariant(&IsardamB, gpType);
      break;
    case annan:
      annanvar = 0;
      tok= ParseVariant(NULL, gpAnnan);
      break;
    case patience:
      tok= ParseVariant(&PatienceB, gpType);
      break;
    case sentinelles:
      SentPionNeutral=False;
      tok= ParseVariant(&SentPionAdverse, gpSentinelles);
      break;

      /*****  exact-maxis  *****/
    case blmax:
      tok= ParseExact(&bl_exact, &bl_ultra);
      break;
    case blmin:
      tok= ParseExact(&bl_exact, &bl_ultra);
      break;
    case blcapt:
      tok= ParseExact(&bl_exact, &bl_ultra);
      break;
    case whmax:
      tok= ParseExact(&wh_exact, &wh_ultra);
      break;
    case whmin:
      tok= ParseExact(&wh_exact, &wh_ultra);
      break;
    case whcapt:
      tok= ParseExact(&wh_exact, &wh_ultra);
      break;

      /*****  anticirce type    *****/
    case anti:
      tok= ParseVariant(&AntiCirCheylan, gpAntiCirce);
      AntiCirType= AntiCirCheylan
        ? AntiCirTypeCheylan : AntiCirTypeCalvet;
      break;
    case antispiegel:
      tok= ParseVariant(&AntiCirCheylan, gpAntiCirce);
      AntiCirType= AntiCirCheylan
        ? AntiCirTypeCheylan : AntiCirTypeCalvet;
      break;
    case antidiagramm:
      tok= ParseVariant(&AntiCirCheylan, gpAntiCirce);
      AntiCirType= AntiCirCheylan
        ? AntiCirTypeCheylan : AntiCirTypeCalvet;
      break;
    case antifile:
      tok= ParseVariant(&AntiCirCheylan, gpAntiCirce);
      AntiCirType= AntiCirCheylan
        ? AntiCirTypeCheylan : AntiCirTypeCalvet;
      break;
    case antisymmetrie:
      tok= ParseVariant(&AntiCirCheylan, gpAntiCirce);
      AntiCirType= AntiCirCheylan
        ? AntiCirTypeCheylan : AntiCirTypeCalvet;
      break;
    case antispiegelfile:
      tok= ParseVariant(&AntiCirCheylan, gpAntiCirce);
      AntiCirType= AntiCirCheylan
        ? AntiCirTypeCheylan : AntiCirTypeCalvet;
      break;
    case antiantipoden:
      tok= ParseVariant(&AntiCirCheylan, gpAntiCirce);
      AntiCirType= AntiCirCheylan
        ? AntiCirTypeCheylan : AntiCirTypeCalvet;
      break;
    case antiequipollents:
      tok= ParseVariant(&AntiCirCheylan, gpAntiCirce);
      AntiCirType= AntiCirCheylan
        ? AntiCirTypeCheylan : AntiCirTypeCalvet;
      break;
    case antisuper:
      tok= ParseVariant(&AntiCirCheylan, gpAntiCirce);
      AntiCirType= AntiCirCheylan
        ? AntiCirTypeCheylan : AntiCirTypeCalvet;
      break;
    case singlebox:
      tok= ParseVariant(NULL, gpType);
      break;
    case promotiononly:
      tok= ReadPieces(promotiononly);
      break;
    case april:
      tok= ReadPieces(april);
      break;
    case koeko:
      koekofunc= nokingcontact;
      nocontactfunc= &koekofunc;
      tok= ParseVariant(NULL, gpKoeko);
      break;
    case antikoeko:
      antikoekofunc= nokingcontact;
      nocontactfunc= &antikoekofunc;
      tok= ParseVariant(NULL, gpKoeko);
      break;
    case white_oscillatingKs:
      OscillatingKingsColour= blanc;
      tok= ParseVariant(NULL, gpOsc);
      break;
    case black_oscillatingKs:
      OscillatingKingsColour= noir;
      tok= ParseVariant(NULL, gpOsc);
      break;
    case swappingkings:
      CondFlag[white_oscillatingKs]= True;
      OscillatingKingsTypeC[blanc]= True;
      CondFlag[black_oscillatingKs]= True;
      OscillatingKingsTypeC[noir]= True;
      tok= ReadNextTokStr();
      break;
    case SAT:
    case strictSAT:
      WhiteSATFlights= strtol(tok= ReadNextTokStr(), &ptr, 10) + 1;
      if (tok == ptr) {
        WhiteSATFlights= 1;
        BlackSATFlights= 1;
        break;
      }
      BlackSATFlights= strtol(tok= ReadNextTokStr(), &ptr, 10) + 1;
      if (tok == ptr) {
        BlackSATFlights= WhiteSATFlights;
        break;
      }
    case BGL:
      BGL_global= false;
      BGL_white= ReadBGLNumber(tok= ReadNextTokStr(), &ptr); 
      if (tok == ptr)  
      {
        BGL_white= BGL_black= BGL_infinity;
        return tok;
      }
      else
      {
        BGL_black= ReadBGLNumber(tok= ReadNextTokStr(), &ptr);
        if (tok == ptr)
        {
          BGL_black= BGL_white;
          BGL_global= true;
          return tok;
        }
      }
      tok= ReadNextTokStr();
      break;
    case geneva:
      tok= ParseRex(&rex_geneva, rexincl);
      break;
    case whvault_king:
      whitenormaltranspieces = false;
      tok= ParseVaultingPieces(BIT(blanc));
      break;
    case blvault_king:
      blacknormaltranspieces = false;
      tok= ParseVaultingPieces(BIT(noir));
      break;
    case vault_king:
      whitenormaltranspieces = false;
      blacknormaltranspieces = false;
      tok= ParseVaultingPieces(BIT(blanc) | BIT(noir));
      break;
    case gridchess: 
      tok = ParseVariant(NULL, gpGrid);
      break;
    default:
      tok= ReadNextTokStr();
    }
  }
  if (! CondCnt) {
    IoErrorMsg(UnrecCondition,0);
  }
  return tok;
} /* ParseCond */

static char *ParseOpt(void) {
  int     indexx,OptCnt= 0;
  char    *tok, *ptr;

  tok= ReadNextTokStr();
  while ((indexx= GetUniqIndex(OptCount, OptTab, tok)) >= -1) {
    if (indexx == -1) {
      IoErrorMsg(OptNotUniq,0);
      continue;
    }
    OptFlag[indexx]= True;
    OptCnt++;
    switch(indexx) {
    case beep:
      tok= ReadNextTokStr();
      if ((maxbeep= atoi(tok)) <= 0) {
        maxbeep= 1;
        /* we did NOT use tok */
        continue;
      } else {
        /* we did use tok */
        break;
      }
    case maxtime:
      tok= ReadNextTokStr();
      if ((maxsolvingtime= atoi(tok)) <= 0) {
        OptFlag[maxtime]= False;
        IoErrorMsg(WrongInt, 0);
        return ReadNextTokStr();
      }
      break;
    case enpassant:
      ReadSquares(ReadEpSquares);
      break;
    case maxsols:
      tok= ReadNextTokStr();
      if ((maxsolutions= atoi(tok)) <= 0) {
        OptFlag[maxsols]= False;
        IoErrorMsg(WrongInt, 0);
        return ReadNextTokStr();
      }
      break;
    case intelligent:
      tok= ReadNextTokStr();
      maxsol_per_matingpos= atoi(tok);
      if (maxsol_per_matingpos > 0) {
        /* we did use tok */
        break;
      }
      else {
        maxsol_per_matingpos= 0;
        /* we did NOT use tok */
        continue;
      }
    case restart:
      tok= ReadNextTokStr();
      if ((RestartNbr= atoi(tok)) <= 0) {
        OptFlag[restart]= False;
        IoErrorMsg(WrongInt, 0);
        return ReadNextTokStr();
      }
      OptFlag[movenbr]= True;
      break;
    case solmenaces:
      droh= strtol(tok= ReadNextTokStr(), &ptr, 10);
      if (tok == ptr) {
        droh= maxply;
        IoErrorMsg(WrongInt, 0);
        return ReadNextTokStr();
      }
      break;
    case solflights:
      maxflights= strtol(tok= ReadNextTokStr(), &ptr, 10);
      if (tok == ptr) {
        IoErrorMsg(WrongInt, 0);
        maxflights = 64;
        return ReadNextTokStr();
      }
      break;
    case soltout:
      if ((maxdefen= atoi(tok= ReadNextTokStr())) <= 0) {
        IoErrorMsg(WrongInt, 0);
        return ReadNextTokStr();
      }
      break;
    case solessais:
      /* for compatibility to older versions. */
      OptFlag[soltout]= True;
      maxdefen= 1;
      break;
    case nontrivial:
      NonTrivialNumber= strtol(tok= ReadNextTokStr(), &ptr, 10);
      if (tok == ptr) {
        IoErrorMsg(WrongInt, 0);
        return ReadNextTokStr();
      }
      NonTrivialLength= strtol(tok= ReadNextTokStr(), &ptr, 10);
      if (tok == ptr) {
        IoErrorMsg(WrongInt, 0);
        NonTrivialLength = maxply;
        return ReadNextTokStr();
      }
      break;
    case postkeyplay:
      OptFlag[solvariantes]= True;
      break;
    case nocastling:
      no_castling= bl_castlings|wh_castlings;
      ReadSquares(ReadNoCastlingSquares);
      break;
    }
    tok= ReadNextTokStr();
  }
  if (! OptCnt) {
    IoErrorMsg(UnrecOption,0);
  }
  return tok;
}

/***** twinning ***** begin *****/

unsigned int TwinChar;

piece  twin_e[64];
Flags  twin_spec[64];
square twin_rb, twin_rn;
imarr  twin_isquare;

void TwinStorePosition(void) {
  int i;

  twin_rb= rb;
  twin_rn= rn;
  for (i= 0; i < 64; i++) {
    twin_e[i]= e[boardnum[i]];
    twin_spec[i]= spec[boardnum[i]];
  }

  for (i= 0; i < maxinum; i++) {
    twin_isquare[i]= isquare[i];
  }
}

void TwinResetPosition(void) {
  int i;

  rb= twin_rb;
  rn= twin_rn;
  for (i= 0; i < 64; i++) {
    e[boardnum[i]]= twin_e[i];
    spec[boardnum[i]]= twin_spec[i];
  }

  for (i= 0; i < maxinum; i++) {
    isquare[i]= twin_isquare[i];
  }
}

square RotMirrSquare(square sq, int what) {
  square ret= sq;

  switch (what) {
  case rot90:     ret= onerow*(sq%onerow)-sq/onerow+(onerow-1);      break;
  case rot180:    ret= haut+bas - sq;             break;
  case rot270:    ret= -onerow*(sq%onerow)+sq/onerow-(onerow-1)+haut+bas; break;
  case mirra1a8:  ret= sq%onerow+onerow*((onerow-1)-sq/onerow);      break;
  case mirra1h1:  ret= ((onerow-1)-sq%onerow)+onerow*(sq/onerow);    break;
  case mirra8h1:  ret= onerow*(sq%onerow)+sq/onerow;          break;
  case mirra1h8:  ret= ((onerow-1)-sq/onerow)+onerow*((onerow-1)-sq%onerow); break;
  }
  return ret;
}

void RotateMirror(int what) {
  piece t_e[64];
  Flags t_spec[64];
  square    t_rb, t_rn, sq1, sq2;
  imarr t_isquare;
  int       i;

  /* save the position to be mirrored/rotated */
  t_rb= rb;
  t_rn= rn;
  for (i= 0; i < 64; i++) {
    t_e[i]= e[boardnum[i]];
    t_spec[i]= spec[boardnum[i]];
  }

  for (i= 0; i < maxinum; i++) {
    t_isquare[i]= isquare[i];
  }

  /* now rotate/mirror */
  /* pieces */
  for (i= 0; i < 64; i++) {
    sq1= boardnum[i];
    sq2= RotMirrSquare(sq1, what);

    e[sq2]= t_e[i];
    spec[sq2]= t_spec[i];

    if (sq1 == t_rb) {
      rb= sq2;
    }
    if (sq1 == t_rn) {
      rn= sq2;
    }
  }

  /* imitators */
  for (i= 0; i < maxinum; i++) {
    sq1= t_isquare[i];
    sq2= RotMirrSquare(sq1, what);
    isquare[i]= sq2;
  }
} /* RotateMirror */

static char *ParseTwinRotate(void) {
  char *tok= ReadNextTokStr();

  if (strcmp(tok, "90") == 0) {
    RotateMirror(rot90);
  }
  else if (strcmp(tok, "180") == 0) {
    RotateMirror(rot180);
  }
  else if (strcmp(tok, "270") == 0) {
    RotateMirror(rot270);
  }
  else {
    IoErrorMsg(UnrecRotMirr,0);
  }

  if (LaTeXout) {
    sprintf(GlobalStr, "%s $%s^\\circ$", TwinTab[TwinRotate], tok);
    strcat(ActTwin, GlobalStr);
  }

  StdString(TwinTab[TwinRotate]);
  StdString(" ");
  StdString(tok);

  return ReadNextTokStr();
}

static char *ParseTwinMirror(void) {
  char *tok= ReadNextTokStr();
  int indexx= GetUniqIndex(TwinCount,TwinTab,tok);

  if (indexx == -1) {
    IoErrorMsg(OptNotUniq,0);
  }
  else {
    switch (indexx) {
    case TwinMirra1h1:  RotateMirror(mirra1h1);  break;
    case TwinMirra1a8:  RotateMirror(mirra1a8);  break;
    case TwinMirra1h8:  RotateMirror(mirra1h8);  break;
    case TwinMirra8h1:  RotateMirror(mirra8h1);  break;
    default:
      IoErrorMsg(UnrecRotMirr,0);
    }

    StdString(TwinTab[TwinMirror]);
    StdString(" ");
    StdString(TwinTab[indexx]);
  }

  return ReadNextTokStr();
}

static char *ParseTwinMove(int indexx) {
  square sq1= 0, sq2= 0;
  char *tok;
  piece p;
  Flags sp;

  /* read the first square */
  while (sq1 == 0) {
    tok= ReadNextTokStr();
    sq1= SquareNum(tok[0], tok[1]);
    if (sq1 == 0) {
      ErrorMsg(WrongSquareList);
      return ReadNextTokStr();
    }
  }

  /* read the second square */
  while (sq2 == 0) {
    tok= ReadNextTokStr();
    sq2= SquareNum(tok[0], tok[1]);
    if (sq2 == 0) {
      ErrorMsg(WrongSquareList);
      return ReadNextTokStr();
    }
  }

  if (e[sq1] == vide) {
    WriteSquare(sq1);
    StdString(": ");
    ErrorMsg(NothingToRemove);
    return ReadNextTokStr();
  }

  /* issue the twinning */
  if (LaTeXout) {
    sprintf(GlobalStr, "\\%c%s %c%c",
            TSTFLAG(spec[sq1], Neutral)
            ? 'n'
            : TSTFLAG(spec[sq1], White) ? 'w' : 's',
            LaTeXPiece(e[sq1]),
            'a'-nr_files_on_board+sq1%onerow,
            '1'-nr_rows_on_board+sq1/onerow);
    strcat(ActTwin, GlobalStr);
  }

  WriteSpec(spec[sq1], e[sq1]!=vide);
  WritePiece(e[sq1]);
  WriteSquare(sq1);
  if (indexx == TwinExchange) {
    StdString("<-->");
    WriteSpec(spec[sq2], e[sq2]!=vide);
    WritePiece(e[sq2]);
    if (LaTeXout) {
      strcat(ActTwin, "{\\lra}");
      sprintf(GlobalStr, "\\%c%s ",
              TSTFLAG(spec[sq2], Neutral)
              ? 'n'
              : TSTFLAG(spec[sq2], White) ? 'w' : 's',
              LaTeXPiece(e[sq2]));
      strcat(ActTwin, GlobalStr);
    }
  }
  else {
    StdString("-->");
    if (LaTeXout) {
      strcat(ActTwin, "{\\ra}");
    }
  }
  WriteSquare(sq2);
  if (LaTeXout) {
    sprintf(GlobalStr, "%c%c",
            'a'-nr_files_on_board+sq2%onerow,
            '1'-nr_rows_on_board+sq2/onerow);
    strcat(ActTwin, GlobalStr);
  }

  /* store the piece in case they are exchanged */
  p= e[sq2];
  sp= spec[sq2];

  /* move the piece from sq1 to sq2 */
  e[sq2]= e[sq1];
  spec[sq2]= spec[sq1];

  /* delete the other piece from p or delete sq1 */
  e[sq1]= indexx == TwinMove ? vide : p;
  spec[sq1]= indexx == TwinMove ? 0 : sp;

  /* update king pointer */
  if (sq1 == rb) {
    rb= sq2;
  }
  else if (sq2 == rb) {
    rb= indexx == TwinExchange ? sq1 : initsquare;
  }
  if (sq1 == rn) {
    rn= sq2;
  }
  else if (sq2 == rn) {
    rn= indexx == TwinExchange ? sq1 : initsquare;
  }

  /* read next token */
  return ReadNextTokStr();

} /* ParseTwinMove */

void MovePieceFromTo(square from, square to) {
  e[to]= e[from];
  spec[to]= spec[from];
  e[from]= vide;
  spec[from]= 0;
  if (from == rb) {
    rb= to;
  }
  if (from == rn) {
    rn= to;
  }
} /* MovePieceFromTo */

static char *ParseTwinShift(void) {
  square sq1= 0, sq2= 0, *bnp;
  char *tok;
  int diffrank, diffcol, minrank, maxrank, mincol, maxcol, r, c;

  /* read the first square */
  while (sq1 == 0) {
    tok= ReadNextTokStr();
    sq1= SquareNum(tok[0], tok[1]);
    if (sq1 == 0) {
      ErrorMsg(WrongSquareList);
    }
  }

  /* read the second square */
  while (sq2 == 0) {
    tok= ReadNextTokStr();
    sq2= SquareNum(tok[0], tok[1]);
    if (sq2 == 0) {
      ErrorMsg(WrongSquareList);
    }
  }

  /* issue the twinning */
  if (LaTeXout) {
    sprintf(GlobalStr, "%s %c%c$\\Rightarrow$%c%c",
            TwinTab[TwinShift],
            'a'-nr_files_on_board+sq1%onerow,
            '1'-nr_rows_on_board+sq1/onerow,
            'a'-nr_files_on_board+sq2%onerow,
            '1'-nr_rows_on_board+sq2/onerow);
    strcat(ActTwin, GlobalStr);
  }

  StdString(TwinTab[TwinShift]);
  StdString(" ");
  WriteSquare(sq1);
  StdString(" ==> ");
  WriteSquare(sq2);

  diffrank= sq2/onerow-sq1/onerow;
  diffcol= sq2%onerow-sq1%onerow;

  minrank= 2*nr_of_slack_rows_below_board + nr_rows_on_board - 1;
  maxrank= 0;
  mincol= onerow-1;
  maxcol= 0;

  for (bnp= boardnum; *bnp; bnp++) {
    if (e[*bnp] != vide) {
      if (*bnp/onerow < minrank) {
        minrank= *bnp/onerow;
      }
      if (*bnp/onerow > maxrank) {
        maxrank= *bnp/onerow;
      }
      if (*bnp%onerow < mincol) {
        mincol= *bnp%onerow;
      }
      if (*bnp%onerow > maxcol) {
        maxcol= *bnp%onerow;
      }
    }    }

  if ( maxcol+diffcol > 15
       || mincol+diffcol <  8
       || maxrank+diffrank > 15
       || minrank+diffrank <  8)
  {
    ErrorMsg(PieceOutside);
  }
  else {
    /* move along columns */
    if (diffrank > 0) {
      for (c= 8; c <= 15; c++) {
        for (r= maxrank; r >= minrank; r--) {
          MovePieceFromTo(onerow*r+c, onerow*(r+diffrank)+c);
        }
      }
    }
    else if (diffrank < 0) {
      for (c= 8; c <= 15; c++) {
        for (r= minrank; r <= maxrank; r++) {
          MovePieceFromTo(onerow*r+c, onerow*(r+diffrank)+c);
        }
      }
    }

    /* move along ranks */
    if (diffcol > 0) {
      for (c= maxcol; c >= mincol; c--) {
        for (r= 8; r <= 15; r++) {
          MovePieceFromTo(onerow*r+c, onerow*r+c+diffcol);
        }
      }
    }
    else if (diffcol < 0) {
      for (c= mincol; c <= maxcol; c++) {
        for (r= 8; r <= 15; r++) {
          MovePieceFromTo(onerow*r+c, onerow*r+c+diffcol);
        }
      }
    }
  }

  /* read next token */
  return ReadNextTokStr();

} /* ParseTwinShift */

static char *ParseTwinRemove(void) {
  square    sq;
  char  *tok;
  boolean   WrongList;

  do {
    WrongList= False;
    tok= ReadNextTokStr();

    if (strlen(tok) % 2) {
      WrongList= True;
    }
    else {
      char *tok2= tok;

      while (*tok2 && !WrongList) {
        if (SquareNum(tok2[0], tok2[1]) == 0) {
          WrongList= True;
        }
        tok2 += 2;
      }
    }
    if (WrongList) {
      ErrorMsg(WrongSquareList);
    }
  } while (WrongList);

  while (*tok) {
    sq= SquareNum(tok[0], tok[1]);

    if (abs(e[sq]) < King) {
      WriteSquare(sq);
      StdString(": ");
      Message(NothingToRemove);
    }
    else {
      if (LaTeXout) {
        strcat(ActTwin, " --");
        strcat(ActTwin,
               TSTFLAG(spec[sq], Neutral)
               ? "\\n"
               : TSTFLAG(spec[sq], White) ? "\\w" : "\\s");
        strcat(ActTwin,
               LaTeXPiece(e[sq]));
        sprintf(GlobalStr, " %c%c",
                'a'-nr_files_on_board+sq%onerow,
                '1'-nr_rows_on_board+sq/onerow);
        strcat(ActTwin, GlobalStr);
      }

      StdString(" -");
      WriteSpec(spec[sq], e[sq]!=vide);
      WritePiece(e[sq]);
      WriteSquare(sq);
      e[sq]= vide;
      spec[sq]= 0;
      if (sq == rb) {
        rb= initsquare;
      }
      if (sq == rn) {
        rn= initsquare;
      }
    }
    tok += 2;
  }

  return ReadNextTokStr();
} /* ParseTwinRemove */

static char *ParseTwinPolish(void) {
  square *bnp;
  square king;

  king= rb;
  rb= rn;
  rn= king;

  for (bnp= boardnum; *bnp; bnp++) {
    if (!TSTFLAG(spec[*bnp], Neutral) && e[*bnp] != vide) {
      e[*bnp]= -e[*bnp];
      spec[*bnp]^= BIT(White)+BIT(Black);
    }
  }

  StdString(TwinTab[TwinPolish]);

  if (LaTeXout) {
    strcat(ActTwin, TwinTab[TwinPolish]);
  }

  return ReadNextTokStr();
}

static char *ParseTwinSubstitute(void) {
  square    *bnp;
  piece p_old, p_new;
  char  *tok;

  switch (strlen(tok= ReadNextTokStr())) {
  case 1:
    p_old= GetPieNamIndex(*tok,' ');
    break;
  case 2:
    p_old= GetPieNamIndex(*tok,tok[1]);
    break;
  default:
    IoErrorMsg(WrongPieceName,0);
    return tok;
  }

  switch (strlen(tok= ReadNextTokStr())) {
  case 1:
    p_new= GetPieNamIndex(*tok,' ');
    break;
  case 2:
    p_new= GetPieNamIndex(*tok,tok[1]);
    break;
  default:
    IoErrorMsg(WrongPieceName,0);
    return tok;
  }

  if (LaTeXout) {
    sprintf(GlobalStr, "{\\w%s} $\\Rightarrow$ \\w%s",
            LaTeXPiece(p_old), LaTeXPiece(p_new));
    strcat(ActTwin, GlobalStr);
  }

  WritePiece(p_old);
  StdString(" ==> ");
  WritePiece(p_new);

  for (bnp= boardnum; *bnp; bnp++) {
    if (e[*bnp] == p_old) {
      e[*bnp]= p_new;
    }
    else if (e[*bnp] == -p_old) {
      e[*bnp]= -p_new;
    }
  }

  return ReadNextTokStr();

}

static char *ParseTwin(void) {
  int       indexx, i;
  char  *tok;
  boolean   continued= False,
    TwinRead= False;

  TwinChar++;
  OptFlag[noboard]= True;

  tok= ReadNextTokStr();

  while (True) {
    Token tk= StringToToken(tok);

    if ( tk == TwinProblem
         || tk == NextProblem
         || tk == EndProblem)
    {
      StdString("\n\n");
      if (LaTeXout) {
        strcat(ActTwin, "{\\newline}");
      }
      return tok;
    }

    switch (indexx= GetUniqIndex(TwinCount,TwinTab,tok)) {
    case -1:
      IoErrorMsg(OptNotUniq,0);
      tok= ReadNextTokStr();
      continue;

    case -2:
      IoErrorMsg(ComNotKnown,0);
      tok= ReadNextTokStr();
      continue;

    case TwinContinued:
      if (TwinRead == True) {
        Message(ContinuedFirst);
      }
      else {
        continued= True;
      }
      tok= ReadNextTokStr();
      continue;
    }

    if (!TwinRead) {
      if (!continued) {
        TwinResetPosition();
      }
      else {
#if !defined(DATABASE)
        if (SortFlag(Proof)) {
          /* fixes bug for continued twinning
             in proof games; changes were made
             to game array!
          */
          for (i=maxsquare-1; i>=0; i--) {
            e[i]=ProofBoard[i];
          }
          for (i= 0; i< 64; i++) {
            spec[boardnum[i]]=ProofSpec[i];
          }
          rn=Proof_rn;
          rb=Proof_rb;
        }
#endif /* DATABASE */
        StdChar('+');
        if (LaTeXout) {
          strcat(ActTwin, "+");
        }
      }

      if (TwinChar <= 'z') {
        sprintf(GlobalStr, "%c) ", TwinChar);
      }
      else {
        sprintf(GlobalStr,
                "%c%d) ",
                'z',
                (TwinChar-'z'-1));
      }
      StdString(GlobalStr);
      if (LaTeXout) {
        strcat(ActTwin, GlobalStr);
      }
    }
    else {
      StdString("  ");
      if (LaTeXout) {
        strcat(ActTwin, ", ");
      }
    } /* !TwinRead */

    TwinRead= True;
    switch(indexx) {
    case TwinMove:
      tok= ParseTwinMove(indexx);
      break;
    case TwinExchange:
      tok= ParseTwinMove(indexx);
      break;
    case TwinRotate:
      tok= ParseTwinRotate();
      break;
    case TwinMirror:
      tok= ParseTwinMirror();
      break;
    case TwinStip:
      InitStip();
      tok= ParseStip();

      /* issue the twinning */
      StdString(AlphaStip);
      if (LaTeXout) {
        strcat(ActTwin, AlphaStip);
        if (OptFlag[solapparent]) {
          strcat(ActTwin, "*");
        }
        if (OptFlag[appseul]) {
          char temp[10];        /* increased due to buffer overflow */
          sprintf(temp, " %c{\\ra}",
                  tolower(*PieSpString[ActLang][White]));
          strcat(ActTwin, temp);
        }
      }
      break;
    case TwinAdd:
      tok= ParsePieSpec('+');
      break;
    case TwinCond:
      InitCond();
      tok= ParseCond();
      WriteConditions(WCleft);
      break;
    case TwinRemove:
      tok= ParseTwinRemove();
      break;
    case TwinPolish:
      tok= ParseTwinPolish();
      break;
    case TwinShift:
      tok= ParseTwinShift();
      break;
    case TwinSubstitute:
      tok= ParseTwinSubstitute();
      break;
    case TwinForsyth:
      tok= ParseForsyth(true);
      break;
    }
  }
} /* ParseTwin */

/***** twinning *****  end  *****/

/* new conditions: PromOnly, AprilChess */
char *ReadPieces(int condition) {
  piece tmp_piece;
  char  *tok;
  boolean   piece_read= False;

  fflush(stdout);
  while (True) {
    switch (strlen(tok= ReadNextTokStr())) {
    case 1:
      tmp_piece= GetPieNamIndex(*tok,' ');
      piece_read= True;
      break;
    case 2:
      tmp_piece= GetPieNamIndex(*tok,tok[1]);
      piece_read= True;
      break;
    default:
      if (!piece_read) {
        CondFlag[condition]= False;
        IoErrorMsg(WrongPieceName,0);
      }
      return tok;
    }
    if (!tmp_piece) {
      IoErrorMsg(WrongPieceName,0);
      break;
    }
    switch (condition) {
    case promotiononly:
      promonly[tmp_piece]= True;
      break;
    case april:
      isapril[tmp_piece]= True;
      break;
    default:
      /* Never mind ... */
      break;
    }
  }
  return tok;
}


Token ReadProblem(Token tk) {
  char *tok;

  /* open mode for protocol and/or TeX file; overwrite existing file(s)
   * if we are doing a regression test */
  char const *open_mode = flag_regression ? "w" : "a";

  if (tk == BeginProblem) {
    LastChar= ' ';
    ReadBeginSpec();
  }
  if (tk == TwinProblem || tk == ZeroPosition) {
    if (tk == ZeroPosition) {
      StdString(TokenTab[ZeroPosition]);
      StdString("\n\n");
      TwinChar= 'a'-1;
      TwinStorePosition();
    }
    tok= ParseTwin();
    while (True) {
      switch (tk= StringToToken(tok)) {
      case -1:
        IoErrorMsg(ComNotUniq,0);
        tok= ReadNextTokStr();
        break;
      case TwinProblem:
        if (enonce) {
          return tk;
        }
        IoErrorMsg(NoStipulation,0);
        tok= ReadNextTokStr();
        break;
      case NextProblem:
        if (enonce) {
          return tk;
        }
        IoErrorMsg(NoStipulation,0);
        tok= ReadNextTokStr();
        break;
      case EndProblem:
        if (enonce) {
          return tk;
        }
        IoErrorMsg(NoStipulation,0);
        tok= ReadNextTokStr();
        break;
      default:
        IoErrorMsg(ComNotKnown,0);
        tok= ReadNextTokStr();
        break;
      }
    }
  }
  else {
    tok= ReadNextTokStr();
    TwinChar= 'a';
    while (True) {
      switch (tk= StringToToken(tok)) {
      case -1:
        IoErrorMsg(ComNotUniq,0);
        tok= ReadNextTokStr();
        break;
      case -2:
        IoErrorMsg(ComNotKnown,0);
        tok= ReadNextTokStr();
        break;
      case BeginProblem:
        tok= ReadNextTokStr();
        break;
      case TwinProblem:
        if (TwinChar == 'a') {
          TwinStorePosition();
        }
      case NextProblem:
        if (enonce) {
          return tk;
        }
        IoErrorMsg(NoStipulation,0);
        tok= ReadNextTokStr();
        break;
      case EndProblem:
        if (enonce) {
          return tk;
        }
        IoErrorMsg(NoStipulation,0);
        tok= ReadNextTokStr();
        break;
      case ZeroPosition:
        return tk;
      case StipToken:
        *AlphaStip='\0';
        tok= ParseStip();
        break;
      case Author:
        strcat(ActAuthor,ReadToEndOfLine());
        strcat(ActAuthor,"\n");
        tok= ReadNextTokStr();
        break;
      case Award:
        strcpy(ActAward,ReadToEndOfLine());
        strcat(ActAward, "\n");
        tok= ReadNextTokStr();
        break;
      case Origin:
        strcat(ActOrigin,ReadToEndOfLine());
        strcat(ActOrigin,"\n");
        tok= ReadNextTokStr();
        break;
      case TitleToken:
        strcat(ActTitle,ReadToEndOfLine());
        strcat(ActTitle,"\n");
        tok= ReadNextTokStr();
        break;
      case PieceToken:
        tok= ParsePieSpec('\0');
        break;
      case CondToken:
        tok= ParseCond();
        break;
      case OptToken:
        tok= ParseOpt();
        break;
      case RemToken:
        if (LastChar != '\n') {
          ReadToEndOfLine();
          if (TraceFile) {
            fputs(InputLine, TraceFile);
            fflush(TraceFile);
          }
          Message(NewLine);
        }
        tok= ReadNextTokStr();
        break;
      case InputToken:
        PushInput(ReadToEndOfLine());
        tok= ReadNextTokStr();
        break;
      case TraceToken:
        if (TraceFile) {
          fclose(TraceFile);
        }
        if ((TraceFile=fopen(ReadToEndOfLine(),open_mode)) == NULL) {
          IoErrorMsg(WrOpenError,0);
        }
        else if (!flag_regression) {
          fputs(StartUp, TraceFile);
          fputs(MMString, TraceFile);
          fflush(TraceFile);
        }
        tok= ReadNextTokStr();
        break;
      case LaTeXPieces:
        tok= ParseLaTeXPieces(ReadNextTokStr());
        break;
      case LaTeXToken:
        LaTeXout= true;
        if (LaTeXFile) {
          LaTeXClose();
          fclose(LaTeXFile);
        }
        if ((LaTeXFile= fopen(ReadToEndOfLine(),open_mode)) == NULL) {
          IoErrorMsg(WrOpenError,0);
          LaTeXout= false;
        }
        else {
          LaTeXOpen();
        }

        if (SolFile) {
          fclose(SolFile);
        }
        if ((SolFile= tmpfile()) == NULL) {
          IoErrorMsg(WrOpenError,0);
        }
        tok= ParseLaTeXPieces(ReadNextTokStr());
        break;
      case SepToken:
        tok= ReadNextTokStr();
        break;
      case Array:
        tok= ReadNextTokStr();
        {
          piece p;
          int i;

          for (i = 0; i < 64; i++) {
            CLEARFL(spec[boardnum[i]]);
            p= e[boardnum[i]]= PAS[i];
            if (p >= roib) {
              SETFLAG(spec[boardnum[i]], White);
            }
            else if (p <= roin) {
              SETFLAG(spec[boardnum[i]], Black);
            }
          }
          rb= square_e1;
          rn= square_e8;
        }
        break;
      case Forsyth:
        tok= ParseForsyth(false);
        break;
      default:
        FtlMsg(InternalError);
      }
    } /* while */
  }
}

void CenterLine(char *s) {
  /* TODO move into one module per platform */
#if defined(ATARI)
#   if defined(__TURBOC__)
  sprintf(GlobalStr, "%s\n", s);
#   else    /* not __TURBOC__ */
  sprintf(GlobalStr, "%*s\n", (36+strlen(s))/2, s);
#   endif   /* __TURBOC__ */
#else   /* not ATARI */
  /* sprintf(GlobalStr, "%*s\n", (36+(int)strlen(s))/2, s); */
  sprintf(GlobalStr, "%*s\n", (38+(int)strlen(s))/2, s);
#endif  /* ATARI */
  StdString(GlobalStr);
}

void MultiCenter(char *s) {
  char *p;

  while ((p=strchr(s,'\n'))) {
    *p= '\0';
    CenterLine(s);
    *p= '\n';
    s= p + 1;
  }
}

void AddSquare(char *List, square i) {
  char    add[4];

  add[0]= ' ';
  add[1]= 'a' - nr_files_on_board + i%onerow;
  add[2]= '1' - nr_rows_on_board + i/onerow;
  add[3]= '\0';
  strcat(List, add);
}

void WritePieces(piece *p, char* CondLine)
{
      /* due to a Borland C++ 4.5 bug we have to use LocalBuf ... */
  char LocalBuf[4];
  while (*p) {
      if (PieceTab[*p][1] != ' ')
      sprintf(LocalBuf, " %c%c",
                  UPCASE(PieceTab[*p][0]),
                  UPCASE(PieceTab[*p][1]));
    else
      sprintf(LocalBuf, " %c",
                  UPCASE(PieceTab[*p][0]));
      strcat(CondLine, LocalBuf);
    p++;
  }
}

void WriteConditions(int alignment) {
  Cond  cond;
  char  CondLine[256];
  int       i;
  boolean   CondPrinted= False;

  for (cond= 1; cond < CondCount; cond++) {
    if (!CondFlag[cond])
      continue;

    if (cond == rexexcl)
      continue;
    if (cond == exact || cond == ultra)
      continue;
    if (cond == einstein
        && (CondFlag[reveinstein] || CondFlag[antieinstein]))
      continue;
    if (  (cond == woozles
           && (CondFlag[biwoozles]
               ||CondFlag[heffalumps]))
          || ((cond == heffalumps || cond == biwoozles)
              && CondFlag[biheffalumps]))
      continue;

    if (cond == volage && CondFlag[hypervolage])
      continue;

    if (cond == chinoises && CondFlag[leofamily])
      continue;

    if (  (cond == gridchess || cond == koeko)
          && CondFlag[contactgrid])
      continue;

    if (cond == tibet && CondFlag[dbltibet])
      continue;

    if (cond == holes)
      continue;

    if (cond == couscous && CondFlag[couscousmirror])
      continue;

    /* WhiteOscillatingKings TypeC + BlackOscillatingKings TypeC == SwappingKings */
    if (((cond == white_oscillatingKs) && OscillatingKingsTypeC[blanc]) ||
        ((cond == black_oscillatingKs) && OscillatingKingsTypeC[noir])) {
      if (CondFlag[swappingkings])
        continue;
    }

    /* Write DEFAULT Conditions */
    strcpy(CondLine, CondTab[cond]);

    if ((cond == blmax || cond == whmax) && flagmaxi)
      strcpy(CondLine, ExtraCondTab[maxi]);

    if (  (cond == blackultraschachzwang
           || cond == whiteultraschachzwang)
          && flagultraschachzwang)
    {
      strcpy(CondLine, ExtraCondTab[ultraschachzwang]);
    }

    if (cond == sentinelles && flagparasent) {
      strcpy(CondLine, "Para");
      strcat(CondLine, CondTab[cond]);
    }

    if (cond == koeko || cond == antikoeko) {
      piece koekop = roib;
      char LocalBuf[4];
      nocontactfunc = cond==koeko ? &koekofunc : &antikoekofunc;
      if (*nocontactfunc == noknightcontact) 
        koekop= cb;
      if (*nocontactfunc == nowazircontact) 
        koekop= vizirb;
      if (*nocontactfunc == noferscontact) 
        koekop= fersb;
      if (*nocontactfunc == nodabbabacontact) 
        koekop= dabb;
      if (*nocontactfunc == noalfilcontact) 
        koekop= alfilb;
      if (*nocontactfunc == nocamelcontact) 
        koekop= chb;
      if (*nocontactfunc == nozebracontact) 
        koekop= zb;
      if (*nocontactfunc == nogiraffecontact) 
        koekop= gib;
      if (*nocontactfunc == noantelopecontact) 
        koekop= antilb;

      if (koekop == roib)
        strcpy(LocalBuf, "");
      else if (PieceTab[koekop][1] != ' ')
        sprintf(LocalBuf, "%c%c-",
                UPCASE(PieceTab[koekop][0]),
                UPCASE(PieceTab[koekop][1]));
      else
        sprintf(LocalBuf, " %c-",
                UPCASE(PieceTab[koekop][0]));

      sprintf(CondLine, "%s%s", LocalBuf, CondTab[cond]);
    }

    if (cond == BGL)
    {
      char buf1[12], buf2[12];
      if (BGL_global)
      {
        sprintf(CondLine, "%s %s", CondTab[cond], WriteBGLNumber(buf1, BGL_white));
      }
      else
      {
        sprintf(CondLine, "%s %s/%s", CondTab[cond], WriteBGLNumber(buf1, BGL_white), WriteBGLNumber(buf2, BGL_black));
      }
    }

  if ( cond == whvault_king || cond == vault_king)
  {
      if (whitetransmpieces[0] != EquiHopper || whitetransmpieces[1] != vide) 
        WritePieces(whitetransmpieces, CondLine);
      if (calc_whtrans_king)
      {
        char LocalBuf[4];
        sprintf(LocalBuf, " -%c",
        UPCASE(PieceTab[King][0]));
        strcat(CondLine, LocalBuf);
      }
  }

  if ( cond == blvault_king )
  {
      if (blacktransmpieces[0] != EquiHopper || blacktransmpieces[1] != vide) 
        WritePieces(blacktransmpieces, CondLine);
      if (calc_bltrans_king)
      {
        char LocalBuf[4];
        sprintf(LocalBuf, " -%c",
        UPCASE(PieceTab[King][0]));
        strcat(CondLine, LocalBuf);
      }
  }
       
    if (cond == promotiononly) {
      /* due to a Borland C++ 4.5 bug we have to use LocalBuf ... */
      char LocalBuf[4];
      piece pp= vide;
      while ((pp= getprompiece[pp]) != vide) {
        if (PieceTab[pp][1] != ' ')
          sprintf(LocalBuf, " %c%c",
                  UPCASE(PieceTab[pp][0]),
                  UPCASE(PieceTab[pp][1]));
        else
          sprintf(LocalBuf, " %c",
                  UPCASE(PieceTab[pp][0]));
        strcat(CondLine, LocalBuf);
      }
      if (strlen(CondLine) <= strlen(CondTab[promotiononly])) {
        /* due to zeroposition, where getprompiece is not */
        /* set (it's set in verifieposition), I suppress  */
        /* output of promotiononly for now.  */
        continue;
      }
    }

    if (cond == april) {
      /* due to a Borland C++ 4.5 bug we have to use LocalBuf...*/
      char LocalBuf[4];
      piece pp;
      for (pp= vide; pp!=derbla; ++pp)
        if (isapril[pp]) {
          if (PieceTab[pp][1] != ' ')
            sprintf(LocalBuf, " %c%c",
                    UPCASE(PieceTab[pp][0]),
                    UPCASE(PieceTab[pp][1]));
          else
            sprintf(LocalBuf, " %c",
                    UPCASE(PieceTab[pp][0]));
          strcat(CondLine, LocalBuf);
        }
    }

    if (cond == imitators) {
      for (i= 0; i < inum[1]; i++) {
        AddSquare(CondLine, isquare[i]);
      }
    }

    if (cond == noiprom && !CondFlag[imitators])
      continue;

    if (cond == magicsquare) {
      square  i;
      for (i= bas; i <= haut; i++) {
        if (TSTFLAG(sq_spec[i], MagicSq)) {
          AddSquare(CondLine, i);
        }
      }
    }
    if (cond == whforsqu) {
      square  i;
      for (i= bas; i <= haut; i++) {
        if (TSTFLAG(sq_spec[i], WhForcedSq)) {
          AddSquare(CondLine, i);
        }
      }
    }
    if (cond == blforsqu) {
      square  i;
      for (i= bas; i <= haut; i++) {
        if (TSTFLAG(sq_spec[i], BlForcedSq)) {
          AddSquare(CondLine, i);
        }
      }
    }

    if (cond == whconforsqu) {
      square  i;
      for (i= bas; i <= haut; i++) {
        if (TSTFLAG(sq_spec[i], WhConsForcedSq)) {
          AddSquare(CondLine, i);
        }
      }
    }

    if (cond == blconforsqu) {
      square  i;
      for (i= bas; i <= haut; i++) {
        if (TSTFLAG(sq_spec[i], BlConsForcedSq)) {
          AddSquare(CondLine, i);
        }
      }
    }

    if (cond == whprom_sq) {
      square  i;
      for (i= bas; i <= haut; i++) {
        if (TSTFLAG(sq_spec[i], WhPromSq)) {
          AddSquare(CondLine, i);
        }
      }
    }
    if (cond == blprom_sq) {
      square  i;
      for (i= bas; i <= haut; i++) {
        if (TSTFLAG(sq_spec[i], BlPromSq)) {
          AddSquare(CondLine, i);
        }
      }
    }

    if (cond == blroyalsq) {
      AddSquare(CondLine, bl_royal_sq);
    }

    if (cond == whroyalsq) {
      AddSquare(CondLine, wh_royal_sq);
    }

    if ((cond == madras && rex_mad)
        || (cond == phantom && rex_phan)
        || (cond == geneva && rex_geneva)
        || (rex_immun
            && (cond == immun
                || cond == immunmalefique
                || cond == immundiagramm))
        || (rex_circe
            && (cond == circe
                || cond == circemalefique
                || cond == circediametral
                || cond == circemalefiquevertical
                || cond == circeclone
                || cond == circeclonemalefique
                || cond == circediagramm)))
    {
      strcat(CondLine, " ");
      strcat(CondLine, CondTab[rexincl]);
    }

    if (  (rex_mess_ex && cond == messigny)
          || (rex_wooz_ex
              && (cond == woozles
                  || cond == biwoozles)))
    {
      strcat(CondLine, " ");
      strcat(CondLine, CondTab[rexexcl]);
    }

    if ( rex_protean_ex && cond == protean)
    {
      strcat(CondLine, "    ");
      strcat(CondLine, CondTab[rexexcl]);
    }

    if (cond == chamcirce && ChameleonSequence[0]) {
      strcat(CondLine, "    ");
      strcat(CondLine, ChameleonSequence);
    }

    if ((cond == isardam) && IsardamB) {
      strcat(CondLine, "    ");
      strcat(CondLine, VariantTypeString[ActLang][TypeB]);
    }

    if (cond == annan) {
      strcat(CondLine, "    ");
      switch (annanvar)
      {
      case 1:
        strcat(CondLine, VariantTypeString[ActLang][TypeB]);
        break;
      case 2:
        strcat(CondLine, VariantTypeString[ActLang][TypeC]);
        break;
      case 3:
        strcat(CondLine, VariantTypeString[ActLang][TypeD]);
        break;
      }
    }

    if (cond == gridchess && OptFlag[suppressgrid]) {
      strcat(CondLine, "  ");
      switch (gridvar)
      {
        case grid_vertical_shift:
          strcat(CondLine, VariantTypeString[ActLang][ShiftRank]);
          break;
        case grid_horizontal_shift:
          strcat(CondLine, VariantTypeString[ActLang][ShiftFile]);
          break;
        case grid_diagonal_shift:
          strcat(CondLine, VariantTypeString[ActLang][ShiftRankFile]);
          break;
        case grid_orthogonal_lines:
          strcat(CondLine, VariantTypeString[ActLang][Orthogonal]);
          /* to do - write lines */
          break;
        case grid_irregular:
          strcat(CondLine, VariantTypeString[ActLang][Irregular]);
          /* to do - write squares */
          break;
      }
    }

    if ((cond == white_oscillatingKs) && OscillatingKingsTypeB[blanc]) {
      strcat(CondLine, "    ");
      strcat(CondLine, VariantTypeString[ActLang][TypeB]);
    }

    if ((cond == black_oscillatingKs) && OscillatingKingsTypeB[noir]) {
      strcat(CondLine, "    ");
      strcat(CondLine, VariantTypeString[ActLang][TypeB]);
    }

    if ((cond == white_oscillatingKs) && OscillatingKingsTypeC[blanc]) {
      if (! CondFlag[swappingkings]) {
        strcat(CondLine, "  ");
        strcat(CondLine, VariantTypeString[ActLang][TypeC]);
      }
    }

    if ((cond == black_oscillatingKs) && OscillatingKingsTypeC[noir]) {
      if (! CondFlag[swappingkings]) {
        strcat(CondLine, "  ");
        strcat(CondLine, VariantTypeString[ActLang][TypeC]);
      }
    }

    if ((cond == patience) && PatienceB) {
      strcat(CondLine, "    ");
      strcat(CondLine, VariantTypeString[ActLang][TypeB]);
    }

    if (CondFlag[singlebox])    {
      strcat(CondLine, "    ");
      if (SingleBoxType==singlebox_type1)
        strcat(CondLine, VariantTypeString[ActLang][Type1]);
      if (SingleBoxType==singlebox_type2)
        strcat(CondLine, VariantTypeString[ActLang][Type2]);
      if (SingleBoxType==singlebox_type3)
        strcat(CondLine, VariantTypeString[ActLang][Type3]);
    }

    if (cond == sentinelles) {
      char pawns[7];
      if (sentinelb == pbb)
        strcat(CondLine, " Berolina");
      if (SentPionAdverse) {
        strcat(CondLine, "  ");
        strcat(CondLine,
               VariantTypeString[ActLang][PionAdverse]);
      }
      if (SentPionNeutral) {
        strcat(CondLine, "  ");
        strcat(CondLine,
               VariantTypeString[ActLang][PionNeutral]);
      }
      if (max_pn !=8 || max_pb != 8) {
        sprintf(pawns, " %u/%u", max_pb, max_pn);
        strcat (CondLine, pawns);
      }
      if (max_pt != 16) {
        sprintf(pawns, " //%u", max_pt);
        strcat (CondLine, pawns);
      }
    }

    if ((cond == SAT || cond == strictSAT) && (WhiteSATFlights != 1 || BlackSATFlights != 1)) {
      char extra[10];
      char roman[][9] = {"","I","II","III","IV","V","VI","VII","VIII"};
      if (WhiteSATFlights == BlackSATFlights)
        sprintf(extra, " %s", roman[WhiteSATFlights-1]);
      else
        sprintf(extra, " %s/%s", roman[WhiteSATFlights-1], roman[BlackSATFlights-1]);
      strcat (CondLine, extra);
    }

    switch (cond) {
    case anti:
    case antispiegel:
    case antidiagramm:
    case antifile:
    case antisymmetrie:
    case antispiegelfile:
    case antiantipoden:
    case antiequipollents:
      /* AntiCirTypeCalvet is default in AntiCirce */
      if (AntiCirType != AntiCirTypeCalvet) {
        strcat(CondLine, "  ");
        strcat(CondLine, VariantTypeString[ActLang][AntiCirType]);
      }
      break;
    case antisuper:
      /* AntiCirTypeCheylan is default in AntiSuperCirce */
      if (AntiCirType != AntiCirTypeCheylan) {
        strcat(CondLine, "  ");
        strcat(CondLine, VariantTypeString[ActLang][AntiCirType]);
      }
      break;
    default:
      break;
    }

    switch (cond) {
    case blmax:
    case blmin:
    case blcapt:
      if (bl_ultra || bl_exact) {
        strcat(CondLine, "  ");
        if (bl_ultra)
          strcat(CondLine, CondTab[ultra]);
        else
          strcat(CondLine, CondTab[exact]);
      }
      break;
    case whmax:
    case whmin:
    case whcapt:
      if (wh_ultra || wh_exact) {
        strcat(CondLine, "  ");
        if (wh_ultra)
          strcat(CondLine, CondTab[ultra]);
        else
          strcat(CondLine, CondTab[exact]);
      }
    default:
      break;
    }
    switch (alignment) {
    case WCcentered:
      CenterLine(CondLine);
      break;

    case WCLaTeX:
      if (CondPrinted) {
        fprintf(LaTeXFile, "{\\newline}\n   ");
      }
      else {
        fprintf(LaTeXFile, " \\condition{");
      }
      LaTeXStr(CondLine);
      break;

    case WCleft:
      if (CondPrinted) {
        if (LaTeXout) {
          strcat(ActTwin, ", ");
        }
        StdString("\n   ");
      }
      StdString(CondLine);
      if (LaTeXout) {
        strcat(ActTwin, CondLine);
      }
      break;
    }
    CondPrinted= True;
  }

  if (alignment == WCLaTeX && CondPrinted) {
    fprintf(LaTeXFile, "}%%\n");
  }
} /* WriteConditions */

void WritePosition() {
  int i, nBlack, nWhite, nNeutr;
  square square, square_a;
  int row, file;
  piece   p,pp;
  char    HLine1[40];
  char    HLine2[40];
  char    PieCnts[20];
  char    StipOptStr[40];
  PieSpec sp;
  char    ListSpec[PieSpCount][256];
  FILE    *OrigSolFile= SolFile;

  static char BorderL[]="+---a---b---c---d---e---f---g---h---+\n";
  static char HorizL[]="%c   .   .   .   .   .   .   .   .   %c\n";
  static char BlankL[]="|                                   |\n";

  SolFile= NULL;

  for (sp= Neutral; sp < PieSpCount; sp++) {
    strcpy(ListSpec[sp], PieSpString[ActLang][sp]);
  }

  StdChar('\n');
  MultiCenter(ActAuthor);
  MultiCenter(ActOrigin);
  MultiCenter(ActAward);
  MultiCenter(ActTitle);


  nBlack= nWhite= nNeutr= 0;
  StdChar('\n');
  StdString(BorderL);
  StdString(BlankL);

  /* Just for visualizing imitators on the board. */                 
  if (CondFlag[imitators]) {
    for (i= 0; i < inum[1]; i++) {
      e[isquare[i]]= -1; 
    }
  }

  for (row=1, square_a = square_a8;
       row<=nr_rows_on_board;
       row++, square_a += dir_down) {
    char const *digits="87654321";
    sprintf(HLine1, HorizL, digits[row-1], digits[row-1]);

    strcpy(HLine2,BlankL);

    for (file= 1, square= square_a;
         file <= nr_files_on_board;
         file++, square += dir_right) {
      char *h1= HLine1 + 4*file;

      if (CondFlag[gridchess] && !OptFlag[suppressgrid])
      {
        if (file < nr_files_on_board
            && GridLegal(square, square+dir_right))
          HLine1[4*file+2] = '|';

        if (row < nr_rows_on_board
            && GridLegal(square, square+dir_down))
          HLine2[4*file-1] = HLine2[4*file] = HLine2[4*file+1] = '-';
      }

      if ((pp= abs(p= e[square])) < King) {
        if (p == -1) {
          /* this is an imitator ! */
          *h1= 'I';
          e[square]= vide; /* "delete" imitator */
        }
        else if (p == obs) {
          /* this is a hole ! */
          *h1= ' ';
        }
        /* else:  the square is empty ! */
        continue;
      }

      for (sp= Neutral + 1; sp < PieSpCount; sp++) {
        if (TSTFLAG(spec[square], sp)) {
          AddSquare(ListSpec[sp], square);
        }
      }

      if (pp<Hunter0 || pp >= (Hunter0 + maxnrhuntertypes)) {
        if ((*h1= PieceTab[pp][1]) != ' ') {
          *h1= UPCASE(*h1);
          h1--;
        }
        *h1--= UPCASE(PieceTab[pp][0]);
      }
      else {
        char *n1 = HLine2 + (h1-HLine1); /* current position on next line */

        unsigned int const hunterIndex = pp-Hunter0;
        assert(hunterIndex<maxnrhuntertypes);

        *h1-- = '/';
        if ((*h1= PieceTab[huntertypes[hunterIndex].away][1]) != ' ') {
          *h1= UPCASE(*h1);
          h1--;
        }
        *h1--= UPCASE(PieceTab[huntertypes[hunterIndex].away][0]);

        --n1;   /* leave pos. below '/' empty */
        if ((*n1= PieceTab[huntertypes[hunterIndex].home][1]) != ' ') {
          *n1= UPCASE(*n1);
        }
        *n1 = UPCASE(PieceTab[huntertypes[hunterIndex].home][0]);
      }

      if (TSTFLAG(spec[square], Neutral)) {
        nNeutr++;
        *h1= '=';
      }
      else if (p < 0) {
        nBlack++;
        *h1= '-';
      }
      else {
        nWhite++;
      }
    }

    StdString(HLine1);
    StdString(HLine2);
  }

  StdString(BorderL);
  if (nNeutr) {
    sprintf(PieCnts, "%d + %d + %dn", nWhite, nBlack, nNeutr);
  }
  else {
    sprintf(PieCnts, "%d + %d", nWhite, nBlack);
  }

  strcpy(StipOptStr, AlphaStip);

  if (droh < enonce - 1) {
    sprintf(StipOptStr+strlen(StipOptStr), "/%d", droh);
    if (maxflights < 64) {
      sprintf(StipOptStr+strlen(StipOptStr), "/%d", maxflights);
    }
  }
  else if (maxflights < 64)
    sprintf(StipOptStr+strlen(StipOptStr), "//%d", maxflights);

  if (NonTrivialLength < enonce - 1) {
    sprintf(StipOptStr+strlen(StipOptStr),
            ";%d,%d", NonTrivialNumber, NonTrivialLength);
  }

  sprintf(GlobalStr, "  %-20s%13s\n", StipOptStr, PieCnts);
  StdString(GlobalStr);

  for (sp= Neutral + 1; sp < PieSpCount; sp++) {
    if (TSTFLAG(PieSpExFlags, sp)) {
      CenterLine(ListSpec[sp]);
    }
  }

  WriteConditions(WCcentered);

  if (OptFlag[halfduplex]) {
    CenterLine(OptString[ActLang][halfduplex]);
  }
  else if (OptFlag[duplex]) {
    CenterLine(OptString[ActLang][duplex]);
  }
  if (OptFlag[quodlibet]) {
    CenterLine(OptString[ActLang][quodlibet]);
  }
  StdChar('\n');

  if (CondFlag[gridchess] && OptFlag[writegrid])
    WriteGrid();

  SolFile= OrigSolFile;
} /* WritePosition */


void WriteGrid(void) 
{
  square square, square_a;
  int row, file;
  char    HLine[40];

  static char BorderL[]="+---a---b---c---d---e---f---g---h---+\n";
  static char HorizL[]="%c                                   %c\n";
  static char BlankL[]="|                                   |\n";

  StdChar('\n');
  StdString(BorderL);
  StdString(BlankL);

  for (row=0, square_a = square_a8;
       row<nr_rows_on_board;
       row++, square_a += dir_down) {
    char const *digits="87654321";
    sprintf(HLine, HorizL, digits[row], digits[row]);

    for (file=0, square= square_a;
         file<nr_files_on_board;
         file++, square += dir_right)
    {
      char g = (GridNum(square))%100;
      HLine[4*file+3]= g>9 ? (g/10)+'0' : ' ';
      HLine[4*file+4]= (g%10)+'0';
    }

    StdString(HLine);
    StdString(BlankL);
  }

  StdString(BorderL);
}

/**** LaTeX output ***** begin *****/

void LaTeXOpen(void) {
  fprintf(LaTeXFile, "\\documentclass{article}%%");
  if (!flag_regression)
    fprintf(LaTeXFile, "%s", VERSIONSTRING);
  fprintf(LaTeXFile, "\n");
  fprintf(LaTeXFile, "\\usepackage{diagram}\n");
  if (ActLang == German) {
    fprintf(LaTeXFile, "\\usepackage{german}\n");
  }
  fprintf(LaTeXFile, "\n\\begin{document}\n\n");
}

void LaTeXClose(void) {
  fprintf(LaTeXFile, "\n\\putsol\n\n\\end{document}\n");
}

void LaTeXStr(char *line) {
  while (*line) {
    switch (*line) {
    case '#':
      fprintf(LaTeXFile, "\\%c", *line);
      break;
    case '&':
      fprintf(LaTeXFile, "\\%c", *line);
      break;
    case '%':
      if (*(line+1) == '%') {
        /* it's introducing a comment */
        fprintf(LaTeXFile, "%%");
        line++;
      }
      else {
        fprintf(LaTeXFile, "\\%%");
      }
      break;
    case '0':
      if (strncmp(line, "0-0-0", 5) == 0) {
        fprintf(LaTeXFile, "{\\OOO}");
        line += 4;
      }
      else if (strncmp(line, "0-0", 3) == 0) {
        fprintf(LaTeXFile, "{\\OO}");
        line += 2;
      }
      else {
        fprintf(LaTeXFile, "%c", *line);
      }
      break;
    case '-':
      if (*(line+1) == '>') {   /* convert -> to \ra   FCO */
        fprintf(LaTeXFile, "{\\ra}");
        line++;
      } else {  /* ordinary minus */
        fprintf(LaTeXFile, "%c", *line);
      }
      break;

    default:
      fprintf(LaTeXFile, "%c", *line);
      fflush(LaTeXFile);         /* non-buffered output  FCO */
    }
    line++;
  }
}

void LaTeXEndDiagram(void) {
  char line[256];

  /* twins */
  if (ActTwin[0] != '\0') {
    fprintf(LaTeXFile, " \\twins{");
    /* remove the last "{\\newline} */
    ActTwin[strlen(ActTwin)-10]= '\0';
    LaTeXStr(ActTwin);
    fprintf(LaTeXFile, "}%%\n");
  }

  /* solution */
  fprintf(LaTeXFile, " \\solution{%%\n");
  rewind(SolFile);
  while (fgets(line, 255, SolFile)) {
    if (!strstr(line, GetMsgString(TimeString))) {
      if (strlen(line) > 1 && line[1] == ')') {
        /* twin */
        fprintf(LaTeXFile, "%c)", line[0]);
      }
      else if (strlen(line) > 2 && line[2] == ')') {
        if (line[0] == '+')        /* twin (continued) */
          fprintf(LaTeXFile, "%c)", line[1]);
        else
          fprintf(LaTeXFile, "%c%c)", line[0], line[1]);
      }
      else if (strlen(line) > 3 && line[3] == ')') {
        /* continued twinning and >z */
        fprintf(LaTeXFile, "%c%c)", line[1], line[2]);
      }
      if (strchr(line, '.')) {   /* line containing a move */
        LaTeXStr(line);
      }
    }
  }

  fprintf(LaTeXFile, " }%%\n");
  fclose(SolFile);

  if ((SolFile= tmpfile()) == NULL) {
    IoErrorMsg(WrOpenError,0);
  }

  if (!(OptFlag[solmenaces]
        || OptFlag[solflights]
        || OptFlag[nontrivial]
        || (OptFlag[intelligent] && maxsol_per_matingpos)
        || FlagTimeOut
        || FlagMaxSolsReached
        || (OptFlag[maxsols] && solutions>=maxsolutions)))
  {
    fprintf(LaTeXFile, " \\Co+%%");
    if (!flag_regression)
      fprintf(LaTeXFile, "%s", VERSIONSTRING);
    fprintf(LaTeXFile, "\n");
  }

  fprintf(LaTeXFile, "\\end{diagram}\n\\hfill\n");
}

void LaTeXBeginDiagram(void) {
  square *bnp;
  boolean firstpiece= true, fairypieces= false, holess= false,
    modifiedpieces=false;
  int i;
  PieSpec sp;
  Flags remspec[PieceCount];
  char ListSpec[PieSpCount][256];
  piece p;
  char    HolesSqList[256] = "";

  for (sp= Neutral; sp < PieSpCount; sp++)
    strcpy(ListSpec[sp], PieSpString[ActLang][sp]);

  fprintf(LaTeXFile, "\\begin{diagram}%%\n");

  /* authors */
  if (ActAuthor[0] != '\0') {
    if (strchr(ActAuthor, ',')) {
      /* , --> correct format */
      char *cp, *endcp = 0;

      while ((cp=strchr(ActAuthor, '\n'))) {
        *cp= ';';
        endcp= cp;
      }
      if (endcp)
        *endcp= '\0';

      sprintf(GlobalStr, " \\author{%s}%%%%\n", ActAuthor);
      LaTeXStr(GlobalStr);

      if (endcp)
        *endcp= '\n';
      while ((cp=strchr(ActAuthor, ';')))
        *cp= '\n';
    }
    else {
      /* reverse first and surnames */
      char *cp1, *cp2, *cp3;

      fprintf(LaTeXFile, " \\author{");
      cp1= ActAuthor;
      while ((cp2=strchr(cp1, '\n'))) {
        *cp2= '\0';
        if (cp1 != ActAuthor)
          fprintf(LaTeXFile, "; ");
        cp3= cp2;
        while (cp3 > cp1 && *cp3 != ' ')
          cp3--;
        /* wrong LaTeX output if the authors surname only given */
        if (cp3 == cp1) {
          /* we got only the surname ! */
          sprintf(GlobalStr, "%s, ", cp3);
        } else {
          /* we got firstname and surname */
          *cp3= '\0';
          sprintf(GlobalStr, "%s, %s", cp3+1, cp1);
        }
        LaTeXStr(GlobalStr);
        *cp3= *cp2= '\n';

        cp1= cp2+1;
      }
      fprintf(LaTeXFile, "}%%\n");
    }
  }

  /* source */
  /* format: [diagram number,] source [issue number,] [date] */
  if (ActOrigin[0] != '\0') {
    char *source= ActOrigin;
    char *date, *eol, *tmp;

    /* diagram number */
    while (strchr(CharChar, *source))
      source++;

    if (*source == ',') {
      *source= '\0';
      fprintf(LaTeXFile, " \\sourcenr{%s}%%\n", ActOrigin);
      *source= ',';
      while (*(++source) == ' ')
        ;
    }
    else {
      source= ActOrigin;
    }

    /* date */
    /* supported formats: year
    **            month/year
    **            month-month/year
    **            day. month. year
    **            day.-day. month. year
    */
    /* year */
    eol= date= strchr(source, '\n');
    *eol= '\0';

    while (strchr("0123456789-", *(date-1))) {
      date--;
    }

    if (date != eol) {
      /* sucessfully parsed a year */
      fprintf(LaTeXFile, " \\year{%s}%%\n", date);

      /* parse month(s) now */
      /* while (*(date-1) == ' ') date--; */
      switch (*(date-1)) {
      case '/':
        /* format is either month/year or month-month/year */
        date--;
        while (*(date-1) == ' ')
          date--;
        tmp= date;
        while (strchr("0123456789-", *(date-1)))
          date--;
        fprintf(LaTeXFile, " \\month%s{%.*s}%%\n",
                strchr(date, '-') ? "s" : "", (int)(tmp-date), date);
        break;

      case '.':
        /* format is either
           day. month. year or day.-day. month. year
        */
        date--;
        tmp= date;
        while (strchr("0123456789", *(date-1)))
          date--;
        fprintf(LaTeXFile, " \\month{%.*s}%%\n",
                (int)(tmp-date), date);
        /* now parse day(s) */
        while (*(--date) == ' ');
        tmp= date;
        while (strchr("0123456789-.", *(date-1)))
          date--;
        fprintf(LaTeXFile, " \\day{%.*s}%%\n",
                (int)(tmp-date), date);
        break;
      }
    } /* month(s), day(s) */

    /* issue number */
    while (*(date-1) == ' ')
      date--;
    if (*(date-1) == ',') {
      /* issue number found */
      tmp= --date;
      while (*(date-1) != ' ')
        date--;
      fprintf(LaTeXFile, " \\issue{%.*s}%%\n",
              (int)(tmp-date), date);
    } /* issue */

    /* default */
    /* source name or complete source if not interpretable */
    while (*(date-1) == ' ')
      date--;
    sprintf(GlobalStr,
            " \\source{%.*s}%%%%\n", (int)(date-source), source);
    LaTeXStr(GlobalStr);

    *eol= '\n';
  }

  /* award */
  if (ActAward[0] != '\0') {
    char *tour= strchr(ActAward, ',');
    char *eol= strchr(ActAward, '\n');
    *eol= '\0';
    if (tour) {
      fprintf(LaTeXFile,
              " \\award{%.*s}%%\n", (int)(tour-ActAward), ActAward);
      while (*(++tour) == ' ');
      fprintf(LaTeXFile, " \\tournament{%s}%%\n", tour);
    } else
      fprintf(LaTeXFile, " \\award{%s}%%\n", ActAward);
    *eol= '\n';
  }

  /* dedication */
  if (ActTitle[0] != '\0') {
    sprintf(GlobalStr, "\\dedication{%s}%%%%\n", ActTitle);
    LaTeXStr(GlobalStr);
  }

  /* pieces & twins */
  if (OptFlag[duplex]) {
    strcat(ActTwin, OptTab[duplex]);
    strcat(ActTwin, "{\\newline}");
  }
  else if (OptFlag[halfduplex]) {
    strcat(ActTwin, OptTab[halfduplex]);
    strcat(ActTwin, "{\\newline}");
  }
  if (OptFlag[quodlibet]) {
    strcat(ActTwin, OptTab[quodlibet]);
    strcat(ActTwin, "{\\newline}");
  }

  /* Just for visualizing imitators on the board. */                 
  if (CondFlag[imitators]) {
    for (i= 0; i < inum[1]; i++)
      e[isquare[i]]= -1;
  }


  fprintf(LaTeXFile, " \\pieces{");

  for (p= vide; p < PieceCount; p++)
    CLEARFL(remspec[p]);

  for (bnp= boardnum; *bnp; bnp++) {
    if (e[*bnp] == 1) {
      /* holes */
      if (holess)
        strcat(HolesSqList, ", ");
      else
        holess= true;
      AddSquare(HolesSqList, *bnp);
    } else if (e[*bnp] != vide) {
      p= abs(e[*bnp]);
      if (!firstpiece)
        fprintf(LaTeXFile, ", ");
      else
        firstpiece= false;

      fprintf(LaTeXFile, "%c%s%c%c",
              TSTFLAG(spec[*bnp], Neutral) ? 'n' :
              TSTFLAG(spec[*bnp], White)   ? 'w' : 's',
              LaTeXPiece(p),
              *bnp%onerow-200%onerow+'a',
              *bnp/onerow-200/onerow+'1');

      if (e[*bnp] == -1) {
        e[*bnp]= vide;
      }
      else if ((p > Bishop) && (LaTeXPiecesAbbr[abs(p)] != NULL)) {
        fairypieces= true;

        if (TSTFLAG(spec[*bnp], Neutral)) {
          SETFLAG(remspec[p], Neutral);
        }
        else if (TSTFLAG(spec[*bnp], White)) {
          SETFLAG(remspec[p], White);
        }
        else {
          SETFLAG(remspec[p], Black);
        }
      }

      for (sp= Neutral + 1; sp < PieSpCount; sp++) {
        if (TSTFLAG(spec[*bnp], sp)) {
          AddSquare(ListSpec[sp], *bnp);
        }
      }
    }
  }
  fprintf(LaTeXFile, "}%%\n");
  fflush(LaTeXFile);

  if (holess) {
    fprintf(LaTeXFile, " \\nofields{");
    fprintf(LaTeXFile, "%s}%%\n", HolesSqList);
    fprintf(LaTeXFile, " \\fieldframe{");
    fprintf(LaTeXFile, "%s}%%\n", HolesSqList);
  }

  for (sp= Neutral + 1; sp < PieSpCount; sp++)
    if (TSTFLAG(PieSpExFlags, sp))
      modifiedpieces =true;     /* to be used below */

  /* stipulation */
  fprintf(LaTeXFile, " \\stipulation{");
  LaTeXStr(ActStip);
  if (OptFlag[solapparent])
    fprintf(LaTeXFile, "*");
  if (OptFlag[appseul]) {
    fprintf(LaTeXFile,
            " %c{\\ra}", tolower(*PieSpString[ActLang][White]));
  }

  fprintf(LaTeXFile, "}%%\n");

  /* conditions */
  if (CondFlag[gridchess] && !OptFlag[suppressgrid]) {
    boolean entry=false;
    square *bnp;
    switch (gridvar)
    {

      case grid_normal:
        fprintf(LaTeXFile, " \\stdgrid%%\n");
        break;

      case grid_vertical_shift:
        fprintf(LaTeXFile, " \\gridlines{h018, h038, h058, h078, v208, v408, v608}%%\n");
        break;

      case grid_horizontal_shift:
        fprintf(LaTeXFile, " \\gridlines{h028, h048, h068, v108, v308, v508, v708}%%\n");
        break;

      case grid_diagonal_shift:
        fprintf(LaTeXFile, " \\gridlines{h018, h038, h058, h078, v108, v308, v508, v708}%%\n");
        break;

      case grid_orthogonal_lines:
        for (i=1; i<8; i++)
          if (GridNum(bas+i-1) != GridNum(bas+i))
          {
            if (!entry)
              fprintf(LaTeXFile, " \\gridlines{");
            else
              fprintf(LaTeXFile, ", ");
            entry= true;
            fprintf(LaTeXFile, " v%d08", i);
          }
        for (i=1; i<8; i++)
          if (GridNum(bas+24*(i-1)) != GridNum(bas+24*i))
          {
            if (!entry)
              fprintf(LaTeXFile, " \\gridlines{");
            else
              fprintf(LaTeXFile, ", ");
            entry= true;
            fprintf(LaTeXFile, " h0%d8", i);
          }
        if (entry)
          fprintf(LaTeXFile, "}%%\n");    
        break;
      
      /* of course, only the following block is necessary */
      case grid_irregular:
        for (bnp = boardnum; *bnp; bnp++)
        {
          int i= *bnp%24-8, j= *bnp/24-8;
          if (i && GridLegal((*bnp)-1, *bnp))
          {
            if (!entry)
              fprintf(LaTeXFile, " \\gridlines{");
            else
              fprintf(LaTeXFile, ", ");
            entry= true;
            fprintf(LaTeXFile, " v%d%d1", i, j);
          }
          if (j && GridLegal((*bnp)-24, *bnp))
          {
            if (!entry)
              fprintf(LaTeXFile, " \\gridlines{");
            else
              fprintf(LaTeXFile, ", ");
            entry= true;
            fprintf(LaTeXFile, " h%d%d1", i, j);
          }
        }
        if (entry)
          fprintf(LaTeXFile, "}%%\n");    
        break;
    }
  }
  WriteConditions(WCLaTeX);

  /* magical squares with frame */
  if (CondFlag[magicsquare]) {
    char    MagicSqList[256] = "";
    boolean firstpiece= true;
    square  i;
 
    fprintf(LaTeXFile, " \\fieldframe{");
    for (i= bas; i <= haut; i++)
      if (TSTFLAG(sq_spec[i], MagicSq)) {
        if (!firstpiece)
          strcat(MagicSqList, ", ");
        else
          firstpiece= false;
        AddSquare(MagicSqList, i);
      }
    fprintf(LaTeXFile, "%s}%%\n", MagicSqList);
  }

  /* fairy pieces, modified pieces, holes */
  if (fairypieces || holess || modifiedpieces) {
    boolean firstline= true;

    fprintf(LaTeXFile, " \\remark{");
    for (p= Bishop+1; p < PieceCount; p++) {
      int q;
      if (!remspec[p])
        continue;

      for (q= Bishop+1; q < p; q++) {
        if (remspec[q]
            && LaTeXPiecesAbbr[p][0] == LaTeXPiecesAbbr[q][0]
            && LaTeXPiecesAbbr[p][1] == LaTeXPiecesAbbr[q][1])
        {
          fprintf(stderr, "+++ Warning: "
                  "double representation '%s' for %s and %s\n",
                  LaTeXPiecesAbbr[q],
                  LaTeXPiecesFull[p], LaTeXPiecesFull[q]);
        }
      }

      if (!firstline)
        fprintf(LaTeXFile, "{\\newline}\n    ");
      fprintf(LaTeXFile, "\\mbox{");
      if (TSTFLAG(remspec[p], White))
        fprintf(LaTeXFile, "\\w%s ", LaTeXPiecesAbbr[p]);
      if (TSTFLAG(remspec[p], Black))
        fprintf(LaTeXFile, "\\s%s ", LaTeXPiecesAbbr[p]);
      if (TSTFLAG(remspec[p], Neutral))
        fprintf(LaTeXFile, "\\n%s ", LaTeXPiecesAbbr[p]);
      fprintf(LaTeXFile, "=%s}", LaTeXPiecesFull[p]);
      firstline= false;
    }

    if (modifiedpieces) {
      for (sp= Neutral + 1; sp < PieSpCount; sp++)
        if (TSTFLAG(PieSpExFlags, sp)) {
          if (!firstline)
            fprintf(LaTeXFile, "{\\newline}\n    ");
          fprintf(LaTeXFile, "%s\n", ListSpec[sp]);
          firstline= false;
        }
    }
 
    if (holess) {
      if (!firstline)
        fprintf(LaTeXFile, "{\\newline}\n    ");
      fprintf(LaTeXFile, "%s %s%%\n",
              CondString[ActLang][holes], HolesSqList);
    }
    fprintf(LaTeXFile, "}%%\n");
  } /* fairy, modified pieces, holes */
} /* LaTeXBeginDiagram */
/**** LaTeX output ***** end *****/

void Tabulate() {
#if defined(DEBUG)
  if (marge < 0)
#if defined(STANDALONE)
  {
    sprintf(GlobalStr, "error: marge = %d negative\n", marge);
    ErrString(GlobalStr);
    exit(9);
  }
#else
  IoErrorMsg(9,0);
#endif    /* STANDALONE */
#endif    /* DEBUG */
  if (marge > 0) {
    sprintf(GlobalStr, "%*c", marge, bl);
    StdString(GlobalStr);
  }
}

void WritePiece(piece p) {
  char p1;

  p= abs(p);
  if (p<Hunter0 || p >= (Hunter0 + maxnrhuntertypes)) {
    StdChar(UPCASE(PieceTab[p][0]));
    if ((p1= PieceTab[p][1]) != ' ') {
      StdChar(UPCASE(p1));
    }
  }
  else {
    unsigned int const i = p-Hunter0;
    assert(i<maxnrhuntertypes);
    WritePiece(huntertypes[i].away);
    StdChar('/');
    WritePiece(huntertypes[i].home);
  }
}

void WriteSquare(square i) {
  StdChar('a' - nr_files_on_board + i % onerow);
  if (OptFlag[duplex] && OptFlag[intelligent] && maincamp == noir) {
    StdChar('8' + nr_rows_on_board - i / onerow);
  }
  else {
    StdChar('1' - nr_rows_on_board + i / onerow);
  }
}


/******************** for standalone testing *********************/
#if defined(STANDALONE)
int main() {
  Token tk= BeginProblem;
  rb= rn= initsquare;
  InputStack[0]= stdin;
  do {
    memset((char *) exist,0,sizeof(exist));
    memset((char *) promonly,0,sizeof(promonly));
    memset((char *) isapril,0,sizeof(isapril));
    memset((char *) StipFlags,0,sizeof(StipFlags));
    memset((char *) OptFlag,0,sizeof(OptFlag));
    memset((char *) CondFlag,0,sizeof(CondFlag));
    memset((char *) e,0,sizeof(e));
    tk= ReadProblem(tk);
    if (!OptFlag[noboard]) {
      WritePosition();
    }
  } while (tk == NextProblem);
}
#endif
/* The input accepted by popeye is defined by the following grammar.
** If there is no space between two nonterminals, then there is also
** no other character allowed in the input. This holds for <SquareName>
** and <SquareList>. Other terminals must be seperated by any non
** alpha or non numeric character.
**
** <PopeyeInput>  ::= <BeginSpec> <PySpecList> <EndSpec>
**
** <BeginSpec>    ::= beginproblem
** <EndSpec>      ::= endproblem | nextproblem <PySpecList> <EndSpec>
**
** <PySpecList>   ::= <PySpec> | <PySpec> <PySpecList>
** <PySpec>   ::= <PieceSpec> | <StipSpec> | <CondSpec> | <AuthorSpec> |
**            <OrigSpec> | <KamikazeSpec> | <ImitatorSpec> |
**            <TitleSpec> | <RemarkSpec>
** <PieceSpec>    ::= pieces <PieceList>
** <KamikazeSpec> ::= kamikaze <PieceList>
** <ImitatorSpec> ::= imitator <SquareList>
** <PieceList>    ::= <MenSpec> <MenList> | <MenSpec> <MenList> <PieceList>
** <MenSpec>      ::= <ColorSpec> <AddSpecList>
** <ColorSpec>    ::= white | black | neutral
** <AddSpecList>  ::= <Empty> | <AddSpec>
** <AddSpec>      ::= royal
** <Empty>    ::=
**
** <MenList>      ::= <ManName> <SquareList> <Seperator> |
**            <ManName> <SquareList> <MenList>
** <ManName>      ::= as actually defined in POPEYE.
** <SquareList>    ::= <SquareName> | <SquareName><SquareList>
** <SquareName>    ::= <ColName><RowName>
** <ColName>      ::= a | b | c | d | e | f | g | h
** <RowName>      ::= 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8
**
** <StipSpec>     ::= stipulation <StipName> <Number>
** <StipName>     ::= as actually defined in popeye
** <Number>   ::= <Digit> | <Digit><Number>
** <Digit>    ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
**
** <CondSpec>     ::= condition <CondList>
** <CondList>     ::= <CondName> | <CondName> <CondList>
** <Condname>     ::= as actually defined in popeye
**
** <AuthorSpec>   ::= author <string> <eol>
** <OrigSpec>     ::= origin <string> <eol>
** <TitleSpec>    ::= title <string> <eol>
** <RemarkSpec>   ::= remark <string> <eol>
** <string>   ::= all characters except <eol>
** <eol>      ::= end of line
**
** Example:
** beginproblem
** author Erich Bartel
** origin Jugendschach 23, Nr.1234
** pieces
** white Ka1 KNf5,Ph2h3
** black Ka8 Qa6
** stipulation #2
** condition circe schwarzerrandzueger
** endproblem
**
** Every reserved identifier (i.E. token may be shortened in the input,
** as long as it is uniquely identified.
** There are some peculiarities about this rule:
** If you specify after pieces wh Ka1
** b Ka3
** with the intention to place the black king on a3, popeye will balk,
** due to the fact that he interpretes b as piece with name ... and ka3
** as squarelist. So you are encouraged to use at least four characters
** when switching from one color to the other.
** Therefore the above input could be shortened to
** beg
** au Erich Bartel
** or Jugendschach 23, Nr.1234
** pi
** wh Ka1 KNf5 PH2h3
** blac Ka8 Qa6
** st #2
** co cir schwarzerr
** end
**
*/
