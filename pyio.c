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
 ** 2009/06/14 SE   New optiion: LastCapture
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
#include <limits.h>

#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyslice.h"
#include "pyquodli.h"
#include "pyrecipr.h"
#include "pynot.h"
#include "pymovein.h"
#include "pyproof.h"
#include "pymovenb.h"
#include "pydegent.h"
#include "pyflight.h"
#include "pynontrv.h"
#include "pythreat.h"
#include "pyreflxg.h"
#include "pydirctg.h"
#include "pyselfgd.h"
#include "pyselfcg.h"
#include "stipulation/goal_reached_tester.h"
#include "pypipe.h"
#include "pyint.h"
#include "pyoutput.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/leaf.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/continuation.h"
#include "stipulation/battle_play/try.h"
#include "stipulation/series_play/branch.h"
#include "stipulation/series_play/parry_fork.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/move.h"
#include "conditions/republican.h"
#include "optimisations/maxsolutions/maxsolutions.h"
#include "optimisations/stoponshortsolutions/stoponshortsolutions.h"
#include "platform/maxtime.h"
#include "platform/maxmem.h"
#include "trace.h"

/* This is pyio.c
** It comprises a new io-Module for popeye.
** The old io was awkward, nonsystematic and I didn't like it.
** Therefore here a new one.
** Despite the remarks in the README file, I give here three languages
** for the input: english, french, german. It can easily be extended
** if necessary.
*/

static char AlphaStip[40];

#define MAXNEST 10
#define UPCASE(c)   toupper(c)      /* (c+('A'-'a')) */
/* This is only correct, cause only lowercase letters are passed
   as arguments
*/

static char Sep[] = "\n";
/* All entries in this table have to be in lower case */
static char *TokenString[LanguageCount][TokenCount] = {
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

#define WCcentered    0
#define WCleft        1
#define WCLaTeX       2

static void WritePieces(piece *p, char* CondLine)
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

static char **CondTab;  /* set according to language */
static char    **ExtraCondTab;

static nocontactfunc_t *nocontactfunc;

char ChameleonSequence[256];

static  FILE    *LaTeXFile, *SolFile;


static void LaTeXStr(char *line)
{
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

static void AddSquare(char *List, square i)
{
  char    add[4];

  add[0]= ' ';
  add[1]= 'a' - nr_files_on_board + i%onerow;
  add[2]= '1' - nr_rows_on_board + i/onerow;
  add[3]= '\0';
  strcat(List, add);
}

static int  AntiCirType;

static void CenterLine(char *s)
{
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

static void WriteConditions(int alignment)
{
  Cond  cond;
  enum
  {
    CondLineLength = 256
  };
  char  CondLine[CondLineLength];
  boolean   CondPrinted= false;

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
    if (((cond == white_oscillatingKs) && OscillatingKingsTypeC[White]) ||
        ((cond == black_oscillatingKs) && OscillatingKingsTypeC[Black])) {
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

    if (cond == imitators)
    {
      unsigned int imi_idx;
      for (imi_idx = 0; imi_idx<inum[1]; imi_idx++)
        AddSquare(CondLine,isquare[imi_idx]);
    }

    if (cond == noiprom && !CondFlag[imitators])
      continue;

    if (cond == magicsquare) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], MagicSq)) {
          AddSquare(CondLine, i);
        }
      }
    }
    if (cond == whforsqu) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], WhForcedSq)) {
          AddSquare(CondLine, i);
        }
      }
    }
    if (cond == blforsqu) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], BlForcedSq)) {
          AddSquare(CondLine, i);
        }
      }
    }

    if (cond == whconforsqu) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], WhConsForcedSq)) {
          AddSquare(CondLine, i);
        }
      }
    }

    if (cond == blconforsqu) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], BlConsForcedSq)) {
          AddSquare(CondLine, i);
        }
      }
    }

    if (cond == whprom_sq) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], WhPromSq)) {
          AddSquare(CondLine, i);
        }
      }
    }
    if (cond == blprom_sq) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
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
      strcat(CondLine, VariantTypeString[UserLanguage][TypeB]);
    }

    if (cond == annan) {
      strcat(CondLine, "    ");
      switch (annanvar)
      {
      case 1:
        strcat(CondLine, VariantTypeString[UserLanguage][TypeB]);
        break;
      case 2:
        strcat(CondLine, VariantTypeString[UserLanguage][TypeC]);
        break;
      case 3:
        strcat(CondLine, VariantTypeString[UserLanguage][TypeD]);
        break;
      }
    }

    if (cond == gridchess && OptFlag[suppressgrid]) {
      strcat(CondLine, "  ");
      switch (gridvar)
      {
        case grid_vertical_shift:
          strcat(CondLine, VariantTypeString[UserLanguage][ShiftRank]);
          break;
        case grid_horizontal_shift:
          strcat(CondLine, VariantTypeString[UserLanguage][ShiftFile]);
          break;
        case grid_diagonal_shift:
          strcat(CondLine, VariantTypeString[UserLanguage][ShiftRankFile]);
          break;
        case grid_orthogonal_lines:
          strcat(CondLine, VariantTypeString[UserLanguage][Orthogonal]);
          /* to do - write lines */
          break;
        case grid_irregular:
          strcat(CondLine, VariantTypeString[UserLanguage][Irregular]);
          /* to do - write squares */
          break;
      }
    }

    if ((cond == white_oscillatingKs) && OscillatingKingsTypeB[White]) {
      strcat(CondLine, "    ");
      strcat(CondLine, VariantTypeString[UserLanguage][TypeB]);
    }

    if ((cond == black_oscillatingKs) && OscillatingKingsTypeB[Black]) {
      strcat(CondLine, "    ");
      strcat(CondLine, VariantTypeString[UserLanguage][TypeB]);
    }

    if ((cond == white_oscillatingKs) && OscillatingKingsTypeC[White]) {
      if (! CondFlag[swappingkings]) {
        strcat(CondLine, "  ");
        strcat(CondLine, VariantTypeString[UserLanguage][TypeC]);
      }
    }

    if ((cond == black_oscillatingKs) && OscillatingKingsTypeC[Black]) {
      if (! CondFlag[swappingkings]) {
        strcat(CondLine, "  ");
        strcat(CondLine, VariantTypeString[UserLanguage][TypeC]);
      }
    }

    if ((cond == patience) && PatienceB) {
      strcat(CondLine, "    ");
      strcat(CondLine, VariantTypeString[UserLanguage][TypeB]);
    }

    if (CondFlag[singlebox])    {
      strcat(CondLine, "    ");
      if (SingleBoxType==singlebox_type1)
        strcat(CondLine, VariantTypeString[UserLanguage][Type1]);
      if (SingleBoxType==singlebox_type2)
        strcat(CondLine, VariantTypeString[UserLanguage][Type2]);
      if (SingleBoxType==singlebox_type3)
        strcat(CondLine, VariantTypeString[UserLanguage][Type3]);
    }

    if (CondFlag[republican])
      republican_write_diagram_caption(CondLine,CondLineLength);

    if (cond == sentinelles) {
      char pawns[7];
      if (sentinelb == pbb)
        strcat(CondLine, " Berolina");
      if (SentPionAdverse) {
        strcat(CondLine, "  ");
        strcat(CondLine,
               VariantTypeString[UserLanguage][PionAdverse]);
      }
      if (SentPionNeutral) {
        strcat(CondLine, "  ");
        strcat(CondLine,
               VariantTypeString[UserLanguage][PionNeutral]);
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
    case antisuper:
      /* AntiCirTypeCalvet is default in AntiCirce */
      if (AntiCirType != AntiCirTypeCalvet) {
        strcat(CondLine, "  ");
        strcat(CondLine, VariantTypeString[UserLanguage][AntiCirType]);
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
          strcat(ActTwinning, ", ");
        }
        StdString("\n   ");
      }
      StdString(CondLine);
      if (LaTeXout) {
        strcat(ActTwinning, CondLine);
      }
      break;
    }
    CondPrinted= true;
  }

  if (alignment == WCLaTeX && CondPrinted)
    fprintf(LaTeXFile, "}%%\n");
} /* WriteConditions */

static void WriteCastlingMutuallyExclusive(void)
{
  /* no need to test in [Black] - information is redundant */
  if (castling_mutual_exclusive[White][queenside_castling-min_castling]!=0
      || castling_mutual_exclusive[White][kingside_castling-min_castling]!=0)
  {
    StdString(OptString[UserLanguage][mutuallyexclusivecastling]);

    if ((castling_mutual_exclusive[White][queenside_castling-min_castling]
         &ra8_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_a1);
      WriteSquare(square_a8);
    }

    if ((castling_mutual_exclusive[White][queenside_castling-min_castling]
         &rh8_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_a1);
      WriteSquare(square_h8);
    }

    if ((castling_mutual_exclusive[White][kingside_castling-min_castling]
         &ra8_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_h1);
      WriteSquare(square_a8);
    }

    if ((castling_mutual_exclusive[White][kingside_castling-min_castling]
         &rh8_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_h1);
      WriteSquare(square_h8);
    }

    StdChar('\n');
  }
}

/***** twinning ***** */

typedef enum
{
  TwinningMove,         /* 0 */
  TwinningExchange,     /* 1 */
  TwinningStip,         /* 2 */
  TwinningStructStip,   /* 3 */
  TwinningAdd,          /* 4 */
  TwinningRemove,       /* 5 */
  TwinningContinued,    /* 6 */
  TwinningRotate,       /* 7 */
  TwinningCond,         /* 8 */
  TwinningPolish,       /* 9 */
  TwinningMirror,      /* 10 */
  TwinningMirra1h1,    /* 11 */
  TwinningMirra1a8,    /* 12 */
  TwinningMirra1h8,    /* 13 */
  TwinningMirra8h1,    /* 14 */
  TwinningShift,       /* 15 */
  TwinningSubstitute,  /* 16 */
  TwinningForsyth,     /* 17 */

  TwinningCount   /* 17 */
} TwinningType;

char    *TwinningString[LanguageCount][TwinningCount] = {
  { /* francais */
    /* 0*/  "deplacement",
    /* 1*/  "echange",
    /* 2*/  "enonce",
    /* 3*/  "senonce",
    /* 4*/  "ajoute",
    /* 5*/  "ote",
    /* 6*/  "enplus",
    /* 7*/  "rotation",
    /* 8*/  "condition",
    /* 9*/  "CouleurEchange",
    /*10*/  "miroir",
    /*11*/  "a1<-->h1",
    /*12*/  "a1<-->a8",
    /*13*/  "a1<-->h8",
    /*14*/  "a8<-->h1",
    /*15*/  "translation",
    /*16*/  "remplace",
    /*17*/  "forsyth"
  },
  { /* German */
    /* 0*/  "versetze",
    /* 1*/  "tausche",
    /* 2*/  "Forderung",
    /* 3*/  "SForderung",
    /* 4*/  "hinzufuegen",
    /* 5*/  "entferne",
    /* 6*/  "ferner",
    /* 7*/  "Drehung",
    /* 8*/  "Bedingung",
    /* 9*/  "Farbwechsel",
    /*10*/  "Spiegelung",
    /*11*/  "a1<-->h1",
    /*12*/  "a1<-->a8",
    /*13*/  "a1<-->h8",
    /*14*/  "a8<-->h1",
    /*15*/  "Verschiebung",
    /*16*/  "ersetze",
    /*17*/  "forsyth"
  },
  { /* English */
    /* 0*/  "move",
    /* 1*/  "exchange",
    /* 2*/  "stipulation",
    /* 3*/  "sstipulation",
    /* 4*/  "add",
    /* 5*/  "remove",
    /* 6*/  "continued",
    /* 7*/  "rotate",
    /* 8*/  "condition",
    /* 9*/  "PolishType",
    /*10*/  "mirror",
    /*11*/  "a1<-->h1",
    /*12*/  "a1<-->a8",
    /*13*/  "a1<-->h8",
    /*14*/  "a8<-->h1",
    /*15*/  "shift",
    /*16*/  "substitute",
    /*17*/  "forsyth"
  }
};

/***** twinning ***** end */

static char **TokenTab; /* set according to language */
static char **OptTab;   /* set according to language */

static char  **VariantTypeTab;
static char **TwinningTab;
static char LastChar;

static  FILE    *TraceFile;
static  FILE    *InputStack[MAXNEST];

static    char *LaTeXPiecesAbbr[PieceCount];
static    char *LaTeXPiecesFull[PieceCount];
char *LaTeXStdPie[8] = { NULL, "C", "K", "B", "D", "S", "T", "L"};

static  int NestLevel=0;

Side OscillatingKingsSide;  /* this is all a hack */

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
static char TokenChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ#=+-%>!.<()~/&|,";
/* Steingewinn ! */
/* introductory move */
/* h##! */
/* dia3.5 */
/* a1<-->h1  */
/* reci-h(=)#n */
/* h~2  do ANY helpmove */

static char CharChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char SepraChar[] = "\n\r;,.:";

static void pyfputc(char c, FILE *f)
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

static void ErrChar(char c)
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

