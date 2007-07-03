/******************** MODIFICATIONS to pyio.c **************************
**
** Date       Who  What
**
** 2002/04/29 FCO  LaTeX changes
**
** 2003/05/18 NG   new option: beep    (if solution encountered)
**
**************************** End of List ******************************/

#ifdef macintosh	/* is always defined on macintosh's  SB */
#	define SEGMIO
#	include "pymac.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>	 /* to import prototype of 'atoi'  StH */

#include <ctype.h>			      /* H.D. */

#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"

/* This is pyio.c
** It comprises a new io-Module for popeye.
** The old io was awkward, nonsystematic and I didn't like it.
** Therefore here a new one.
** Despite the remarks in the README file, I give here three languages
** for the input: english, french, german. It can easily be extended
** if necessary.
**							ElB.
*/

#ifndef DATABASE   /* V3.39  TLi */
static
#endif
      char AlphaStip[20];

#define MAXNEST 10
#define UPCASE(c)   toupper(c)	    /* (c+('A'-'a'))   H.D. */
/* This is only correct, cause only lowercase letters are passed
   as arguments						ElB 1993/05/21
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
#define WCleft	      1
#define WCLaTeX       2

void WriteConditions(int alignment);	  /* V3.40  TLi */

/***** twinning ***** V3.40 TLi */

#define TwinMove	 0
#define TwinExchange	 1
#define TwinStip	 2
#define TwinAdd		 3
#define TwinRemove	 4
#define TwinContinued	 5
#define TwinRotate	 6
#define TwinCond	 7
#define TwinPolish	 8
#define TwinMirror	 9
#define TwinMirra1h1	10
#define TwinMirra1a8	11
#define TwinMirra1h8	12
#define TwinMirra8h1	13
#define TwinShift	14
#define TwinSubstitute	15
#define TwinCount	16

char	*TwinString[LangCount][TwinCount] = {
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
     /*15*/  "remplace"
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
     /*15*/  "ersetze"
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
     /*15*/  "substitute"
    }
};

/***** twinning ***** end */

static char	**TokenTab;	/* set according to language */
static char	**OptTab;	/* set according to language */
static char	**CondTab;	/* set according to language */

static char	 **VariantTypeTab; /* V3.50 SE */
static char    **ExtraCondTab;	/* V3.62 SE */
static char	**TwinTab;	/* -- " --  V3.40  TLi */
static char	LastChar;

static	FILE	*TraceFile;
static	FILE	*InputStack[MAXNEST];

static	FILE	*LaTeXFile, *SolFile;	/* V3.46  TLi */
static	  char *LaTeXPiecesAbbr[PieceCount];
static	  char *LaTeXPiecesFull[PieceCount];	  /* V3.46  TLi */
void LaTeXStr(char *line);		/* V3.46  TLi */
char *LaTeXStdPie[8] = { NULL, "C", "K", "B", "D", "S", "T", "L"};    /* V3.55	NG */

static	int	NestLevel=0;

extern echiquier ProofBoard, PosA;		      /* V3.50 SE */
extern square Proof_rb, Proof_rn, rbA, rnA;
extern smallint ProofSpec[64], SpecA[64];

void	OpenInput(char *s)
{
     if((InputStack[0] = fopen(s,"r")) == NULL)
     {
	  InputStack[0] = stdin;
     }
}

void	CloseInput(void)
{
     if(InputStack[0] != stdin)
     {
	  fclose(InputStack[0]);
     }
}

/* These two arrays should have the same size */
#define LINESIZE	256

static char	InputLine[LINESIZE];	/* This array contains the input as is */
static char	TokenLine[LINESIZE];	/* This array contains the lowercase input */

static char SpaceChar[] = " \t\n\r;,.:";
static char TokenChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ#=+-%>!.<()~/";
	       /* Steingewinn !  V3.03	TLi */
	       /* introductory move  V3.31  TLi */
	       /* h#/=	 V3.31	TLi -- removed V3.44  TLi */
	       /* h##!	 V3.32	TLi */
	       /* dia3.5  V3.35  TLi */
	       /* a1<-->h1  V3.41  TLi */
	       /* reci-h(=)#n  V3.44  TLi */
	       /* h~2  do ANY helpmove	V3.65  SE, NG */

static char CharChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		    /* V3.46  TLi */

static char SepraChar[] = "\n\r;,.:";

void pyfputc(char c, FILE *f)
{
#ifndef QUIET
     fputc(c,f);
     fflush(f);
     if (TraceFile) {
	  fputc(c,TraceFile);
	  fflush(TraceFile);
     }
     if (SolFile) {	     /* V3.46  TLi */
	  fputc(c,SolFile);
	  fflush(SolFile);
     }
#endif
}