static char NextChar(void)
{
  int ch;
  static boolean eof= false;

  if ((ch= getc(InputStack[NestLevel])) != -1)
    return LastChar=ch;
  else
  {
    if (eof)
    {
      if (PopInput() < 0)
        FtlMsg(EoFbeforeEoP);
      return NextChar();
    }
    eof= true;
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

static boolean sncmp(char *a, char *b)
{
  while (*b)
  {
    if ((isupper((int)*a) ? tolower((int)*a) : *a) == *b++)
      /* EBCDIC support ! HD */
      ++a;
    else
      return false;
  }

  return true;
}

enum
{
  unrecognised_delta = 0,
  ambiguous_delta = 1
};

static unsigned int GetIndex(unsigned int index, unsigned int limit,
                             char **list, char *tok)
{
  while (index<limit)
    if (sncmp(list[index],tok))
      return index;
    else
      ++index;

  return limit+unrecognised_delta;
}

static unsigned int GetUniqIndex(unsigned int limit, char **list, char *tok)
{
  unsigned int const index = GetIndex(0,limit,list,tok);
  if (index==limit+unrecognised_delta)
    return index;
  else
  {
    if (strlen(tok)==strlen(list[index]))
      return index;
    else if (GetIndex(index+1,limit,list,tok)==limit+unrecognised_delta)
      return index;
    else
      return limit+ambiguous_delta;
  }
}

static Token StringToToken(char *tok)
{
  return GetUniqIndex(TokenCount,TokenTab,tok);
}

static long int ReadBGLNumber(char* inptr, char** endptr)
{
  /* input must be of form - | {d}d(.|,(d(d))) where d=digit ()=0 or 1 {}=0 or more 
     in - and all other cases return infinity (no limit) */
  char buf[12];
  int res= BGL_infinity;
  size_t len, dp;
  char* dpp;
  *endptr= inptr;
  while (**endptr && strchr("0123456789.,-", **endptr))
    /* isdigit(**endptr) || **endptr == '.' || **endptr == ',' || **endptr == '-')) */
    (*endptr)++;
  len= (*endptr-inptr);
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
  dp= len-(dpp-buf);
  if (!dp)
    return 100*(long int)atoi(buf);
  while ((size_t)(dpp-buf) < len) {
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

static void ReadBeginSpec(void)
{
  while (true)
  {
    char *tok = ReadNextTokStr();
    TokenTab = TokenString[0];
    for (UserLanguage= 0; UserLanguage<LanguageCount; UserLanguage++)
    {
      TokenTab= &(TokenString[UserLanguage][0]);
      if (GetUniqIndex(TokenCount,TokenTab,tok)==BeginProblem)
      {
        OptTab= &(OptString[UserLanguage][0]);
        CondTab= &(CondString[UserLanguage][0]);
        TwinningTab= &(TwinningString[UserLanguage][0]);
        VariantTypeTab= &(VariantTypeString[UserLanguage][0]);
        ExtraCondTab= &(ExtraCondString[UserLanguage][0]);
        PieceTab= PieNamString[UserLanguage];
        PieSpTab= PieSpString[UserLanguage];
        InitMsgTab(UserLanguage);
        return;
      }
    }

    IoErrorMsg(NoBegOfProblem, 0);
  }
}

static int GetPieNamIndex(char a,char b)
{
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

static square SquareNum(char a,char b)
{
  if ('a'<=a && a<='h' && '1'<=b && b<='8')
    return square_a1 + (a-'a')*dir_right +(b-'1')*dir_up;
  else
    return initsquare;
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

      tok = ReadNextTokStr();
      LaTeXPiecesAbbr[Name]= (char *)malloc(sizeof(char)*(strlen(tok)+1));
      i= 0;
      while (tok[i]) {
        /* to avoid compiler warnings below made "better readable" */
        /*      LaTeXPiecesAbbr[Name][i]= tok[i++]+ 'A' - 'a';          */
        LaTeXPiecesAbbr[Name][i]= tok[i] + 'A' - 'a';
        i++;
      }
      LaTeXPiecesAbbr[Name][i]= tok[i];

      tok = ReadToEndOfLine();
      LaTeXPiecesFull[Name]= (char *)malloc(sizeof(char)*(strlen(tok)+1));
      strcpy(LaTeXPiecesFull[Name], tok);

      tok = ReadNextTokStr();
    }
  }

  return tok;
}

static char *LaTeXPiece(piece Name) {
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

static char *ParseSquareList(char *tok,
                             PieNam Name,
                             Flags Spec,
                             char echo)
{
  /* We interprete the tokenString as SquareList
     If we return always the next tokenstring
  */
  int SquareCnt= 0;

  while (true)
  {
    square const Square = SquareNum(*tok,tok[1]);
    if (tok[0]!=0 && tok[1]!=0 && Square!=initsquare)
    {
      if (e[Square] != vide)
      {
        if (!echo)
        {
          WriteSquare(Square);
          StdChar(' ');
          Message(OverwritePiece);
        }
        if (Square == rb)
          rb= initsquare;
        if (Square == rn)
          rn= initsquare;
      }
      /* echo the piece if desired -- twinning */
      if (echo == '+')
      {
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
          strcat(ActTwinning, GlobalStr);
        }
        if (e[Square] == vide)
          StdChar(echo);
        WriteSpec(Spec, Name!=vide);
        WritePiece(Name);
        WriteSquare(Square);
        StdChar(' ');
      }
      if (TSTFLAG(Spec, Neutral)) {
        Spec |= BIT(Black) + BIT(White);
      }
      if (echo != 1) {
        spec[Square] = Spec;
        e[Square] = TSTFLAG(Spec,White) ? Name : -Name;
      } 
      else {
        pprise[1] = TSTFLAG(Spec,White) ? Name : -Name;
        pprispec[1] = Spec;
        move_generation_stack[1].capture = Square;
      }
      tok+= 2;
      SquareCnt++;
      continue;
    }
    if (SquareCnt) {
      if (*tok || (echo == 1 && SquareCnt != 1)) {
        ErrorMsg(WrongSquareList);
      }
      return ReadNextTokStr();
    }
    ErrorMsg(MissngSquareList);
    tok = ReadNextTokStr();
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
  int     NameCnt= 0;
  char    *btok;
  PieNam  Name;
  size_t  l;

  while (true) {
    char const * const hunterseppos = strchr(tok,hunterseparator);
    btok = tok; /* Save it, if we want to return it */
    if (hunterseppos!=0 && hunterseppos-tok<=2) {
      PieNam away, home;
      tok = PrsPieShortcut((hunterseppos-tok)%2==1,tok,&away);
      ++tok; /* skip slash */
      l= strlen(tok);
      tok = PrsPieShortcut(l%2==1,tok,&home);
      Name = MakeHunterType(away,home);
    }
    else {
      l= strlen(tok);
      tok = PrsPieShortcut(l%2==1,tok,&Name);
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
        tok = ReadNextTokStr();
      tok = ParseSquareList(tok, Name, Spec, echo);
      CLRFLAG(Spec, Royal);
    }
    else if (hunterseppos!=0)
      tok = ReadNextTokStr();
    else
      if (NameCnt > 0)
        return btok;
      else {
        IoErrorMsg(WrongPieceName,0);
        tok = ReadNextTokStr();
      }
  }
}

static square NextSquare(square sq)
{
  if (sq%onerow<nr_of_slack_files_left_of_board+nr_files_on_board-1)
    return sq+1;
  else if (sq>=square_a2 && sq<=square_h8)
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

static char *ParseForsyth(boolean output)
{
  piece pc;
  int num;
  square sq = square_a8;
  square const *bnp;
  boolean NeutralFlag= false;
  char *tok = ReadNextCaseSensitiveTokStr();

  for (bnp= boardnum; *bnp; bnp++)
    e[*bnp]= vide;

  rb = initsquare;
  rn = initsquare;

  sprintf(GlobalStr, "  %s  \n", tok);
  if (output)
    StdString(tok);

  while (sq && *tok)
  {
    if (isdigit((int)*tok))
    {
      num= (*tok++) - '0';
      if (isdigit((int)*tok))
        num += num * 9 + (*tok++) - '0';
      for (;num && sq;num--)
      {
        e[sq]= vide;
        spec[sq]= BorderSpec;
        sq= NextSquare(sq);
      }
      NeutralFlag= false;
    }
    else if (isalpha((int)*tok))
    {
      pc= GetPieNamIndex(tolower(*tok),' ');
      if (pc>=King) {
        sq= SetSquare(sq,
                      pc,
                      islower((int)InputLine[(tok++)-TokenLine]),
                      &NeutralFlag);
        if (NeutralFlag) 
          SETFLAG(PieSpExFlags,Neutral);
      }
      else
        tok++;           /* error */
    }
    else if (*tok == '.')
    {
      if (*(tok+1) == '=')
      {
        NeutralFlag= true;
        tok++;
      }
      pc= GetPieNamIndex(tolower(*(tok+1)), tolower(*(tok+2)));
      if (pc>=King)
      {
        sq= SetSquare(sq,
                      pc,
                      islower((int)InputLine[(tok+1-TokenLine)]),
                      &NeutralFlag);
        if (NeutralFlag) 
          SETFLAG(PieSpExFlags,Neutral);
      }
      tok += 3;
    }
    else if (*tok == '=')
    {
      NeutralFlag= true;
      tok++;
    }
    else
      tok++;
  }

  return ReadNextTokStr();
}

static char *ParsePieSpec(char echo)
{
  /* We read the PieceSpecifikation.
  ** The first token we cannot interprete as PieceSpec
  ** When a color specification was suplied we try to
  ** parse the following MenList.
  ** If no ColorSpec is given, and we had already one, we return
  ** the token, to the caller.
  */
  Flags   PieSpFlags;
  int     SpecCnt= 0;
  char    *tok;
  Flags   ColorFlag= (BIT(White) | BIT(Black) | BIT(Neutral));

  tok = ReadNextTokStr();
  while (true)
  {
    CLEARFL(PieSpFlags);
    while (true)
    {
      PieSpec const ps = GetUniqIndex(PieSpCount,PieSpTab,tok);
      if (ps==PieSpCount+unrecognised_delta)
        break;
      else if (ps==PieSpCount+ambiguous_delta)
        IoErrorMsg(PieSpecNotUniq,0);
      else
      {
        Flags TmpFlg= PieSpFlags&ColorFlag;
        if (TmpFlg
            && (TmpFlg & BIT(ps))
            && (TmpFlg != (Flags)BIT(ps)))
          IoErrorMsg(WBNAllowed,0);
        else
        {
          SETFLAG(PieSpFlags,ps);
          SETFLAG(PieSpExFlags,ps);
        }
      }

      tok = ReadNextTokStr();
    }

    if (PieSpFlags & ColorFlag)
    {
      tok = PrsPieNam(tok, PieSpFlags, echo);
      SpecCnt++;
    }
    else
    {
      if (SpecCnt)
        return tok;
      IoErrorMsg(NoColorSpec,0);
      tok = ReadNextTokStr();
    }
  }
}
/* map input strings to goals */
typedef struct
{
    char const *inputText;
    goal_type goal;
 } goalInputConfig_t;

/* make sure that input strings that are substrings of other strings
 * appear *after* them! */
static goalInputConfig_t const goalInputConfig[nr_goals] =
{
  {   "##!",  goal_countermate   }
  , { "##",   goal_doublemate    }
  , { "#=",   goal_mate_or_stale }
  , { "#",    goal_mate          }
  , { "==",   goal_dblstale      }
  , { "!=",   goal_autostale     }
  , { "=",    goal_stale         }
  , { "z",    goal_target        }
  , { "+",    goal_check         }
  , { "x",    goal_capture       }
  , { "%",    goal_steingewinn   }
  , { "ep",   goal_ep            }
  , { "ctr",  goal_circuitB      }
  , { "ct",   goal_circuit       }
  , { "<>r",  goal_exchangeB     }
  , { "<>",   goal_exchange      }
  , { "00",   goal_castling      }
  , { "~",    goal_any           }
  , { "dia",  goal_proof         }
  , { "a=>b", goal_atob          }
};

static char *ParseLength(char *tok,
                         SliceType type,
                         stip_length_type *length,
                         stip_length_type *min_length)
{
  char *end;
  unsigned long tmp_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  if (tok!=0 && *tok==0)
    /* allow white space before length, e.g. "dia 4" */
  {
    tok = ReadNextTokStr();
    if (tok!=0)
      strcat(AlphaStip,tok); /* append to printed stipulation */
  }

  tmp_length = strtoul(tok,&end,10);
  TraceValue("%ld\n",tmp_length);

  if (tok==end || tmp_length>UINT_MAX)
  {
    IoErrorMsg(WrongInt,0);
    tok = 0;
  }
  else
  {
    *length = tmp_length;

    tok = end;

    switch (type)
    {
      case STHelpMove:
        /* we count half moves in help play */
        *length *= 2;
        *length += slack_length_help-1;

        if (strncmp(tok,".5",2)==0)
        {
          ++*length;
          tok += 2;
          *min_length = slack_length_help;
        }
        else
          *min_length = slack_length_help+1;

        if (*length==slack_length_help-1)
        {
          IoErrorMsg(WrongInt,0);
          tok = 0;
        }
        break;

      case STAttackMove:
        if (*length==0)
        {
          IoErrorMsg(WrongInt,0);
          tok = 0;
        }
        else
        {
          *length *= 2;
          *length += slack_length_battle-1;
          *min_length = slack_length_battle+1;
        }
        break;

      case STSeriesMove:
        if (*length==0)
        {
          IoErrorMsg(WrongInt,0);
          tok = 0;
        }
        else
        {
          *length += slack_length_series-1;
          *min_length = slack_length_series+1;
        }
        break;

      default:
        assert(0);
        break;
    }
  }
 
  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseGoal(char *tok, slice_index proxy)
{
  goalInputConfig_t const *gic;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  for (gic = goalInputConfig; gic!=goalInputConfig+nr_goals; ++gic)
    if (strstr(tok,gic->inputText)==tok)
    {
      if (gic->goal==goal_target)
      {
        slice_index const leaf = alloc_leaf_slice();
        Goal const goal = { goal_target, SquareNum(tok[1],tok[2]) };
        slice_index const tester = alloc_goal_reached_tester_slice(goal);
        pipe_link(tester,leaf);
        pipe_link(proxy,tester);

        if (goal.target==initsquare)
        {
          IoErrorMsg(MissngSquareList, 0);
          tok = 0;
        }
        else
          tok += 3;
        break;
      }
      else if (gic->goal==goal_mate_or_stale)
      {
        slice_index const leaf_mate = alloc_leaf_slice();
        Goal const mate_goal = { goal_mate, initsquare };
        slice_index const tester_mate = alloc_goal_reached_tester_slice(mate_goal);
        slice_index const proxy_mate = alloc_proxy_slice();

        slice_index const leaf_stale = alloc_leaf_slice();
        Goal const stale_goal = { goal_stale, initsquare };
        slice_index const tester_stale = alloc_goal_reached_tester_slice(stale_goal);
        slice_index const proxy_stale = alloc_proxy_slice();
        slice_index const quod = alloc_quodlibet_slice(proxy_mate,proxy_stale);

        pipe_link(proxy_mate,tester_mate);
        pipe_link(tester_mate,leaf_mate);

        pipe_link(proxy_stale,tester_stale);
        pipe_link(tester_stale,leaf_stale);

        pipe_link(proxy,quod);

        tok += 2;
        break;
      }
      else
      {
        slice_index const leaf = alloc_leaf_slice();
        Goal const goal = { gic->goal, initsquare };
        slice_index const tester = alloc_goal_reached_tester_slice(goal);

        pipe_link(tester,leaf);
        pipe_link(proxy,tester);

        if (gic->goal==goal_atob)
        {
          int i;
          
          ProofSaveStartPosition();

          /* InitBoard() does much more than the following: */
          for (i = 0; i<nr_squares_on_board; i++)
          {
            spec[i] = EmptySpec;
            e[boardnum[i]] = vide;
          }
          for (i = 0; i<maxinum; i++)
            isquare[i] = initsquare;

          tok += 4;
        }
        else if (gic->goal==goal_proof)
          tok += 3;
        else
          tok += strlen(gic->inputText);
        break;
      }
    }

  if (gic==goalInputConfig+nr_goals)
  {
    IoErrorMsg(UnrecStip, 0);
    tok = 0;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static void alloc_reci_end(slice_index *proxy_nonreci,
                           slice_index *proxy_reci,
                           slice_index proxy_to_nonreci,
                           slice_index proxy_to_reci)
{
  *proxy_nonreci = alloc_help_branch(slack_length_help+2,slack_length_help+2,
                                     proxy_to_nonreci);
  *proxy_reci = alloc_help_branch(slack_length_help+1,slack_length_help+1,
                                  proxy_to_reci);
}

static char *ParseRecigoal_type(char *tok,
                           slice_index *proxy_nonreci,
                           slice_index *proxy_reci)
{
  char *result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  if (*tok=='(')
  {
    char const *closingParenPos = strchr(tok,')');
    if (closingParenPos!=0)
    {
      slice_index const proxy_to_reci = alloc_proxy_slice();
      tok = ParseGoal(tok+1,proxy_to_reci);
      if (tok!=0)
      {
        if (tok==closingParenPos)
        {
          slice_index const proxy_to_nonreci = alloc_proxy_slice();
          result = ParseGoal(tok+1,proxy_to_nonreci);
          if (result!=NULL)
          {
            slice_index const nonreci = slices[proxy_to_nonreci].u.pipe.next;
            slices[nonreci].starter = Black;
            alloc_reci_end(proxy_nonreci,proxy_reci,
                           proxy_to_nonreci,proxy_to_reci);
          }
        }
        else
          IoErrorMsg(UnrecStip, 0);
      }
    }
    else
      IoErrorMsg(UnrecStip, 0);
  }
  else
  {
    slice_index const proxy_to_nonreci = alloc_proxy_slice();
    result = ParseGoal(tok,proxy_to_nonreci);
    if (result!=NULL)
    {
      slice_index const leaf = alloc_leaf_slice();
      slice_index const nonreci = slices[proxy_to_nonreci].u.pipe.next;
      Goal const goal = slices[nonreci].u.goal_reached_tester.goal;
      slice_index const reci_tester = alloc_goal_reached_tester_slice(goal);
      slice_index const proxy_to_reci = alloc_proxy_slice();
      pipe_link(reci_tester,leaf);
      pipe_link(proxy_to_reci,reci_tester);
      alloc_reci_end(proxy_nonreci,proxy_reci,
                     proxy_to_nonreci,proxy_to_reci);
      slices[nonreci].starter = Black;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseReciEnd(char *tok, slice_index proxy)
{
  slice_index op1 = no_slice;
  slice_index op2 = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  tok = ParseRecigoal_type(tok,&op1,&op2);
  if (op1!=no_slice && op2!=no_slice)
  {
    slice_index const reci = alloc_reciprocal_slice(op1,op2);
    pipe_link(proxy,reci);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a h stipulation
 * @param tok stipulation after "h"
 * @param proxy identifies proxy slice where to append branch
 * @param proxy_next identifes proxy slice to append to branch
 * @return input after stipulation if successful, 0 otherwise
 */
static char *ParseH(char *tok, slice_index proxy, slice_index proxy_next)
{
  stip_length_type length;
  stip_length_type min_length;
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_next);
  TraceFunctionParamListEnd();

  result = ParseLength(tok,STHelpMove,&length,&min_length);
  if (result!=0)
  {
    slice_index const branch = alloc_help_branch(length+1,min_length+1,
                                                 proxy_next);
    if ((length-slack_length_help)%2==0)
    {
      slice_index const inverter = alloc_move_inverter_solvable_filter();
      pipe_link(proxy,inverter);
      pipe_link(inverter,branch);
    }
    else
      pipe_link(proxy,branch);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse a ser-h stipulation
 * @param tok stipulation after "ser-h"
 * @param proxy identifies proxy slice where to append series
 * @param proxy_next identifes proxy slice to append to series
 * @return input after stipulation if successful, 0 otherwise
 */
static char *ParseSerH(char *tok, slice_index proxy, slice_index proxy_next)
{
  stip_length_type length;
  stip_length_type min_length;
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_next);
  TraceFunctionParamListEnd();

  result = ParseLength(tok,STSeriesMove,&length,&min_length);
  if (result!=0)
  {
    slice_index const proxy_help = alloc_proxy_slice();
    slice_index const series = alloc_series_branch(length+1,min_length,
                                                   proxy_help);
    pipe_link(proxy_help,
              alloc_help_branch(slack_length_help+1,slack_length_help+1,
                                proxy_next));
    pipe_link(proxy,series);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse a ser-h stipulation
 * @param tok stipulation after "ser-h"
 * @param proxy identifies proxy slice where to append series
 * @param proxy_next identifes proxy slice to append to series
 * @return input after stipulation if successful, 0 otherwise
 */
static char *ParseSerS(char *tok, slice_index proxy, slice_index proxy_next)
{
  stip_length_type length;
  stip_length_type min_length;
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_next);
  TraceFunctionParamListEnd();

  result = ParseLength(tok,STSeriesMove,&length,&min_length);
  if (result!=0)
  {
    slice_index const
        defense_branch = alloc_defense_branch(slack_length_battle+1,
                                              slack_length_battle+1);
    slice_index const series = alloc_series_branch(length+1,min_length,
                                                   defense_branch);
    slice_insert_self_guards(defense_branch,proxy_next);
    pipe_link(proxy,series);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParsePlay(char *tok, slice_index proxy)
{
  /* seriesmovers with introductory moves */
  char *arrowpos = strstr(tok,"->");
  char *result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  if (arrowpos!=0)
  {
    char *end;
    unsigned long const intro_len= strtoul(tok,&end,10);
    if (intro_len<1 || tok==end || end!=arrowpos)
      IoErrorMsg(WrongInt, 0);
    else
    {
      slice_index const proxy_next = alloc_proxy_slice();
      result = ParsePlay(arrowpos+2,proxy_next);
      if (result!=0 && slices[proxy_next].u.pipe.next!=no_slice)
      {
        /* >=1 move of starting side required */
        stip_length_type const min_length = 1+slack_length_series;
        slice_index const branch = alloc_series_branch(intro_len
                                                       +slack_length_series,
                                                       min_length,
                                                       proxy_next);
        pipe_link(proxy,branch);
      }
    }
  }

  else if (strncmp("exact-", tok, 6) == 0)
  {
    result = ParsePlay(tok+6,proxy);
    if (result!=0)
    {
      OptFlag[nothreat] = true;
      stip_make_exact();
    }
  }

  else if (strncmp("ser-reci-h",tok,10) == 0)
  {
    slice_index const proxy_next = alloc_proxy_slice();

    /* skip over "ser-reci-h" */
    tok = ParseReciEnd(tok+10,proxy_next);
    if (tok!=0 && slices[proxy_next].u.pipe.next!=no_slice)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseLength(tok,STSeriesMove,&length,&min_length);
      if (result!=0)
      {
        slice_index const mi = alloc_move_inverter_solvable_filter();
        slice_index const guard = alloc_selfcheck_guard_solvable_filter();
        slice_index const branch = alloc_series_branch(length,min_length,
                                                       proxy_next);
        pipe_link(mi,guard);
        pipe_link(guard,slices[proxy_next].u.pipe.next);
        pipe_link(proxy_next,mi);
        pipe_link(proxy,branch);
        slices[slices[proxy_next].u.pipe.next].starter = White;

        set_output_mode(output_mode_line);
      }
    }
  }

  else if (strncmp("ser-hs",tok,6)==0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok+6,proxy_next); /* skip over "ser-hs" */
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        stip_length_type length;
        stip_length_type min_length;
        result = ParseLength(tok,STSeriesMove,&length,&min_length);
        if (result!=0)
        {
          slice_index const
              defense_branch = alloc_defense_branch(slack_length_battle+1,
                                                    slack_length_battle+1);
          slice_insert_self_guards(defense_branch,proxy_next);
          /* in ser-hs, the series is 1 half-move longer than in usual
           * series play! */
          ++length;
          if (length==slack_length_series)
            pipe_link(proxy,defense_branch);
          else
          {
            stip_length_type const help_length = slack_length_help+1;
            slice_index const help = alloc_help_branch(help_length,help_length,
                                                       defense_branch);
            slice_index const branch = alloc_series_branch(length,min_length,
                                                           help);
        
            pipe_link(proxy,branch);
          }

          slices[next].starter = White;
        }

        set_output_mode(output_mode_line);
      }
    }
  }

  else if (strncmp("ser-h",tok,5) == 0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok+5,proxy_next); /* skip over "ser-h" */
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        result = ParseSerH(tok,proxy,proxy_next);
        if (result!=0)
        {
          switch (slices[next].u.goal_reached_tester.goal.type)
          {
            case goal_proof:
              slices[proxy].starter = White;
              break;

            default:
              slices[next].starter = Black;
              break;
          }

          set_output_mode(output_mode_line);
        }
      }
    }
  }

  else if (strncmp("ser-s",tok,5) == 0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok+5,proxy_next); /* skip over "ser-s" */
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        result = ParseSerS(tok,proxy,proxy_next);
        if (result!=0)
          slices[next].starter = White;

        OptFlag[solvariantes] = true;

        set_output_mode(output_mode_line);
      }
    }
  }

  else if (strncmp("ser-r",tok,5) == 0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok+5,proxy_next); /* skip over "ser-r" */
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        slice_index const leaf = alloc_leaf_slice();
        Goal const goal = slices[next].u.goal_reached_tester.goal;
        slice_index const avoided_tester = alloc_goal_reached_tester_slice(goal);
        slice_index const
            avoided = alloc_attack_move_slice(slack_length_battle+1,
                                              slack_length_battle+1);
        slice_index const not = alloc_not_slice(avoided);
        slice_index const proxy_avoided = alloc_proxy_slice();

        pipe_link(avoided_tester,leaf);
        pipe_link(avoided,avoided_tester);
        pipe_link(proxy_avoided,not);

        result = ParseSerH(tok,proxy,proxy_next);
        slice_insert_reflex_filters_semi(proxy,proxy_avoided);
        slices[next].starter = White;

        set_output_mode(output_mode_line);
      }
    }
  }

  else if (strncmp("ser-",tok,4) == 0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok+4,proxy_next); /* skip over "ser-" */
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        stip_length_type length;
        stip_length_type min_length;
        result = ParseLength(tok,STSeriesMove,&length,&min_length);
        if (result!=0)
        {
          slice_index const branch = alloc_series_branch(length+1,min_length,
                                                         proxy_next);
          pipe_link(proxy,branch);
          slices[next].starter = Black;

          set_output_mode(output_mode_line);
        }
      }
    }
  }

  else if (strncmp("phser-",tok,6) == 0)
  {
    result = ParsePlay(tok+2,proxy); /* skip over ph */
    if (result!=0)
    {
      slice_index const next = slices[proxy].u.pipe.next;
      if (slice_has_structure(next,slice_structure_pipe))
      {
        slice_index const proxy = alloc_proxy_slice();
        slice_index const help = alloc_help_move_slice(slack_length_help+1,
                                                       slack_length_help+1);
        slice_index const guard = branch_find_slice(STSelfCheckGuardSeriesFilter,
                                                    next);
        convert_to_parry_series_branch(next,proxy);
        pipe_link(proxy,help);
        pipe_link(help,slices[guard].prev);

        set_output_mode(output_mode_line);
      }
    }
  }

  else if (strncmp("pser-h",tok,6) == 0)
  {
    result = ParsePlay(tok+1,proxy);
    if (result!=0)
    {
      slice_index const next = slices[proxy].u.pipe.next;
      assert(slice_has_structure(next,slice_structure_pipe));

      {
        slice_index const proxy = alloc_proxy_slice();
        slice_index const help = alloc_help_move_slice(slack_length_help+1,
                                                       slack_length_help+1);
        slice_index const guard = branch_find_slice(STSelfCheckGuardSeriesFilter,
                                                    next);
        convert_to_parry_series_branch(next,proxy);
        pipe_link(proxy,help);
        pipe_link(help,slices[guard].prev);

        set_output_mode(output_mode_line);
      }
    }
  }

  else if (strncmp("pser-",tok,5) == 0)
  {
    /* this deals with all kinds of non-help parry series */
    result = ParsePlay(tok+1,proxy);
    if (result!=0)
    {
      slice_index const next = slices[proxy].u.pipe.next;
      if (slice_has_structure(next,slice_structure_pipe))
      {
        slice_index const proxy = alloc_proxy_slice();
        slice_index const
            solver = alloc_continuation_solver_slice(slack_length_battle+2,
                                                     slack_length_battle+2);
        slice_index const
            writer = alloc_continuation_writer_slice(slack_length_battle+2,
                                                     slack_length_battle+2);
        slice_index const def = alloc_defense_move_slice(slack_length_battle+2,
                                                         slack_length_battle+2);
        slice_index const guard = branch_find_slice(STSelfCheckGuardSeriesFilter,
                                                    next);
        convert_to_parry_series_branch(next,proxy);
        pipe_link(proxy,solver);
        pipe_link(solver,writer);
        pipe_link(writer,def);
        pipe_link(def,slices[guard].prev);

        set_output_mode(output_mode_line);
      }
    }
  }

  else if (strncmp("reci-h",tok,6) == 0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    char * const tok2 = ParseReciEnd(tok+6, /* skip over "reci-h" */
                                     proxy_next);
    if (tok2!=0 && slices[proxy_next].u.pipe.next!=no_slice)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseLength(tok2,STHelpMove,&length,&min_length);
      --length;
      if ((length-slack_length_help)%2==0)
        --min_length;
      else
        ++min_length;
      if (result!=0)
      {
        if (length==slack_length_help && min_length==slack_length_help)
          pipe_link(proxy,slices[proxy_next].u.pipe.next);
        else
        {
          slice_index const help = alloc_help_branch(length,min_length,
                                                     proxy_next);
          if ((length-slack_length_help)%2==1)
          {
            slice_index const inverter = alloc_move_inverter_solvable_filter();
            slice_index const guard = alloc_selfcheck_guard_solvable_filter();
            pipe_set_successor(proxy,inverter);
            pipe_link(inverter,guard);
            pipe_link(guard,help);
          }
          else
            pipe_link(proxy,help);
        }

        slices[slices[proxy].u.pipe.next].starter = Black;

        set_output_mode(output_mode_line);
      }
    }
  }

  else if (strncmp("dia",tok,3)==0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok,proxy_next);
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        stip_length_type length;
        stip_length_type min_length;
        result = ParseLength(tok,STHelpMove,&length,&min_length);
        if (result!=0)
        {
          Side const next_starter = ((length-slack_length_help)%2==0
                                     ? Black
                                     : White);
          slice_index const branch = alloc_help_branch(length+1,min_length+1,
                                                       proxy_next);
          pipe_link(proxy,branch);
          slices[next].starter = next_starter;

          set_output_mode(output_mode_line);
        }
      }
    }
  }

  else if (strncmp("a=>b",tok,4)==0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok,proxy_next);
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        stip_length_type length;
        stip_length_type min_length;
        result = ParseLength(tok,STHelpMove,&length,&min_length);
        if (result!=0)
        {
          Side const next_starter = ((length-slack_length_help)%2==0
                                     ? White
                                     : Black);
          slice_index const branch = alloc_help_branch(length+1,min_length+1,
                                                       proxy_next);
          pipe_link(proxy,branch);
          slices[next].starter = next_starter;

          set_output_mode(output_mode_line);
        }
      }
    }
  }

  else if (strncmp("hs",tok,2)==0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok+2,proxy_next); /* skip over "hs" */
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        stip_length_type length;
        stip_length_type min_length;
        result = ParseLength(tok,STHelpMove,&length,&min_length);
        if (result!=0)
        {
          slice_index const
              defense_branch = alloc_defense_branch(slack_length_battle+1,
                                                    slack_length_battle+1);
          slice_index const branch = alloc_help_branch(length,min_length,
                                                       defense_branch);
          slice_insert_self_guards(defense_branch,proxy_next);
          if ((length-slack_length_help)%2==0)
          {
            slice_index const inverter = alloc_move_inverter_solvable_filter();
            slice_index const guard = alloc_selfcheck_guard_solvable_filter();
            pipe_link(proxy,inverter);
            pipe_link(inverter,guard);
            pipe_link(guard,branch);
          }
          else
            pipe_link(proxy,branch);

          slices[next].starter = White;

          set_output_mode(output_mode_line);
        }
      }
    }
  }

  else if (strncmp("hr",tok,2)==0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok+2,proxy_next); /* skip over "hr" */
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        stip_length_type length;
        stip_length_type min_length;
        result = ParseLength(tok,STHelpMove,&length,&min_length);
        if (result!=0)
        {
          slice_index const leaf = alloc_leaf_slice();
          Goal const goal = slices[next].u.goal_reached_tester.goal;
          slice_index const avoided_tester = alloc_goal_reached_tester_slice(goal);
          slice_index const
              avoided = alloc_attack_move_slice(slack_length_battle+1,
                                                slack_length_battle+1);
          slice_index const not = alloc_not_slice(avoided);
          slice_index const proxy_avoided = alloc_proxy_slice();

          slice_index const branch = alloc_help_branch(length+1,min_length+1,
                                                       proxy_next);

          pipe_link(avoided_tester,leaf);
          pipe_link(avoided,avoided_tester);
          pipe_link(proxy_avoided,not);

          slice_insert_reflex_filters_semi(branch,proxy_avoided);
          if ((length-slack_length_help)%2==0)
          {
            slice_index const inverter = alloc_move_inverter_solvable_filter();
            slice_index const guard = alloc_selfcheck_guard_solvable_filter();
            pipe_link(proxy,inverter);
            pipe_link(inverter,guard);
            pipe_link(guard,branch);
          }
          else
            pipe_link(proxy,branch);

          slices[next].starter = White;

          set_output_mode(output_mode_line);
        }
      }
    }
  }

  else if (*tok=='h')
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok+1,proxy_next); /* skip over "h" */
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        result = ParseH(tok,proxy,proxy_next);
        slices[next].starter = Black;

        set_output_mode(output_mode_line);
      }
    }
  }

  else if (strncmp("semi-r",tok,6)==0)
  {
    slice_index const proxy_avoided = alloc_proxy_slice();
    tok = ParseGoal(tok+6,proxy_avoided); /* skip over "semi-r" */
    if (tok!=0)
    {
      slice_index const avoided_next = slices[proxy_avoided].u.pipe.next;
      if (avoided_next!=no_slice)
      {
        stip_length_type length;
        stip_length_type min_length;
        result = ParseLength(tok,STAttackMove,&length,&min_length);
        if (result!=0)
        {
          slice_index const
              avoided_attack = alloc_attack_move_slice(slack_length_battle+1,
                                                       slack_length_battle+1);
          slice_index const not_attack = alloc_not_slice(avoided_attack);
          slice_index const branch = alloc_battle_branch(length+1,
                                                         min_length+1);
          pipe_link(avoided_attack,avoided_next);
          pipe_link(not_attack,avoided_attack);
          pipe_link(proxy_avoided,not_attack);

          slice_insert_reflex_filters_semi(branch,proxy_avoided);
          pipe_set_successor(proxy,branch);
          slices[avoided_next].starter = White;

          set_output_mode(output_mode_tree);
        }
      }
    }
  }

  else if (*tok=='s')
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok+1,proxy_next); /* skip over "s" */
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        stip_length_type length;
        stip_length_type min_length;
        result = ParseLength(tok,STAttackMove,&length,&min_length);
        if (result!=0)
        {
          slice_index const branch = alloc_battle_branch(length+1,
                                                         min_length+1);
          slice_insert_self_guards(branch,proxy_next);
          pipe_set_successor(proxy,branch);
          slices[next].starter = White;

          set_output_mode(output_mode_tree);
        }
      }
    }
  }

  else if (*tok=='r')
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok+1,proxy_next);/* skip over 'r' */
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        stip_length_type length;
        stip_length_type min_length;
        result = ParseLength(tok,STAttackMove,&length,&min_length);
        if (result!=0)
        {
          slice_index const leaf = alloc_leaf_slice();
          Goal const goal = slices[next].u.goal_reached_tester.goal;
          slice_index const avoided_tester = alloc_goal_reached_tester_slice(goal);
          slice_index const
              avoided_attack = alloc_attack_move_slice(slack_length_battle+1,
                                                       slack_length_battle+1);
          slice_index const not_attack = alloc_not_slice(avoided_attack);
          slice_index const proxy_avoided_attack = alloc_proxy_slice();

          slice_index const
              avoided_defense = alloc_attack_move_slice(slack_length_battle+1,
                                                        slack_length_battle+1);
          slice_index const not_defense = alloc_not_slice(avoided_defense);
          slice_index const proxy_avoided_defense = alloc_proxy_slice();

          slice_index const branch = alloc_battle_branch(length+1,
                                                         min_length+1);
          pipe_link(avoided_tester,leaf);
          pipe_link(avoided_attack,avoided_tester);
          pipe_link(not_attack,avoided_attack);
          pipe_link(proxy_avoided_attack,not_attack);

          pipe_link(avoided_defense,next);
          pipe_link(proxy_avoided_defense,not_defense);
          slice_insert_reflex_filters(branch,
                                      proxy_avoided_attack,
                                      proxy_avoided_defense);
          pipe_set_successor(proxy,branch);
          slices[next].starter = White;

          set_output_mode(output_mode_tree);
        }
      }
    }
  }

  else
  {
    slice_index const proxy_next = alloc_proxy_slice();
    tok = ParseGoal(tok,proxy_next);
    if (tok!=0)
    {
      slice_index const next = slices[proxy_next].u.pipe.next;
      if (next!=no_slice)
      {
        stip_length_type length;
        stip_length_type min_length;
        result = ParseLength(tok,STAttackMove,&length,&min_length);
        if (result!=0)
        {
          slice_index const branch = alloc_battle_branch(length,min_length);
          slice_insert_direct_guards(branch,proxy_next);
          pipe_set_successor(proxy,branch);
          slices[next].starter = Black;

          set_output_mode(output_mode_tree);
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseStip(void)
{
  char *tok = ReadNextTokStr();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  root_slice = alloc_proxy_slice();

  strcpy(AlphaStip,tok);
  if (ParsePlay(tok,root_slice)
      && slices[root_slice].u.pipe.next!=no_slice
      && ActStip[0]=='\0')
    strcpy(ActStip, AlphaStip);

  tok = ReadNextTokStr();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Advance to next token while parsing a structured stipulation
 * @param tok current position in current token
 * @return tok, if we are within the current token; next token otherwise
 */
static char *ParseStructuredStip_skip_whitespace(char *tok)
{
  if (tok[0]==0)
  {
    strcat(AlphaStip,TokenLine);
    strcat(AlphaStip," ");
    tok = ReadNextTokStr();
    TraceValue("%s\n",tok);
  }

  return tok;
}

/* Parse starter of stipulation
 * @param tok input token
 * @return starter; no_side if starter couldn't be parsed
 */
Side ParseStructuredStip_starter(char *tok)
{
  Side result = no_side;
  PieSpec ps;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  /* We don't make any unsafe assumptions here; PieSpec enumerators
   * are initialised in terms of nr_sides */
  ps = GetUniqIndex(nr_sides,PieSpTab,tok);
  if (ps==nr_sides+ambiguous_delta)
    IoErrorMsg(PieSpecNotUniq,0);
  else if (ps<nr_sides)
    result = (Side)ps;

  TraceFunctionExit(__func__);
  TraceEnumerator(Side,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Parse the type of a leaf
 * @param tok input token
 * @return type of leaf; no_slice_type if type couldn't be parsed
 */
static SliceType ParseStructuredStip_leaf_type(char type_char)
{
  SliceType result = no_slice_type;
    
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%c",type_char);
  TraceFunctionParamListEnd();

  switch (type_char)
  {
    case 'd':
      result = STGoalReachedTester;
      break;

    case 'h':
      result = STGoalReachedTester;
      break;

    case 's':
      result = STGoalReachedTester;
      break;

    default:
      result = no_slice_type;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Parse a leaf
 * @param tok input token
 * @param result index of leaf; no_slice if branch couldn't be parsed
 * @param startLikeBranch true iff the starter is the same piece as in
 *                        the closest branch
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_leaf(char *tok,
                                      slice_index proxy,
                                      boolean startLikeBranch)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",startLikeBranch);
  TraceFunctionParamListEnd();

  /* e.g. d= for a direct leaf with goal stalemate */
  switch (ParseStructuredStip_leaf_type(tok[0]))
  {
//       if (startLikeBranch)
//         tok = ParseGoal(tok+1,proxy);
//       else
//         tok = ParseGoal(tok+1,proxy);
//       break;

//     case STGoalReachedTester:
//       tok = ParseGoal(tok+1,proxy);
//       break;

    default:
      tok = 0;
      break;
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseStructuredStip_operand(char *tok,
                                         slice_index proxy,
                                         boolean startLikeBranch);

/* Parse a direct branch
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves 
 * @param result index of branch; no_slice if branch couldn't be
 *               parsed
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_d(char *tok,
                                          stip_length_type min_length,
                                          stip_length_type max_length,
                                          slice_index proxy)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  if (min_length==0 || min_length==max_length)
  {
    boolean const nextStartLikeBranch = max_length%2==0;
    slice_index const proxy_operand = alloc_proxy_slice();
    tok = ParseStructuredStip_operand(tok,proxy_operand,nextStartLikeBranch);
    if (tok!=0)
    {
      min_length += slack_length_battle+1+max_length%2;
      max_length += slack_length_battle+1;
      {
        slice_index branch = alloc_battle_branch(max_length,min_length);

        /* TODO get this right */
        if ((max_length-slack_length_battle-1)%2==0)
          slice_insert_direct_guards(branch,proxy_operand);
        else
        {
          if (slices[slices[proxy_operand].u.pipe.next].type==STGoalReachedTester)
            slice_insert_self_guards(branch,proxy_operand);
          else
            slice_insert_reflex_filters_semi(branch,proxy_operand);
        }

        pipe_set_successor(proxy,branch);
      }
    }
  }
  else
    tok = 0;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a help branch
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves 
 * @param result index of branch; no_slice if branch couldn't be
 *               parsed
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_h(char *tok,
                                          stip_length_type min_length,
                                          stip_length_type max_length,
                                          slice_index proxy)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  if (min_length==0 || min_length==max_length)
  {
    boolean const nextStartLikeBranch = max_length%2==0;
    slice_index const proxy_to_op = alloc_proxy_slice();
    tok = ParseStructuredStip_operand(tok,proxy_to_op,nextStartLikeBranch);
    if (tok!=0)
    {
      if (min_length==0)
        min_length = max_length%2==0 ? 0 : 1;

      min_length += slack_length_help;
      max_length += slack_length_help;

      pipe_link(proxy,alloc_help_branch(max_length,min_length,proxy_to_op));
    }
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a series branch
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves 
 * @param result index of branch; no_slice if branch couldn't be
 *               parsed
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_ser(char *tok,
                                            stip_length_type min_length,
                                            stip_length_type max_length,
                                            slice_index proxy)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  if (min_length==0 || min_length==max_length)
  {
    slice_index const proxy_to_operand = alloc_proxy_slice();
    boolean const nextStartLikeBranch = false;
    tok = ParseStructuredStip_operand(tok,proxy_to_operand,nextStartLikeBranch);
    if (tok!=0)
    {
      if (min_length==0)
        min_length = 1;

      min_length += slack_length_series;
      max_length += slack_length_series;

      pipe_set_successor(proxy,
                         alloc_series_branch(max_length,min_length,
                                             proxy_to_operand));
    }
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse the length indication of a branch
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves 
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_length(char *tok,
                                               stip_length_type *min_length,
                                               stip_length_type *max_length)
{
  char *end;
  unsigned long length_long = strtoul(tok,&end,10);
  
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();
  
  if (end==tok || length_long==0 || length_long>UINT_MAX)
    tok = 0;
  else
  {
    tok = ParseStructuredStip_skip_whitespace(end);
    if (tok[0]==',')
    {
      *min_length = (stip_length_type)length_long;
      tok = ParseStructuredStip_skip_whitespace(tok+1);
      length_long = strtoul(tok,&end,10);
      if (end==tok || length_long==0 || length_long>UINT_MAX)
        tok = 0;
      else
      {
        *max_length = (stip_length_type)length_long;
        tok = end;
      }
    }
    else
    {
      *min_length = 0;
      *max_length = (stip_length_type)length_long;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a stipulation branch
 * @param tok input token
 * @param result index of branch; no_slice if branch couldn't be
 *               parsed
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch(char *tok,
                                        slice_index proxy)
{
  stip_length_type min_length;
  stip_length_type max_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  tok = ParseStructuredStip_branch_length(tok,&min_length,&max_length);

  if (tok!=0)
  {
    if (strncmp(tok,"ser",3)==0)
      tok = ParseStructuredStip_branch_ser(tok+3,min_length,max_length,proxy);
    else if (tok[0]=='d')
      tok = ParseStructuredStip_branch_d(tok+1,min_length,max_length,proxy);
    else if (tok[0]=='h')
      tok = ParseStructuredStip_branch_h(tok+1,min_length,max_length,proxy);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a not operator
 * @param tok input token
 * @param result index of branch; no_slice if operator couldn't be
 *               parsed
 * @param startLikeBranch true iff the starter is the same piece as in
 *                        the closest branch
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_not(char *tok,
                                     slice_index proxy,
                                     boolean startLikeBranch)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",startLikeBranch);
  TraceFunctionParamListEnd();
  
  tok = ParseStructuredStip_operand(tok+1,proxy,startLikeBranch);

  {
    slice_index const operand = slices[proxy].u.pipe.next;
    if (tok!=0 && operand!=no_slice)
    {
      slice_index const not = alloc_not_slice(operand);
      pipe_set_successor(proxy,not);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a move inversion
 * @param tok input token
 * @param result index of branch; no_slice if operator couldn't be
 *               parsed
 * @param startLikeBranch true iff the starter is the same piece as in
 *                        the closest branch
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_move_inversion(char *tok,
                                                slice_index proxy,
                                                boolean startLikeBranch)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",startLikeBranch);
  TraceFunctionParamListEnd();
  
  tok = ParseStructuredStip_operand(tok+1,proxy,!startLikeBranch);

  {
    slice_index const operand = slices[proxy].u.pipe.next;
    if (tok!=0 && operand!=no_slice)
    {
      slice_index const inverter = alloc_move_inverter_solvable_filter();
      slice_index const guard = alloc_selfcheck_guard_solvable_filter();
      pipe_append(proxy,inverter);
      pipe_link(inverter,guard);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a stipulation operator
 * @param tok input token
 * @param result type of operator; no_slice_type if operand couldn't
 *               be parsed
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_operator(char *tok, SliceType *result)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  /* allow space between operands */
  tok = ParseStructuredStip_skip_whitespace(tok);

  if (tok[0]=='&')
  {
    ++tok;
    *result = STReciprocal;
  }
  else if (tok[0]=='|')
  {
    ++tok;
    *result = STQuodlibet;
  }
  else
    *result = no_slice_type;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a stipulation expression
 * @param tok input token
 * @param result index of expression slice; no_slice if expression
 *               can't be parsed
 * @param startLikeBranch true iff the starter is the same piece as in
 *                        the closest branch
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_expression(char *tok,
                                            slice_index proxy,
                                            boolean startLikeBranch)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",startLikeBranch);
  TraceFunctionParamListEnd();

  {
    slice_index const operand1 = alloc_proxy_slice();
    tok = ParseStructuredStip_operand(tok,operand1,startLikeBranch);
    if (tok!=0 && slices[operand1].u.pipe.next!=no_slice)
    {
      SliceType operator_type;
      tok = ParseStructuredStip_operator(tok,&operator_type);
      if (tok!=0 && operator_type!=no_slice_type)
      {
        slice_index const operand2 = alloc_proxy_slice();
        tok = ParseStructuredStip_expression(tok,operand2,startLikeBranch);
        if (tok!=0 && slices[operand2].u.pipe.next!=no_slice)
          switch (operator_type)
          {
            case STReciprocal:
            {
              slice_index const reci = alloc_reciprocal_slice(operand1,
                                                              operand2);
              pipe_link(proxy,reci);
              break;
            }

            case STQuodlibet:
            {
              slice_index const quod = alloc_quodlibet_slice(operand1,
                                                             operand2);
              pipe_link(proxy,quod);
              break;
            }

            default:
              assert(0);
              break;
          }
      }
      else
      {
        if (slices[slices[operand1].u.pipe.next].prev==operand1)
          pipe_link(proxy,slices[operand1].u.pipe.next);
        else
          pipe_set_successor(proxy,slices[operand1].u.pipe.next);
      }
    }
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a parenthesised stipulation expression
 * @param tok input token
 * @param result index of expression slice; no_slice if expression
 *               can't be parsed
 * @param startLikeBranch true iff the starter is the same piece as in
 *                        the closest branch
 * @return remainder of input token; 0 if parsing failed
 */
static char *
ParseStructuredStip_parenthesised_expression(char *tok,
                                             slice_index proxy,
                                             boolean startLikeBranch)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",startLikeBranch);
  TraceFunctionParamListEnd();

  tok = ParseStructuredStip_expression(tok+1,proxy,startLikeBranch);

  if (tok!=0)
  {
    /* allow space before closing parenthesis */
    tok = ParseStructuredStip_skip_whitespace(tok);

    if (tok[0]==')')
      ++tok;
    else
      pipe_set_successor(proxy,no_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse an stipulation operand
 * @param tok input token
 * @param result index of operand; no_slice if operand couldn't be
 *               parsed
 * @param startLikeBranch true iff the starter is the same piece as in
 *                        the closest branch
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_operand(char *tok,
                                         slice_index proxy,
                                         boolean startLikeBranch)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",startLikeBranch);
  TraceFunctionParamListEnd();

  /* allow space between operands */
  tok = ParseStructuredStip_skip_whitespace(tok);

  if (tok[0]=='(')
    tok = ParseStructuredStip_parenthesised_expression(tok,
                                                       proxy,
                                                       startLikeBranch);
  else if (tok[0]=='!')
    /* !d# - white at the move does *not* deliver mate */
    tok = ParseStructuredStip_not(tok,proxy,startLikeBranch);
  else if (tok[0]=='-')
    /* -3hh# - h#2 by the non-starter */
    tok = ParseStructuredStip_move_inversion(tok,proxy,startLikeBranch);
  else if (isdigit(tok[0]))
    /* e.g. 2dd# for a #2 */
    tok = ParseStructuredStip_branch(tok,proxy);
  else
    /* e.g. d= for a =1 */
    tok = ParseStructuredStip_leaf(tok,proxy,startLikeBranch);
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a structured stipulation (keyword sstipulation)
 * @return token following structured stipulation
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip(slice_index proxy)
{
  char *tok = 0;
  Side starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  AlphaStip[0] = 0;

  tok = ReadNextTokStr();
  starter = ParseStructuredStip_starter(tok);
  if (starter!=no_side)
  {
    strcat(AlphaStip,TokenLine);
    strcat(AlphaStip," ");
    tok = ReadNextTokStr();
    tok = ParseStructuredStip_expression(tok,proxy,true);
    if (tok==0)
      tok = ReadNextTokStr();
    else if (root_slice!=no_slice)
      stip_impose_starter(starter);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}


/* to make function ReadSquares in PYIO.C more convenient define
 * ReadImitators und ReadHoles and ReadEpSquares too. They can have
 * any positiv number, but must not coincide with
 * MagicSq...BlConsForcedSq.  TLi
 * Must also not coincide with  WhPromSq  and  BlPromSq.   NG
 */
typedef enum
{
  ReadImitators = nrSquareFlags,
  ReadHoles,             
  ReadEpSquares,         
  ReadFrischAuf,         
  ReadBlRoyalSq,         
  ReadWhRoyalSq,         
  ReadNoCastlingSquares, 
  ReadGrid
} SquareListContext;

static char *ReadSquares(SquareListContext context)
{
  char *tok = ReadNextTokStr();
  char *lastTok = tok;
  unsigned int nr_squares_read = 0;

  size_t const l = strlen(tok);
  if (l%2==1)
  {
    if (context!=ReadFrischAuf && context!=ReadGrid)
      IoErrorMsg(WrongSquareList, 0);
    currentgridnum = 0;
    return tok;
  }

  while (*tok)
  {
    square const sq = SquareNum(*tok,tok[1]);
    if (sq==initsquare)
    {
      if (context==ReadGrid || nr_squares_read!=0)
      {
        currentgridnum = 0;
        return lastTok;
      }
      if (context!=ReadFrischAuf || nr_squares_read!=0)
      {
        IoErrorMsg(WrongSquareList,0);
        return tok;
      }
    }
    else
    {
      switch (context)
      {
        case ReadFrischAuf:
          if (e[sq]==vide || e[sq]==obs || is_pawn(e[sq]))
            Message(NoFrischAufPromPiece);
          else
            SETFLAG(spec[sq], FrischAuf);
          break;

        case ReadImitators:
          isquare[nr_squares_read] = sq;
          break;

        case ReadHoles:
          e[sq]= obs;
          break;

        case ReadEpSquares:
          switch (nr_squares_read)
          {
            case 0:
              ep[1]= sq;
              break;
            case 1:
              ep2[1]= sq;
              break;
            default:
              Message(ToManyEpKeySquares);
          }
          break;

        case ReadBlRoyalSq:
          bl_royal_sq= sq;
          break;

        case ReadWhRoyalSq:
          wh_royal_sq= sq;
          break;

        case ReadNoCastlingSquares:
          switch (sq)
          {
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
          ClearGridNum(sq);
          sq_spec[sq] += currentgridnum << Grid;
          break;  

        default:
          SETFLAG(sq_spec[sq],context);
          break;
      }

      ++nr_squares_read;
      tok += 2;
    }
  }

  if (context==ReadImitators)
  {
    ply n;
    for (n = 1; n<=maxply; n++)
      inum[n] = nr_squares_read;
  }

  /* This is an ugly hack, but due to the new feature ReadFrischAuf,
     we need the returning of the token tok, and this leads to
     mistakes with other conditions that require reading a SquareList.
  */
  if (context==ReadFrischAuf)
    tok = ReadNextTokStr();

  return tok;
} /* ReadSquares */

static char *ParseRex(boolean *rex, Cond what)
{
  char *tok = ReadNextTokStr();
  *rex = what==GetUniqIndex(CondCount,CondTab,tok);
  if (*rex)
    tok = ReadNextTokStr();
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
  gpGrid,
  gpRepublican
} VariantGroup;

static char *ParseMaximumPawn(unsigned int *result,
                              unsigned int defaultVal,
                              unsigned int boundary)
{
  char *tok = ReadNextTokStr();

  char *end;
  unsigned long tmp = strtoul(tok,&end,10);
  if (tok==end || tmp>boundary)
    *result = defaultVal;
  else
    *result = tmp;

  return end;
}

static char *ParseVariant(boolean *is_variant_set, VariantGroup group) {
  char    *tok=ReadNextTokStr();

  if (is_variant_set!=NULL)
    *is_variant_set= false;

  do
  {
    VariantType type = GetUniqIndex(VariantTypeCount,VariantTypeTab,tok);

    if (type==VariantTypeCount+unrecognised_delta)
      break;

    if (type==VariantTypeCount+ambiguous_delta)
      IoErrorMsg(CondNotUniq,0);
    else if (type==TypeB && group==gpType)
      *is_variant_set= true;
    else if (type==TypeB && group==gpOsc)
      OscillatingKingsTypeB[OscillatingKingsSide]= true;
    else if (type==TypeC && group==gpOsc)
      OscillatingKingsTypeC[OscillatingKingsSide]= true;
    else if (type==TypeB && group==gpAnnan)
      annanvar= 1;
    else if (type==TypeC && group==gpAnnan)
      annanvar= 2;
    else if (type==TypeD && group==gpAnnan)
      annanvar= 3;
    else if (type==Type1 && group==gpType)
      SingleBoxType = singlebox_type1;
    else if (type==Type2 && group==gpType)
      SingleBoxType = singlebox_type2;
    else if (type==Type3 && group==gpType)
      SingleBoxType = singlebox_type3;
    else if (type==Type1 && group==gpRepublican)
      *is_variant_set = true;
    else if (type==Type2 && group==gpRepublican)
      *is_variant_set = false;
    else if (type==PionAdverse && group==gpSentinelles)
      *is_variant_set= true;
    else if (type==PionNeutral && group==gpSentinelles)
      SentPionNeutral= true;
    else if (type==PionNoirMaximum && group==gpSentinelles)
      tok = ParseMaximumPawn(&max_pn,8,64);
    else if (type==PionBlancMaximum && group==gpSentinelles)
      tok = ParseMaximumPawn(&max_pb,8,64);
    else if (type==PionTotalMaximum && group==gpSentinelles)
      tok = ParseMaximumPawn(&max_pt,16,64);
    else if (type==ParaSent && group==gpSentinelles)
      flagparasent= true;
    else if (type==SentBerolina && group==gpSentinelles)
    {
      sentinelb= pbb;
      sentineln= pbn;
    }
    else if (type==AntiCirTypeCheylan && group==gpAntiCirce)
      *is_variant_set= true;
    else if (type==AntiCirTypeCalvet && group==gpAntiCirce)
      *is_variant_set= false;
    else if (type==Neighbour && group==gpKoeko)
    {
      piece tmp_piece;
      tok = ReadNextTokStr();
      switch (strlen(tok))
      {
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

      switch (tmp_piece)
      {
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
    else if (type==ShiftRank && group==gpGrid)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++) 
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 8)/2)+4*((*bnp/24-7)/2)) << Grid;
      }
      gridvar = grid_vertical_shift;
    }
    else if (type==ShiftFile && group==gpGrid)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++) 
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 7)/2)+5*((*bnp/24-8)/2)) << Grid;
      }
      gridvar = grid_horizontal_shift;
    }
    else if (type==ShiftRankFile && group==gpGrid)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++) 
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 7)/2)+5*((*bnp/24-7)/2)) << Grid;
      }
      gridvar = grid_diagonal_shift;
    }
    else if (type==Orthogonal && group==gpGrid)
    {
      square const *bnp;
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
    else if (type==Irregular && group==gpGrid)
    {
      square const *bnp;
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
    else if (type==ExtraGridLines && group==gpGrid)
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
    tok = ReadNextTokStr();
  } while (group==gpSentinelles || group==gpGrid);

  return tok;
}

static char *ParseExact(boolean *ex_flag, boolean *ul_flag)
{
  char    *tok;

  tok = ReadNextTokStr();
  *ul_flag= ultra==GetUniqIndex(CondCount,CondTab,tok);
  if (*ul_flag)
  {
    *ex_flag= true;
    CondFlag[ultra]= true;
    tok = ReadNextTokStr();
  }
  else
  {
    *ex_flag= exact==GetUniqIndex(CondCount,CondTab,tok);
    if (*ex_flag)
    {
      CondFlag[exact]= true;
      tok = ReadNextTokStr();
    }
  }
  return tok;
} /* ParseExact */

static char *ParseVaultingPieces(Flags fl)
{
  piece p;
  char  *tok;
  int tp = 0;
  boolean gotpiece;

  while (true) {
    gotpiece = false;
    tok = ReadNextTokStr();
    switch (strlen(tok))
    {
    case 1:
      p= GetPieNamIndex(*tok,' ');
      gotpiece = true;
      break;

    case 2:
      p= GetPieNamIndex(*tok,tok[1]);
      gotpiece = true;
      break;

    default:
      switch (GetUniqIndex(VariantTypeCount,VariantTypeTab,tok))
      {
        case Transmuting:
          if (TSTFLAG(fl, White)) 
            calc_whtrans_king= true;
          if (TSTFLAG(fl, Black)) 
            calc_bltrans_king= true;
          break;

        default:
          return tok;
      }
    }
    if (gotpiece)
    {
      if (TSTFLAG(fl, White)) {
        whitetransmpieces[tp] = p;
      }
      if (TSTFLAG(fl, Black)) {
        blacktransmpieces[tp] = p;
      }
      tp++;
      if (TSTFLAG(fl, White)) {
        whitetransmpieces[tp] = vide;
      }
      if (TSTFLAG(fl, Black)) {
        blacktransmpieces[tp] = vide;
      }
    }
  }
  return tok;
}

static char *ReadChameleonCirceSequence(void)
{
  piece old_piece, new_piece;
  char  *tok, newpiece[3];

  old_piece= vide;

  for (new_piece= vide; new_piece < PieceCount; new_piece++) {
    NextChamCircePiece[new_piece]= new_piece;
  }
  ChameleonSequence[0]= '\0';

  while (true)
  {
    tok = ReadNextTokStr();
    switch (strlen(tok))
    {
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
  unsigned int CondCnt = 0;

  tok = ReadNextTokStr();
  while (true)
  {
    Cond const indexx = GetUniqIndex(CondCount,CondTab,tok);
    if (indexx==CondCount+unrecognised_delta)
    {
      ExtraCond extra = GetUniqIndex(ExtraCondCount,ExtraCondTab,tok);
      if (extra==ExtraCondCount+unrecognised_delta)
        break;
      else
      {
        switch (extra)
        {
          case maxi:
            flagmaxi= true;
            tok = ParseExact(&CondFlag[exact],&CondFlag[ultra]);
            CondCnt++;
            break;

          case ultraschachzwang:
            flagultraschachzwang= true;
            tok = ReadNextTokStr();
            CondCnt++;
            break;

          case ExtraCondCount+ambiguous_delta:
            IoErrorMsg(CondNotUniq,0);
            tok = ReadNextTokStr();
            break;

          default:
            IoErrorMsg(UnrecCondition,0);
            break;
        }
        continue;
      }
    }

    if (indexx==CondCount+ambiguous_delta)
    {
      IoErrorMsg(CondNotUniq,0);
      tok = ReadNextTokStr();
      continue;
    }

    CondFlag[indexx]= true;

    CondCnt++;

    switch (indexx)
    {
      case rexincl:
        if (CondFlag[exact])
          IoErrorMsg(NonSenseRexiExact, 0);
        break;
      case biheffalumps:
        CondFlag[heffalumps]= true;
        CondFlag[biwoozles]= true;
        CondFlag[woozles]= true;
        break;
      case heffalumps:
        CondFlag[woozles]= true;
        break;
      case biwoozles:
        CondFlag[woozles]= true;
        break;
      case hypervolage:
        CondFlag[volage]= true;
        break;
      case leofamily:
        CondFlag[chinoises]= true;
        break;
      case eiffel:
        flag_madrasi= true;
        break;
      case contactgrid:
        CondFlag[gridchess] = true;
        CondFlag[koeko] = true;
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
        CondFlag[tibet]= true;
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
        calc_bltrans_king= true;
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
      case circecage:
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
        SetDiaRen(PieSpExFlags, square_h8);
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
        SetDiaRen(PieSpExFlags, square_h8);
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
        SetDiaRen(PieSpExFlags, square_h8);
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
      case immuncage:
        immunrenai = rencage;
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

      default:
        break;
    }

    switch (indexx)
    {
      case frischauf:
        tok = ReadSquares(ReadFrischAuf);
        break;
      case messigny:
        tok = ParseRex(&rex_mess_ex, rexexcl);
        break;
      case woozles:
        tok = ParseRex(&rex_wooz_ex, rexexcl);
        break;
      case biwoozles:
        tok = ParseRex(&rex_wooz_ex, rexexcl);
        break;
      case immun:
        tok = ParseRex(&rex_immun, rexincl);
        break;
      case immunmalefique:
        tok = ParseRex(&rex_immun, rexincl);
        break;
      case immundiagramm:
        tok = ParseRex(&rex_immun, rexincl);
        break;
      case chamcirce:
        ReadChameleonCirceSequence();
        break;
      case circe:
        tok = ParseRex(&rex_circe, rexincl);
        break;
      case circemalefique:
        tok = ParseRex(&rex_circe, rexincl);
        break;
      case circediagramm:
        tok = ParseRex(&rex_circe, rexincl);
        break;
      case circeclone:
        tok = ParseRex(&rex_circe, rexincl);
        break;
      case circeclonemalefique:
        tok = ParseRex(&rex_circe, rexincl);
        break;
      case circemalefiquevertical:
        tok = ParseRex(&rex_circe, rexincl);
        break;
      case protean:
        tok = ParseRex(&rex_protean_ex, rexexcl);
        break;
      case phantom:
        tok = ParseRex(&rex_phan, rexincl);
        break;
      case madras:
        tok = ParseRex(&rex_mad, rexincl);
        flag_madrasi= true;
        break;
      case isardam:
        tok = ParseVariant(&IsardamB, gpType);
        break;
      case annan:
        annanvar = 0;
        tok = ParseVariant(NULL, gpAnnan);
        break;
      case patience:
        tok = ParseVariant(&PatienceB, gpType);
        break;
      case sentinelles:
        SentPionNeutral=false;
        tok = ParseVariant(&SentPionAdverse, gpSentinelles);
        break;

        /*****  exact-maxis  *****/
      case blmax:
        tok = ParseExact(&bl_exact, &bl_ultra);
        break;
      case blmin:
        tok = ParseExact(&bl_exact, &bl_ultra);
        break;
      case blcapt:
        tok = ParseExact(&bl_exact, &bl_ultra);
        break;
      case whmax:
        tok = ParseExact(&wh_exact, &wh_ultra);
        break;
      case whmin:
        tok = ParseExact(&wh_exact, &wh_ultra);
        break;
      case whcapt:
        tok = ParseExact(&wh_exact, &wh_ultra);
        break;

        /*****  anticirce type    *****/
      case anti:
        tok = ParseVariant(&AntiCirCheylan, gpAntiCirce);
        AntiCirType= AntiCirCheylan
            ? AntiCirTypeCheylan : AntiCirTypeCalvet;
        break;
      case antispiegel:
        tok = ParseVariant(&AntiCirCheylan, gpAntiCirce);
        AntiCirType= AntiCirCheylan
            ? AntiCirTypeCheylan : AntiCirTypeCalvet;
        break;
      case antidiagramm:
        tok = ParseVariant(&AntiCirCheylan, gpAntiCirce);
        AntiCirType= AntiCirCheylan
            ? AntiCirTypeCheylan : AntiCirTypeCalvet;
        break;
      case antifile:
        tok = ParseVariant(&AntiCirCheylan, gpAntiCirce);
        AntiCirType= AntiCirCheylan
            ? AntiCirTypeCheylan : AntiCirTypeCalvet;
        break;
      case antisymmetrie:
        tok = ParseVariant(&AntiCirCheylan, gpAntiCirce);
        AntiCirType= AntiCirCheylan
            ? AntiCirTypeCheylan : AntiCirTypeCalvet;
        break;
      case antispiegelfile:
        tok = ParseVariant(&AntiCirCheylan, gpAntiCirce);
        AntiCirType= AntiCirCheylan
            ? AntiCirTypeCheylan : AntiCirTypeCalvet;
        break;
      case antiantipoden:
        tok = ParseVariant(&AntiCirCheylan, gpAntiCirce);
        AntiCirType= AntiCirCheylan
            ? AntiCirTypeCheylan : AntiCirTypeCalvet;
        break;
      case antiequipollents:
        tok = ParseVariant(&AntiCirCheylan, gpAntiCirce);
        AntiCirType= AntiCirCheylan
            ? AntiCirTypeCheylan : AntiCirTypeCalvet;
        break;
      case antisuper:
        tok = ParseVariant(&AntiCirCheylan, gpAntiCirce);
        AntiCirType= AntiCirCheylan
            ? AntiCirTypeCheylan : AntiCirTypeCalvet;
        break;
      case singlebox:
        tok = ParseVariant(NULL, gpType);
        break;
      case republican:
      {
        boolean RepublicanType1 = false;
        tok = ParseVariant(&RepublicanType1, gpRepublican);
        RepublicanType = RepublicanType1 ? republican_type1 : republican_type2;
        break;
      }
      case promotiononly:
        tok = ReadPieces(promotiononly);
        break;
      case april:
        tok = ReadPieces(april);
        break;
      case koeko:
        koekofunc= nokingcontact;
        nocontactfunc= &koekofunc;
        tok = ParseVariant(NULL, gpKoeko);
        break;
      case antikoeko:
        antikoekofunc= nokingcontact;
        nocontactfunc= &antikoekofunc;
        tok = ParseVariant(NULL, gpKoeko);
        break;
      case white_oscillatingKs:
        OscillatingKingsSide= White;
        tok = ParseVariant(NULL, gpOsc);
        break;
      case black_oscillatingKs:
        OscillatingKingsSide= Black;
        tok = ParseVariant(NULL, gpOsc);
        break;
      case swappingkings:
        CondFlag[white_oscillatingKs]= true;
        OscillatingKingsTypeC[White]= true;
        CondFlag[black_oscillatingKs]= true;
        OscillatingKingsTypeC[Black]= true;
        tok = ReadNextTokStr();
        break;
      case SAT:
      case strictSAT:
        tok = ReadNextTokStr();
        WhiteSATFlights= strtol(tok,&ptr,10) + 1;
        if (tok == ptr) {
          WhiteSATFlights= 1;
          BlackSATFlights= 1;
          break;
        }
        tok = ReadNextTokStr();
        BlackSATFlights= strtol(tok,&ptr,10) + 1;
        if (tok == ptr) {
          BlackSATFlights= WhiteSATFlights;
          break;
        }
      case BGL:
        BGL_global= false;
        tok = ReadNextTokStr();
        BGL_white= ReadBGLNumber(tok,&ptr); 
        if (tok == ptr)  
        {
          BGL_white= BGL_black= BGL_infinity;
          return tok;
        }
        else
        {
          tok = ReadNextTokStr();
          BGL_black= ReadBGLNumber(tok,&ptr);
          if (tok == ptr)
          {
            BGL_black= BGL_white;
            BGL_global= true;
            return tok;
          }
        }
        tok = ReadNextTokStr();
        break;
      case geneva:
        tok = ParseRex(&rex_geneva, rexincl);
        break;
      case whvault_king:
        whitenormaltranspieces = false;
        tok = ParseVaultingPieces(BIT(White));
        break;
      case blvault_king:
        blacknormaltranspieces = false;
        tok = ParseVaultingPieces(BIT(Black));
        break;
      case vault_king:
        whitenormaltranspieces = false;
        blacknormaltranspieces = false;
        tok = ParseVaultingPieces(BIT(White) | BIT(Black));
        break;
      case gridchess: 
        tok = ParseVariant(NULL, gpGrid);
        break;
      default:
        tok = ReadNextTokStr();
    }
  }

  if (CondCnt==0)
    IoErrorMsg(UnrecCondition,0);

  return tok;
} /* ParseCond */

static void ReadMutuallyExclusiveCastling(void)
{
  char const *tok = ReadNextTokStr();
  if (strlen(tok)==4)
  {
    square const white_rook_square = SquareNum(tok[0],tok[1]);
    square const black_rook_square = SquareNum(tok[2],tok[3]);
    if (game_array.board[white_rook_square]==tb
        && game_array.board[black_rook_square]==tn)
    {
      square const white_castling = (white_rook_square==square_a1
                                     ? queenside_castling
                                     : kingside_castling);
      castling_flag_type const white_flag = (white_rook_square==square_a1
                                             ? ra1_cancastle
                                             : rh1_cancastle);
      square const black_castling = (black_rook_square==square_a8
                                     ? queenside_castling
                                     : kingside_castling);
      castling_flag_type const black_flag = (black_rook_square==square_a8
                                             ? ra8_cancastle
                                             : rh8_cancastle);
      castling_mutual_exclusive[White][white_castling-min_castling] |= black_flag;
      castling_mutual_exclusive[Black][black_castling-min_castling] |= white_flag;
      return;
    }
  }

  ErrorMsg(MissngSquareList);
}


static char *ParseOpt(void)
{
  Opt indexx;
  unsigned int OptCnt = 0;
  char    *tok;

  tok = ReadNextTokStr();
  for (indexx = GetUniqIndex(OptCount,OptTab,tok);
       indexx<OptCount+unrecognised_delta;
       indexx = GetUniqIndex(OptCount,OptTab,tok))
  {
    if (indexx==OptCount+ambiguous_delta)
    {
      IoErrorMsg(OptNotUniq,0);
      continue;
    }
    OptFlag[indexx]= true;
    OptCnt++;

    switch(indexx)
    {
      case beep:
        tok = ReadNextTokStr();
        if ((maxbeep= atoi(tok)) <= 0)
        {
          maxbeep= 1;
          /* we did NOT use tok */
          continue;
        }
        else
          /* we did use tok */
          break;

      case maxtime:
      {
        char *end;
        unsigned long value;
        tok = ReadNextTokStr();
        value = strtoul(tok,&end,10);
        if (*end!=0 || value==0 || value>maxtimeMaximumSeconds())
        {
          OptFlag[maxtime]= false;
          IoErrorMsg(WrongInt, 0);
          return ReadNextTokStr();
        }
        else
          setOptionMaxtime((maxtime_type)value);
        break;
      }

      case enpassant:
        ReadSquares(ReadEpSquares);
        break;

      case maxsols:
        tok = ReadNextTokStr();
        if (!read_max_solutions(tok))
        {
          OptFlag[maxsols] = false;
          IoErrorMsg(WrongInt,0);
          return ReadNextTokStr();
        }
        break;

      case intelligent:
      {
        char *end;
        tok = ReadNextTokStr();
        maxsol_per_matingpos= strtoul(tok,&end,10);
        if (end==tok)
        {
          maxsol_per_matingpos = ULONG_MAX;
          /* we did NOT consume tok */
          continue;
        }
        else
          /* we did use consume */
          break;
      }

      case restart:
        tok = ReadNextTokStr();
        if (!read_restart_number(tok))
        {
          OptFlag[restart] = false;
          IoErrorMsg(WrongInt,0);
          return ReadNextTokStr();
        }
        OptFlag[movenbr]= true;
        break;

      case solmenaces:
        tok = ReadNextTokStr();
        if (read_max_threat_length(tok))
        {
          OptFlag[degeneratetree] = true;
          init_degenerate_tree(get_max_threat_length());
        }
        else
        {
          OptFlag[solmenaces] = false;
          IoErrorMsg(WrongInt,0);
          return ReadNextTokStr();
        }
        break;

      case solflights:
        tok = ReadNextTokStr();
        if (!read_max_flights(tok))
        {
          OptFlag[solflights] = false;
          IoErrorMsg(WrongInt,0);
          return ReadNextTokStr();
        }
        break;

      case soltout:
        tok = ReadNextTokStr();
        if (!read_max_nr_refutations(tok))
        {
          OptFlag[soltout] = false;
          IoErrorMsg(WrongInt,0);
          return ReadNextTokStr();
        }
        break;

      case solessais:
        OptFlag[soltout]= true; /* for compatibility to older versions. */
        set_max_nr_refutations(1);
        break;

      case nontrivial:
      {
        tok = ReadNextTokStr();
        if (read_max_nr_nontrivial(tok))
        {
          tok = ReadNextTokStr();
          if (read_min_length_nontrivial(tok))
          {
            OptFlag[degeneratetree] = true;
            init_degenerate_tree(get_min_length_nontrivial());
          }
          else
          {
            OptFlag[nontrivial] = false;
            IoErrorMsg(WrongInt, 0);
            return ReadNextTokStr();
          }
        }
        else
        {
          OptFlag[nontrivial] = false;
          IoErrorMsg(WrongInt, 0);
          return ReadNextTokStr();
        }
        break;
      }

      case postkeyplay:
        OptFlag[solvariantes]= true;
        break;

      case quodlibet:
        if (!transform_to_quodlibet())
          Message(QuodlibetNotApplicable);
        break;

      case nocastling:
        no_castling= bl_castlings|wh_castlings;
        ReadSquares(ReadNoCastlingSquares);
        break;

      case lastcapture:
        tok = ParsePieSpec(1);
        break;

      case mutuallyexclusivecastling:
        ReadMutuallyExclusiveCastling();
        break;

      default:
        /* no extra action required */
        break;
    }
    if (indexx != lastcapture)
      tok = ReadNextTokStr();
  }

  if (OptCnt==0)
    IoErrorMsg(UnrecOption,0);

  return tok;
}

/***** twinning ***** begin *****/

static unsigned int TwinNumber;

piece  twin_e[nr_squares_on_board];
Flags  twin_spec[nr_squares_on_board];
square twin_rb, twin_rn;
imarr  twin_isquare;

static void TwinStorePosition(void)
{
  int i;

  twin_rb= rb;
  twin_rn= rn;
  for (i= 0; i < nr_squares_on_board; i++)
  {
    twin_e[i]= e[boardnum[i]];
    twin_spec[i]= spec[boardnum[i]];
  }

  for (i= 0; i < maxinum; i++)
    twin_isquare[i]= isquare[i];
}

static void TwinResetPosition(void)
{
  int i;

  rb= twin_rb;
  rn= twin_rn;
  for (i= 0; i < nr_squares_on_board; i++) {
    e[boardnum[i]]= twin_e[i];
    spec[boardnum[i]]= twin_spec[i];
  }

  for (i= 0; i < maxinum; i++)
    isquare[i]= twin_isquare[i];
}

void transformPosition(SquareTransformation transformation)
{
  piece t_e[nr_squares_on_board];
  Flags t_spec[nr_squares_on_board];
  square t_rb, t_rn, sq1, sq2;
  imarr t_isquare;
  int i;

  /* save the position to be mirrored/rotated */
  t_rb = rb;
  t_rn = rn;
  for (i = 0; i<nr_squares_on_board; i++)
  {
    t_e[i] = e[boardnum[i]];
    t_spec[i] = spec[boardnum[i]];
  }

  for (i = 0; i<maxinum; i++)
    t_isquare[i] = isquare[i];

  /* now rotate/mirror */
  /* pieces */
  for (i = 0; i<nr_squares_on_board; i++)
  {
    sq1 = boardnum[i];
    sq2 = transformSquare(sq1,transformation);

    e[sq2] = t_e[i];
    spec[sq2] = t_spec[i];

    if (sq1==t_rb)
      rb = sq2;
    if (sq1==t_rn)
      rn = sq2;
  }

  /* imitators */
  for (i= 0; i<maxinum; i++)
  {
    sq1 = t_isquare[i];
    sq2 = transformSquare(sq1, transformation);
    isquare[i]= sq2;
  }
} /* transformPosition */

static char *ParseTwinningRotate(void)
{
  char *tok = ReadNextTokStr();

  if (strcmp(tok,"90")==0)
    transformPosition(rot90);
  else if (strcmp(tok,"180")==0)
    transformPosition(rot180);
  else if (strcmp(tok,"270")==0)
    transformPosition(rot270);
  else
    IoErrorMsg(UnrecRotMirr,0);

  if (LaTeXout)
  {
    sprintf(GlobalStr, "%s $%s^\\circ$", TwinningTab[TwinningRotate], tok);
    strcat(ActTwinning, GlobalStr);
  }

  StdString(TwinningTab[TwinningRotate]);
  StdString(" ");
  StdString(tok);

  return ReadNextTokStr();
}

static char *ParseTwinningMirror(void)
{
  char *tok = ReadNextTokStr();
  TwinningType indexx= GetUniqIndex(TwinningCount,TwinningTab,tok);

  if (indexx==TwinningCount+ambiguous_delta)
    IoErrorMsg(OptNotUniq,0);
  else
  {
    switch (indexx)
    {
      case TwinningMirra1h1:
        transformPosition(mirra1h1);
        break;

      case TwinningMirra1a8:
        transformPosition(mirra1a8);
        break;

      case TwinningMirra1h8:
        transformPosition(mirra1h8);
        break;

      case TwinningMirra8h1:
        transformPosition(mirra8h1);
        break;

      default:
        IoErrorMsg(UnrecRotMirr,0);
        break;
    }

    StdString(TwinningTab[TwinningMirror]);
    StdString(" ");
    StdString(TwinningTab[indexx]);
  }

  return ReadNextTokStr();
}

static char *ParseTwinningMove(int indexx)
{
  square sq1= 0, sq2= 0;
  char *tok;
  piece p;
  Flags sp;

  /* read the first square */
  while (sq1 == 0)
  {
    tok = ReadNextTokStr();
    sq1= SquareNum(tok[0], tok[1]);
    if (sq1 == initsquare) {
      ErrorMsg(WrongSquareList);
      return ReadNextTokStr();
    }
  }

  /* read the second square */
  while (sq2 == 0)
  {
    tok = ReadNextTokStr();
    sq2= SquareNum(tok[0], tok[1]);
    if (sq2 == initsquare) {
      ErrorMsg(WrongSquareList);
      return ReadNextTokStr();
    }
  }

  if (e[sq1] == vide)
  {
    WriteSquare(sq1);
    StdString(": ");
    ErrorMsg(NothingToRemove);
    return ReadNextTokStr();
  }

  /* issue the twinning */
  if (LaTeXout)
  {
    sprintf(GlobalStr, "\\%c%s %c%c",
            TSTFLAG(spec[sq1], Neutral)
            ? 'n'
            : TSTFLAG(spec[sq1], White) ? 'w' : 's',
            LaTeXPiece(e[sq1]),
            'a'-nr_files_on_board+sq1%onerow,
            '1'-nr_rows_on_board+sq1/onerow);
    strcat(ActTwinning, GlobalStr);
  }

  WriteSpec(spec[sq1], e[sq1]!=vide);
  WritePiece(e[sq1]);
  WriteSquare(sq1);
  if (indexx == TwinningExchange) {
    StdString("<-->");
    WriteSpec(spec[sq2], e[sq2]!=vide);
    WritePiece(e[sq2]);
    if (LaTeXout) {
      strcat(ActTwinning, "{\\lra}");
      sprintf(GlobalStr, "\\%c%s ",
              TSTFLAG(spec[sq2], Neutral)
              ? 'n'
              : TSTFLAG(spec[sq2], White) ? 'w' : 's',
              LaTeXPiece(e[sq2]));
      strcat(ActTwinning, GlobalStr);
    }
  }
  else {
    StdString("-->");
    if (LaTeXout) {
      strcat(ActTwinning, "{\\ra}");
    }
  }
  WriteSquare(sq2);
  if (LaTeXout) {
    sprintf(GlobalStr, "%c%c",
            'a'-nr_files_on_board+sq2%onerow,
            '1'-nr_rows_on_board+sq2/onerow);
    strcat(ActTwinning, GlobalStr);
  }

  /* store the piece in case they are exchanged */
  p= e[sq2];
  sp= spec[sq2];

  /* move the piece from sq1 to sq2 */
  e[sq2]= e[sq1];
  spec[sq2]= spec[sq1];

  /* delete the other piece from p or delete sq1 */
  e[sq1]= indexx == TwinningMove ? vide : p;
  spec[sq1]= indexx == TwinningMove ? 0 : sp;

  /* update king pointer */
  if (sq1 == rb) {
    rb= sq2;
  }
  else if (sq2 == rb) {
    rb= indexx == TwinningExchange ? sq1 : initsquare;
  }
  if (sq1 == rn) {
    rn= sq2;
  }
  else if (sq2 == rn) {
    rn= indexx == TwinningExchange ? sq1 : initsquare;
  }

  /* read next token */
  return ReadNextTokStr();

} /* ParseTwinningMove */

static void MovePieceFromTo(square from, square to)
{
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

static char *ParseTwinningShift(void) {
  square sq1= 0, sq2= 0;
  square const *bnp;
  char *tok;
  int diffrank, diffcol, minrank, maxrank, mincol, maxcol, r, c;

  /* read the first square */
  while (sq1 == 0) {
    tok = ReadNextTokStr();
    sq1= SquareNum(tok[0], tok[1]);
    if (sq1 == initsquare) {
      ErrorMsg(WrongSquareList);
    }
  }

  /* read the second square */
  while (sq2 == 0) {
    tok = ReadNextTokStr();
    sq2= SquareNum(tok[0], tok[1]);
    if (sq2 == initsquare) {
      ErrorMsg(WrongSquareList);
    }
  }

  /* issue the twinning */
  if (LaTeXout) {
    sprintf(GlobalStr, "%s %c%c$\\Rightarrow$%c%c",
            TwinningTab[TwinningShift],
            'a'-nr_files_on_board+sq1%onerow,
            '1'-nr_rows_on_board+sq1/onerow,
            'a'-nr_files_on_board+sq2%onerow,
            '1'-nr_rows_on_board+sq2/onerow);
    strcat(ActTwinning, GlobalStr);
  }

  StdString(TwinningTab[TwinningShift]);
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

} /* ParseTwinningShift */

static char *ParseTwinningRemove(void) {
  square    sq;
  char  *tok;
  boolean   WrongList;

  do {
    WrongList= false;
    tok = ReadNextTokStr();

    if (strlen(tok) % 2) {
      WrongList= true;
    }
    else {
      char *tok2= tok;

      while (*tok2 && !WrongList) {
        if (SquareNum(tok2[0], tok2[1]) == initsquare) {
          WrongList= true;
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
        strcat(ActTwinning, " --");
        strcat(ActTwinning,
               TSTFLAG(spec[sq], Neutral)
               ? "\\n"
               : TSTFLAG(spec[sq], White) ? "\\w" : "\\s");
        strcat(ActTwinning,
               LaTeXPiece(e[sq]));
        sprintf(GlobalStr, " %c%c",
                'a'-nr_files_on_board+sq%onerow,
                '1'-nr_rows_on_board+sq/onerow);
        strcat(ActTwinning, GlobalStr);
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
} /* ParseTwinningRemove */

static char *ParseTwinningPolish(void) {
  square const *bnp;
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

  StdString(TwinningTab[TwinningPolish]);

  if (LaTeXout) {
    strcat(ActTwinning, TwinningTab[TwinningPolish]);
  }

  return ReadNextTokStr();
}

static char *ParseTwinningSubstitute(void) {
  square const *bnp;
  piece p_old, p_new;
  char  *tok;

  tok = ReadNextTokStr();
  switch (strlen(tok))
  {
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
  
  tok = ReadNextTokStr();
  switch (strlen(tok))
  {
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
    strcat(ActTwinning, GlobalStr);
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

void WriteTwinNumber(void)
{
  if (TwinNumber-1<='z'-'a')
    sprintf(GlobalStr, "%c) ", 'a'+TwinNumber-1);
  else
    sprintf(GlobalStr, "z%u) ", (unsigned int)(TwinNumber-1-('z'-'a')));

  StdString(GlobalStr);
  if (LaTeXout)
    strcat(ActTwinning, GlobalStr);
}

static char *ParseTwinning(boolean *stipChanged)
{
  char  *tok = ReadNextTokStr();
  boolean continued= false;
  boolean TwinningRead= false;

  ++TwinNumber;
  OptFlag[noboard]= true;
  *stipChanged = false;

  while (true)
  {
    TwinningType twinning;
    Token tk = StringToToken(tok);

    if (tk==TwinProblem
        || tk==NextProblem
        || tk==EndProblem)
    {
      Message(NewLine);
      if (LaTeXout)
        strcat(ActTwinning, "{\\newline}");
      return tok;
    }
    
    twinning = GetUniqIndex(TwinningCount,TwinningTab,tok);
    switch (twinning)
    {
      case TwinningCount+ambiguous_delta:
        IoErrorMsg(OptNotUniq,0);
        tok = ReadNextTokStr();
        continue;

      case TwinningCount+unrecognised_delta:
        IoErrorMsg(ComNotKnown,0);
        tok = ReadNextTokStr();
        continue;

      case TwinningContinued:
        if (TwinningRead == true) {
          Message(ContinuedFirst);
        }
        else {
          continued= true;
        }
        tok = ReadNextTokStr();
        continue;

      default:
        break;
    }

    if (!TwinningRead)
    {
      if (continued)
      {
        StdChar('+');
        if (LaTeXout)
          strcat(ActTwinning, "+");
      }
      else
        TwinResetPosition();

      WriteTwinNumber();
    }
    else
    {
      StdString("  ");
      if (LaTeXout)
        strcat(ActTwinning, ", ");
    } /* !TwinningRead */

    TwinningRead= true;
    switch(twinning)
    {
      case TwinningMove:
        tok = ParseTwinningMove(twinning);
        break;
      case TwinningExchange:
        tok = ParseTwinningMove(twinning);
        break;
      case TwinningRotate:
        tok = ParseTwinningRotate();
        break;
      case TwinningMirror:
        tok = ParseTwinningMirror();
        break;
      case TwinningStip:
        *stipChanged = true;
        InitStip();
        tok = ParseStip();

        /* issue the twinning */
        StdString(AlphaStip);
        if (LaTeXout) {
          strcat(ActTwinning, AlphaStip);
          if (OptFlag[solapparent]) {
            strcat(ActTwinning, "*");
          }
          if (OptFlag[whitetoplay]) {
            char temp[10];        /* increased due to buffer overflow */
            sprintf(temp, " %c{\\ra}",
                    tolower(*PieSpString[UserLanguage][White]));
            strcat(ActTwinning, temp);
          }
        }
        break;
      case TwinningStructStip:
        *stipChanged = true;
        InitStip();
        root_slice = alloc_proxy_slice();
        tok = ParseStructuredStip(root_slice);

        /* issue the twinning */
        StdString(AlphaStip);
        if (LaTeXout) {
          strcat(ActTwinning, AlphaStip);
          if (OptFlag[solapparent]) {
            strcat(ActTwinning, "*");
          }
          if (OptFlag[whitetoplay]) {
            char temp[10];        /* increased due to buffer overflow */
            sprintf(temp, " %c{\\ra}",
                    tolower(*PieSpString[UserLanguage][White]));
            strcat(ActTwinning, temp);
          }
        }
        break;
      case TwinningAdd:
        tok = ParsePieSpec('+');
        break;
      case TwinningCond:
        InitCond();
        tok = ParseCond();
        WriteConditions(WCleft);
        break;
      case TwinningRemove:
        tok = ParseTwinningRemove();
        break;
      case TwinningPolish:
        tok = ParseTwinningPolish();
        break;
      case TwinningShift:
        tok = ParseTwinningShift();
        break;
      case TwinningSubstitute:
        tok = ParseTwinningSubstitute();
        break;
      case TwinningForsyth:
        tok = ParseForsyth(true);
        break;
      default:
        /* no further action required */
        break;
    }
  }
} /* ParseTwinning */

/***** twinning *****  end  *****/

/* new conditions: PromOnly, AprilChess */
char *ReadPieces(int condition) {
  piece tmp_piece;
  char  *tok;
  boolean   piece_read= false;

  fflush(stdout);
  while (true)
  {
    tok = ReadNextTokStr();
    switch (strlen(tok)) {
    case 1:
      tmp_piece= GetPieNamIndex(*tok,' ');
      piece_read= true;
      break;
    case 2:
      tmp_piece= GetPieNamIndex(*tok,tok[1]);
      piece_read= true;
      break;
    default:
      if (!piece_read) {
        CondFlag[condition]= false;
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
      promonly[tmp_piece]= true;
      break;
    case april:
      isapril[tmp_piece]= true;
      break;
    default:
      /* Never mind ... */
      break;
    }
  }
  return tok;
}


Token ReadTwin(Token tk, boolean *stipChanged)
{
  char *tok;

  /* open mode for protocol and/or TeX file; overwrite existing file(s)
   * if we are doing a regression test */
  char const *open_mode = flag_regression ? "w" : "a";

  if (tk==BeginProblem)
  {
    LastChar= ' ';
    ReadBeginSpec();
  }

  if (tk == TwinProblem || tk == ZeroPosition)
  {
    if (tk==ZeroPosition)
    {
      StdString("\n");
      StdString(TokenTab[ZeroPosition]);
      StdString("\n\n");
      TwinNumber= 0;
      TwinStorePosition();
    }
    tok = ParseTwinning(stipChanged);

    while (true)
    {
      tk = StringToToken(tok);
      switch (tk)
      {
        case TokenCount+ambiguous_delta:
          IoErrorMsg(ComNotUniq,0);
          tok = ReadNextTokStr();
          break;

        case TwinProblem:
          if (root_slice!=no_slice)
            return tk;
          else
          {
            IoErrorMsg(NoStipulation,0);
            tok = ReadNextTokStr();
            break;
          }

        case NextProblem:
        case EndProblem:
          if (root_slice!=no_slice)
            return tk;
          else
          {
            IoErrorMsg(NoStipulation,0);
            tok = ReadNextTokStr();
            break;
          }

        default:
          IoErrorMsg(ComNotKnown,0);
          tok = ReadNextTokStr();
          break;
      }
    }
  }
  else
  {
    tok = ReadNextTokStr();
    TwinNumber= 1;
    while (true)
    {
      tk = StringToToken(tok);
      switch (tk)
      {
        case TokenCount+ambiguous_delta:
          IoErrorMsg(ComNotUniq,0);
          tok = ReadNextTokStr();
          break;

        case TokenCount+unrecognised_delta:
          IoErrorMsg(ComNotKnown,0);
          tok = ReadNextTokStr();
          break;

        case BeginProblem:
          tok = ReadNextTokStr();
          break;

        case TwinProblem:
          if (TwinNumber==1)
            TwinStorePosition();

          if (root_slice!=no_slice)
            return tk;
          else
          {
            IoErrorMsg(NoStipulation,0);
            tok = ReadNextTokStr();
            break;
          }

        case NextProblem:
        case EndProblem:
          if (root_slice!=no_slice)
            return tk;
          else
          {
            IoErrorMsg(NoStipulation,0);
            tok = ReadNextTokStr();
            break;
          }

        case ZeroPosition:
          return tk;

        case StipToken:
          *AlphaStip='\0';
          tok = ParseStip();
          break;

        case StructStipToken:
          *AlphaStip='\0';
          root_slice = alloc_proxy_slice();
          tok = ParseStructuredStip(root_slice);
          break;

        case Author:
          strcat(ActAuthor,ReadToEndOfLine());
          strcat(ActAuthor,"\n");
          tok = ReadNextTokStr();
          break;

        case Award:
          strcpy(ActAward,ReadToEndOfLine());
          strcat(ActAward, "\n");
          tok = ReadNextTokStr();
          break;

        case Origin:
          strcat(ActOrigin,ReadToEndOfLine());
          strcat(ActOrigin,"\n");
          tok = ReadNextTokStr();
          break;

        case TitleToken:
          strcat(ActTitle,ReadToEndOfLine());
          strcat(ActTitle,"\n");
          tok = ReadNextTokStr();
          break;

        case PieceToken:
          tok = ParsePieSpec('\0');
          break;

        case CondToken:
          tok = ParseCond();
          break;

        case OptToken:
          tok = ParseOpt();
          break;

        case RemToken:
          if (LastChar != '\n')
          {
            ReadToEndOfLine();
            if (TraceFile!=NULL)
            {
              fputs(InputLine, TraceFile);
              fflush(TraceFile);
            }
            Message(NewLine);
          }
          tok = ReadNextTokStr();
          break;

        case InputToken:
          PushInput(ReadToEndOfLine());
          tok = ReadNextTokStr();
          break;

        case TraceToken:
          if (TraceFile!=NULL)
            fclose(TraceFile);

          TraceFile = fopen(ReadToEndOfLine(),open_mode);
          if (TraceFile==NULL)
            IoErrorMsg(WrOpenError,0);
          else if (!flag_regression)
          {
            fputs(versionString,TraceFile);
            fputs(maxmemString(),TraceFile);
            fflush(TraceFile);
          }
          tok = ReadNextTokStr();
          break;

        case LaTeXPieces:
          tok = ParseLaTeXPieces(ReadNextTokStr());
          break;

        case LaTeXToken:
          LaTeXout = true;
          if (LaTeXFile!=NULL)
          {
            LaTeXClose();
            fclose(LaTeXFile);
          }

          LaTeXFile= fopen(ReadToEndOfLine(),open_mode);
          if (LaTeXFile==NULL)
          {
            IoErrorMsg(WrOpenError,0);
            LaTeXout= false;
          }
          else
            LaTeXOpen();

          if (SolFile!=NULL)
            fclose(SolFile);

          SolFile = tmpfile();
          if (SolFile==NULL)
            IoErrorMsg(WrOpenError,0);
          else
            tok = ParseLaTeXPieces(ReadNextTokStr());
          break;

        case SepToken:
          tok = ReadNextTokStr();
          break;

        case Array:
          tok = ReadNextTokStr();
          {
            int i;
            for (i = 0; i<nr_squares_on_board; i++)
            {
              piece const p = PAS[i];
              e[boardnum[i]] = PAS[i];
              CLEARFL(spec[boardnum[i]]);
              if (p >= roib)
                SETFLAG(spec[boardnum[i]], White);
              else if (p <= roin)
                SETFLAG(spec[boardnum[i]], Black);
            }
            rb = square_e1;
            rn = square_e8;
          }
          break;

        case Forsyth:
          tok = ParseForsyth(false);
          break;

        default:
          FtlMsg(InternalError);
      }
    } /* while */
  }
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

void WritePosition() {
  int nBlack, nWhite, nNeutr;
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

  unsigned int const fileWidth = 4;

  SolFile= NULL;

  for (sp= Neutral; sp < PieSpCount; sp++)
    strcpy(ListSpec[sp], PieSpString[UserLanguage][sp]);

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
  if (CondFlag[imitators])
  {
    unsigned int imi_idx;
    for (imi_idx = 0; imi_idx<inum[1]; imi_idx++)
      e[isquare[imi_idx]]= -obs;
  }

  for (row=1, square_a = square_a8;
       row<=nr_rows_on_board;
       row++, square_a += dir_down)
  {
    char const *digits="87654321";
    sprintf(HLine1, HorizL, digits[row-1], digits[row-1]);

    strcpy(HLine2,BlankL);

    for (file= 1, square= square_a;
         file <= nr_files_on_board;
         file++, square += dir_right) {
      char *h1= HLine1 + fileWidth*file;

      if (CondFlag[gridchess] && !OptFlag[suppressgrid])
      {
        if (file < nr_files_on_board
            && GridLegal(square, square+dir_right))
          HLine1[fileWidth*file+2] = '|';

        if (row < nr_rows_on_board
            && GridLegal(square, square+dir_down))
        {
          HLine2[fileWidth*file-1] = '-';
          HLine2[fileWidth*file] = '-';
          HLine2[fileWidth*file+1] = '-';
        }
      }

      if ((pp= abs(p= e[square])) < King)
      {
        if (p == -obs)
        {
          /* this is an imitator ! */
          *h1= 'I';
          e[square]= vide; /* "delete" imitator */
        }
        else if (p == obs)
          /* this is a hole ! */
          *h1= ' ';
        /* else:  the square is empty ! */
        continue;
      }

      for (sp= Neutral + 1; sp < PieSpCount; sp++)
        if (TSTFLAG(spec[square], sp))
          AddSquare(ListSpec[sp], square);

      if (pp<Hunter0 || pp >= (Hunter0 + maxnrhuntertypes))
      {
        if ((*h1= PieceTab[pp][1]) != ' ')
        {
          *h1= UPCASE(*h1);
          h1--;
        }
        *h1--= UPCASE(PieceTab[pp][0]);
      }
      else
      {
        char *n1 = HLine2 + (h1-HLine1); /* current position on next line */

        unsigned int const hunterIndex = pp-Hunter0;
        assert(hunterIndex<maxnrhuntertypes);

        *h1-- = '/';
        if ((*h1= PieceTab[huntertypes[hunterIndex].away][1]) != ' ')
        {
          *h1= UPCASE(*h1);
          h1--;
        }
        *h1--= UPCASE(PieceTab[huntertypes[hunterIndex].away][0]);

        --n1;   /* leave pos. below '/' empty */
        if ((*n1= PieceTab[huntertypes[hunterIndex].home][1]) != ' ')
          *n1= UPCASE(*n1);
        *n1 = UPCASE(PieceTab[huntertypes[hunterIndex].home][0]);
      }

      if (TSTFLAG(spec[square], Neutral))
      {
        nNeutr++;
        *h1= '=';
      }
      else if (p < 0)
      {
        nBlack++;
        *h1= '-';
      }
      else
        nWhite++;
    }

    StdString(HLine1);
    StdString(HLine2);
  }

  StdString(BorderL);
  if (nNeutr>0)
    sprintf(PieCnts, "%d + %d + %dn", nWhite, nBlack, nNeutr);
  else
    sprintf(PieCnts, "%d + %d", nWhite, nBlack);

  strcpy(StipOptStr, AlphaStip);

  if (OptFlag[solmenaces])
  {
    sprintf(StipOptStr+strlen(StipOptStr), "/%u", get_max_threat_length());
    if (OptFlag[solflights])
      sprintf(StipOptStr+strlen(StipOptStr), "/%d", get_max_flights());
  }
  else if (OptFlag[solflights])
    sprintf(StipOptStr+strlen(StipOptStr), "//%d", get_max_flights());

  if (OptFlag[nontrivial])
    sprintf(StipOptStr+strlen(StipOptStr),
            ";%d,%u",
            max_nr_nontrivial,get_min_length_nontrivial());

  {
    size_t const stipOptLength = strlen(StipOptStr);
    int const pieceCntWidth = (stipOptLength>nr_files_on_board*fileWidth
                               ? 1
                               : nr_files_on_board*fileWidth-stipOptLength+1);
    sprintf(GlobalStr, "  %s%*s\n", StipOptStr, pieceCntWidth, PieCnts);
    StdString(GlobalStr);
  }

  for (sp = Neutral+1; sp<PieSpCount; sp++)
    if (TSTFLAG(PieSpExFlags,sp))
      CenterLine(ListSpec[sp]);

  WriteConditions(WCcentered);

  WriteCastlingMutuallyExclusive();

  if (OptFlag[halfduplex])
    CenterLine(OptString[UserLanguage][halfduplex]);
  else if (OptFlag[duplex])
    CenterLine(OptString[UserLanguage][duplex]);

  if (OptFlag[quodlibet])
    CenterLine(OptString[UserLanguage][quodlibet]);

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
    fprintf(LaTeXFile, "%s", versionString);
  fprintf(LaTeXFile, "\n");
  fprintf(LaTeXFile, "\\usepackage{diagram}\n");
  if (UserLanguage == German) {
    fprintf(LaTeXFile, "\\usepackage{german}\n");
  }
  fprintf(LaTeXFile, "\n\\begin{document}\n\n");
}

void LaTeXClose(void) {
  fprintf(LaTeXFile, "\n\\putsol\n\n\\end{document}\n");
}

void LaTeXEndDiagram(void) {
  char line[256];

  /* twins */
  if (ActTwinning[0] != '\0') {
    fprintf(LaTeXFile, " \\twins{");
    /* remove the last "{\\newline} */
    ActTwinning[strlen(ActTwinning)-10]= '\0';
    LaTeXStr(ActTwinning);
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
        || maxsol_per_matingpos!=ULONG_MAX
        || max_solutions_reached()
        || FlagMaxSolsPerMatingPosReached
        || has_short_solution_been_found_in_problem()
        || hasMaxtimeElapsed()))
  {
    fprintf(LaTeXFile, " \\Co+%%");
    if (!flag_regression)
      fprintf(LaTeXFile, "%s", versionString);
    fprintf(LaTeXFile, "\n");
  }

  fprintf(LaTeXFile, "\\end{diagram}\n\\hfill\n");
}

void LaTeXBeginDiagram(void)
{
  boolean firstpiece= true, fairypieces= false, holess= false,
    modifiedpieces=false;
  PieSpec sp;
  Flags remspec[PieceCount];
  char ListSpec[PieSpCount][256];
  piece p;
  char    HolesSqList[256] = "";
  square const *bnp;

  for (sp= Neutral; sp < PieSpCount; sp++)
    strcpy(ListSpec[sp], PieSpString[UserLanguage][sp]);

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
  if (ActAward[0] != '\0')
  {
    char *tour= strchr(ActAward, ',');
    char *eol= strchr(ActAward, '\n');
    *eol= '\0';
    if (tour)
    {
      fprintf(LaTeXFile,
              " \\award{%.*s}%%\n", (int)(tour-ActAward), ActAward);
      while (*(++tour) == ' ');
      fprintf(LaTeXFile, " \\tournament{%s}%%\n", tour);
    } else
      fprintf(LaTeXFile, " \\award{%s}%%\n", ActAward);
    *eol= '\n';
  }

  /* dedication */
  if (ActTitle[0] != '\0')
  {
    sprintf(GlobalStr, "\\dedication{%s}%%%%\n", ActTitle);
    LaTeXStr(GlobalStr);
  }

  /* pieces & twins */
  if (OptFlag[duplex])
  {
    strcat(ActTwinning, OptTab[duplex]);
    strcat(ActTwinning, "{\\newline}");
  }
  else if (OptFlag[halfduplex])
  {
    strcat(ActTwinning, OptTab[halfduplex]);
    strcat(ActTwinning, "{\\newline}");
  }
  if (OptFlag[quodlibet])
  {
    strcat(ActTwinning, OptTab[quodlibet]);
    strcat(ActTwinning, "{\\newline}");
  }

  /* Just for visualizing imitators on the board. */                 
  if (CondFlag[imitators])
  {
    unsigned int imi_idx;
    for (imi_idx = 0; imi_idx<inum[1]; imi_idx++)
      e[isquare[imi_idx]]= -obs;
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

      if (e[*bnp] == -obs) {
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
  if (OptFlag[whitetoplay])
    fprintf(LaTeXFile,
            " %c{\\ra}", tolower(*PieSpString[UserLanguage][White]));

  fprintf(LaTeXFile, "}%%\n");

  /* conditions */
  if (CondFlag[gridchess] && !OptFlag[suppressgrid]) {
    boolean entry=false;
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
      {
        unsigned int i;
        for (i=1; i<8; i++)
          if (GridNum(square_a1+i-1) != GridNum(square_a1+i))
          {
            if (!entry)
              fprintf(LaTeXFile, " \\gridlines{");
            else
              fprintf(LaTeXFile, ", ");
            entry= true;
            fprintf(LaTeXFile, " v%u08", i);
          }
        for (i=1; i<8; i++)
          if (GridNum(square_a1+24*(i-1)) != GridNum(square_a1+24*i))
          {
            if (!entry)
              fprintf(LaTeXFile, " \\gridlines{");
            else
              fprintf(LaTeXFile, ", ");
            entry= true;
            fprintf(LaTeXFile, " h0%u8", i);
          }
        if (entry)
          fprintf(LaTeXFile, "}%%\n");    
        break;
      }
      
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
  if (CondFlag[magicsquare])
  {
    char    MagicSqList[256] = "";
    boolean first_magic_piece= true;
    square  i;
 
    fprintf(LaTeXFile, " \\fieldframe{");
    for (i= square_a1; i <= square_h8; i++)
      if (TSTFLAG(sq_spec[i], MagicSq))
      {
        if (!first_magic_piece)
          strcat(MagicSqList, ", ");
        else
          first_magic_piece= false;
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
              CondString[UserLanguage][holes], HolesSqList);
    }
    fprintf(LaTeXFile, "}%%\n");
  } /* fairy, modified pieces, holes */
} /* LaTeXBeginDiagram */
/**** LaTeX output ***** end *****/

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

void WriteSquare(square i)
{
  StdChar('a' - nr_files_on_board + i%onerow);
  if (isBoardReflected)
    StdChar('8' + nr_rows_on_board - i/onerow);
  else
    StdChar('1' - nr_rows_on_board + i/onerow);
}


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