void pyfputs(char *s, FILE *f)
{
#ifndef QUIET
     fputs(s,f);
     fflush(f);
     if (TraceFile) {
	  fputs(s,TraceFile);
	  fflush(TraceFile);
     }
     if (SolFile) { /* V3.46  TLi */
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

void StdString(char *s)
{
     pyfputs(s, stdout);
}

void ErrString(char *s)
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

static char NextChar(void)					/* H.D. 10.02.93 */
{
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

static char *ReadToEndOfLine(void)				/* H.D. 10.02.93 */
{
     char   ch,*p;

     while (strchr(SpaceChar,ch= NextChar()));
     *(p= InputLine)= ch;
     while ((*++p= NextChar()) != '\n');
     if (p >= (InputLine + sizeof(InputLine)))
	  FtlMsg(InpLineOverflow);
     *p= '\0';
     return InputLine;
}

static char *ReadNextTokStr(void)				/* H.D. 10.02.93 */
{
     char ch,*p,*t;

     ch= LastChar;
     while (strchr(SpaceChar,ch))
	  ch= NextChar();
     if (strchr(TokenChar,ch)) {
	  p= InputLine;
	  t= TokenLine;
	  do {
	       *p++= ch;
/*	       *t++= (isupper(ch)?tolower(ch):ch);	*/	/*
v3.74  NG */
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


boolean sncmp(char *a, char *b) {
    while (*b) {
/*	if ((isupper(*a)?tolower(*a):*a) != *b++) {	*/ /* V3.74  NG */
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

static void ReadBeginSpec(void) {		    /* H.D. 10.02.93 */
    char *tok;

    while (True) {
	TokenTab= TokenString[0];
	tok= ReadNextTokStr();
	for (ActLang= 0; ActLang<LangCount; ActLang++) {
	    TokenTab= &(TokenString[ActLang][0]);
	    if (GetUniqIndex(TokenCount,TokenTab,tok) == BeginProblem) {
		OptTab= &(OptString[ActLang][0]);
		CondTab= &(CondString[ActLang][0]);
		TwinTab= &(TwinString[ActLang][0]);	/* V3.40  TLi */
		VariantTypeTab= &(VariantTypeString[ActLang][0]);
							/* v3.50 SE */
		ExtraCondTab= &(ExtraCondString[ActLang][0]);
							/* V3.62 SE */
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

int FieldNum(char a,char b)
{
     if ('a' <= a && a <= 'h' && '1' <= b && b <= '8')
	  return bas + a - 'a' + (b - '1') * 24;	  /* V2.60  NG */
     else
	  return 0;
}

static boolean SetKing(smallint *kingfield, smallint field)
{
     if (*kingfield == initsquare) {
	  *kingfield= field;
	  return False;
     }
     else if (*kingfield != field) {
	  IoErrorMsg(OneKing, 0);
	  return True;
     }
     else
	  return False;
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

    if (strlen(tok) < 3) {		   /* V3.55, V3.57  TLi */
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
	    LaTeXPiecesAbbr[Name]= malloc(sizeof(char)*(strlen(tok)+1));
	    i= 0;
	    while (tok[i]) {
		LaTeXPiecesAbbr[Name][i]= tok[i++]+ 'A' - 'a';
	    }
	    LaTeXPiecesAbbr[Name][i]= tok[i];

	    tok= ReadToEndOfLine();
	    LaTeXPiecesFull[Name]= malloc(sizeof(char)*(strlen(tok)+1));
	    strcpy(LaTeXPiecesFull[Name], tok);

	    tok= ReadNextTokStr();
	}
    }

    return tok;       /* V3.55	TLi */
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

static char *ParseFieldList(
    char	*tok,
    PieNam	Name,
    Flags	Spec,
    char	echo)					/* V3.40  TLi */
{
    /* We interprete the tokenString as FieldList
       If we return always the next tokenstring
     */
    int     Field, FieldCnt= 0;

    while (True) {
	if (*tok && tok[1] && (Field=FieldNum(*tok,tok[1]))) {
	    if (e[Field] != vide) {			/* V3.44  TLi */
		if (!echo) {
		    WriteSquare(Field);
		    StdChar(' ');
		    Message(OverwritePiece);
		}
		if (Field == rb) {
		    rb= initsquare;
		}
		if (Field == rn) {
		    rn= initsquare;
		}
	    }
	    /* echo the piece if desired -- twinning   V3.40  TLi */
	    if (echo) {
		if (LaTeXout) {				/* V3.47  NG */
		    /* LaTeX  V3.46  TLi */
		    sprintf(GlobalStr,
		      "%s\\%c%s %c%c",
		      e[Field] == vide ? "+" : "",
		      TSTFLAG(Spec, Neutral)
			? 'n'
			: TSTFLAG(Spec, White) ? 'w' : 's',
		      LaTeXPiece(Name),
		      'a'-8+Field%24, '1'-8+Field/24);
		    strcat(ActTwin, GlobalStr);
		}
		if (e[Field] == vide) {
		    StdChar(echo);
		}
		WriteSpec(Spec, Name);			/* V3.50  TLi */
		WritePiece(Name);
		WriteSquare(Field);
		StdChar(' ');
	    }
	    /* Neutral = Black + White */
	    if (TSTFLAG(Spec, Neutral)) {		/* V3.33  TLi */
		Spec |= BIT(Black) + BIT(White);
	    }
	    spec[Field] = Spec;
	    e[Field] = TSTFLAG(Spec, White)		/* V3.33  TLi */
		       ? Name
		       : - Name;
	    if (Name == King || TSTFLAG(Spec,Royal)) {
		/* modif. fuer neutr. KK   V3.02  TLi, V3.47  NG */
		if (TSTFLAG(Spec, White)) {
		    if (SetKing(&rb, Field)) {
			return ReadNextTokStr();
		    }
		}
		if (TSTFLAG(Spec, Black)) {
		    if (SetKing(&rn, Field)) {
			return ReadNextTokStr();
		    }
		}
	    }
	    tok+= 2;
	    FieldCnt++;
	    continue;
	}
	if (FieldCnt) {
	    if (*tok) {
		ErrorMsg(WrongFieldList);
	    }
	    return ReadNextTokStr();
	}
	ErrorMsg(MissngFieldList);
	tok= ReadNextTokStr();
    }
}

static char *PrsPieNam(char *tok, Flags Spec, char echo)  /* V3.40  TLi */
{
     /* We read from tok the name of the piece
     **
     */
     int     l, NameCnt= 0;
     char    *btok;
     PieNam  Name;

     while (True) {
	  l= strlen(btok= tok); /* Save it, if we want to return it */
	  if (l&1) {
	       Name= GetPieNamIndex(*tok,' ');
	       tok++;
	  }
	  else {
	       Name= GetPieNamIndex(*tok,tok[1]);
	       tok+= 2;
	  }
	  if (Name >= King) {
	       if (l >= 3 && !strchr("12345678",tok[1]))
		    return btok;
		    /* We return here not the next tokenstring
		    ** since this string is not a Men/Fieldlist
		    ** and therefore deserves processing by
		    ** ParsePieSpec
		    */
	       NameCnt++;
	       if (!*tok)
		    tok= ReadNextTokStr();
	       tok= ParseFieldList(tok, Name, Spec, echo);  /* V3.40  TLi */
	       CLRFLAG(Spec, Royal);
	  }
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
    if (sq%24 < 15) {
	return ++sq;
    }
    else if ((sq > bas + 7) && (sq <= haut)) {
	return sq - 31;
    }
    else {
	return initsquare;
    }
}

square SetSquare(square sq, piece p, boolean bw, boolean *neut)
{
    e[sq]= bw ? -p : p;
    spec[sq]= bw ? BIT(Black) : BIT(White);
    if (*neut) {
	spec[sq]= BIT(Black) | BIT(White) | BIT(Neutral);
    }
    if (p == King) {
	if (bw)
	    SetKing(&rn, sq);
	else
	    SetKing(&rb, sq);
    }
    *neut= false;
    return NextSquare(sq);
}

static char *ParseForsyth(void) {
    piece pc;
    int num;
    square sq= 368;
    boolean NeutralFlag= false;
    char* tok= ReadNextTokStr();
    sprintf(GlobalStr, "  %s  \n", tok);
    while (sq && *tok) {
/* 	if (isdigit(*tok)) {	*/	/*V3.74  NG */
	if (isdigit((int)*tok)) {
	    num= (*tok++) - '0';
/*	    if (isdigit(*tok))	*/	/* V3.74  NG */
	    if (isdigit((int)*tok))
		num += num * 9 + (*tok++) - '0';
	    for (;num && sq;num--) {
		e[sq]= vide;
		spec[sq]= EmptySpec;
		sq= NextSquare(sq);
	    }
	    NeutralFlag= false;
	}
/*	else if (isalpha(*tok)) {	*/	/* V3.74  NG */
	else if (isalpha((int)*tok)) {
	    pc= GetPieNamIndex(*tok,' ');
	    if (pc >= King) {
		sq= SetSquare(sq, pc,
/*		 islower(InputLine[(tok++) - TokenLine]), &NeutralFlag);	*/	/* V3.74  NG */
		 islower((int)InputLine[(tok++) - TokenLine]), &NeutralFlag);
	    }
	    else {
		tok++;			 /* error */
	    }
	}
	else if (*tok == '.') {
	    if (*(tok+1) == '=') {
		NeutralFlag= true;
		tok++;
	    }
	    pc= GetPieNamIndex(*(tok+1), *(tok+2));
	    if (pc >= King) {
		sq= SetSquare(sq, pc,
/*		 islower(InputLine[(tok+1 - TokenLine)]), &NeutralFlag);	*/	/* V3.74  NG */
		 islower((int)InputLine[(tok+1 - TokenLine)]), &NeutralFlag);
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

static char *ParsePieSpec(char echo) {			/* V3.40  TLi */
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
		  && (TmpFlg != BIT(ps)))
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
	    tok= PrsPieNam(tok, PieSpFlags, echo);	/* V3.40  TLi */
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
    /* seriesmovers with introducory moves  V3.44  TLi */
    if (strstr(tok,"->")) {
	if ((introenonce= atoi(tok)) < 1) {
	    IoErrorMsg(WrongInt, 0);
	}
	StipFlags |= FlowBit(Intro);
	tok= strstr(tok, "->")+2;
    }
    if (strncmp("exact-", tok, 6) == 0) {   /* V3.0... TLi */
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
	strcpy(NonReciAlphaEnd, " dia");
	AlphaEnd= NonReciAlphaEnd;
	return tok+3;
    }
#ifndef DATABASE
    /* transform position a into position b  V3.70  SE */
    if (strncmp("a=>b", tok, 4) == 0) {
	int i;
	StipFlags|= SortBit(Proof);
	strcpy(NonReciAlphaEnd, " a=>b");
	AlphaEnd= NonReciAlphaEnd;
	for (i=maxsquare-1; i>=0; i--) {
	    PosA[i]=e[i];
	}
	for (i= 0; i< 64; i++) {
	    SpecA[boardnum[i]]=spec[i];
	    spec[i]= EmptySpec;
	    e[boardnum[i]]= vide;
	}
	rnA=rn;
	rbA=rb;
	rn=rb=initsquare;
	flag_atob= true;
	return tok+4;
    }
#endif
    return tok;
}

static char *ParseSort(char *tok)
{
     if (SortFlag(Proof))   /* proof games   V3.35   TLi */
	  return tok;

     switch (*tok) {
     case 'h':
	  StipFlags|= SortBit(Help);
	  if (*(++tok) == 's') {		  /* V2.90c  TLi */
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

boolean ParsedReciStip;		/* V3.44  TLi */

static char *ParsStips(char *tok) {
    if (SortFlag(Proof)) {		 /* proof games   V3.35   TLi */
	return tok;
    }

    ParsedReciStip= false;	/* V3.44  TLi */

    /* initialization */
    ReciDoubleMate =
    NonReciDoubleMate =
    CounterMate = false;		/* V3.32  TLi */

    /* parsing reci stip */
    if (FlowFlag(Reci) && *tok == '(' && strchr(tok, ')')) {

	char ReciStip[128];
	sprintf(ReciStip, "%s", tok+1);

	tok= strchr(ReciStip, ')');
	*tok= '\0';
	tok++;

	sprintf(ReciAlphaEnd, " %s", ReciStip);

	/* parsing of ## and # exchange    V3.45  NG */
	if (strstr(tok, "##!")) {
	    ReciStipulation= stip_doublemate;
	    ReciDoubleMate = true;
	    CounterMate = true;
	    strcpy(NonReciAlphaEnd, " ##!");
	    return tok+3;
	}
	else if (strstr(ReciStip, "##")) {
	    ReciStipulation= stip_doublemate;
	    ReciDoubleMate = true;
	}
	else if (strstr(ReciStip, "#=")) {		/* V3.60 SE */
	    ReciStipulation= stip_mate_or_stale;
	    ReciDoubleMate = true;
	}
	else if (strstr(ReciStip, "#")) {
	    ReciStipulation= stip_mate;
	}
	else if (strstr(ReciStip, "==")) {
	    /* parsing of == and = exchange    V3.45  TLi */
	    ReciStipulation= stip_dblstale;
	}
	else if (strstr(ReciStip, "!=")) {
	    ReciStipulation= stip_autostale;
	}
	else if (strstr(ReciStip, "=")) {
	    ReciStipulation= stip_stale;
	}
	else if (ReciStip[0] == 'z') {
	    ReciStipulation= stip_target;
	    ReciAlphaEnd[2]= '\0';
	    ReciTargetField= FieldNum(ReciStip[1], ReciStip[2]);
	    if (!ReciTargetField) {
		IoErrorMsg(MissngFieldList, 0);
		return (char *)0;
	    }
	}
	else if (strstr(ReciStip, "+")) {
	    ReciStipulation= stip_check;
	}
	else if (strstr(ReciStip, "x")) {
	    ReciStipulation= stip_capture;
	}
	else if (strstr(ReciStip, "%")) {
	    ReciStipulation= stip_steingewinn;
	}
	else if (strstr(ReciStip, "ep")) {
	    ReciStipulation= stip_ep;
	}
	else if (strstr(ReciStip, "ctr")) {
	    ReciStipulation= stip_circuitB;
	}
	else if (strstr(ReciStip, "<>r")) {
	    ReciStipulation= stip_exchangeB;
	}
	else if (strstr(ReciStip, "ct")) {
	    ReciStipulation= stip_circuit;
	}
	else if (strstr(ReciStip, "<>")) {
	    ReciStipulation= stip_exchange;
	}
	else if (strstr(ReciStip, "00")) {
	    ReciStipulation= stip_castling;
	    ReciAlphaEnd[0]= '\0';
	}
	else if (strstr(ReciStip, "~")) {
	    ReciStipulation= stip_any;
	}
	else { IoErrorMsg(UnrecStip, 0);
	    return (char *)0;
	}
	ParsedReciStip= true;				/* V3.44  TLi */
    } /* parsing reci stip */

    /* parsing ordinary stip */
    if (strstr(tok, "##!")) {
	NonReciStipulation= stip_doublemate;
	NonReciDoubleMate = true;
	CounterMate = true;
	strcpy(NonReciAlphaEnd, " ##!");
	return tok+3;
    }
    else if (strstr(tok, "##")) {
	NonReciStipulation= stip_doublemate;
	NonReciDoubleMate = true;
	strcpy(NonReciAlphaEnd, " ##");
	return tok+2;
    }
    else if (strstr(tok, "#=")) {			/* V3.60 SE */
	NonReciStipulation= stip_mate_or_stale;
	NonReciDoubleMate = true;
	strcpy(NonReciAlphaEnd, " #=");
	return tok+2;
    }
    else if (strstr(tok, "#")) {
	NonReciStipulation= stip_mate;
	strcpy(NonReciAlphaEnd, " #");
	return tok+1;
    }
    else if (strstr(tok, "==")) {
	/* parsing of == and = exchange			V3.45  TLi */
	NonReciStipulation= stip_dblstale;
	strcpy(NonReciAlphaEnd, " ==");
	return tok+2;
    }
    else
    if (strstr(tok, "!=")) {				/* V3.50 SE */
       NonReciStipulation= stip_autostale;
       strcpy(NonReciAlphaEnd, " !=");
       return tok+2;
    }
    else if (strstr(tok, "=")) {
	NonReciStipulation= stip_stale;
	strcpy(NonReciAlphaEnd, " =");
	return tok+1;
    }
    else if (tok[0] == 'z') {
	NonReciStipulation= stip_target;
	NonReciTargetField= FieldNum(tok[1], tok[2]);
	if (!NonReciTargetField) {
	    IoErrorMsg(MissngFieldList, 0);
	    return (char *)0;
	}
	strcpy(NonReciAlphaEnd, " z");
	return tok+3;
    }
    else if (strstr(tok, "+")) {
	NonReciStipulation= stip_check;
	strcpy(NonReciAlphaEnd, " +");
	return tok+1;
    }
    else if (strstr(tok, "x")) {
	NonReciStipulation= stip_capture;
	strcpy(NonReciAlphaEnd, " x");
	return tok+1;
    }
    else if (strstr(tok, "%")) {
	NonReciStipulation= stip_steingewinn;
	strcpy(NonReciAlphaEnd, " %");
	return tok+1;
    }
    else if (strstr(tok, "ep")) {
	NonReciStipulation= stip_ep;
	strcpy(NonReciAlphaEnd, "");
	return tok+2;
    }
    else if (strstr(tok, "ctr")) {
	NonReciStipulation= stip_circuitB;
	strcpy(NonReciAlphaEnd, "");
	return tok+3;
    }
    else if (strstr(tok, "<>r")) {
	NonReciStipulation= stip_exchangeB;
	strcpy(NonReciAlphaEnd, "");
	return tok+3;
    }
    else if (strstr(tok, "ct")) {
	NonReciStipulation= stip_circuit;
	strcpy(NonReciAlphaEnd, "");
	return tok+2;
    }
    else if (strstr(tok, "<>")) {
	NonReciStipulation= stip_exchange;
	strcpy(NonReciAlphaEnd, "");
	return tok+2;
    }
    else if (strstr(tok, "00")) {
	NonReciStipulation= stip_castling;
	strcpy(NonReciAlphaEnd, "");
	return tok+2;
    }
    else if (strstr(tok, "~")) {
	NonReciStipulation= stip_any;
	strcpy(NonReciAlphaEnd, "");
	return tok+1;		 /* V3.65  SE */
    }
    else {
	IoErrorMsg(UnrecStip, 0);
	return (char *)0;
    }
} /* ParsStips */

static char *ParseStip(void)			/* H.D. 10.02.93 */
{
     char *tok;

     StipFlags= 0;
     tok= ReadNextTokStr();
     strcpy(AlphaStip,tok);
     tok= ParsStips(ParseSort(ParseFlow(tok)));
     if (tok) {
	  /* set defaults   V3.44  TLi */
	  stipulation = NonReciStipulation;		/* V3.44  TLi */
	  TargetField = NonReciTargetField;		/* V3.44  TLi */
	  DoubleMate = NonReciDoubleMate;		/* V3.44  TLi */
	  AlphaEnd = NonReciAlphaEnd;			/* V3.44  TLi */

	  /* set reci stip if not parsed */
	  if (FlowFlag(Reci) && !ParsedReciStip) {    /* V3.44	TLi */
	      ReciStipulation= NonReciStipulation;
	      ReciTargetField = NonReciTargetField;
	      ReciDoubleMate = NonReciDoubleMate;
	      strcpy(ReciAlphaEnd, NonReciAlphaEnd);
	  }

	  if (!*tok) {
	       tok= ReadNextTokStr();
	       strcat(AlphaStip, tok);
	  }
	  if (!(SortFlag(Proof) && FlowFlag(Alternate))) {
							/* V3.35  TLi,
							   V3.42  NG */
	       if ((enonce=atoi(tok)) < 1)		/* V3.03  TLi */
		    IoErrorMsg(WrongInt, 0);
	  } else {
	       if ((enonce=2*atoi(tok)) < 0)		/* V3.03  TLi */
		    IoErrorMsg(WrongInt, 0);
	       while (*tok && '0' <= *tok && *tok <= '9')
		    tok++;
	       if (tok && *tok == '.' && (tok+1) && *(tok+1) == '5')
		    enonce++;
	  }
     }
     if (enonce && ActStip[0] == '\0')			/* V3.46  TLi */
	 strcpy(ActStip, AlphaStip);
     return ReadNextTokStr();
}

static char *ReadSquares(smallint which) {
    char     *tok = ReadNextTokStr();
    square   i;
    short    k, l;
    ply      n;
    smallint EpSquaresRead= 0;				/* V3.37  TLi */

    l=strlen(tok);
    if (l&1) {
	if (which != ReadFrischAuf) {
	    IoErrorMsg(WrongFieldList, 0);
	}
	return tok;
    }
    i= k= 0;
    while (*tok) {
	i= FieldNum(*tok, tok[1]);
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

	      case ReadEpSquares:			/* V3.37  TLi */
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

	      case ReadBlRoyalSq:			/* V3.50  TLi */
		rn= bl_royal_sq= i;
		break;

	      case ReadWhRoyalSq:			/* V3.50  TLi */
		rb= wh_royal_sq= i;
		break;

	      case ReadNoCastlingSquares:		/* V3.55  NG */
		switch (i) {
		  case 200:
		    no_castling&= 0x57;
		    break;
		  case 204:
		    no_castling&= 0x37;
		    break;
		  case 207:
		    no_castling&= 0x67;
		    break;
		  case 368:
		    no_castling&= 0x75;
		    break;
		  case 372:
		    no_castling&= 0x73;
		    break;
		  case 375:
		    no_castling&= 0x76;
		    break;
		  default:
		    break;
		}
		break;

	      default:
		SETFLAG(sq_spec[i], which);
		break;
	    }
	    tok+= 2;
	}
	else {
	    if (which != ReadFrischAuf || k != 0) {    /* V3.41b  TLi */
		IoErrorMsg(WrongFieldList, 0);
	    }
	    return tok;
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
    /* of a SquareList. */				/* V3.42  NG */
    if (which == ReadFrischAuf) {			/* V3.42  NG */
	tok = ReadNextTokStr();
    }
    return tok;
} /* ReadSquares */

static char *ParseRex(boolean *rex, Cond what) {	/* V3.55  TLi */
     char    *tok;
     tok= ReadNextTokStr();
     *rex= (what==GetUniqIndex(CondCount, CondTab, tok));
     if (*rex) {
	  tok= ReadNextTokStr();
    }
    return tok;
}

#define gpType 1
#define gpSentinelles 2
#define gpAntiCirce 3
static char *ParseVariant(boolean *type, int group) {	  /* SE, V3.50
							     NG */
    int	    VariantType;
    char    *tok=ReadNextTokStr();

    SentPionNeutral=False;
    *type= False;

    do {
	VariantType =
	  GetUniqIndex(VariantTypeCount, VariantTypeTab, tok);

	if (VariantType < -1) {
	    break;
	}

	if (VariantType == -1) {
	    IoErrorMsg(CondNotUniq,0);
	}
	else if (VariantType==TypeB && group==gpType) {
	    *type= True;
	}
	else if (VariantType==Type1 && group==gpType) { /* V3.73  NG */
	    sbtype1= True;
	}
	else if (VariantType==Type2 && group==gpType) { /* V3.73  NG */
	    sbtype2= True;
	}
	else if (VariantType==Type3 && group==gpType) { /* V3.73  NG */
	    sbtype3= True;
	}
	else if (VariantType==PionAdverse && group==gpSentinelles) {
	    *type= True;
	}
	else if (VariantType==PionNeutral && group==gpSentinelles) {
	    SentPionNeutral= True;
	}
	else if (VariantType==PionNoirMaximum && group==gpSentinelles) {
	    /* V3.60 SE */
	    tok= ReadNextTokStr();
	    max_pn= atoi(tok);
	    while (*tok && '0' <= *tok && *tok <= '9') {
		tok++;
	    }
	    if (max_pn < 0 || max_pn > 64)
		max_pn=8;
	}
	else if (VariantType==PionBlancMaximum && group==gpSentinelles)
	{
	    /* V3.60 SE */
	    tok= ReadNextTokStr();
	    max_pb= atoi(tok);
	    while (*tok && '0' <= *tok && *tok <= '9') {
		tok++;
	    }
	    if (max_pb < 0 || max_pb > 64) {
		max_pb=8;
	    }
	}
	else if (VariantType==PionTotalMaximum && group==gpSentinelles)
	{
	    /* V3.63 SE */
	    tok= ReadNextTokStr();
	    max_pt= atoi(tok);
	    while (*tok && '0' <= *tok && *tok <= '9') {
		tok++;
	    }
	    if (max_pb < 0 || max_pb > 64) {
		max_pb=16;
	    }
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
	else {
	    IoErrorMsg(NonsenseCombination,0);
	}
	tok= ReadNextTokStr();
    } while (group==gpSentinelles);

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

int  AntiCirType;					/* V3.39  TLi */
char ChameleonSequence[256];

char *ReadChameleonCirceSequence(void) {		/* V3.45  NG */
    piece	old_piece, new_piece;
    char	*tok, newpiece[3];

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

static char *ParseCond(void)			     /* H.D. 10.02.93 */
{
    char    *tok;
    int     indexx;
    int     CondCnt= 0;

    tok= ReadNextTokStr();			       /* V2.70c TLi */
    while ((indexx= GetUniqIndex(CondCount,CondTab,tok)) >= -2) {
	/* V3.62 SE */

	if (indexx == -2) {				  /* V3.62 SE */
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
	    tok= ReadNextTokStr();			 /* V3.1  TLi */
	    continue;
	}
	CondFlag[indexx]= True;

	CondCnt++;
	switch (indexx) {
	  case rexincl:					/* V2.70c TLi */
	    if (CondFlag[exact])			/* V3.20  NG */
		IoErrorMsg(NonSenseRexiExact, 0);
	    break;
	  case biheffalumps:				/* V3.55  TLi */
	    CondFlag[heffalumps]= True;			/* V3.57  NG */
	    CondFlag[biwoozles]= True;
	    CondFlag[woozles]= True;
	    break;
	  case heffalumps:				/* V3.55  TLi */
	    CondFlag[woozles]= True;
	    break;
	  case biwoozles:				/* V3.55  TLi */
	    CondFlag[woozles]= True;
	    break;
	  case hypervolage:
	    CondFlag[volage]= True;
	    break;
	  case leofamily:
	    CondFlag[chinoises]= True;
	    break;
	  case eiffel:					/* V3.60  TLi */
	    flag_madrasi= true;
	    break;
	  case contactgrid:				/* py2.4c  NG */
	    CondFlag[gridchess]=
	    CondFlag[koeko]= True;
	    break;
	  case imitators:				/* V2.4d  TM */
	    ReadSquares(ReadImitators);     /* V2.90  NG, V3.20  TLi */
	    break;
	  case blroyalsq:				/* V3.50  TLi */
	    ReadSquares(ReadBlRoyalSq);
	    break;
	  case whroyalsq:				/* V3.50  TLi */
	    ReadSquares(ReadWhRoyalSq);
	    break;
	  case magic:					/* V2.90c TLi */
	    ReadSquares(MagicSq);	     /* V2.90  NG, V3.20  TLi */
	    break;
	  case dbltibet:			       /* V2.90c  TLi */
	    CondFlag[tibet]= True;
	    break;
	  case holes:					/* V2.90  NG */
	    ReadSquares(ReadHoles);			/* V3.20  TLi */
	    break;
	  case blmax:					/* V3.0  TLi */
	    black_length= len_max;
	    flagblackmummer= true;
	    break;
	  case blmin:					/* V3.0  TLi */
	    black_length= len_min;
	    flagblackmummer= true;
	    break;
	  case blcapt:					/* V3.0  TLi */
	    black_length= len_capt;
	    flagblackmummer= true;
	    break;
	  case blfollow:				/* V3.0  TLi */
	    black_length= len_follow;
	    flagblackmummer= true;
	    break;
	  case whmax:					/* V3.0  TLi */
	    white_length= len_max;
	    flagwhitemummer= true;
	    break;
	  case whmin:					/* V3.0  TLi */
	    white_length= len_min;
	    flagwhitemummer= true;
	    break;
	  case whcapt:					/* V3.0  TLi */
	    white_length= len_capt;
	    flagwhitemummer= true;
	    break;
	  case whfollow:				/* V3.0  TLi */
	    white_length= len_follow;
	    flagwhitemummer= true;
	    break;
	  case duellist:				/* V3.0  TLi */
	    white_length= len_whduell;
	    black_length= len_blduell;
	    flagwhitemummer= flagblackmummer= true;
	    break;
	  case alphabetic:
	    white_length= len_alphabetic;
	    black_length= len_alphabetic;
	    flagwhitemummer= flagblackmummer= true;
	    break;
	  case trans_king:		     /* V3.02  TLi, V3.47  NG */
	    CondFlag[whtrans_king]= true;
	    CondFlag[bltrans_king]= true;
	    CondFlag[whrefl_king]= true;
	    CondFlag[blrefl_king]= true;
	    break;
	  case refl_king:		     /* V3.02  TLi, V3.47  NG */
	    CondFlag[whrefl_king]= true;
	    CondFlag[blrefl_king]= true;
	    break;
	  case whtrans_king:				/* V3.47  NG */
	    CondFlag[whrefl_king]= true;
	    break;
	  case bltrans_king:				/* V3.47  NG */
	    CondFlag[blrefl_king]= true;
	    break;
	  case antieinstein:				/* V3.50  TLi */
	    CondFlag[einstein]= true;
	    break;
	  case reveinstein:
	    CondFlag[einstein]= true;
	    break;
	  case whforsqu:				/* V3.20  NG */
	    ReadSquares(WhForcedSq);			/* V3.20  TLi */
	    white_length= len_whforcedsquare;
	    flagwhitemummer= true;
	    break;
	  case blforsqu:				/* V3.20  NG */
	    ReadSquares(BlForcedSq);			/* V3.20  TLi */
	    black_length= len_blforcedsquare;
	    flagblackmummer= true;
	    break;
	  case whconforsqu:				/* V3.20  NG */
	    ReadSquares(WhConsForcedSq);		/* V3.20  TLi */
	    wh_ultra=
	    wh_exact= true;
	    white_length= len_whforcedsquare;
	    flagwhitemummer= true;
	    break;
	  case blconforsqu:				/* V3.20  NG */
	    ReadSquares(BlConsForcedSq);		/* V3.20  TLi */
	    bl_ultra=
	    bl_exact= true;
	    black_length= len_blforcedsquare;
	    flagblackmummer= true;
	    break;
	    /*****  V3.1  TLi  *****/
	    /* different types of circe */
	  case couscousmirror:				/* V3.50  TLi */
	    anycirprom= true;
	    anycirce= true;
	    CondFlag[couscous]= true;
	    circerenai= renspiegel;
	    break;
	  case pwc:
	    circerenai= renpwc;
	    anycirprom= true;
	    anycirce= true;				/* V3.45  TLi */
	    break;					/* V3.45  TLi */
	  case couscous:
	    anycirprom= true;
	    anycirce= true;				/* V3.45  TLi */
	    break;					/* V3.45  TLi */
	  case parrain:
	    anycirprom= true;
	    anycirce= true;				/* V3.45  TLi */
	    break;					/* V3.45  TLi */
	  case circediametral:				/* V3.44  TLi */
	    circerenai= rendiametral;
	    anycirce= true;
	    break;
	  case frischauf:			       /* V3.41b  TLi */
	    anycirce= true;
	    break;
	  case circe:
	    anycirce= true;
	    break;
	  case chamcirce:
	    anycirce= true;
	    break;
	  case circemalefiquevertical:			/* V3.42  NG */
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
	  case circerank:				/* V3.45  TLi */
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
	    SetDiaRen(PieSpExFlags, haut);		/* V3.22  TLi */
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
	    SetDiaRen(PieSpExFlags, haut);		/* V3.22  TLi */
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
	    antirenai= renequipollents;
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
	    SetDiaRen(PieSpExFlags, haut);		/* V3.22  TLi */
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
	  case mars:					/* V3.46  SE */
	    marsrenai= rennormal;
	    anymars= true;
	    break;
	  case marsmirror:				/* V3.46  TLi */
	    marsrenai= renspiegel;
	    anymars= true;
	    break;
	  case phantom:					/* V3.47  NG */
	    marsrenai= rennormal;
	    anymars= is_phantomchess= true;
	    break;
	  case plus:
	    marsrenai= renplus;
	    anymars= true;
	    break;
	  case whprom_sq:				/* V3.57  NG */
	    ReadSquares(WhPromSq);
	    break;
	  case blprom_sq:				/* V3.57  NG */
	    ReadSquares(BlPromSq);
	    break;
	}

	switch (indexx) {				/* V2.70c TLi */
	  case frischauf:			/* V3.41b TLi */
	    tok= ReadSquares(ReadFrischAuf);
	    break;
	  case messigny:			/* V3.55 TLi */
	    tok= ParseRex(&rex_mess_ex, rexexcl);
	    break;
	  case woozles:					/* V3.55  TLi */
	    tok= ParseRex(&rex_wooz_ex, rexexcl);
	    break;
	  case biwoozles:
	    tok= ParseRex(&rex_wooz_ex, rexexcl);
	    break;
	  case immun:					/* V2.80c  TLi */
	    tok= ParseRex(&rex_immun, rexincl);
	    break;
	  case immunmalefique:
	    tok= ParseRex(&rex_immun, rexincl);
	    break;
	  case immundiagramm:
	    tok= ParseRex(&rex_immun, rexincl);
	    break;
	  case chamcirce:				/* V3.45  TLi */
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
	  case circemalefiquevertical:			/* V3.42  NG */
	    tok= ParseRex(&rex_circe, rexincl);
	    break;
	  case phantom:					/* V3.51 NG */
	    tok= ParseRex(&rex_phan, rexincl);
	    break;
	  case madras:
	    tok= ParseRex(&rex_mad, rexincl);
	    flag_madrasi= true;					/* V3.60 TLi */
	    break;
	  case isardam:					/* V3.50 SE */
	    tok= ParseVariant(&IsardamB, gpType);
	    break;
	  case patience:				/* V3.50 SE */
	    tok= ParseVariant(&PatienceB, gpType);
	    break;
	  case sentinelles:		      /* V3.50 SE */
	    tok= ParseVariant(&SentPionAdverse, gpSentinelles);
	    break;
	    /*****  exact-maxis  V3.1  TLi  *****/
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
	    /*****  anticirce type  V3.39  TLi	*****/
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
	  case singlebox:				/* V3.73  NG */
	    tok= ParseVariant(&SingleBoxType, gpType);
	    break;
	  case promotiononly:				/* V3.44  NG */
	    tok= ReadPieces(promotiononly);
	    break;
	  case april:					/* V3.44  NG */
	    tok= ReadPieces(april);
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

static char *ParseOpt(void) {			   /* H.D. 10.02.93 */
    int     indexx,OptCnt= 0;
    char    *tok, *ptr;					/* V3.62  TLi */

    tok= ReadNextTokStr();				/* V3.43  NG */
    while ((indexx= GetUniqIndex(OptCount, OptTab, tok)) >= -1) {
							/* V3.43  NG */
	if (indexx == -1) {
	    IoErrorMsg(OptNotUniq,0);
	    continue;
	}
	OptFlag[indexx]= True;
	OptCnt++;
	switch(indexx) {
	  case beep:					/* V3.77  NG */
	    tok= ReadNextTokStr();
	    if ((maxbeep= atoi(tok)) <= 0) {
		maxbeep= 1;
		/* we did NOT use tok */
		continue;
	    } else {
		/* we did use tok */
		break;
	    }
	  case maxtime:					/* V3.53  NG */
	    tok= ReadNextTokStr();
	    if ((maxsolvingtime= atoi(tok)) <= 0) {
		OptFlag[maxtime]= False;
		IoErrorMsg(WrongInt, 0);
		return ReadNextTokStr();
	    }
	    break;
	  case enpassant:				/* V3.37  TLi */
	    ReadSquares(ReadEpSquares);
	    break;
	  case maxsols:				       /* V3.12 StHoe */
	    tok= ReadNextTokStr();
	    if ((maxsolutions= atoi(tok)) <= 0) {
		OptFlag[maxsols]= False;
		IoErrorMsg(WrongInt, 0);
		return ReadNextTokStr();
	    }
	    break;
	  case intelligent:				/* V3.53  TLi */
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
	  case restart:					/* V3.44  TLi */
	    tok= ReadNextTokStr();
	    if ((RestartNbr= atoi(tok)) <= 0) {
		OptFlag[restart]= False;
		IoErrorMsg(WrongInt, 0);
		return ReadNextTokStr();
	    }
	    OptFlag[movenbr]= True;
	    break;
	  case solmenaces:		       /* changed V3.62  TLi */
	    droh= strtol(tok= ReadNextTokStr(), &ptr, 10);
	    if (tok == ptr) {
		droh= maxply;
		IoErrorMsg(WrongInt, 0);
		return ReadNextTokStr();
	    }
	    break;
	  case solflights:		      /* changed V3.62	TLi */
	    maxflights= strtol(tok= ReadNextTokStr(), &ptr, 10);
	    if (tok == ptr) {
		IoErrorMsg(WrongInt, 0);
		maxflights = 64;			/* V3.13  NG */
		return ReadNextTokStr();
	    }
	    break;
	  case soltout:					/* V2.90c TLi */
	    if ((maxdefen= atoi(tok= ReadNextTokStr())) <= 0) {
		IoErrorMsg(WrongInt, 0);
		return ReadNextTokStr();
	    }
	    break;					/* V2.1c  TLi */
	  case solessais:
	    /* for compatibility to older versions.  V2.90  NG */
	    OptFlag[soltout]= True;
	    maxdefen= 1;
	    break;
	  case nontrivial:				/* V3.32  TLi */
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
	  case postkeyplay:				/* V3.41  TLi */
	    OptFlag[solvariantes]= True;
	    break;
	  case nocastling:				/* V3.55  NG */
	    no_castling= 0x77;
	    ReadSquares(ReadNoCastlingSquares);
	    break;
	}
	tok= ReadNextTokStr();				 /* V3.43  NG */
    }
    if (! OptCnt) {
	IoErrorMsg(UnrecOption,0);
    }
    return tok;
}

/***** twinning ***** begin *****/

unsigned char TwinChar;

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
       case rot90:     ret= 24*(sq%24)-sq/24+23;      break;
       case rot180:    ret= 575-sq;		      break;
       case rot270:    ret= -24*(sq%24)+sq/24-23+575; break;
       case mirra1a8:  ret= sq%24+24*(23-sq/24);      break;
       case mirra1h1:  ret= (23-sq%24)+24*(sq/24);    break;
       case mirra8h1:  ret= 24*(sq%24)+sq/24;	      break;
       case mirra1h8:  ret= (23-sq/24)+24*(23-sq%24); break;
    }
    return ret;
}

void RotateMirror(int what) {
    piece	t_e[64];
    Flags	t_spec[64];
    square	t_rb, t_rn, sq1, sq2;
    imarr	t_isquare;
    int		i;

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

static char *ParseTwinRotate(void) {			/* V3.40  TLi */
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

    if (LaTeXout) {		    /* V3.52  NG */
	/* LaTeX  V3.46  TLi */
	sprintf(GlobalStr, "%s $%s^\\circ$", TwinTab[TwinRotate], tok);
	strcat(ActTwin, GlobalStr);
    }

    StdString(TwinTab[TwinRotate]);
    StdString(" ");
    StdString(tok);

    return ReadNextTokStr();
}

static char *ParseTwinMirror(void) {			/* V3.40  TLi */
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

static char *ParseTwinMove(int indexx) {		/* V3.40  TLi */
    square sq1= 0, sq2= 0;
    char *tok;
    piece p;
    Flags sp;

    /* read the first square */
    while (sq1 == 0) {
	tok= ReadNextTokStr();
	sq1= FieldNum(tok[0], tok[1]);
	if (sq1 == 0) {
	    ErrorMsg(WrongFieldList);
	    return ReadNextTokStr();
	}
    }

    /* read the second square */
    while (sq2 == 0) {
	tok= ReadNextTokStr();
	sq2= FieldNum(tok[0], tok[1]);
	if (sq2 == 0) {
	    ErrorMsg(WrongFieldList);
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
    if (LaTeXout) {					/* V3.47  NG */
	/* LaTeX  V3.46  TLi */
	sprintf(GlobalStr, "\\%c%s %c%c",
	  TSTFLAG(spec[sq1], Neutral)
	    ? 'n'
	    : TSTFLAG(spec[sq1], White) ? 'w' : 's',
	  LaTeXPiece(e[sq1]), 'a'-8+sq1%24, '1'-8+sq1/24);
	strcat(ActTwin, GlobalStr);
    }

    WriteSpec(spec[sq1], e[sq1]);
    WritePiece(e[sq1]);
    WriteSquare(sq1);
    if (indexx == TwinExchange) {
	StdString("<-->");
	WriteSpec(spec[sq2], e[sq2]);
	WritePiece(e[sq2]);
	if (LaTeXout) {					/* V3.47  NG */
	    strcat(ActTwin, "{\\lra}");			/* V3.74  FCO */
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
	if (LaTeXout) {					/* V3.52  NG */
	    strcat(ActTwin, "{\\ra}");			/* V3.74  FCO */
	}
    }
    WriteSquare(sq2);
    if (LaTeXout) {					 /* V3.47  NG */
	sprintf(GlobalStr, "%c%c", 'a'-8+sq2%24, '1'-8+sq2/24);
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

static char *ParseTwinShift(void) {			/* V3.40  TLi */
    square sq1= 0, sq2= 0, *bnp;
    char *tok;
    int diffrank, diffcol, minrank, maxrank, mincol, maxcol, r, c;

    /* read the first square */
    while (sq1 == 0) {
	tok= ReadNextTokStr();
	sq1= FieldNum(tok[0], tok[1]);
	if (sq1 == 0) {
	    ErrorMsg(WrongFieldList);
	}
    }

    /* read the second square */
    while (sq2 == 0) {
	tok= ReadNextTokStr();
	sq2= FieldNum(tok[0], tok[1]);
	if (sq2 == 0) {
	    ErrorMsg(WrongFieldList);
	}
    }

    /* issue the twinning */
    if (LaTeXout) {					/* V3.52  NG */
	/* LaTeX  V3.46  TLi */
	sprintf(GlobalStr, "%s %c%c$\\Rightarrow$%c%c",
	  TwinTab[TwinShift],
	  'a'-8+sq1%24, '1'-8+sq1/24, 'a'-8+sq2%24, '1'-8+sq2/24);
	strcat(ActTwin, GlobalStr);
    }

    StdString(TwinTab[TwinShift]);
    StdString(" ");
    WriteSquare(sq1);
    StdString(" ==> ");
    WriteSquare(sq2);

    diffrank= sq2/24-sq1/24;
    diffcol= sq2%24-sq1%24;

    minrank= 23;
    maxrank= 0;
    mincol= 23;
    maxcol= 0;

    for (bnp= boardnum; *bnp; bnp++) {
	if (e[*bnp] != vide) {
	    if (*bnp/24 < minrank) {
		minrank= *bnp/24;
	    }
	    if (*bnp/24 > maxrank) {
		maxrank= *bnp/24;
	    }
	    if (*bnp%24 < mincol) {
		mincol= *bnp%24;
	    }
	    if (*bnp%24 > maxcol) {
		maxcol= *bnp%24;
	    }
	}
    }

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
		    MovePieceFromTo(24*r+c, 24*(r+diffrank)+c);
		}
	    }
	}
	else if (diffrank < 0) {
	    for (c= 8; c <= 15; c++) {
		for (r= minrank; r <= maxrank; r++) {
		    MovePieceFromTo(24*r+c, 24*(r+diffrank)+c);
		}
	    }
	}

	/* move along ranks */
	if (diffcol > 0) {
	    for (c= maxcol; c >= mincol; c--) {
		for (r= 8; r <= 15; r++) {
		    MovePieceFromTo(24*r+c, 24*r+c+diffcol);
		}
	    }
	}
	else if (diffcol < 0) {				/* V3.44  TLi */
	    for (c= mincol; c <= maxcol; c++) {
		for (r= 8; r <= 15; r++) {
		    MovePieceFromTo(24*r+c, 24*r+c+diffcol);
		}
	    }
	}
    }

    /* read next token */
    return ReadNextTokStr();

} /* ParseTwinShift */

static char *ParseTwinRemove(void) {
    square	sq;
    char	*tok;
    boolean	WrongList;

    do {
	WrongList= False;
	tok= ReadNextTokStr();

	if (strlen(tok) % 2) {
	    WrongList= True;
	}
	else {
	    char *tok2= tok;

	    while (*tok2 && !WrongList) {
		if (FieldNum(tok2[0], tok2[1]) == 0) {
		    WrongList= True;
		}
		tok2 += 2;
	    }
	}
	if (WrongList) {
	    ErrorMsg(WrongFieldList);
	}
    } while (WrongList);

    while (*tok) {
	sq= FieldNum(tok[0], tok[1]);

	if (abs(e[sq]) < King) {			/* V3.54  TLi */
	    WriteSquare(sq);
	    StdString(": ");
	    Message(NothingToRemove);
	}
	else {
	    if (LaTeXout) {				 /* V3.47  NG */
		/* LaTeX */
		strcat(ActTwin, " --");
		strcat(ActTwin,
		  TSTFLAG(spec[sq], Neutral)
		  ? "\\n"
		  : TSTFLAG(spec[sq], White) ? "\\w" : "\\s");
		strcat(ActTwin,
		  LaTeXPiece(e[sq]));
		sprintf(GlobalStr, " %c%c", 'a'-8+sq%24, '1'-8+sq/24);
		strcat(ActTwin, GlobalStr);
	    }

	    StdString(" -");
	    WriteSpec(spec[sq], e[sq]);
	    WritePiece(e[sq]);
	    WriteSquare(sq);
	    e[sq]= vide;
	    spec[sq]= 0;
	    if (sq == rb) {				/* V3.64  TLi */
		rb= initsquare;				/* V3.64  TLi */
	    }
	    if (sq == rn) {				/* V3.64  TLi */
		rn= initsquare;				/* V3.64  TLi */
	    }
	}
	tok += 2;					/* V3.64  TLi */
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

    if (LaTeXout) {					/* V3.52  NG */
	strcat(ActTwin, TwinTab[TwinPolish]);		/* V3.46  TLi */
    }	/* LaTeXout */

    return ReadNextTokStr();
}

static char *ParseTwinSubstitute(void) {		/* V3.41  TLi */
    square	*bnp;
    piece	p_old, p_new;
    char	*tok;

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

    if (LaTeXout) {					 /* V3.47  NG */
	/* LaTeX  V3.46  TLi */
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
    int		indexx, i;
    char	*tok;
    boolean	continued= False,
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
	    if (LaTeXout) {				/* V3.52  NG */
		/* LaTeX  V3.46  TLi */
		strcat(ActTwin, "{\\newline}");		/* V3.74  FCO */
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
#ifndef DATABASE   /* V3.52  TLi */
		if (SortFlag(Proof)) {			  /* V3.50 SE */
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
		if (LaTeXout) {				 /* V3.52  NG */
		    strcat(ActTwin, "+");
		}
	    }

	    if (TwinChar <= 'z') {			/* V3.53  TLi */
		sprintf(GlobalStr, "%c) ", TwinChar);
	    }
	    else {
		sprintf(GlobalStr,
		  "%c%d) ",
		  (TwinChar-'a'-1)%('z'-'a') + 'a',
		  (TwinChar-'a'-1)/('z'-'a'));
	    }
	    StdString(GlobalStr);
	    if (LaTeXout) {				/* V3.52  NG */
		strcat(ActTwin, GlobalStr);
	    }
	}
	else {
	    StdString("  ");
	    if (LaTeXout) {				/* V3.52  NG */
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
	    if (LaTeXout) {				/* V3.52  NG */
		/* LaTeX  V3.46  TLi */
		strcat(ActTwin, AlphaStip);
		if (OptFlag[solapparent]) {
		    strcat(ActTwin, "*");
		}
		if (OptFlag[appseul]) {
		    char temp[5];
		    sprintf(temp, " %c{\\ra}",		/* V3.74  FCO */
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
	}
    }
} /* ParseTwin */

/***** twinning *****  end  *****/

/* new conditions: PromOnly, AprilChess */   /* V3.64  ThM, NG */
char *ReadPieces(int condition) {
    piece	tmp_piece;
    char	*tok;
    boolean	piece_read= False;			/* V3.57  NG */

    fflush(stdout);
    while (True) {
	switch (strlen(tok= ReadNextTokStr())) {
	  case 1:
	    tmp_piece= GetPieNamIndex(*tok,' ');
	    piece_read= True;				/* V3.57  NG */
	    break;
	  case 2:
	    tmp_piece= GetPieNamIndex(*tok,tok[1]);
	    piece_read= True;				/* V3.57  NG */
	    break;
	  default:
	    if (!piece_read) {				/* V3.57  NG */
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

    if (tk == BeginProblem) {
	LastChar= ' ';
	ReadBeginSpec();
    }
    if (tk == TwinProblem || tk == ZeroPosition) {	/* V3.40  TLi */
	if (tk == ZeroPosition) {			/* V3.41  TLi */
	    StdString(TokenTab[ZeroPosition]);
	    StdString("\n\n");
	    TwinChar= 'a'-1;
	    TwinStorePosition();
	}
	tok= ParseTwin();				/* V3.40  TLi */
	while (True) {
	    switch (tk= StringToToken(tok)) {
	      case -1:
		IoErrorMsg(ComNotUniq,0);
		tok= ReadNextTokStr();
		break;
	      case TwinProblem:				/* V3.40  TLi */
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
    else {						/* V3.40  TLi */
	tok= ReadNextTokStr();
	TwinChar= 'a';					/* V3.40  TLi */
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
	      case TwinProblem:				/* V3.40  TLi */
		if (TwinChar == 'a') {			/* V3.40  TLi */
		    TwinStorePosition();		/* V3.40  TLi */
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
	      case ZeroPosition:			/* V3.41  TLi */
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
	      case Award:				/* V3.46  TLi */
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
		    Message(NewLine);			/* V3.1  TLi */
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
		if ((TraceFile=fopen(ReadToEndOfLine(),"a")) == NULL) {
		    IoErrorMsg(WrOpenError,0);
		}
		fputs(StartUp, TraceFile);
		fputs(MMString, TraceFile);		/* V3.37  NG */
		fflush(TraceFile);
		tok= ReadNextTokStr();
		break;
	      case LaTeXPieces:		     /* V3.47  ElB 1998-01-01 */
		tok= ParseLaTeXPieces(ReadNextTokStr());
		break;
	      case LaTeXToken:				/* V3.46  TLi */
		LaTeXout= true;
		if (LaTeXFile) {
		    LaTeXClose();
		    fclose(LaTeXFile);
		}
		if ((LaTeXFile= fopen(ReadToEndOfLine(),"a")) == NULL) {
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
		    rb= 204;
		    rn= 372;
		}
		break;
	      case Forsyth:
		tok= ParseForsyth();
		break;
	      default:
		FtlMsg(InternalError);
	    }
	} /* while */
    }
}

void CenterLine(char *s)
{
#ifdef ATARI
#	ifdef __TURBOC__
	  sprintf(GlobalStr, "%s\n", s);
#	else	/* not __TURBOC__ */
	  sprintf(GlobalStr, "%*s\n", (36+strlen(s))/2, s);
#	endif	/* __TURBOC__ */
#else	/* not ATARI */
     /* sprintf(GlobalStr, "%*s\n", (36+(int)strlen(s))/2, s); */
     sprintf(GlobalStr, "%*s\n", (38+(int)strlen(s))/2, s);   /* V3.53	TLi */
#endif	/* ATARI */
     StdString(GlobalStr);
}

void MultiCenter(char *s) {
    char *p;

    while ((p=strchr(s,'\n'))) {
	*p= '\0';
	CenterLine(s);
	*p= '\n';					/* V3.46  TLi */
	s= p + 1;
    }
}

void AddSquare(char *List, square i) {
    char    add[4];

    add[0]= ' ';
    add[1]= 'a' - 8 + i % 24;
    add[2]= '1' - 8 + i / 24;
    add[3]= '\0';
    strcat(List, add);
}

void WriteConditions(int alignment) {			/* V3.40  TLi */
    Cond	cond;
    char	CondLine[256];
    int		i;
    boolean	CondPrinted= False;

    for (cond= 1; cond < CondCount; cond++) {	       /* V2.70c  TLi */
	if (!CondFlag[cond])
	    continue;

	if (cond == rexexcl)				/* V3.55  TLi */
	    continue;
	if (cond == exact || cond == ultra)		/* V3.1  TLi */
	    continue;
	if (cond == einstein
	  && (CondFlag[reveinstein] || CondFlag[antieinstein]))
							/* V3.50  TLi */
	    continue;
	if (  (cond == woozles
	       && (CondFlag[biwoozles]	   /* V3.55  TLi */
		   ||CondFlag[heffalumps]))
	    || ((cond == heffalumps || cond == biwoozles)
		 && CondFlag[biheffalumps]))
	{
	    continue;
	}

	if (cond == volage && CondFlag[hypervolage])	/* V2.1c  NG */
	    continue;

	if (cond == chinoises && CondFlag[leofamily])	/* V2.1c  NG */
	    continue;

	if (  (cond == gridchess || cond == koeko)	/* V2.4c  NG */
	    && CondFlag[contactgrid])
	{
	    continue;
	}
	if (cond == tibet && CondFlag[dbltibet])       /* V2.90c  TLi */
	    continue;

	if (cond == refl_king && CondFlag[trans_king])
	    continue;

	if (  cond == whrefl_king
	  && (CondFlag[refl_king] || CondFlag[whtrans_king]))
	{
	    continue;
	}

	if (cond == blrefl_king
	  && (CondFlag[refl_king] || CondFlag[bltrans_king]))
	{
	    continue;
	}

	if (cond == whtrans_king && CondFlag[trans_king])
	    continue;

	if (cond == bltrans_king && CondFlag[trans_king])
	    continue;

	if (cond == holes)			/* V2.90  NG */
	    continue;

	if (cond == couscous && CondFlag[couscousmirror])  /* V3.46  TLi */
	    continue;

	strcpy(CondLine, CondTab[cond]);	/* V3.40  TLi */

	if ((cond == blmax || cond == whmax) && flagmaxi) /* V3.62 SE */
	    strcpy(CondLine, ExtraCondTab[maxi]);

	if (  (cond == blackultraschachzwang
	       || cond == whiteultraschachzwang)
	    && flagultraschachzwang)
	{
	    strcpy(CondLine, ExtraCondTab[ultraschachzwang]);
	}

	if (cond == sentinelles && flagparasent) {	  /* V3.62 SE */
	    strcpy(CondLine, "Para");
	    strcat(CondLine, CondTab[cond]);
	}

	if (cond == promotiononly) {		    /* V3.44  NG */
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
		/* output of promotiononly for now.  V3.44  NG	  */
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

	if (cond == imitators) {		/* V2.4d  TM */
	    for (i= 0; i < inum[1]; i++) {
		AddSquare(CondLine, isquare[i]);
	    }
	}

	if (cond == noiprom && !CondFlag[imitators])	/* V3.02  TLi */
	    continue;

	if (cond == magic) {			/* V2.90c  TLi */
	    square  i;
	    for (i= bas; i <= haut; i++) {
		if (TSTFLAG(sq_spec[i], MagicSq)) {
		    AddSquare(CondLine, i);
		}
	    }
	}
	if (cond == whforsqu) {			/* V3.20  NG */
	    square  i;
	    for (i= bas; i <= haut; i++) {
		if (TSTFLAG(sq_spec[i], WhForcedSq)) {	/* V3.20  TLi */
		    AddSquare(CondLine, i);
		}
	    }
	}
	if (cond == blforsqu) {			/* V3.20  NG */
	    square  i;
	    for (i= bas; i <= haut; i++) {
		if (TSTFLAG(sq_spec[i], BlForcedSq)) { /* V3.20  TLi */
		    AddSquare(CondLine, i);
		}
	    }
	}

	if (cond == whconforsqu) {		   /* V3.20  NG */
	    square  i;
	    for (i= bas; i <= haut; i++) {
		if (TSTFLAG(sq_spec[i], WhConsForcedSq)) {
							/* V3.20  TLi */
		    AddSquare(CondLine, i);
		}
	    }
	}

	if (cond == blconforsqu) {			/* V3.20  NG */
	    square  i;
	    for (i= bas; i <= haut; i++) {
		if (TSTFLAG(sq_spec[i], BlConsForcedSq)) {
							/* V3.20  TLi */
		    AddSquare(CondLine, i);
		}
	    }
	}

	if (cond == whprom_sq) {			/* V3.57  NG */
	    square  i;
	    for (i= bas; i <= haut; i++) {
		if (TSTFLAG(sq_spec[i], WhPromSq)) {
		    AddSquare(CondLine, i);
		}
	    }
	}
	if (cond == blprom_sq) {			/* V3.57  NG */
	    square  i;
	    for (i= bas; i <= haut; i++) {
		if (TSTFLAG(sq_spec[i], BlPromSq)) {
		    AddSquare(CondLine, i);
		}
	    }
	}

	if (cond == blroyalsq) {			/* V3.50  TLi */
	    AddSquare(CondLine, bl_royal_sq);
	}

	if (cond == whroyalsq) {			/* V3.50  TLi */
	    AddSquare(CondLine, wh_royal_sq);
	}

	if ((cond == madras && rex_mad)			/* V2.90c TLi */
	  || (cond == phantom && rex_phan)		/* V3.51  NG */
	  || (rex_immun
	      && (cond == immun
		  || cond == immunmalefique
		  || cond == immundiagramm))
	  || (rex_circe
	      && (cond == circe
		  || cond == circemalefique
		  || cond == circediametral
		  || cond == circemalefiquevertical	/* V3.42  NG */
		  || cond == circeclone
		  || cond == circeclonemalefique
		  || cond == circediagramm)))
	{
	    strcat(CondLine, "	");
	    strcat(CondLine, CondTab[rexincl]);
	}

	if (  (rex_mess_ex && cond == messigny)		/* V3.55  TLi */
	    || (rex_wooz_ex
		&& (cond == woozles
		    || cond == biwoozles)))
	{
	    strcat(CondLine, "	");
	    strcat(CondLine, CondTab[rexexcl]);
	}

	if (cond == chamcirce && ChameleonSequence[0]) {/* V3.45  TLi */
	    strcat(CondLine, "	");
	    strcat(CondLine, ChameleonSequence);
	}

	if ((cond == isardam) && IsardamB) {		  /* V3.50 SE */
	    strcat(CondLine, "	");
	    strcat(CondLine, VariantTypeString[ActLang][TypeB]);
	}

	if ((cond == patience) && PatienceB) {		 /* V3.50 SE */
	    strcat(CondLine, "	");
	    strcat(CondLine, VariantTypeString[ActLang][TypeB]);
	}

	if (cond == singlebox) {			/* V3.73 NG */
	    strcat(CondLine, "	");
	    if (sbtype1)
		strcat(CondLine, VariantTypeString[ActLang][Type1]);
	    if (sbtype2)
		strcat(CondLine, VariantTypeString[ActLang][Type2]);
	    if (sbtype3)
		strcat(CondLine, VariantTypeString[ActLang][Type3]);
	}

	if (cond == sentinelles) {			/* V3.63  NG */
	    char pawns[7];
	    if (sentinelb == pbb)
		strcat(CondLine, " Berolina");
	    if (SentPionAdverse) {			/* V3.50 SE */
		strcat(CondLine, "  ");
		strcat(CondLine,
		  VariantTypeString[ActLang][PionAdverse]);
	    }
	    if (SentPionNeutral) {			/* V3.50 SE */
		strcat(CondLine, "  ");
		strcat(CondLine,
		  VariantTypeString[ActLang][PionNeutral]);
	    }
	    if (max_pn !=8 || max_pb != 8) {		/* V3.60 SE */
		sprintf(pawns, " %i/%i", max_pb, max_pn);
		strcat (CondLine, pawns);
	    }
	    if (max_pt != 16) {				/* V3.63 SE */
		sprintf(pawns, " //%i", max_pt);
		strcat (CondLine, pawns);
	    }
	}

	switch (cond) {					/* V3.39  TLi */
	  case anti:
	  case antispiegel:
	  case antidiagramm:
	  case antifile:
	  case antisymmetrie:
	  case antispiegelfile:
	  case antiantipoden:
	  case antiequipollents:
	    strcat(CondLine, "	 ");
	    strcat(CondLine, VariantTypeString[ActLang][AntiCirType]);
							/* v3.50 SE */
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
	}
	switch (alignment) {

	  case WCcentered:
	    CenterLine(CondLine);
	    break;

	  case WCLaTeX:
	    if (CondPrinted) {
		fprintf(LaTeXFile, "{\\newline}\n   ");	/* V3.74  FCO */
	    }
	    else {
		fprintf(LaTeXFile, " \\condition{");
	    }
	    LaTeXStr(CondLine);
	    break;

	  case WCleft:
	    if (CondPrinted) {
		if (LaTeXout) {				/* V3.52  NG */
		    strcat(ActTwin, ", ");
		}
		StdString("\n	");
	    }
	    StdString(CondLine);
	    if (LaTeXout) {				/* V3.52  NG */
		strcat(ActTwin, CondLine);
	    }
	    break;
	}
	CondPrinted= True;
    }

    if (alignment == WCLaTeX && CondPrinted) {		/* V3.46  TLi */
	fprintf(LaTeXFile, "}%%\n");
    }
} /* WriteConditions */

void WritePosition() {
    smallint field, i, j, nBlack, nWhite, nNeutr;
    piece   p,pp;
    char    HLine1[40];
    char    PieCnts[20];
    char    StipOptStr[40];
    PieSpec sp;			     /* V3.1  TLi */
    char    ListSpec[PieSpCount][256];	    /* V3.1  TLi */
    FILE    *OrigSolFile= SolFile;	     /* V3.46  TLi */

		 /* V3.31  ElB */
    static char BorderL[]="+---a---b---c---d---e---f---g---h---+\n";
    static char HorizL[]="|   .   .   .   .   .   .   .   .   |\n";
    static char BlankL[]="|                                   |\n";
		 /* V3.31  ElB */

    SolFile= NULL;			     /* V3.46  TLi */

    for (sp= Neutral; sp < PieSpCount; sp++) {		/* V3.1  TLi */
	strcpy(ListSpec[sp], PieSpString[ActLang][sp]);
    }

    StdChar('\n');
    MultiCenter(ActAuthor);
    MultiCenter(ActOrigin);
    MultiCenter(ActAward);		     /* V3.46  TLi */
    MultiCenter(ActTitle);


    nBlack= nWhite= nNeutr= 0;
    StdChar('\n');
    StdString(BorderL);
    StdString(BlankL);
    field= haut - 7;			     /* V2.60  NG */

    if (CondFlag[imitators]) {		   /* Just for visualizing    */
	for (i= 0; i < inum[1]; i++) {	       /* imitators on the    */
	    e[isquare[i]]= -1;		   /* board. V2.60, V3.56  NG */
	}
    }

    for (i=0; i<8; i++) {				/* V3.31  ElB */
	char *digits="87654321";
	strcpy(HLine1,HorizL);
	HLine1[0]= digits[i];
	HLine1[sizeof(HorizL)-3]= digits[i];
	   /* V3.31  ElB */

	for (j= 1; j <= 8; j++, field++) {
	    char *h1;
	    h1= HLine1 + (j * 4);
	    if ((pp= abs(p= e[field])) < King) {
		if (p == -1) {				/* V2.90  NG */
		    /* this is a imitator ! */
		    *h1= 'I';
		    e[field]= vide; /* "delete" imitator */
		}
		else if (p == obs) {			/* V2.90  NG */
		    /* this is a hole ! */
		    *h1= ' ';
		}
		/* else:  the field is empty ! */
		continue;
	    }
	    /*****  V3.1  TLi  *****/
	    for (sp= Neutral + 1; sp < PieSpCount; sp++) {
		if (TSTFLAG(spec[field], sp)) {
		    AddSquare(ListSpec[sp], field);
		}
	    }

	    if ((*h1= PieceTab[pp][1]) != ' ') {
		*h1= UPCASE(*h1);
		h1--;
	    }
	    *h1--= UPCASE(PieceTab[pp][0]);
	    if (p < 0) {
		*h1= '-';
	    }
	    if (TSTFLAG(spec[field], Neutral)) {
		nNeutr++;
		*h1= '=';
	    }
	    else if (p < 0) {
		nBlack++;
	    }
	    else {
		nWhite++;
	    }
	}
	StdString(HLine1);
	StdString(BlankL);
	field-= 32;			 /* V2.60  NG */
    }
    StdString(BorderL);
    if (nNeutr) {
	sprintf(PieCnts, "%d + %d + %dn", nWhite, nBlack, nNeutr);
    }
    else {
	sprintf(PieCnts, "%d + %d", nWhite, nBlack);
    }

    strcpy(StipOptStr, AlphaStip);
    /* V3.12  TLi */
    if (droh < enonce - 1) {	   /* changed V3.62  TLi */
	sprintf(StipOptStr+strlen(StipOptStr), "/%d", droh);
	if (maxflights < 64) {
	    sprintf(StipOptStr+strlen(StipOptStr), "/%d", maxflights);
	}
    }
    else {
	if (maxflights < 64) {
	    sprintf(StipOptStr+strlen(StipOptStr), "//%d", maxflights);
	}
    }
    /* V3.32  TLi */
    if (NonTrivialLength < enonce - 1) {	/* changed V3.62  TLi */
	sprintf(StipOptStr+strlen(StipOptStr),
	  ";%d,%d", NonTrivialNumber, NonTrivialLength);
    }

    sprintf(GlobalStr, "  %-20s%13s\n", StipOptStr, PieCnts);
    StdString(GlobalStr);

    /*****  V3.1  TLi	*****/
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

    SolFile= OrigSolFile;	     /* V3.46  TLi */
} /* WritePosition */

/**** LaTeX output ***** begin *****/

void LaTeXOpen(void) {			/* V3.46  TLi */
    /* LaTeX2e stuff */
    fprintf(LaTeXFile, "\\documentclass{article}%%%s\n", VERSIONSTRING);
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
          case '-':		/* V3.74  FCO */
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

void LaTeXEndDiagram(void) {				/* V3.46  TLi */
    char line[256];

    /* twins */
    if (ActTwin[0] != '\0') {
	fprintf(LaTeXFile, " \\twins{");
	/* remove the last "{\\newline} */		/* V3.74  FCO */
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
		if (line[0] == '+')	       /* twin (continued) */
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

    if	(!(OptFlag[solmenaces]
	    || OptFlag[solflights]
	    || OptFlag[nontrivial]
	    || (OptFlag[intelligent]			/* V3.53  TLi */
		&& maxsol_per_matingpos)
	    || FlagTimeOut				/* V3.54  NG */
	    || FlagMaxSolsReached			/* V3.60  NG */
	    || (OptFlag[maxsols]			/* V3.54  NG */
		&& (solutions >= maxsolutions))))
    {
	fprintf(LaTeXFile, " \\Co+%%%s\n", VERSIONSTRING);
    }

    fprintf(LaTeXFile, "\\end{diagram}\n\\hfill\n");
}

void LaTeXBeginDiagram(void) {				/* V3.46  TLi */
    square *bnp;
    boolean firstpiece= true, fairypieces= false, holess= false,
        modifiedpieces=false;
    smallint i;
    PieSpec sp;
    Flags remspec[PieceCount];
    char ListSpec[PieSpCount][256];
    piece p;
    char    HolesSqList[256] = "";

    for (sp= Neutral; sp < PieSpCount; sp++)        	/* V3.74  FCO */
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
		*cp3= '\0';
		sprintf(GlobalStr, "%s, %s", cp3+1, cp1);
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
	**		      month/year
	**		      month-month/year
	**		      day. month. year
	**		      day.-day. month. year
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
	strcat(ActTwin, "{\\newline}");		/* V3.74  FCO */
    }
    else if (OptFlag[halfduplex]) {
	strcat(ActTwin, OptTab[halfduplex]);
	strcat(ActTwin, "{\\newline}");		/* V3.74  FCO */
    }
    if (OptFlag[quodlibet]) {
	strcat(ActTwin, OptTab[quodlibet]);
	strcat(ActTwin, "{\\newline}");		/* V3.74  FCO */
    }

    if (CondFlag[imitators]) {		    /* Just for visualizing */
	 for (i= 0; i < inum[1]; i++)	 /* imitators on the	 */
	      e[isquare[i]]= -1;      /* board.     V2.60  NG */
    }


    fprintf(LaTeXFile, " \\pieces{");

    for (p= vide; p < PieceCount; p++)
	CLEARFL(remspec[p]);

    for (bnp= boardnum; *bnp; bnp++) {
	if (e[*bnp] == 1) {		/* V3.74  TLi */
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
	      *bnp%24-200%24+'a',
	      *bnp/24-200/24+'1');

	    if (e[*bnp] == -1) {
		e[*bnp]= vide;
	    }
	    else if ((p > Bishop) && (LaTeXPiecesAbbr[abs(p)] != NULL)) {
							/* V3.74  TLi */
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

	    /*****  V3.1  TLi  *****/
	    for (sp= Neutral + 1; sp < PieSpCount; sp++) {
	    if (TSTFLAG(spec[*bnp], sp)) {
		    AddSquare(ListSpec[sp], *bnp);
		}
	    }
	}
    }
    fprintf(LaTeXFile, "}%%\n");
    fflush(LaTeXFile);

    if (holess) {				/* V3.74  TLi */
        fprintf(LaTeXFile, " \\nofields{");
        fprintf(LaTeXFile, "%s}%%\n", HolesSqList);
        fprintf(LaTeXFile, " \\fieldframe{");
        fprintf(LaTeXFile, "%s}%%\n", HolesSqList);
    }

    for (sp= Neutral + 1; sp < PieSpCount; sp++)	/* V3.74  FCO */
        if (TSTFLAG(PieSpExFlags, sp))
            modifiedpieces =true;  	/* to be used below  FCO */

    /* stipulation */
    fprintf(LaTeXFile, " \\stipulation{");
    LaTeXStr(ActStip);
    if (OptFlag[solapparent])
	fprintf(LaTeXFile, "*");
    if (OptFlag[appseul]) {
	fprintf(LaTeXFile,
	  " %c{\\ra}", tolower(*PieSpString[ActLang][White]));	/* V3.74  FCO */
    }

    fprintf(LaTeXFile, "}%%\n");

    /* conditions */
    if (CondFlag[gridchess]) {
	fprintf(LaTeXFile, " \\stdgrid%%\n");
    }
    WriteConditions(WCLaTeX);

    /* magical fields with frame ;  FCO */
    if (CondFlag[magic]) {
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
    if (fairypieces || holess || modifiedpieces) {	/* V3.74  FCO/TLi */
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
		fprintf(LaTeXFile, "{\\newline}\n    ");	/* V3.74  FCO */
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

        if (modifiedpieces) {				/* V3.74  FCO */
            for (sp= Neutral + 1; sp < PieSpCount; sp++)
                if (TSTFLAG(PieSpExFlags, sp)) {
                    if (!firstline)
                        fprintf(LaTeXFile, "{\\newline}\n    ");
                    fprintf(LaTeXFile, "%s\n", ListSpec[sp]);
                    firstline= false;
            }
        }
 
        if (holess) {					/* V3.74  TLi */
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
#ifdef DEBUG
    if (marge < 0)
#ifdef STANDALONE
    {
	sprintf(GlobalStr, "error: marge = %d negative\n", marge);
	ErrString(GlobalStr);
	exit(9);
    }
#else
    IoErrorMsg(9,0);
#endif	  /* STANDALONE */
#endif	  /* DEBUG */
    if (marge > 0) {
	sprintf(GlobalStr, "%*c", marge, bl);
	StdString(GlobalStr);
    }
}

void WritePiece(piece p) {
    char p1;

    p= abs(p);
    StdChar(UPCASE(PieceTab[p][0]));
    if ((p1= PieceTab[p][1]) != ' ') {
	StdChar(UPCASE(p1));
    }
}

void WriteSquare(square i) {
    StdChar('a' - 8 + i % 24);				/* V2.60  NG */
    if (OptFlag[duplex] && OptFlag[intelligent] && maincamp == noir) {
	/* V3.50  TLi */
	StdChar('8' + 8 - i / 24);
    }
    else {
	StdChar('1' - 8 + i / 24);			/* V2.60  NG */
    }
}


/******************** for standalone testing *********************/
#ifdef STANDALONE
int main() {
    Token tk= BeginProblem;
    rb= rn= initsquare;	     /* V2.60  NG */
    InputStack[0]= stdin;
    do {
	memset((char *) exist,0,sizeof(exist));		/* V3.43  NG */
	memset((char *) promonly,0,sizeof(promonly));	/* V3.43  NG */
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
** no other character allowed in the input. This holds for <FieldName>
** and <FieldList>. Other terminals must be seperated by any non
** alpha or non numeric character.
**
** <PopeyeInput>  ::= <BeginSpec> <PySpecList> <EndSpec>
**
** <BeginSpec>	  ::= beginproblem
** <EndSpec>	  ::= endproblem | nextproblem <PySpecList> <EndSpec>
**
** <PySpecList>   ::= <PySpec> | <PySpec> <PySpecList>
** <PySpec>	  ::= <PieceSpec> | <StipSpec> | <CondSpec> | <AuthorSpec> |
**		      <OrigSpec> | <KamikazeSpec> | <ImitatorSpec> |
**		      <TitleSpec> | <RemarkSpec>
** <PieceSpec>	  ::= pieces <PieceList>
** <KamikazeSpec> ::= kamikaze <PieceList>
** <ImitatorSpec> ::= imitator <FieldList>
** <PieceList>	  ::= <MenSpec> <MenList> | <MenSpec> <MenList> <PieceList>
** <MenSpec>	  ::= <ColorSpec> <AddSpecList>
** <ColorSpec>	  ::= white | black | neutral
** <AddSpecList>  ::= <Empty> | <AddSpec>
** <AddSpec>	  ::= royal
** <Empty>	  ::=
**
** <MenList>	  ::= <ManName> <FieldList> <Seperator> |
**		      <ManName> <FieldList> <MenList>
** <ManName>	  ::= as actually defined in POPEYE.
** <FieldList>	  ::= <FieldName> | <FieldName><FieldList>
** <FieldName>	  ::= <ColName><RowName>
** <ColName>	  ::= a | b | c | d | e | f | g | h
** <RowName>	  ::= 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8
**
** <StipSpec>	  ::= stipulation <StipName> <Number>
** <StipName>	  ::= as actually defined in popeye
** <Number>	  ::= <Digit> | <Digit><Number>
** <Digit>	  ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
**
** <CondSpec>	  ::= condition <CondList>
** <CondList>	  ::= <CondName> | <CondName> <CondList>
** <Condname>	  ::= as actually defined in popeye
**
** <AuthorSpec>   ::= author <string> <eol>
** <OrigSpec>	  ::= origin <string> <eol>
** <TitleSpec>	  ::= title <string> <eol>
** <RemarkSpec>   ::= remark <string> <eol>
** <string>	  ::= all characters except <eol>
** <eol>	  ::= end of line
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
** as fieldlist. So you are encouraged to use at least four characters
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
