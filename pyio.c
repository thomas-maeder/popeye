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
 ** 2009/06/14 SE   New option: LastCapture
 **
 ** 2012/01/27 NG   AlphabeticChess now possible for white or black only.
 **
 ** 2012/02/04 NG   New condition: Chess 8/1 (invented: Werner Keym, 5/2011)
 **
 **************************** End of List ******************************/

#if defined(macintosh)    /* is always defined on macintosh's  SB */
#   define SEGMIO
#   include "platform/unix/mac.h"
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
#include "pieces/pieces.h"
#include "options/options.h"
#include "conditions/conditions.h"
#include "pymsg.h"
#include "solving/solve.h"
#include "solving/castling.h"
#include "pyproof.h"
#include "stipulation/pipe.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "input/plaintext/token.h"
#include "pieces/walks/hunters.h"
#include "pieces/attributes/neutral/neutral.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/move_inverter.h"
#include "stipulation/if_then_else.h"
#include "stipulation/boolean/or.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/not.h"
#include "stipulation/boolean/true.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/mate/reached_tester.h"
#include "stipulation/goals/stalemate/reached_tester.h"
#include "stipulation/goals/doublestalemate/reached_tester.h"
#include "stipulation/goals/target/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/capture/reached_tester.h"
#include "stipulation/goals/steingewinn/reached_tester.h"
#include "stipulation/goals/enpassant/reached_tester.h"
#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/goals/countermate/reached_tester.h"
#include "stipulation/goals/castling/reached_tester.h"
#include "stipulation/goals/autostalemate/reached_tester.h"
#include "stipulation/goals/circuit/reached_tester.h"
#include "stipulation/goals/exchange/reached_tester.h"
#include "stipulation/goals/circuit_by_rebirth/reached_tester.h"
#include "stipulation/goals/exchange_by_rebirth/reached_tester.h"
#include "stipulation/goals/any/reached_tester.h"
#include "stipulation/goals/proofgame/reached_tester.h"
#include "stipulation/goals/atob/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/chess81/reached_tester.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/battle_play/defense_adapter.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/help_play/adapter.h"
#include "solving/find_shortest.h"
#include "solving/play_suppressor.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/try.h"
#include "solving/move_generator.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/move_effect_journal.h"
#include "conditions/isardam.h"
#include "conditions/synchronous.h"
#include "conditions/protean.h"
#include "conditions/madrasi.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/republican.h"
#include "conditions/bgl.h"
#include "conditions/grid.h"
#include "conditions/check_zigzag.h"
#include "conditions/patience.h"
#include "conditions/sat.h"
#include "conditions/oscillating_kings.h"
#include "conditions/kobul.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/april.h"
#include "conditions/circe/chameleon.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/cheylan.h"
#include "conditions/sentinelles.h"
#include "conditions/magic_square.h"
#include "conditions/mummer.h"
#include "conditions/transmuting_kings/super.h"
#include "conditions/immune.h"
#include "conditions/geneva.h"
#include "conditions/koeko/koeko.h"
#include "conditions/koeko/anti.h"
#include "conditions/phantom.h"
#include "conditions/annan.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/vaulting_kings.h"
#include "conditions/imitator.h"
#include "conditions/messigny.h"
#include "conditions/woozles.h"
#include "conditions/football.h"
#include "conditions/singlebox/type1.h"
#include "options/degenerate_tree.h"
#include "options/nontrivial.h"
#include "options/maxthreatlength.h"
#include "options/maxflightsquares.h"
#include "options/movenumbers.h"
#include "options/maxsolutions/maxsolutions.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "platform/beep.h"
#include "platform/maxtime.h"
#include "platform/maxmem.h"
#include "debugging/trace.h"

/* This is pyio.c
** It comprises a new io-Module for popeye.
** The old io was awkward, nonsystematic and I didn't like it.
** Therefore here a new one.
** Despite the remarks in the README file, I give here three languages
** for the input: english, french, german. It can easily be extended
** if necessary.
*/

static char AlphaStip[200];

#define MAXNEST 10
#define UPCASE(c)   toupper(c)      /* (c+('A'-'a')) */
/* This is only correct, cause only lowercase letters are passed
   as arguments
*/

static char ActAuthor[256];
static char ActOrigin[256];
static char ActTitle[256];
static char ActTwinning[1532];
static char ActAward[256];
static char ActStip[37];

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

static void WritePieces(PieNam const *p, char* CondLine)
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
static char **mummer_strictness_tab;

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
      break;
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

    if ((cond==whitealphabetic || cond==blackalphabetic)
        && CondFlag[alphabetic])
      continue;

    if ((cond==whvault_king || cond==blvault_king)
        && CondFlag[vault_king])
      continue;

    if ((cond==whtrans_king || cond==bltrans_king)
        && CondFlag[trans_king])
      continue;

    /* Write DEFAULT Conditions */
    strcpy(CondLine, CondTab[cond]);

    if ((cond == blmax || cond == whmax) && ExtraCondFlag[maxi])
      strcpy(CondLine, ExtraCondTab[maxi]);

    if ((cond==blackultraschachzwang || cond==whiteultraschachzwang)
        && ExtraCondFlag[ultraschachzwang])
      strcpy(CondLine, ExtraCondTab[ultraschachzwang]);

    if (cond == sentinelles && flagparasent) {
      strcpy(CondLine, "Para");
      strcat(CondLine, CondTab[cond]);
    }

    if (cond == koeko || cond == antikoeko) {
      PieNam koekop = King;
      char LocalBuf[4];
      nocontactfunc_t const nocontactfunc = cond==koeko ? koeko_nocontact : antikoeko_nocontact;
      if (nocontactfunc == noknightcontact)
        koekop= Knight;
      if (nocontactfunc == nowazircontact)
        koekop= Wesir;
      if (nocontactfunc == noferscontact)
        koekop= Fers;
      if (nocontactfunc == nodabbabacontact)
        koekop= Dabbaba;
      if (nocontactfunc == noalfilcontact)
        koekop= Alfil;
      if (nocontactfunc == nocamelcontact)
        koekop= Camel;
      if (nocontactfunc == nozebracontact)
        koekop= Zebra;
      if (nocontactfunc == nogiraffecontact)
        koekop= Giraffe;
      if (nocontactfunc == noantelopecontact)
        koekop= Antilope;

      if (koekop == King)
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
      char buf1[12];
      char buf2[12];
      if (BGL_global)
      {
        WriteBGLNumber(buf1, BGL_values[White]);
        sprintf(CondLine, "%s %s", CondTab[cond], buf1);
      }
      else
      {
        WriteBGLNumber(buf1, BGL_values[White]);
        WriteBGLNumber(buf2, BGL_values[Black]);
        sprintf(CondLine, "%s %s/%s", CondTab[cond],buf1,buf2);
      }
    }

    if ( cond == kobulkings )
    {
      if (!kobulking[White])
      {
        strcat(CondLine, " Black");
      }
      if (!kobulking[Black])
      {
        strcat(CondLine, " White");
      }
    }

    if ( cond == whvault_king || cond == vault_king)
    {
        if (king_vaulters[White][0] != EquiHopper || king_vaulters[White][1] != Empty)
          WritePieces(king_vaulters[White], CondLine);
        if (vaulting_kings_transmuting[White])
        {
          char LocalBuf[4];
          sprintf(LocalBuf, " -%c",
          UPCASE(PieceTab[King][0]));
          strcat(CondLine, LocalBuf);
        }
    }

    if ( cond == blvault_king )
    {
        if (king_vaulters[Black][0] != EquiHopper || king_vaulters[Black][1] != Empty)
          WritePieces(king_vaulters[Black], CondLine);
        if (vaulting_kings_transmuting[Black])
        {
          char LocalBuf[4];
          sprintf(LocalBuf, " -%c",
          UPCASE(PieceTab[King][0]));
          strcat(CondLine, LocalBuf);
        }
    }

    if (cond == promotiononly)
    {
      /* due to a Borland C++ 4.5 bug we have to use LocalBuf ... */
      char LocalBuf[4];
      PieNam pp = Empty;
      while (true)
      {
        pp = pieces_pawns_promotee_chain[pieces_pawns_promotee_chain_orthodox][pp];
        if (pp==Empty)
          break;

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
        /* due to zeroposition, where pieces_pawns_promotee_chain is not */
        /* set (it's set in verifieposition), I suppress  */
        /* output of promotiononly for now.  */
        continue;
      }
    }

    if (cond == football) {
      /* due to a Borland C++ 4.5 bug we have to use LocalBuf ... */
      char LocalBuf[4];
      PieNam pp= Empty;
      while (true)
      {
        pp = next_football_substitute[pp];
        if (pp==Empty)
          break;

        if (PieceTab[pp][1] != ' ')
          sprintf(LocalBuf, " %c%c",
                  UPCASE(PieceTab[pp][0]),
                  UPCASE(PieceTab[pp][1]));
        else
          sprintf(LocalBuf, " %c",
                  UPCASE(PieceTab[pp][0]));
        strcat(CondLine, LocalBuf);
      }
      if (strlen(CondLine) <= strlen(CondTab[football])) {
        /* due to zeroposition, where pieces_pawns_promotee_chain is not */
        /* set (it's set in verifieposition), I suppress  */
        /* output of promotiononly for now.  */
        continue;
      }
    }

    if (cond == april) {
      /* due to a Borland C++ 4.5 bug we have to use LocalBuf...*/
      char LocalBuf[4];
      PieNam pp;
      for (pp = Empty; pp!=PieceCount; ++pp)
        if (is_april_kind[pp]) {
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
      for (imi_idx = 0; imi_idx<number_of_imitators; imi_idx++)
        AddSquare(CondLine,isquare[imi_idx]);
    }

    if (cond == noiprom && !CondFlag[imitators])
      continue;

    if (cond == magicsquare) {
      square  i;
      if (magic_square_type==magic_square_type2)
      {
        strcat(CondLine, " ");
        strcat(CondLine, VariantTypeString[UserLanguage][Type2]);
      }
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], MagicSq)) {
          AddSquare(CondLine, i);
        }
      }
    }
    if (cond == whforsqu || cond == whconforsqu)
    {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], WhForcedSq)) {
          AddSquare(CondLine, i);
        }
      }
    }
    if (cond == blforsqu || cond == blconforsqu) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], BlForcedSq)) {
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
      AddSquare(CondLine, royal_square[Black]);
    }

    if (cond == whroyalsq) {
      AddSquare(CondLine, royal_square[White]);
    }

    if (cond==wormholes)
    {
      square i;
      for (i = square_a1; i<=square_h8; ++i)
        if (TSTFLAG(sq_spec[i],Wormhole))
          AddSquare(CondLine,i);
    }

    if ((cond == madras && madrasi_is_rex_inclusive)
        || (cond == phantom && phantom_chess_rex_inclusive)
        || (cond == geneva && rex_geneva)
        || (immune_is_rex_inclusive
            && (cond == immun
                || cond == immunmalefique
                || cond == immundiagramm))
        || (circe_is_rex_inclusive
            && (cond == circe
                || cond == circemalefique
                || cond == circediametral
                || cond == circemalefiquevertical
                || cond == circeclone
                || cond == circeclonemalefique
                || cond == circediagramm
                || cond == circefile)))
    {
      strcat(CondLine, " ");
      strcat(CondLine, CondTab[rexincl]);
    }

    if ((messigny_rex_exclusive && cond == messigny)
        || (woozles_rex_exclusive
            && (cond==woozles || cond==biwoozles
                || cond==heffalumps || cond==biheffalumps)))
    {
      strcat(CondLine, " ");
      strcat(CondLine, CondTab[rexexcl]);
    }

    if ( protean_is_rex_exclusive && cond == protean)
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
      switch (annan_type)
      {
        case annan_type_A:
          break;
        case annan_type_B:
          strcat(CondLine, VariantTypeString[UserLanguage][TypeB]);
          break;
        case annan_type_C:
          strcat(CondLine, VariantTypeString[UserLanguage][TypeC]);
          break;
        case annan_type_D:
          strcat(CondLine, VariantTypeString[UserLanguage][TypeD]);
          break;
      }
    }

    if (cond == gridchess && OptFlag[suppressgrid]) {
      strcat(CondLine, "  ");
      switch (grid_type)
      {
        case grid_normal:
          /* nothing */
          break;
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

    if (cond==singlebox)
    {
      strcat(CondLine, "    ");
      if (SingleBoxType==singlebox_type1)
        strcat(CondLine, VariantTypeString[UserLanguage][Type1]);
      if (SingleBoxType==singlebox_type2)
        strcat(CondLine, VariantTypeString[UserLanguage][Type2]);
      if (SingleBoxType==singlebox_type3)
        strcat(CondLine, VariantTypeString[UserLanguage][Type3]);
    }

    if (cond == republican)
      republican_write_diagram_caption(CondLine,CondLineLength);

    if (cond == sentinelles) {
      char pawns[7];
      if (sentinelle == BerolinaPawn)
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
      if (sentinelles_max_nr_pawns[Black] !=8 || sentinelles_max_nr_pawns[White] != 8) {
        sprintf(pawns, " %u/%u", sentinelles_max_nr_pawns[White], sentinelles_max_nr_pawns[Black]);
        strcat (CondLine, pawns);
      }
      if (sentinelles_max_nr_pawns_total != 16) {
        sprintf(pawns, " //%u", sentinelles_max_nr_pawns_total);
        strcat (CondLine, pawns);
      }
    }

    if ((cond == SAT || cond == strictSAT) && (SAT_max_nr_allowed_flights[White] != 1 || SAT_max_nr_allowed_flights[Black] != 1)) {
      char extra[10];
      char roman[][9] = {"","I","II","III","IV","V","VI","VII","VIII"};
      if (SAT_max_nr_allowed_flights[White] == SAT_max_nr_allowed_flights[Black])
        sprintf(extra, " %s", roman[SAT_max_nr_allowed_flights[White]-1]);
      else
        sprintf(extra, " %s/%s", roman[SAT_max_nr_allowed_flights[White]-1], roman[SAT_max_nr_allowed_flights[Black]-1]);
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
      if (mummer_strictness[Black]>mummer_strictness_regular)
      {
        strcat(CondLine, "  ");
        if (mummer_strictness[Black]==mummer_strictness_ultra)
          strcat(CondLine, mummer_strictness_tab[mummer_strictness_ultra]);
        else
          strcat(CondLine, mummer_strictness_tab[mummer_strictness_exact]);
      }
      break;
    case whmax:
    case whmin:
    case whcapt:
      if (mummer_strictness[White]>mummer_strictness_regular)
      {
        strcat(CondLine, "  ");
        if (mummer_strictness[White]==mummer_strictness_ultra)
          strcat(CondLine, mummer_strictness_tab[mummer_strictness_ultra]);
        else
          strcat(CondLine, mummer_strictness_tab[mummer_strictness_exact]);
      }
      break;
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
         &ra_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_a1);
      WriteSquare(square_a8);
    }

    if ((castling_mutual_exclusive[White][queenside_castling-min_castling]
         &rh_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_a1);
      WriteSquare(square_h8);
    }

    if ((castling_mutual_exclusive[White][kingside_castling-min_castling]
         &ra_cancastle))
    {
      StdChar(' ');
      WriteSquare(square_h1);
      WriteSquare(square_a8);
    }

    if ((castling_mutual_exclusive[White][kingside_castling-min_castling]
         &rh_cancastle))
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

static char SpaceChar[] = " \t\n\r;.,";
static char LineSpaceChar[] = " \t;.,";
static char TokenChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ#=+-%>!.<()~/&|:[]{}";
/* Steingewinn ! */
/* introductory move */
/* h##! */
/* dia3.5 */
/* a1<-->h1  */
/* reci-h(=)#n */
/* h~2  do ANY helpmove */

static char CharChar[] = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char SepraChar[] = "\n\r;.,";

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

/* advance LastChar to the next1 input character */
static void NextChar(void)
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

/* read into InputLine until the next1 end of line */
static void ReadToEndOfLine(void)
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

static char *ReadNextTokStr(void)
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

static char *ReadNextCaseSensitiveTokStr(void) {
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

static unsigned int GetIndex(unsigned int index, unsigned int limit,
                             char **list, char *tok)
{
  while (index<limit)
    if (sncmp(list[index],tok))
      return index;
    else
      ++index;

  return limit;
}

static unsigned int GetUniqIndex(unsigned int limit, char **list, char *tok)
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

void WriteBGLNumber(char* buf, long int num)
{
  if (num == BGL_infinity)
    sprintf(buf, "-");
  else if (num % 100 == 0)
    sprintf(buf, "%i", (int) (num / 100));
  else if (num % 10 == 0)
    sprintf(buf, "%i.%1i", (int) (num / 100), (int) ((num % 100) / 10));
  else
    sprintf(buf, "%i.%.2i", (int) (num / 100), (int) (num % 100));
}

static char    *mummer_strictness_string[LanguageCount][nr_mummer_strictness] = {
{
  /* French */
    "",
    "",
    "exact",
    "ultra"
  },{
  /* German */
      "",
      "",
      "exakt",
      "Ultra"
  },{
  /* English */
      "",
      "",
      "exact",
      "ultra"
  }
};

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
        mummer_strictness_tab = &(mummer_strictness_string[UserLanguage][0]);
        PieceTab= PieNamString[UserLanguage];
        PieSpTab= PieSpString[UserLanguage];
        ColorTab= ColorString[UserLanguage];
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

/* All following Parse - Procedures return the next1 StringToken or
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

      ReadToEndOfLine();
      tok = InputLine;
      LaTeXPiecesFull[Name]= (char *)malloc(sizeof(char)*(strlen(tok)+1));
      strcpy(LaTeXPiecesFull[Name], tok);

      tok = ReadNextTokStr();
    }
  }

  return tok;
}

static char *LaTeXPiece(PieNam Name)
{
  if (Name > Bishop) {
    if (LaTeXPiecesAbbr[Name] == NULL) {
      ErrorMsg(UndefLatexPiece);
      return "??";
    } else
      return LaTeXPiecesAbbr[Name];
  } else
    return LaTeXStdPie[Name];
} /* LaTeXPiece */

static void signal_overwritten_square(square Square)
{
  WriteSquare(Square);
  StdChar(' ');
  Message(OverwritePiece);
}

static void echo_added_piece(Flags Spec, PieNam Name, square Square)
{
  if (LaTeXout) {
    sprintf(GlobalStr,
            "%s\\%c%s %c%c",
            is_square_empty(Square) ? "+" : "",
            is_piece_neutral(Spec)
            ? 'n'
            : TSTFLAG(Spec, White) ? 'w' : 's',
            LaTeXPiece(Name),
            'a'-nr_of_slack_files_left_of_board+Square%onerow,
            '1'-nr_of_slack_rows_below_board+Square/onerow);
    strcat(ActTwinning, GlobalStr);
  }

  if (is_square_empty(Square))
    StdChar('+');

  WriteSpec(Spec,Name, Name!=Empty);
  WritePiece(Name);
  WriteSquare(Square);
  StdChar(' ');
}

typedef enum
{
  piece_addition_initial,
  piece_addition_twinning
} piece_addition_type;

static char *ParseSquareList(char *tok,
                                PieNam Name,
                                Flags Spec,
                                piece_addition_type type)
{
  /* We interpret the tokenString as SquareList
     If we return always the next1 tokenstring
  */
  unsigned int SquareCnt = 0;

  while (true)
  {
    square const Square = SquareNum(*tok,tok[1]);
    if (tok[0]!=0 && tok[1]!=0 && Square!=initsquare)
    {
      if (!is_square_empty(Square))
      {
        if (type==piece_addition_initial)
          signal_overwritten_square(Square);

        if (Square==king_square[White])
          king_square[White] = initsquare;
        if (Square==king_square[Black])
          king_square[Black] = initsquare;
      }

      if (type==piece_addition_twinning)
        echo_added_piece(Spec,Name,Square);

      occupy_square(Square,Name,Spec);
      tok += 2;
      ++SquareCnt;
    }
    else if (SquareCnt==0)
    {
      ErrorMsg(MissngSquareList);
      tok = ReadNextTokStr();
    }
    else
    {
      if (tok[0]!=0)
        ErrorMsg(WrongSquareList);
      break;
    }
  }

  return ReadNextTokStr();
}

static char *PrsPieShortcut(boolean onechar, char *tok, PieNam *pienam)
{
  if (onechar)
  {
    *pienam= GetPieNamIndex(*tok,' ');
    tok++;
  }
  else
  {
    *pienam= GetPieNamIndex(*tok,tok[1]);
    tok+= 2;
  }

  return tok;
}

static char *ParsePieceName(char *tok, PieNam *name)
{
  size_t len_token;
  char const * const hunterseppos = strchr(tok,hunterseparator);
  if (hunterseppos!=0 && hunterseppos-tok<=2)
  {
    PieNam away, home;
    tok = PrsPieShortcut((hunterseppos-tok)%2==1,tok,&away);
    ++tok; /* skip slash */
    len_token = strlen(tok);
    tok = PrsPieShortcut(len_token%2==1,tok,&home);
    *name = hunter_make_type(away,home);
    if (*name==Invalid)
      IoErrorMsg(HunterTypeLimitReached,maxnrhuntertypes);
  }
  else
  {
    len_token = strlen(tok);
    tok = PrsPieShortcut(len_token%2==1,tok,name);
  }

  return tok;
}

static char *ParsePieceNameAndSquares(char *tok, Flags Spec, piece_addition_type type)
{
  /* We read from tok the name of the piece */
  int     NameCnt= 0;
  char    *btok;

  while (true)
  {
    PieNam Name;
    btok = tok; /* Save it, if we want to return it */
    tok = ParsePieceName(tok,&Name);

    if (Name>=King)
    {
      if (strchr("12345678",tok[1])==0)
        break;
      /* We have read a character (pair) that is a piece name short cut, but tok
       * isn't a piecename squarelist squence.
       * E.g. tok=="Black"; we have read 'B' for Bauer or Bishop, but "lack"
       * isn't a list of squares.
       */
      NameCnt++;
      if (*tok==0)
        tok = ReadNextTokStr();
      tok = ParseSquareList(tok, Name, Spec, type);
      /* undocumented feature: "royal" only applies to the immediately next
       * piece indication because there can be at most 1 royal piece per side
       */
      CLRFLAG(Spec,Royal);
    }
    else if (NameCnt>0)
      return btok;
    else
    {
      IoErrorMsg(WrongPieceName,0);
      tok = ReadNextTokStr();
    }
  }

  return btok;
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

static void SetSquare(square sq, PieNam p, boolean bw, boolean neut)
{
  if (neut)
  {
    occupy_square(sq,p,NeutralMask);
    SETFLAGMASK(some_pieces_flags,NeutralMask);
  }
  else
    occupy_square(sq, p, bw ? BIT(Black) : BIT(White));
}

static char *ParseForsyth(boolean output)
{
  int num;
  square sq = square_a8;
  square const *bnp;
  boolean NeutralFlag= false;
  char *tok = ReadNextCaseSensitiveTokStr();

  for (bnp = boardnum; *bnp; bnp++)
    empty_square(*bnp);

  king_square[White] = initsquare;
  king_square[Black] = initsquare;

  sprintf(GlobalStr, "  %s  \n", tok);
  if (output)
    StdString(tok);

  while (sq && *tok)
  {
    if (isdigit((int)*tok))
    {
      num= (*tok++) - '0';
      if (isdigit((int)*tok))
        num += num*9 + (*tok++) - '0';
      for (;num && sq;num--)
        sq = NextSquare(sq);
      NeutralFlag= false;
    }
    else if (isalpha((int)*tok))
    {
      PieNam const pc= GetPieNamIndex(tolower(*tok),' ');
      if (pc>=King)
      {
        SetSquare(sq,
                  pc,
                  islower((int)InputLine[(tok++)-TokenLine]),
                  NeutralFlag);
        NeutralFlag = false;
        sq = NextSquare(sq);
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
      {
        PieNam const pc= GetPieNamIndex(tolower(*(tok+1)), tolower(*(tok+2)));
        if (pc>=King)
        {
          SetSquare(sq,
                    pc,
                    islower((int)InputLine[(tok+1-TokenLine)]),
                    NeutralFlag);
          NeutralFlag = false;
          sq = NextSquare(sq);
        }
        tok += 3;
      }
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

static Flags ParseColor(char *tok, boolean color_is_mandatory)
{
  Colors const color = GetUniqIndex(nr_colors,ColorTab,tok);
  if (color==nr_colors)
  {
    if (color_is_mandatory)
      IoErrorMsg(NoColorSpec,0);
    return 0;
  }
  else if (color>nr_colors)
  {
    IoErrorMsg(PieSpecNotUniq,0);
    return 0;
  }
  else if (color==color_neutral)
    return NeutralMask;
  else
    return BIT(color);
}

static char *ParsePieceFlags(Flags *flags)
{
  char *tok;

  while (true)
  {
    tok = ReadNextTokStr();

    {
      PieSpec const ps = GetUniqIndex(PieSpCount-nr_sides,PieSpTab,tok);
      if (ps==PieSpCount-nr_sides)
        break;
      else if (ps>PieSpCount-nr_sides)
        IoErrorMsg(PieSpecNotUniq,0);
      else
        SETFLAG(*flags,ps+nr_sides);
    }
  }

  return tok;
}

static char *ParseSquareLastCapture(char *tok, PieNam Name, Flags Spec)
{
  square const Square = SquareNum(*tok,tok[1]);
  if (Square==initsquare || tok[2]!=0)
  {
    ErrorMsg(WrongSquareList);
    return tok;
  }
  else
  {
    move_effect_journal_store_retro_capture(Square,Name,Spec);
    return ReadNextTokStr();
  }
}

static char *ParsePieceNameAndSquareLastCapture(char *tok, Flags Spec)
{
  PieNam Name;

  tok = ParsePieceName(tok,&Name);

  if (Name>=King)
  {
    if (tok[0]==0)
      tok = ReadNextTokStr();
    tok = ParseSquareLastCapture(tok,Name,Spec);
  }
  else
  {
    IoErrorMsg(WrongPieceName,0);
    tok = ReadNextTokStr();
  }

  return tok;
}

static char *ParseLastCapturedPiece(void)
{
  int nr_groups = 0;
  char *tok = ReadNextTokStr();
  while (true)
  {
    Flags PieSpFlags = ParseColor(tok,nr_groups==0);
    if (PieSpFlags==0)
      break;
    else
    {
      ++nr_groups;

      if (is_piece_neutral(PieSpFlags))
        SETFLAGMASK(some_pieces_flags,NeutralMask);

      {
        Flags nonColorFlags = 0;
        tok = ParsePieceFlags(&nonColorFlags);
        PieSpFlags |= nonColorFlags;
        some_pieces_flags |= nonColorFlags;
      }

      tok = ParsePieceNameAndSquareLastCapture(tok,PieSpFlags);
    }
  }

  return tok;
}

static char *ParsePieces(piece_addition_type type)
{
  int nr_groups = 0;
  char *tok = ReadNextTokStr();
  while (true)
  {
    Flags PieSpFlags = ParseColor(tok,nr_groups==0);
    if (PieSpFlags==0)
      break;
    else
    {
      ++nr_groups;

      if (is_piece_neutral(PieSpFlags))
        SETFLAGMASK(some_pieces_flags,NeutralMask);

      {
        Flags nonColorFlags = 0;
        tok = ParsePieceFlags(&nonColorFlags);
        PieSpFlags |= nonColorFlags;
        some_pieces_flags |= nonColorFlags;
      }

      tok = ParsePieceNameAndSquares(tok,PieSpFlags,type);
    }
  }

  return tok;
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
  {   "##!",  goal_countermate         }
  , { "##",   goal_doublemate          }
  , { "#=",   goal_mate_or_stale       }
  , { "#",    goal_mate                }
  , { "==",   goal_dblstale            }
  , { "!=",   goal_autostale           }
  , { "=",    goal_stale               }
  , { "z",    goal_target              }
  , { "+",    goal_check               }
  , { "x",    goal_capture             }
  , { "%",    goal_steingewinn         }
  , { "ep",   goal_ep                  }
  , { "ctr",  goal_circuit_by_rebirth  }
  , { "ct",   goal_circuit             }
  , { "<>r",  goal_exchange_by_rebirth }
  , { "<>",   goal_exchange            }
  , { "00",   goal_castling            }
  , { "~",    goal_any                 }
  , { "dia",  goal_proofgame           }
  , { "a=>b", goal_atob                }
  , { "c81",  goal_chess81             }
};

typedef enum
{
  play_length_minimum,
  play_length_exact
} play_length_type;

static char *ParseLength(char *tok, stip_length_type *length)
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
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseBattleLength(char *tok, stip_length_type *length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  tok = ParseLength(tok,length);
  if (tok!=0)
  {
    if (*length==0)
    {
      IoErrorMsg(WrongInt,0);
      tok = 0;
    }
    else
    {
      /* we count half moves in battle play */
      *length *= 2;
      *length += slack_length-1;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseHelpLength(char *tok,
                             stip_length_type *length,
                             stip_length_type *min_length,
                             play_length_type play_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  tok = ParseLength(tok,length);
  if (tok!=0)
  {
    /* we count half moves in help play */
    *length *= 2;
    *length += slack_length;

    if (strncmp(tok,".5",2)==0)
    {
      ++*length;
      tok += 2;
      *min_length = slack_length+1;
    }
    else
      *min_length = slack_length;

    if (*length<slack_length)
    {
      IoErrorMsg(WrongInt,0);
      tok = 0;
    }
    else if (play_length==play_length_exact)
      *min_length = *length;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseSeriesLength(char *tok,
                               stip_length_type *length,
                               stip_length_type *min_length,
                               play_length_type play_length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  tok = ParseLength(tok,length);
  if (tok!=0)
  {
    if (*length==0)
    {
      IoErrorMsg(WrongInt,0);
      tok = 0;
    }
    else
    {
      /* we count half moves in series play */
      *length *= 2;
      *length += slack_length-1;
      if (play_length==play_length_minimum)
        *min_length = slack_length+1;
      else
        *min_length = *length;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static goalInputConfig_t const *detectGoalType(char *tok)
{
  goalInputConfig_t const *gic;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  for (gic = goalInputConfig; gic!=goalInputConfig+nr_goals; ++gic)
    if (gic->inputText!=0 && strstr(tok,gic->inputText)==tok)
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",(gic-goalInputConfig)<nr_goals);
  TraceFunctionResultEnd();
  return gic;
}

static char *ParseGoal(char *tok, slice_index proxy)
{
  goalInputConfig_t const *gic;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  gic = detectGoalType(tok);
  if (gic-goalInputConfig==nr_goals)
  {
    IoErrorMsg(UnrecStip, 0);
    tok = 0;
  }
  else
  {
    Goal goal = { gic->goal, initsquare };
    tok += strlen(gic->inputText);
    TraceValue("%s",gic->inputText);
    TraceValue("%s\n",tok);

    switch (gic->goal)
    {
      case goal_target:
      {
        goal.target = SquareNum(tok[0],tok[1]);

        if (goal.target==initsquare)
        {
          IoErrorMsg(MissngSquareList, 0);
          tok = 0;
        }
        else
        {
          pipe_link(proxy,alloc_goal_target_reached_tester_system(goal.target));
          tok += 2; /* skip over target square indication */
        }
        break;
      }

      case goal_mate_or_stale:
      {
        Goal const goal = { goal_mate_or_stale, initsquare };
        slice_index const immobile_tester = alloc_goal_immobile_reached_tester_system();
        pipe_link(proxy,alloc_goal_reached_tester_slice(goal,immobile_tester));
        break;
      }

      case goal_mate:
      {
        Goal const goal = { goal_mate, initsquare };
        slice_index const mate_tester = alloc_goal_mate_reached_tester_system();
        pipe_link(proxy,alloc_goal_reached_tester_slice(goal,mate_tester));
        break;
      }

      case goal_stale:
        pipe_link(proxy,alloc_goal_stalemate_reached_tester_system());
        break;

      case goal_dblstale:
        pipe_link(proxy,alloc_goal_doublestalemate_reached_tester_system());
        break;

      case goal_check:
        pipe_link(proxy,alloc_goal_check_reached_tester_system());
        break;

      case goal_capture:
        pipe_link(proxy,alloc_goal_capture_reached_tester_system());
        break;

      case goal_steingewinn:
        pipe_link(proxy,alloc_goal_steingewinn_reached_tester_system());
        break;

      case goal_ep:
        pipe_link(proxy,alloc_goal_enpassant_reached_tester_system());
        break;

      case goal_doublemate:
        pipe_link(proxy,alloc_doublemate_mate_reached_tester_system());
        break;

      case goal_countermate:
        pipe_link(proxy,alloc_goal_countermate_reached_tester_system());
        break;

      case goal_castling:
        pipe_link(proxy,alloc_goal_castling_reached_tester_system());
        break;

      case goal_autostale:
        pipe_link(proxy,alloc_goal_autostalemate_reached_tester_system());
        break;

      case goal_circuit:
        pipe_link(proxy,alloc_goal_circuit_reached_tester_system());
        SETFLAGMASK(some_pieces_flags,PieceIdMask);
        break;

      case goal_exchange:
        pipe_link(proxy,alloc_goal_exchange_reached_tester_system());
        SETFLAGMASK(some_pieces_flags,PieceIdMask);
        break;

      case goal_circuit_by_rebirth:
        pipe_link(proxy,alloc_goal_circuit_by_rebirth_reached_tester_system());
        SETFLAGMASK(some_pieces_flags,PieceIdMask);
        break;

      case goal_exchange_by_rebirth:
        pipe_link(proxy,alloc_goal_exchange_by_rebirth_reached_tester_system());
        SETFLAGMASK(some_pieces_flags,PieceIdMask);
        break;

      case goal_any:
        pipe_link(proxy,alloc_goal_any_reached_tester_system());
        break;

      case goal_proofgame:
        pipe_link(proxy,alloc_goal_proofgame_reached_tester_system());
        break;

      case goal_atob:
      {
        pipe_link(proxy,alloc_goal_atob_reached_tester_system());

        ProofSaveStartPosition();

        {
          /* used to call InitBoard(), which does much more than the following: */
          int i;
          for (i = 0; i<nr_squares_on_board; i++)
            empty_square(boardnum[i]);
          for (i = 0; i<maxinum; i++)
            isquare[i] = initsquare;
        }

        break;
      }

      case goal_chess81:
        pipe_link(proxy,alloc_goal_chess81_reached_tester_system());
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

static void alloc_reci_end(slice_index proxy_nonreci,
                           slice_index proxy_reci,
                           slice_index proxy_to_nonreci,
                           slice_index proxy_to_reci)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_nonreci);
  TraceFunctionParam("%u",proxy_reci);
  TraceFunctionParam("%u",proxy_to_nonreci);
  TraceFunctionParam("%u",proxy_to_reci);
  TraceFunctionParamListEnd();

  {
    slice_index const branch_nonreci = alloc_help_branch(slack_length+2,
                                                         slack_length+2);
    slice_index const branch_reci = alloc_help_branch(slack_length+1,
                                                      slack_length+1);

    help_branch_set_end_goal(branch_nonreci,proxy_to_nonreci,1);
    link_to_branch(proxy_nonreci,branch_nonreci);

    help_branch_set_end_goal(branch_reci,proxy_to_reci,1);
    link_to_branch(proxy_reci,branch_reci);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static char *ParseReciGoal(char *tok,
                           slice_index proxy_nonreci,
                           slice_index proxy_reci)
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
            slice_index const nonreci = slices[proxy_to_nonreci].next1;
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
      slice_index const nonreci_testing = slices[proxy_to_nonreci].next1;
      slice_index const nonreci_tester = slices[nonreci_testing].next1;
      slice_index const proxy_to_reci = stip_deep_copy(proxy_to_nonreci);
      alloc_reci_end(proxy_nonreci,proxy_reci,
                     proxy_to_nonreci,proxy_to_reci);
      slices[nonreci_tester].starter = Black;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseReciEnd(char *tok, slice_index proxy)
{
  slice_index op1;
  slice_index op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  op1 = alloc_proxy_slice();
  op2 = alloc_proxy_slice();

  tok = ParseReciGoal(tok,op1,op2);
  if (slices[op1].next1!=no_slice && slices[op2].next1!=no_slice)
  {
    slice_index const reci = alloc_and_slice(op1,op2);
    pipe_link(proxy,reci);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static void attach_help_branch(stip_length_type length,
                               slice_index proxy,
                               slice_index branch)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  if ((length-slack_length)%2==1)
  {
    slice_index const inverter = alloc_move_inverter_slice();
    pipe_link(proxy,inverter);
    link_to_branch(inverter,branch);
  }
  else
    link_to_branch(proxy,branch);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void select_output_mode(slice_index si, output_mode mode)
{
  slice_index const prototype = alloc_output_mode_selector(mode);
  branch_insert_slices(si,&prototype,1);
}

static char *ParseBattle(char *tok,
                         slice_index proxy,
                         slice_index proxy_goal,
                         play_length_type play_length,
                         boolean ends_on_defense)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_goal);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParam("%u",ends_on_defense);
  TraceFunctionParamListEnd();

  result = ParseGoal(tok,proxy_goal);
  if (result!=0)
  {
    stip_length_type length = 0;
    result = ParseBattleLength(result,&length);
    if (ends_on_defense)
      ++length;
    if (result!=0)
    {
      stip_length_type const min_length = (play_length==play_length_minimum
                                           ? slack_length+1
                                           : length-1);
      link_to_branch(proxy,alloc_battle_branch(length,min_length));
      stip_impose_starter(proxy,White);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseHelp(char *tok,
                       slice_index proxy,
                       slice_index proxy_goal,
                       play_length_type play_length,
                       boolean shorten)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_goal);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParam("%u",shorten);
  TraceFunctionParamListEnd();

  result = ParseGoal(tok,proxy_goal);
  if (result!=0)
  {
    stip_length_type length;
    stip_length_type min_length;
    result = ParseHelpLength(result,&length,&min_length,play_length);
    if (result!=0)
    {
      slice_index const branch = alloc_help_branch(length,min_length);
      if (shorten)
        help_branch_shorten(branch);
      attach_help_branch(length,proxy,branch);
      select_output_mode(proxy,output_mode_line);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseHelpDia(char *tok,
                          slice_index proxy,
                          play_length_type play_length)
{
  char *result;
  slice_index proxy_to_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParamListEnd();

  proxy_to_goal = alloc_proxy_slice();
  result = ParseGoal(tok,proxy_to_goal);
  if (result!=0)
  {
    stip_length_type length;
    stip_length_type min_length;
    result = ParseHelpLength(result,&length,&min_length,play_length);
    if (result!=0)
    {
      link_to_branch(proxy,alloc_help_branch(length,min_length));
      select_output_mode(proxy,output_mode_line);
      help_branch_set_end_goal(proxy,proxy_to_goal,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseSeries(char *tok,
                         slice_index proxy,
                         slice_index proxy_goal,
                         play_length_type play_length)
{
  char *result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",proxy_goal);
  TraceFunctionParam("%u",play_length);
  TraceFunctionParamListEnd();

  result = ParseGoal(tok,proxy_goal);
  if (result!=0)
  {
    stip_length_type length;
    stip_length_type min_length;
    result = ParseSeriesLength(result,&length,&min_length,play_length);
    if (result!=0)
    {
      slice_index const branch = alloc_series_branch(length,min_length);
      link_to_branch(proxy,branch);
      select_output_mode(proxy,output_mode_line);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

/* Produce the sequence of slices that tests whether a self stipulation has been
 * solved
 * @param proxy_to_goal identifies sequence of slices testing for the goal
 * @return identifier of the entry slice
 */
slice_index MakeEndOfSelfPlay(slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_defense_branch(slack_length+1,slack_length+1);
  battle_branch_insert_self_end_of_branch_goal(result,proxy_to_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_index MakeSemireflexBranch(slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  {
    slice_index const branch = alloc_battle_branch(slack_length+1,
                                                   slack_length);
    result = alloc_proxy_slice();
    link_to_branch(result,branch);
    battle_branch_insert_direct_end_of_branch_goal(branch,proxy_to_goal);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_index MakeReflexBranch(slice_index proxy_to_semi)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",proxy_to_semi);
  TraceFunctionParamListEnd();

  result = stip_deep_copy(proxy_to_semi);
  pipe_append(result,alloc_not_slice());

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParsePlay(char *tok,
                       slice_index root_slice_hook,
                       slice_index proxy,
                       play_length_type play_length)
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
      result = ParsePlay(arrowpos+2,root_slice_hook,proxy_next,play_length);
      if (result!=0 && slices[proxy_next].next1!=no_slice)
      {
        /* >=1 move of starting side required */
        stip_length_type const min_length = 1+slack_length;
        slice_index const branch = alloc_series_branch(2*intro_len
                                                       +slack_length-1,
                                                       min_length);
        help_branch_set_end(branch,proxy_next,1);
        link_to_branch(proxy,branch);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (strncmp("exact-", tok, 6) == 0)
    result = ParsePlay(tok+6,root_slice_hook,proxy,play_length_exact);

  else if (strncmp("ser-reci-h",tok,10) == 0)
  {
    slice_index const proxy_next = alloc_proxy_slice();

    /* skip over "ser-reci-h" */
    tok = ParseReciEnd(tok+10,proxy_next);
    if (tok!=0 && slices[proxy_next].next1!=no_slice)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseSeriesLength(tok,&length,&min_length,play_length);
      if (result!=0)
      {
        slice_index const branch = alloc_series_branch(length-1,min_length+1);
        help_branch_set_end(branch,proxy_next,1);
        link_to_branch(proxy,branch);

        stip_impose_starter(proxy_next,Black);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (strncmp("ser-hs",tok,6)==0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    tok = ParseGoal(tok+6,proxy_to_goal); /* skip over "ser-hs" */
    if (tok!=0)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseSeriesLength(tok,&length,&min_length,play_length);
      if (result!=0)
      {
        slice_index const defense_branch = MakeEndOfSelfPlay(proxy_to_goal);

        /* in ser-hs, the series is 1 half-move longer than in usual
         * series play! */
        if (length==slack_length)
          pipe_link(proxy,defense_branch);
        else
        {
          slice_index const series = alloc_series_branch(length,min_length);

          slice_index const help_proxy = alloc_proxy_slice();
          slice_index const help = alloc_help_branch(slack_length+1,
                                                     slack_length+1);
          link_to_branch(help_proxy,help);
          help_branch_set_end_forced(help_proxy,defense_branch,1);
          help_branch_set_end(series,help_proxy,1);
          link_to_branch(proxy,series);
        }

        stip_impose_starter(proxy_to_goal,White);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (strncmp("ser-h",tok,5) == 0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseSeries(tok+5,proxy,proxy_to_goal,play_length); /* skip over "ser-h" */
    if (result!=0)
    {
      slice_index const help = alloc_help_branch(slack_length+1,
                                                 slack_length+1);
      help_branch_set_end_goal(help,proxy_to_goal,1);
      help_branch_set_end(proxy,help,1);

      {
        slice_index const next = slices[proxy_to_goal].next1;
        assert(next!=no_slice);
        if (slices[next].type==STGoalReachedTester
            && slices[next].u.goal_handler.goal.type==goal_proofgame)
          stip_impose_starter(proxy_to_goal,White);
        else
          stip_impose_starter(proxy_to_goal,Black);
      }
    }
  }

  else if (strncmp("ser-s",tok,5) == 0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseSeries(tok+5,proxy,proxy_to_goal,play_length); /* skip over "ser-s" */
    if (result!=0)
    {
      help_branch_set_end_forced(proxy,MakeEndOfSelfPlay(proxy_to_goal),1);
      stip_impose_starter(proxy_to_goal,White);
    }
  }

  else if (strncmp("ser-r",tok,5) == 0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseSeries(tok+5,proxy,proxy_to_goal,play_length); /* skip over "ser-r" */
    if (result!=0)
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_to_goal);
      help_branch_set_end_forced(proxy,proxy_semi,1);
      series_branch_insert_constraint(proxy,MakeReflexBranch(proxy_semi));
      stip_impose_starter(proxy_to_goal,White);
    }
  }

  else if (strncmp("ser-",tok,4) == 0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseSeries(tok+4,proxy,proxy_to_goal,play_length); /* skip over "ser-" */
    if (result!=0)
    {
      help_branch_set_end_goal(proxy,proxy_to_goal,1);
      stip_impose_starter(proxy_to_goal,Black);
    }
  }

  else if (strncmp("phser-r",tok,7) == 0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+7, /* skip over phser-r */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result!=0)
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_to_goal);
      help_branch_set_end_forced(proxy,proxy_semi,1);
      if (help_branch_insert_constraint(proxy,MakeReflexBranch(proxy_semi),0))
      {
        help_branch_insert_check_zigzag(proxy);
        stip_impose_starter(proxy_to_goal,White);
      }
      else
        result = 0;
    }
  }

  else if (strncmp("phser-s",tok,7) == 0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+7, /* skip over phser-s */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result!=0)
    {
      help_branch_set_end_forced(proxy,MakeEndOfSelfPlay(proxy_to_goal),1);
      help_branch_insert_check_zigzag(proxy);
      stip_impose_starter(proxy_to_goal,White);
    }
  }

  else if (strncmp("phser-",tok,6) == 0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+6, /* skip over phser- */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result!=0)
    {
      help_branch_set_end_goal(proxy,proxy_to_goal,1);
      help_branch_insert_check_zigzag(proxy);
      stip_impose_starter(proxy_to_goal,Black);
    }
  }

  else if (strncmp("pser-hs",tok,7) == 0)
  {
    slice_index const proxy_to_goal = alloc_proxy_slice();
    tok = ParseGoal(tok+7,proxy_to_goal); /* skip over "ser-hs" */
    if (tok!=0)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseSeriesLength(tok,&length,&min_length,play_length);
      if (result!=0)
      {
        slice_index const series = alloc_help_branch(length,min_length);
        slice_index const help_proxy = alloc_proxy_slice();
        slice_index const help = alloc_help_branch(slack_length+1,
                                                   slack_length+1);
        slice_index const defense_branch = MakeEndOfSelfPlay(proxy_to_goal);
        link_to_branch(help_proxy,help);
        help_branch_set_end_forced(help_proxy,defense_branch,1);
        help_branch_set_end(series,help_proxy,1);
        link_to_branch(proxy,series);
        help_branch_insert_check_zigzag(proxy);
        stip_impose_starter(proxy_to_goal,White);
        select_output_mode(proxy,output_mode_line);
      }
    }
  }

  else if (strncmp("pser-h",tok,6) == 0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+6, /* skip over pser-h */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result!=0)
    {
      slice_index const to_goal = slices[proxy_to_goal].next1;
      slice_index const nested = alloc_help_branch(slack_length+1,
                                                   slack_length+1);
      help_branch_set_end_goal(nested,proxy_to_goal,1);
      help_branch_set_end(proxy,nested,1);
      help_branch_insert_check_zigzag(proxy);
      if (slices[to_goal].type==STGoalReachedTester
          && slices[to_goal].u.goal_handler.goal.type==goal_proofgame)
        stip_impose_starter(proxy_to_goal,White);
      else
        stip_impose_starter(proxy_to_goal,Black);
    }
  }

  else if (strncmp("pser-r",tok,6) == 0)
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+6, /* skip over pser-r */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_to_goal);
      battle_branch_insert_end_of_branch_forced(proxy,proxy_semi);
      battle_branch_insert_attack_constraint(proxy,MakeReflexBranch(proxy_semi));
      battle_branch_insert_defense_check_zigzag(proxy);
      select_output_mode(proxy,output_mode_line);
      stip_impose_starter(proxy_to_goal,White);
    }
  }

  else if (strncmp("pser-s",tok,6) == 0)
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+6, /* skip over pser-s */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      battle_branch_insert_direct_end_of_branch(proxy,
                                                MakeEndOfSelfPlay(proxy_to_goal));
      stip_impose_starter(proxy_to_goal,Black);
      select_output_mode(proxy,output_mode_line);
      battle_branch_insert_defense_check_zigzag(proxy);
    }
  }

  else if (strncmp("pser-",tok,5) == 0)
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+5, /* skip over pser- */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      select_output_mode(proxy,output_mode_line);
      battle_branch_insert_direct_end_of_branch_goal(proxy,proxy_to_goal);
      battle_branch_insert_defense_check_zigzag(proxy);
    }
  }

  else if (strncmp("reci-h",tok,6) == 0)
  {
    slice_index const proxy_next = alloc_proxy_slice();
    char * const tok2 = ParseReciEnd(tok+6, /* skip over "reci-h" */
                                     proxy_next);
    if (tok2!=0 && slices[proxy_next].next1!=no_slice)
    {
      stip_length_type length;
      stip_length_type min_length;
      result = ParseHelpLength(tok2,&length,&min_length,play_length);

      if (result!=0)
      {
        if (length==slack_length+1)
        {
          /* at least 2 half moves requried for a reciprocal stipulation */
          IoErrorMsg(StipNotSupported,0);
          result = 0;
        }
        else
        {
          if (length==slack_length+2)
          {
            pipe_link(proxy,slices[proxy_next].next1);
            dealloc_slice(proxy_next);
          }
          else
          {
            stip_length_type const min_length2 = (min_length<slack_length+2
                                                  ? min_length
                                                  : min_length-2);
            slice_index const branch = alloc_help_branch(length-2,min_length2);
            help_branch_set_end(branch,proxy_next,1);
            attach_help_branch(length,proxy,branch);
          }

          stip_impose_starter(proxy_next,Black);
          select_output_mode(proxy,output_mode_line);
        }
      }
    }
  }

  else if (strncmp("dia",tok,3)==0)
  {
    result = ParseHelpDia(tok,proxy,play_length);
    if (result!=0)
      stip_impose_starter(proxy,White);
  }
  else if (strncmp("a=>b",tok,4)==0)
  {
    result = ParseHelpDia(tok,proxy,play_length);
    if (result!=0)
      stip_impose_starter(proxy,Black);
  }

  else if (strncmp("hs",tok,2)==0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+2, /* skip over "hs" */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result!=0)
    {
      help_branch_set_end_forced(proxy,MakeEndOfSelfPlay(proxy_to_goal),1);
      stip_impose_starter(proxy_to_goal,White);
    }
  }

  else if (strncmp("hr",tok,2)==0)
  {
    boolean const shorten = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+2, /* skip over "hr" */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result!=0)
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_to_goal);
      help_branch_set_end_forced(proxy,proxy_semi,1);
      if (help_branch_insert_constraint(proxy,MakeReflexBranch(proxy_semi),0))
        stip_impose_starter(proxy_to_goal,White);
      else
        result = 0;
    }
  }

  else if (*tok=='h')
  {
    boolean const shorten = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseHelp(tok+1, /* skip over "h" */
                       proxy,proxy_to_goal,
                       play_length,shorten);
    if (result!=0)
    {
      help_branch_set_end_goal(proxy,proxy_to_goal,1);
      stip_impose_starter(proxy_to_goal,Black);
    }
  }

  else if (strncmp("semi-r",tok,6)==0)
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+6, /* skip over "semi-r" */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      battle_branch_insert_end_of_branch_forced(proxy,
                                                MakeSemireflexBranch(proxy_to_goal));
      select_output_mode(proxy,output_mode_tree);
      stip_impose_starter(proxy_to_goal,White);
    }
  }

  else if (*tok=='s')
  {
    boolean const ends_on_defense = true;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+1, /* skip over 's' */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      select_output_mode(proxy,output_mode_tree);
      battle_branch_insert_self_end_of_branch_goal(proxy,proxy_to_goal);
    }
  }

  else if (*tok=='r')
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok+1, /* skip over 'r' */
                         proxy,proxy_to_goal,
                         play_length,ends_on_defense);
    if (result!=0)
    {
      slice_index const proxy_semi = MakeSemireflexBranch(proxy_to_goal);
      battle_branch_insert_end_of_branch_forced(proxy,proxy_semi);
      battle_branch_insert_attack_constraint(proxy,MakeReflexBranch(proxy_semi));
      select_output_mode(proxy,output_mode_tree);
      stip_impose_starter(proxy_to_goal,White);
    }
  }

  else
  {
    boolean const ends_on_defense = false;
    slice_index const proxy_to_goal = alloc_proxy_slice();
    result = ParseBattle(tok,proxy,proxy_to_goal,play_length,ends_on_defense);
    if (result!=0)
    {
      select_output_mode(proxy,output_mode_tree);
      battle_branch_insert_direct_end_of_branch_goal(proxy,proxy_to_goal);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",result);
  TraceFunctionResultEnd();
  return result;
}

static char *ParseStip(slice_index root_slice_hook)
{
  char *tok = ReadNextTokStr();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  CLRFLAGMASK(some_pieces_flags,PieceIdMask);

  strcpy(AlphaStip,tok);
  if (ParsePlay(tok,root_slice_hook,root_slice_hook,play_length_minimum))
  {
    if (slices[root_slice_hook].next1!=no_slice
        && ActStip[0]=='\0')
      strcpy(ActStip, AlphaStip);
  }

  tok = ReadNextTokStr();

  /* signal to our caller that the stipulation has changed */
  slices[root_slice_hook].starter = no_side;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Advance to next1 token while parsing a structured stipulation
 * @param tok current position in current token
 * @return tok, if we are within the current token; next1 token otherwise
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
  Side ps;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  /* We don't make any unsafe assumptions here; PieSpec enumerators
   * are initialised in terms of nr_sides */
  ps = GetUniqIndex(nr_sides,ColorTab,tok);
  if (ps>nr_sides)
    IoErrorMsg(PieSpecNotUniq,0);
  else if (ps<nr_sides)
    result = ps;

  TraceFunctionExit(__func__);
  TraceEnumerator(Side,result,"");
  TraceFunctionResultEnd();
  return result;
}

typedef enum
{
  expression_type_goal,
  expression_type_attack, /* this includes help and series */
  expression_type_defense
} expression_type;

/* Parse a stipulation expression
 * @param tok input token
 * @param proxy index of expression slice; no_slice if expression
 *              can't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_expression(char *tok,
                                            slice_index proxy,
                                            expression_type *type,
                                            unsigned int level);

/* extend expression_type with nested_branch_type_forced */
typedef enum
{
  nested_branch_type_goal,
  nested_branch_type_attack,
  nested_branch_type_defense,
  nested_branch_type_forced
} nested_branch_type;

/* Parse a nested branch
 * @param tok input token
 * @param proxy_operand identifier of proxy slice where to attach the branch
 * @param type address of object where to return the operand type of the branch
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_nested_branch(char *tok,
                                               slice_index proxy_operand,
                                               nested_branch_type *type,
                                               unsigned int level)
{
  expression_type * etype = (expression_type *)type;
  boolean is_forced = false;
  boolean is_suppressed = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy_operand);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  if (tok[0]=='>')
  {
    is_forced = true;
    ++tok;
  }

  if (tok[0]=='/')
  {
    is_suppressed = true;
    ++tok;
  }

  tok = ParseStructuredStip_expression(tok,proxy_operand,etype,level+1);

  if (tok!=0)
  {
    if (is_suppressed)
    {
      slice_index const prototype = alloc_play_suppressor_slice();
      branch_insert_slices(proxy_operand,&prototype,1);
      if (branch_find_slice(STPlaySuppressor,
                            proxy_operand,
                            stip_traversal_context_intro)
          ==no_slice)
        pipe_append(proxy_operand,alloc_play_suppressor_slice());
    }

    if (is_forced)
    {
      if (*etype==expression_type_attack)
      *type = nested_branch_type_forced;
      else
        tok = 0;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse an "a operand"
 * @param tok input token
 * @param branch identifier of entry slice of "ad branch"
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_a_operand(char *tok,
                                                  slice_index branch,
                                                  unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",branch);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  while (tok!=0)
  {
    tok = ParseStructuredStip_skip_whitespace(tok);
    if (tok[0]=='[')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      nested_branch_type nested_type;

      tok = ParseStructuredStip_nested_branch(tok+1,proxy_operand,&nested_type,level);
      if (tok!=0 && tok[0]==']')
      {
        ++tok;
        switch (nested_type)
        {
          case nested_branch_type_goal:
            battle_branch_insert_direct_end_of_branch_goal(branch,proxy_operand);
            break;

          case nested_branch_type_attack:
          case nested_branch_type_forced:
            battle_branch_insert_end_of_branch_forced(branch,proxy_operand);
            break;

          case nested_branch_type_defense:
            battle_branch_insert_direct_end_of_branch(branch,proxy_operand);
            break;

          default:
            assert(0);
            break;
        }
      }
    }
    else if (tok[0]=='{')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      expression_type nested_type;
      tok = ParseStructuredStip_expression(tok+1,proxy_operand,&nested_type,level+1);
      if (tok!=0 && tok[0]=='}')
      {
        ++tok;
        battle_branch_insert_defense_constraint(branch,proxy_operand);
      }
      else
        tok = 0;
    }
    else
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse an "a operand"
 * @param tok input token
 * @param identifier of entry slice of "ad branch"
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_d_operand(char *tok,
                                                  slice_index branch,
                                                  unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",branch);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  while (tok!=0)
  {
    tok = ParseStructuredStip_skip_whitespace(tok);
    if (tok[0]=='[')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      nested_branch_type nested_type;
      tok = ParseStructuredStip_nested_branch(tok+1,proxy_operand,&nested_type,level);
      if (tok!=0 && tok[0]==']')
      {
        ++tok;
        switch (nested_type)
        {
          case nested_branch_type_goal:
            battle_branch_insert_self_end_of_branch_goal(branch,proxy_operand);
            break;

          case nested_branch_type_attack:
          case nested_branch_type_defense:
            battle_branch_insert_self_end_of_branch(branch,proxy_operand);
            break;

          case nested_branch_type_forced:
            tok = 0;
            break;

          default:
            assert(0);
            break;
        }
      }
    }
    else if (tok[0]=='{')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      expression_type nested_type;
      tok = ParseStructuredStip_expression(tok+1,proxy_operand,&nested_type,level+1);
      if (tok!=0 && tok[0]=='}')
      {
        ++tok;
        if (nested_type==expression_type_goal)
          battle_branch_insert_attack_goal_constraint(branch,proxy_operand);
        else
          battle_branch_insert_attack_constraint(branch,proxy_operand);
      }
      else
        tok = 0;
    }
    else
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Make a "da branch"
 * @param min_length minimum length indicated by the user (0 if (s)he didn't)
 * @param max_length maximum length indicated by the user
 * @param level nesting level of the operand (0 means top level)
 * @return identifier of branch entry slice
 */
static slice_index ParseStructuredStip_make_branch_d(stip_length_type min_length,
                                                     stip_length_type max_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParamListEnd();

  max_length += slack_length+1;
  min_length += slack_length+1;

  if (min_length>=max_length)
    min_length = max_length-1;

  result = alloc_battle_branch(max_length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse a "da branch"
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves
 * @param proxy index of branch; no_slice if branch couldn't be
 *              parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_d(char *tok,
                                          stip_length_type min_length,
                                          stip_length_type max_length,
                                          slice_index proxy,
                                          unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  {
    slice_index const branch = ParseStructuredStip_make_branch_d(min_length,
                                                                 max_length);
    link_to_branch(proxy,battle_branch_make_postkeyplay(branch));

    tok = ParseStructuredStip_branch_d_operand(tok,proxy,level);
    if (tok!=0 && tok[0]=='a')
    {
      tok = ParseStructuredStip_branch_a_operand(tok+1,proxy,level);
      if (tok!=0 && level==0)
        select_output_mode(proxy,output_mode_tree);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Make an "ad branch"
 * @param min_length minimum length indicated by the user (0 if (s)he didn't)
 * @param max_length maximum length indicated by the user
 * @return identifier of branch entry slice
 */
static slice_index ParseStructuredStip_make_branch_a(stip_length_type min_length,
                                                     stip_length_type max_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParamListEnd();

  max_length += slack_length;
  min_length += slack_length+1;

  if (min_length>=max_length)
    min_length = max_length-1;

  result = alloc_battle_branch(max_length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse an "ad" branch
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves
 * @param proxy index of branch; no_slice if branch couldn't be
 *              parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_a(char *tok,
                                          stip_length_type min_length,
                                          stip_length_type max_length,
                                          slice_index proxy,
                                          unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  {
    slice_index const branch = ParseStructuredStip_make_branch_a(min_length,
                                                                 max_length);
    link_to_branch(proxy,branch);

    tok = ParseStructuredStip_branch_a_operand(tok,proxy,level);
    if (tok!=0 && tok[0]=='d')
    {
      tok = ParseStructuredStip_branch_d_operand(tok+1,proxy,level);
      if (tok!=0 && level==0)
        select_output_mode(proxy,output_mode_tree);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a "h operand"
 * @param tok input token
 * @param branch identifier of entry slice of "hh branch"
 * @param parity indicates after which help move of the branch to insert
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_h_operand(char *tok,
                                                  slice_index branch,
                                                  unsigned int parity,
                                                  unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",branch);
  TraceFunctionParam("%u",parity);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  while (tok!=0)
  {
    tok = ParseStructuredStip_skip_whitespace(tok);
    if (tok[0]=='[')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      nested_branch_type nested_type;

      tok = ParseStructuredStip_nested_branch(tok+1,proxy_operand,&nested_type,level);
      if (tok!=0 && tok[0]==']')
      {
        ++tok;
        switch (nested_type)
        {
          case nested_branch_type_goal:
            help_branch_set_end_goal(branch,proxy_operand,parity);
            break;

          case nested_branch_type_forced:
            help_branch_set_end_forced(branch,proxy_operand,parity);
            break;

          case nested_branch_type_defense:
          case nested_branch_type_attack:
            help_branch_set_end(branch,proxy_operand,parity);
            break;

          default:
            assert(0);
            break;
        }
      }
      else
        tok = 0;
    }
    else if (tok[0]=='{')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      expression_type nested_type;
      tok = ParseStructuredStip_expression(tok+1,proxy_operand,&nested_type,level+1);
      if (tok!=0 && tok[0]=='}'
          && help_branch_insert_constraint(branch,proxy_operand,parity))
        ++tok;
      else
        tok = 0;
    }
    else
      break;
    TraceValue("%s\n",tok);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Make an "hh branch"
 * @param min_length minimum length indicated by the user (0 if (s)he didn't)
 * @param max_length maximum length indicated by the user
 * @return identifier of branch entry slice
 */
static slice_index ParseStructuredStip_make_branch_h(stip_length_type min_length,
                                                     stip_length_type max_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParamListEnd();

  max_length += slack_length;

  if (min_length==0)
    min_length = slack_length+(max_length-slack_length)%2;
  else
  {
    min_length += slack_length;
    if (min_length>max_length)
      min_length = max_length;
  }

  result = alloc_help_branch(max_length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse a help branch
 * @param tok input token
 * @param min_length minimal number of half moves
 * @param max_length maximal number of half moves
 * @param proxy index of branch; no_slice if branch couldn't be
 *              parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_h(char *tok,
                                          stip_length_type min_length,
                                          stip_length_type max_length,
                                          slice_index proxy,
                                          unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  {
    slice_index const branch = ParseStructuredStip_make_branch_h(min_length,
                                                                 max_length);
    link_to_branch(proxy,branch);

    tok = ParseStructuredStip_branch_h_operand(tok,proxy,max_length,level);
    if (tok!=0 && tok[0]=='h')
    {
      tok = ParseStructuredStip_branch_h_operand(tok+1,proxy,max_length+1,level);
      if (level==0)
        select_output_mode(proxy,output_mode_line);
    }
    else
      tok = 0;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Make an "s branch"
 * @param min_length minimum length indicated by the user (0 if (s)he didn't)
 * @param max_length maximum length indicated by the user
 * @return identifier of branch entry slice
 */
static slice_index ParseStructuredStip_make_branch_s(stip_length_type min_length,
                                                     stip_length_type max_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParamListEnd();

  max_length *= 2;
  max_length += slack_length-1;

  if (min_length==0)
    min_length = slack_length+1;
  else
  {
    min_length *= 2;
    min_length += slack_length-1;
    if (min_length>max_length)
      min_length = max_length;
  }

  result = alloc_series_branch(max_length,min_length);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Parse a "s operand"
 * @param tok input token
 * @param identifier of entry slice of "s branch"
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_s_operand(char *tok,
                                                  slice_index branch,
                                                  unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",branch);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  while (tok!=0)
  {
    tok = ParseStructuredStip_skip_whitespace(tok);
    if (tok[0]=='[')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      nested_branch_type nested_type;

      tok = ParseStructuredStip_nested_branch(tok+1,proxy_operand,&nested_type,level);
      if (tok!=0 && tok[0]==']')
      {
        ++tok;
        switch (nested_type)
        {
          case nested_branch_type_goal:
            help_branch_set_end_goal(branch,proxy_operand,1);
            break;

          case nested_branch_type_defense:
          case nested_branch_type_forced:
            help_branch_set_end_forced(branch,proxy_operand,1);
            break;

          case nested_branch_type_attack:
            help_branch_set_end(branch,proxy_operand,1);
            break;

          default:
            assert(0);
            break;
        }
      }
      else
        tok = 0;
    }
    else if (tok[0]=='{')
    {
      slice_index const proxy_operand = alloc_proxy_slice();
      expression_type nested_type;
      tok = ParseStructuredStip_expression(tok+1,proxy_operand,&nested_type,level+1);
      if (tok!=0 && tok[0]=='}')
      {
        ++tok;
        if (nested_type==expression_type_goal)
          series_branch_insert_goal_constraint(branch,proxy_operand);
        else
          series_branch_insert_constraint(branch,proxy_operand);
      }
      else
        tok = 0;
    }
    else
      break;
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
 * @param proxy index of branch; no_slice if branch couldn't be
 *              parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch_s(char *tok,
                                          stip_length_type min_length,
                                          stip_length_type max_length,
                                          slice_index proxy,
                                          unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",max_length);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  {
    slice_index const branch = ParseStructuredStip_make_branch_s(min_length,
                                                                 max_length);
    link_to_branch(proxy,branch);

    tok = ParseStructuredStip_branch_s_operand(tok,proxy,level);
    if (tok!=0 && level==0)
      select_output_mode(proxy,output_mode_line);
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
    if (tok[0]==':')
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
 * @param proxy index of branch; no_slice if branch couldn't be
 *              parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_branch(char *tok,
                                        slice_index proxy,
                                        expression_type *type,
                                        unsigned int level)
{
  stip_length_type min_length;
  stip_length_type max_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  tok = ParseStructuredStip_branch_length(tok,&min_length,&max_length);

  if (tok!=0)
  {
    if (tok[0]=='d')
    {
      *type = expression_type_defense;
      tok = ParseStructuredStip_branch_d(tok+1,min_length,max_length,proxy,level);
    }
    else
    {
      *type = expression_type_attack;
      if (tok[0]=='s')
        tok = ParseStructuredStip_branch_s(tok+1,min_length,max_length,proxy,level);
      else if (tok[0]=='a')
        tok = ParseStructuredStip_branch_a(tok+1,min_length,max_length,proxy,level);
      else if (tok[0]=='h')
        tok = ParseStructuredStip_branch_h(tok+1,min_length,max_length,proxy,level);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse an stipulation operand
 * @param tok input token
 * @param proxy index of operand; no_slice if operand couldn't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_operand(char *tok,
                                         slice_index proxy,
                                         expression_type *type,
                                         unsigned int level);

/* Parse a not operator
 * @param tok input token
 * @param proxy index of branch; no_slice if operator couldn't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_not(char *tok,
                                     slice_index proxy,
                                     expression_type *type,
                                     unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  tok = ParseStructuredStip_operand(tok+1,proxy,type,level);
  if (tok!=0)
  {
    if (*type==expression_type_goal)
    {
      slice_index const tester = branch_find_slice(STGoalReachedTester,
                                                   proxy,
                                                   stip_traversal_context_intro);
      assert(tester!=no_slice);
      pipe_append(slices[tester].next2,alloc_not_slice());
      slices[tester].u.goal_handler.goal.type = goal_negated;
    }
    else
      pipe_append(proxy,alloc_not_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a move inversion
 * @param tok input token
 * @param proxy index of branch; no_slice if operator couldn't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_move_inversion(char *tok,
                                                slice_index proxy,
                                                expression_type *type,
                                                unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  tok = ParseStructuredStip_operand(tok+1,proxy,type,level);

  {
    slice_index const operand = slices[proxy].next1;
    if (tok!=0 && operand!=no_slice)
    {
      slice_index const prototype = alloc_move_inverter_slice();
      branch_insert_slices(proxy,&prototype,1);
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
static char *ParseStructuredStip_operator(char *tok, slice_type *result)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  /* allow space between operands */
  tok = ParseStructuredStip_skip_whitespace(tok);

  if (tok[0]=='&')
  {
    ++tok;
    *result = STAnd;
  }
  else if (tok[0]=='|')
  {
    ++tok;
    *result = STOr;
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
 * @param proxy index of expression slice; no_slice if expression
 *              can't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_expression(char *tok,
                                            slice_index proxy,
                                            expression_type *type,
                                            unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  {
    slice_index const operand1 = alloc_proxy_slice();
    tok = ParseStructuredStip_operand(tok,operand1,type,level);
    if (tok!=0 && slices[operand1].next1!=no_slice)
    {
      slice_type operator_type;
      tok = ParseStructuredStip_operator(tok,&operator_type);
      if (tok!=0 && operator_type!=no_slice_type)
      {
        if (*type==expression_type_defense)
          tok = 0;
        else
        {
          slice_index const operand2 = alloc_proxy_slice();
          expression_type type2;
          tok = ParseStructuredStip_expression(tok,operand2,&type2,level);
          if (tok!=0 && slices[operand2].next1!=no_slice)
          {
            if (*type==type2)
              switch (operator_type)
              {
                case STAnd:
                {
                  slice_index const and = alloc_and_slice(operand1,operand2);
                  pipe_link(proxy,and);
                  break;
                }

                case STOr:
                {
                  slice_index const or = alloc_or_slice(operand1,operand2);
                  pipe_link(proxy,or);
                  break;
                }

                default:
                  assert(0);
                  break;
              }
            else
              tok = 0;
          }
        }
      }
      else
      {
        if (slices[slices[operand1].next1].prev==operand1)
          pipe_link(proxy,slices[operand1].next1);
        else
          pipe_set_successor(proxy,slices[operand1].next1);

        dealloc_slice(operand1);
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
 * @param proxy index of expression slice; no_slice if expression
 *              can't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *
ParseStructuredStip_parenthesised_expression(char *tok,
                                             slice_index proxy,
                                             expression_type *type,
                                             unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  tok = ParseStructuredStip_expression(tok+1,proxy,type,level);

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
 * @param proxy index of operand; no_slice if operand couldn't be parsed
 * @param level nesting level of the operand (0 means top level)
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip_operand(char *tok,
                                         slice_index proxy,
                                         expression_type *type,
                                         unsigned int level)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParam("%u",level);
  TraceFunctionParamListEnd();

  /* allow space between operands */
  tok = ParseStructuredStip_skip_whitespace(tok);

  if (tok[0]=='(')
    tok = ParseStructuredStip_parenthesised_expression(tok,proxy,type,level);
  else if (tok[0]=='!')
    /* !d# - white at the move does *not* deliver mate */
    tok = ParseStructuredStip_not(tok,proxy,type,level);
  else if (tok[0]=='-')
    /* -3hh# - h#2 by the non-starter */
    tok = ParseStructuredStip_move_inversion(tok,proxy,type,level);
  else if (isdigit(tok[0]) && tok[0]!='0')
    /* e.g. 3ad# for a #2 - but not 00 (castling goal!)*/
    tok = ParseStructuredStip_branch(tok,proxy,type,level);
  else
  {
    /* e.g. d= for a =1 */
    *type = expression_type_goal;
    tok = ParseGoal(tok,proxy);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

/* Parse a structured stipulation (keyword sstipulation)
 * @return token following structured stipulation
 * @return remainder of input token; 0 if parsing failed
 */
static char *ParseStructuredStip(slice_index root_slice_hook)
{
  char *tok = 0;
  Side starter;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",root_slice_hook);
  TraceFunctionParamListEnd();

  CLRFLAGMASK(some_pieces_flags,PieceIdMask);

  AlphaStip[0] = 0;

  tok = ReadNextTokStr();
  starter = ParseStructuredStip_starter(tok);
  if (starter!=no_side)
  {
    expression_type type;
    strcat(AlphaStip,TokenLine);
    strcat(AlphaStip," ");
    tok = ReadNextTokStr();
    tok = ParseStructuredStip_expression(tok,root_slice_hook,&type,0);
    if (tok==0)
      tok = ReadNextTokStr();
    else if (slices[root_slice_hook].next1!=no_slice)
      stip_impose_starter(root_slice_hook,starter);
  }

  /* signal to our caller that the stipulation has changed */
  slices[root_slice_hook].starter = no_side;

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
    if (context!=ReadGrid)
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
      if (nr_squares_read!=0)
      {
        IoErrorMsg(WrongSquareList,0);
        return tok;
      }
    }
    else
    {
      switch (context)
      {
        case ReadImitators:
          isquare[nr_squares_read] = sq;
          break;

        case ReadHoles:
          block_square(sq);
          break;

        case ReadEpSquares:
        {
          move_effect_journal_index_type const base = move_effect_journal_base[nbply];
          move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

          switch (nr_squares_read)
          {
            case 0:
              move_effect_journal[movement].u.piece_movement.from = sq;
              break;

            case 1:
              move_effect_journal[movement].u.piece_movement.to = sq;
              break;

            case 2:
              en_passant_remember_multistep_over(move_effect_journal[movement].u.piece_movement.to);
              move_effect_journal[movement].u.piece_movement.to = sq;
              break;

            case 3:
              en_passant_remember_multistep_over(move_effect_journal[movement].u.piece_movement.to);
              move_effect_journal[movement].u.piece_movement.to = sq;
              break;

            default:
              Message(TooManyEpKeySquares);
              break;
          }
          break;
        }

        case ReadBlRoyalSq:
          royal_square[Black]= sq;
          break;

        case ReadWhRoyalSq:
          royal_square[White]= sq;
          break;

        case ReadNoCastlingSquares:
          switch (sq)
          {
            case square_a1:
              CLRFLAGMASK(castling_flags_no_castling,ra_cancastle<<(White*black_castling_offset));
              break;
            case square_e1:
              CLRFLAGMASK(castling_flags_no_castling,k_cancastle<<(White*black_castling_offset));
              break;
            case square_h1:
              CLRFLAGMASK(castling_flags_no_castling,rh_cancastle<<(White*black_castling_offset));
              break;
            case square_a8:
              CLRFLAGMASK(castling_flags_no_castling,ra_cancastle<<(Black*black_castling_offset));
              break;
            case square_e8:
              CLRFLAGMASK(castling_flags_no_castling,k_cancastle<<(Black*black_castling_offset));
              break;
            case square_h8:
              CLRFLAGMASK(castling_flags_no_castling,rh_cancastle<<(Black*black_castling_offset));
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
    number_of_imitators = nr_squares_read;

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
  gpRepublican,
  gpMagicSquare,
  gpColour
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

static nocontactfunc_t *nocontactfunc_parsed;

static char *ParseVariant(boolean *is_variant_set, VariantGroup group) {
  char    *tok=ReadNextTokStr();

  if (is_variant_set!=NULL && group != gpColour)
    *is_variant_set= false;

  do
  {
    VariantType type = GetUniqIndex(VariantTypeCount,VariantTypeTab,tok);

    if (type==VariantTypeCount)
      break;

    if (type>VariantTypeCount)
      IoErrorMsg(CondNotUniq,0);
    else if (type==TypeB && group==gpType)
      *is_variant_set= true;
    else if (type==TypeB && group==gpOsc)
      OscillatingKingsTypeB[OscillatingKingsSide]= true;
    else if (type==TypeC && group==gpOsc)
      OscillatingKingsTypeC[OscillatingKingsSide]= true;
    else if (type==TypeB && group==gpAnnan)
      annan_type= annan_type_B;
    else if (type==TypeC && group==gpAnnan)
      annan_type= annan_type_C;
    else if (type==TypeD && group==gpAnnan)
      annan_type= annan_type_D;
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
    else if (type==Type1 && group==gpMagicSquare)
      *is_variant_set = false;
    else if (type==Type2 && group==gpMagicSquare)
      *is_variant_set = true;
    else if (type==PionAdverse && group==gpSentinelles)
      *is_variant_set= true;
    else if (type==PionNeutral && group==gpSentinelles)
      SentPionNeutral= true;
    else if (type==PionNoirMaximum && group==gpSentinelles)
      tok = ParseMaximumPawn(&sentinelles_max_nr_pawns[Black],8,64);
    else if (type==PionBlancMaximum && group==gpSentinelles)
      tok = ParseMaximumPawn(&sentinelles_max_nr_pawns[White],8,64);
    else if (type==PionTotalMaximum && group==gpSentinelles)
      tok = ParseMaximumPawn(&sentinelles_max_nr_pawns_total,16,64);
    else if (type==ParaSent && group==gpSentinelles)
      flagparasent= true;
    else if (type==SentBerolina && group==gpSentinelles)
      sentinelle = BerolinaPawn;
    else if (type==AntiCirTypeCheylan && group==gpAntiCirce)
      *is_variant_set= true;
    else if (type==AntiCirTypeCalvet && group==gpAntiCirce)
      *is_variant_set= false;
    else if (type==Neighbour && group==gpKoeko)
    {
      PieNam tmp_piece;
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
        case King:
          break;
        case Knight:
          *nocontactfunc_parsed= noknightcontact;
          break;
        case Wesir:
          *nocontactfunc_parsed= nowazircontact;
          break;
        case Fers:
          *nocontactfunc_parsed= noferscontact;
          break;
        case Camel:
          *nocontactfunc_parsed= nocamelcontact;
          break;
        case Alfil:
          *nocontactfunc_parsed= noalfilcontact;
          break;
        case Zebra:
          *nocontactfunc_parsed= nozebracontact;
          break;
        case Dabbaba:
          *nocontactfunc_parsed= nodabbabacontact;
          break;
        case Giraffe:
          *nocontactfunc_parsed= nogiraffecontact;
          break;
        case Antilope:
          *nocontactfunc_parsed= noantelopecontact;
          break;
        default:
          IoErrorMsg(WrongPieceName,0);
          break;
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
      grid_type = grid_vertical_shift;
    }
    else if (type==ShiftFile && group==gpGrid)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 7)/2)+5*((*bnp/24-8)/2)) << Grid;
      }
      grid_type = grid_horizontal_shift;
    }
    else if (type==ShiftRankFile && group==gpGrid)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 7)/2)+5*((*bnp/24-7)/2)) << Grid;
      }
      grid_type = grid_diagonal_shift;
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
      grid_type = grid_orthogonal_lines;
    }
    else if (type==Irregular && group==gpGrid)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
      ClearGridNum(*bnp);
      grid_type = grid_irregular;
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
            grid_type= grid_irregular;
          }
        }
        else
        {
          break;
        }
      }
      continue;
    }
    else if (group == gpColour)
    {
      if (type == WhiteOnly)
        is_variant_set[Black] = false;
      if (type == BlackOnly)
        is_variant_set[White] = false;
    }
    else {
      return tok;
    }
    tok = ReadNextTokStr();
  } while (group==gpSentinelles || group==gpGrid || group ==gpColour);

  return tok;
}

static char *ParseMummerStrictness(mummer_strictness_type *strictness)
{
  char *tok = ReadNextTokStr();

  if (mummer_strictness_ultra==GetUniqIndex(nr_mummer_strictness,mummer_strictness_tab,tok))
  {
    *strictness = mummer_strictness_ultra;
    tok = ReadNextTokStr();
  }
  else if (mummer_strictness_exact==GetUniqIndex(nr_mummer_strictness,mummer_strictness_tab,tok))
  {
    *strictness = mummer_strictness_exact;
    tok = ReadNextTokStr();
  }
  else
    *strictness = mummer_strictness_regular;

  return tok;
}

static char *ParseVaultingPieces(Side side)
{
  char  *tok;

  while (true)
  {
    tok = ReadNextTokStr();
    switch (strlen(tok))
    {
      case 1:
      {
        PieNam const p = GetPieNamIndex(*tok,' ');
        if (side!=Black)
          append_king_vaulter(White,p);
        if (side!=White)
          append_king_vaulter(Black,p);
        break;
      }

      case 2:
      {
        PieNam const p = GetPieNamIndex(*tok,tok[1]);
        if (side!=Black)
          append_king_vaulter(White,p);
        if (side!=White)
          append_king_vaulter(Black,p);
        break;
      }

      default:
        if (GetUniqIndex(VariantTypeCount,VariantTypeTab,tok)==Transmuting)
        {
          if (side!=Black)
            vaulting_kings_transmuting[White]= true;
          if (side!=White)
            vaulting_kings_transmuting[Black]= true;
        }
        else
          return tok;
        break;
    }
  }

  return tok;
}

static boolean handle_chameleon_circe_reborn_piece(PieNam from, PieNam to,
                                                   char const *tok)
{
  boolean result;
  char to_str[3];

  if (to==Empty)
  {
    IoErrorMsg(WrongPieceName,0);
    result = false;
  }
  else
  {
    if (from!=Empty)
    {
      chameleon_circe_set_reborn_piece_explicit(from,to);
      strcat(ChameleonSequence, "->");
    }
    sprintf(to_str,
            "%c%c",
            UPCASE(tok[0]),
            tok[1]==' ' ? '\0' : UPCASE(tok[1]));
    strcat(ChameleonSequence,to_str);
    result = true;
  }

  return result;
}

static char *ReadChameleonCirceSequence(void)
{
  PieNam from = Empty;

  chameleon_circe_reset_reborn_pieces();

  ChameleonSequence[0]= '\0';

  while (true)
  {
    char *tok = ReadNextTokStr();
    switch (strlen(tok))
    {
      case 1:
      {
        PieNam const to = GetPieNamIndex(tok[0],' ');
        if (handle_chameleon_circe_reborn_piece(from,to,tok))
          from = to;
        else
          return tok;
        break;
      }

      case 2:
      {
        PieNam const to = GetPieNamIndex(tok[0],tok[1]);
        if (handle_chameleon_circe_reborn_piece(from,to,tok))
          from = to;
        else
          return tok;
        break;
      }

      default:
        return tok;
    }
  }

  return 0; /* avoid compiler warning */
}

static char *ParseCond(void)
{
  char    *tok, *ptr;
  unsigned int CondCnt = 0;

  tok = ReadNextTokStr();
  while (true)
  {
    Cond const indexx = GetUniqIndex(CondCount,CondTab,tok);
    TraceValue("%s",tok);
    TraceValue("%u\n",indexx);
    if (indexx==CondCount)
    {
      ExtraCond const extra = GetUniqIndex(ExtraCondCount,ExtraCondTab,tok);
      if (extra>ExtraCondCount)
      {
        IoErrorMsg(CondNotUniq,0);
        tok = ReadNextTokStr();
        break;
      }
      else if (extra==ExtraCondCount)
        break;
      else
      {
        ExtraCondFlag[extra] = true;

        switch (extra)
        {
          case maxi:
            tok = ParseMummerStrictness(&mummer_strictness_default_side);
            ++CondCnt;
            break;

          case ultraschachzwang:
            tok = ReadNextTokStr();
            ++CondCnt;
            break;

          default:
            assert(0);
            break;
        }

        continue;
      }
    }

    if (indexx>CondCount)
    {
      IoErrorMsg(CondNotUniq,0);
      tok = ReadNextTokStr();
      continue;
    }

    CondFlag[indexx]= true;

    CondCnt++;

    switch (indexx)
    {
      case hypervolage:
        CondFlag[volage]= true;
        break;
      case leofamily:
        CondFlag[chinoises]= true;
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
        magic_square_type = magic_square_type1;
        ReadSquares(MagicSq);
        break;
      case wormholes:
        ReadSquares(Wormhole);
        break;
      case dbltibet:
        CondFlag[tibet]= true;
        break;
      case holes:
        ReadSquares(ReadHoles);
        break;
      case trans_king:
        CondFlag[whtrans_king] = true;
        CondFlag[bltrans_king] = true;
        calc_reflective_king[White]= true;
        calc_reflective_king[Black]= true;
        break;
      case whtrans_king:
        calc_reflective_king[White]= true;
        break;
      case bltrans_king:
        calc_reflective_king[Black]= true;
        break;
      case whsupertrans_king:
        calc_reflective_king[White]= true;
        break;
      case blsupertrans_king:
        calc_reflective_king[Black]= true;
        break;
      case refl_king:
        CondFlag[whrefl_king] = true;
        CondFlag[blrefl_king] = true;
        calc_reflective_king[White]= true;
        calc_reflective_king[Black]= true;
        break;
      case whrefl_king:
        calc_reflective_king[White]= true;
        break;
      case blrefl_king:
        calc_reflective_king[Black]= true;
        break;
      case vault_king:
        CondFlag[whvault_king] = true;
        CondFlag[blvault_king] = true;
        calc_reflective_king[White]= true;
        calc_reflective_king[Black]= true;
        vaulting_kings_transmuting[White] = false;
        vaulting_kings_transmuting[Black] = false;
        break;
      case whvault_king:
        calc_reflective_king[White]= true;
        vaulting_kings_transmuting[White] = false;
        break;
      case blvault_king:
        calc_reflective_king[Black]= true;
        vaulting_kings_transmuting[Black] = false;
        break;
      case whforsqu:
        ReadSquares(WhForcedSq);
        break;
      case blforsqu:
        ReadSquares(BlForcedSq);
        break;
      case whconforsqu:
        ReadSquares(WhForcedSq);
        break;
      case blconforsqu:
        ReadSquares(BlForcedSq);
        break;

        /* different types of circe */
      case couscousmirror:
        anycirprom= true;
        anycirce= true;
        CondFlag[couscous]= true;
        circe_determine_rebirth_square= renspiegel;
        break;
      case pwc:
        circe_determine_rebirth_square= renpwc;
        anycirprom= true;
        anycirce= true;
        break;
      case couscous:
        anycirprom= true;
        anycirce= true;
        break;
      case parrain:
      case contraparrain:
        anycirprom= true;
        anycirce= true;
        anyparrain= true;
        break;
      case circediametral:
        circe_determine_rebirth_square= rendiametral;
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
      case circecage:
        circe_determine_rebirth_square= 0;
        anycirprom= true;
        anycirce= true;
        break;
      case circeequipollents:
        circe_determine_rebirth_square= renequipollents;
        anycirce= true;
        anycirprom= true;
        break;
      case circemalefique:
        circe_determine_rebirth_square= renspiegel;
        anycirce= true;
        break;
      case circerank:
        circe_determine_rebirth_square= renrank;
        anycirce= true;
        break;
      case circefile:
        circe_determine_rebirth_square= renfile;
        anycirce= true;
        break;
      case circefilemalefique:
        circe_determine_rebirth_square= renspiegelfile;
        anycirce= true;
        break;
      case circediagramm:
        circe_determine_rebirth_square= rendiagramm;
        anycirce= true;
        break;
      case circesymmetry:
        circe_determine_rebirth_square= rensymmetrie;
        anycirce= true;
        anycirprom= true;
        break;
      case circeantipoden:
        circe_determine_rebirth_square= renantipoden;
        anycirce= true;
        anycirprom= true;
        break;
      case circeclonemalefique:
        circe_determine_rebirth_square= renspiegel;
        anycirce= true;
        anyclone= true;
        break;
      case circeclone:
        circe_determine_rebirth_square = rennormal;
        anycirce= true;
        anyclone= true;
        break;
      case circeassassin:
        anycirce = true;
        break;
      case circetakeandmake:
        anycirce= true;
        anycirprom= true;
        break;

        /* different types of anticirce */
      case circeturncoats:
        anycirce= true;
        circe_determine_rebirth_square= rennormal;
        break;
      case circedoubleagents:
        anycirce= true;
        circe_determine_rebirth_square= renspiegel;
        break;
      case anti:
        anyanticirce= true;
        break;
      case antispiegel:
        anticirce_determine_rebirth_square= renspiegel;
        anyanticirce= true;
        break;
      case antidiagramm:
        anticirce_determine_rebirth_square= rendiagramm;
        anyanticirce= true;
        break;
      case antifile:
        anticirce_determine_rebirth_square= renfile;
        anyanticirce= true;
        break;
      case antisymmetrie:
        anticirce_determine_rebirth_square= rensymmetrie;
        anyanticirce= true;
        break;
      case antispiegelfile:
        anticirce_determine_rebirth_square= renspiegelfile;
        anyanticirce= true;
        break;
      case antiantipoden:
        anticirce_determine_rebirth_square= renantipoden;
        anyanticirce= true;
        anyanticirprom = true;
        break;
      case antiequipollents:
        anticirce_determine_rebirth_square= renequipollents_anti;
        anyanticirce= true;
        anyanticirprom = true;
        break;
      case antisuper:
        anyanticirce= true;
        anyanticirprom = true;
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
        marscirce_determine_rebirth_square= rennormal;
        anymars= true;
        break;
      case marsmirror:
        marscirce_determine_rebirth_square= renspiegel;
        anymars= true;
        break;
      case antimars:
        marscirce_determine_rebirth_square= rennormal;
        anyantimars= true;
        break;
      case antimarsmirror:
        marscirce_determine_rebirth_square= renspiegel;
        anyantimars= true;
        break;
      case antimarsantipodean:
        marscirce_determine_rebirth_square= renantipoden;
        anyantimars= true;
        break;
      case phantom:
        marscirce_determine_rebirth_square= rennormal;
        break;
      case plus:
        marscirce_determine_rebirth_square= 0;
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
        anygeneva= true;
        break;

      default:
        break;
    }

    switch (indexx)
    {
      case messigny:
        tok = ParseRex(&messigny_rex_exclusive, rexexcl);
        break;
      case woozles:
      case biwoozles:
      case heffalumps:
      case biheffalumps:
        tok = ParseRex(&woozles_rex_exclusive, rexexcl);
        break;
      case immun:
      case immunmalefique:
      case immundiagramm:
        tok = ParseRex(&immune_is_rex_inclusive, rexincl);
        break;
      case chamcirce:
        ReadChameleonCirceSequence();
        break;
      case circe:
      case circemalefique:
      case circefile:
      case circeequipollents:
      case circediagramm:
      case circesymmetry:
      case circeclone:
      case circefilemalefique:
      case circeclonemalefique:
      case circeantipoden:
      case circemalefiquevertical:
      case circediametral:
      case circerank:
      case frischauf:
        tok = ParseRex(&circe_is_rex_inclusive, rexincl);
        break;
      case protean:
        tok = ParseRex(&protean_is_rex_exclusive, rexexcl);
        break;
      case phantom:
        tok = ParseRex(&phantom_chess_rex_inclusive,rexincl);
        break;
      case madras:
        tok = ParseRex(&madrasi_is_rex_inclusive, rexincl);
        break;
      case isardam:
        tok = ParseVariant(&IsardamB, gpType);
        break;
      case annan:
        annan_type = annan_type_A;
        tok = ParseVariant(NULL, gpAnnan);
        break;
      case kobulkings:
        kobulking[White] = kobulking[Black] = true;
        tok = ParseVariant(kobulking, gpColour);
        if (!kobulking[White] && !kobulking[Black])
          kobulking[White] = kobulking[Black] = true;
        break;
      case sentinelles:
        SentPionNeutral=false;
        tok = ParseVariant(&SentPionAdverse, gpSentinelles);
        break;

        /*****  exact-maxis  *****/
      case blmax:
        tok  = ParseMummerStrictness(&mummer_strictness[Black]);
        break;
      case whmax:
        tok  = ParseMummerStrictness(&mummer_strictness[White]);
        break;
      case blmin:
        tok  = ParseMummerStrictness(&mummer_strictness[Black]);
        break;
      case whmin:
        tok  = ParseMummerStrictness(&mummer_strictness[White]);
        break;
      case blcapt:
        tok  = ParseMummerStrictness(&mummer_strictness[Black]);
        break;
      case whcapt:
        tok  = ParseMummerStrictness(&mummer_strictness[White]);
        break;

      case blconforsqu:
        mummer_strictness[Black] = mummer_strictness_ultra;
        tok = ReadNextTokStr();
        break;
      case whconforsqu:
        mummer_strictness[White] = mummer_strictness_ultra;
        tok = ReadNextTokStr();
        break;

      case blfollow:
      case blackalphabetic:
      case blacksynchron:
      case blackantisynchron:
      case blsupertrans_king:
      case blforsqu:
      case schwarzschacher:
        mummer_strictness[Black] = mummer_strictness_regular;
        tok = ReadNextTokStr();
        break;

      case whfollow:
      case whitealphabetic:
      case whitesynchron:
      case whiteantisynchron:
      case whsupertrans_king:
      case whforsqu:
        mummer_strictness[White] = mummer_strictness_regular;
        tok = ReadNextTokStr();
        break;

      case alphabetic:
        mummer_strictness[Black] = mummer_strictness_regular;
        mummer_strictness[White] = mummer_strictness_regular;
        tok = ReadNextTokStr();
        break;

      case duellist:
        mummer_strictness[Black] = mummer_strictness_regular;
        mummer_strictness[White] = mummer_strictness_regular;
        tok = ReadNextTokStr();
        break;

      case losingchess:
        mummer_strictness[Black] = mummer_strictness_regular;
        mummer_strictness[White] = mummer_strictness_regular;
        OptFlag[sansrn] = true;
        OptFlag[sansrb] = true;
        tok = ReadNextTokStr();
        break;

      case dynasty:
      case extinction:
        OptFlag[sansrn] = true;
        OptFlag[sansrb] = true;
        tok = ReadNextTokStr();
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
      case magicsquare:
      {
        boolean MagicSquaresType2 = false;
        tok = ParseVariant(&MagicSquaresType2, gpMagicSquare);
        magic_square_type = MagicSquaresType2 ? magic_square_type2 : magic_square_type1;
        break;
      }
      case promotiononly:
        tok = ReadPieces(promotiononly);
        break;
      case football:
        football_are_substitutes_limited = false;
        tok = ReadPieces(football);
        break;
      case april:
        tok = ReadPieces(april);
        if (CondFlag[april])
        {
          circe_determine_rebirth_square= 0;
          anycirprom= true;
          anycirce= true;
        }
        break;
      case koeko:
        koeko_nocontact= &nokingcontact;
        nocontactfunc_parsed= &koeko_nocontact;
        tok = ParseVariant(NULL, gpKoeko);
        break;
      case antikoeko:
        antikoeko_nocontact= nokingcontact;
        nocontactfunc_parsed= &antikoeko_nocontact;
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
        SAT_max_nr_allowed_flights[White] = strtoul(tok,&ptr,10) + 1;
        if (tok == ptr) {
          SAT_max_nr_allowed_flights[White]= 1;
          SAT_max_nr_allowed_flights[Black]= 1;
          break;
        }
        tok = ReadNextTokStr();
        SAT_max_nr_allowed_flights[Black] = strtoul(tok,&ptr,10) + 1;
        if (tok == ptr)
          SAT_max_nr_allowed_flights[Black]= SAT_max_nr_allowed_flights[White];
        break;
      case BGL:
        BGL_global= false;
        tok = ReadNextTokStr();
        BGL_values[White] = ReadBGLNumber(tok,&ptr);
        if (tok == ptr)
        {
          BGL_values[White] = BGL_infinity;
          BGL_values[Black] = BGL_infinity;
          return tok;
        }
        else
        {
          tok = ReadNextTokStr();
          BGL_values[Black]= ReadBGLNumber(tok,&ptr);
          if (tok == ptr)
          {
            BGL_values[Black] = BGL_values[White];
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
        tok = ParseVaultingPieces(White);
        break;
      case blvault_king:
        tok = ParseVaultingPieces(Black);
        break;
      case vault_king:
        tok = ParseVaultingPieces(no_side);
        break;
      case gridchess:
        tok = ParseVariant(NULL, gpGrid);
        break;
      default:
        tok = ReadNextTokStr();
        break;
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
    if (game_array.board[white_rook_square]==Rook
        && game_array.board[black_rook_square]==Rook)
    {
      square const white_castling = (white_rook_square==square_a1
                                     ? queenside_castling
                                     : kingside_castling);
      castling_flag_type const white_flag = (white_rook_square==square_a1
                                             ? ra_cancastle
                                             : rh_cancastle);
      square const black_castling = (black_rook_square==square_a8
                                     ? queenside_castling
                                     : kingside_castling);
      castling_flag_type const black_flag = (black_rook_square==square_a8
                                             ? ra_cancastle
                                             : rh_cancastle);
      castling_mutual_exclusive[White][white_castling-min_castling] |= black_flag;
      castling_mutual_exclusive[Black][black_castling-min_castling] |= white_flag;
      return;
    }
  }

  ErrorMsg(MissngSquareList);
}


static char *ParseOpt(slice_index root_slice_hook)
{
  Opt indexx;
  unsigned int OptCnt = 0;
  char    *tok;

  tok = ReadNextTokStr();
  for (indexx = GetUniqIndex(OptCount,OptTab,tok);
       indexx<OptCount;
       indexx = GetUniqIndex(OptCount,OptTab,tok))
  {
    if (indexx>OptCount)
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
        if (read_nr_beeps(tok))
          break;
        else
          /* tok doesn't indicate the number of beeps - hopefully,
           * it contains the next1 option
           */
          continue;

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
        tok = ReadNextTokStr();
        if (read_max_nr_solutions_per_target_position(tok))
          break;
        else
          continue;

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

      case nocastling:
        castling_flags_no_castling = bl_castlings|wh_castlings;
        ReadSquares(ReadNoCastlingSquares);
        break;

      case lastcapture:
        tok = ParseLastCapturedPiece();
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

PieNam twin_e[nr_squares_on_board];
Flags  twin_spec[nr_squares_on_board];
square twin_rb, twin_rn;
imarr  twin_isquare;

static void TwinStorePosition(void)
{
  int i;

  twin_rb= king_square[White];
  twin_rn= king_square[Black];
  for (i= 0; i < nr_squares_on_board; i++)
  {
    twin_e[i] = get_walk_of_piece_on_square(boardnum[i]);
    twin_spec[i] = spec[boardnum[i]];
  }

  for (i= 0; i < maxinum; i++)
    twin_isquare[i]= isquare[i];
}

static void TwinResetPosition(void)
{
  int i;

  king_square[White]= twin_rb;
  king_square[Black]= twin_rn;

  for (i= 0; i < nr_squares_on_board; i++)
    switch (twin_e[i])
    {
      case Empty:
        empty_square(boardnum[i]);
        break;

      case Invalid:
        block_square(boardnum[i]);
        break;

      default:
        occupy_square(boardnum[i],twin_e[i],twin_spec[i]);
        break;
    }

  for (i= 0; i < maxinum; i++)
    isquare[i]= twin_isquare[i];
}

static void transformPosition(SquareTransformation transformation)
{
  PieNam t_e[nr_squares_on_board];
  Flags t_spec[nr_squares_on_board];
  square t_rb, t_rn, sq1, sq2;
  imarr t_isquare;
  int i;

  /* save the position to be mirrored/rotated */
  t_rb = king_square[White];
  t_rn = king_square[Black];
  for (i = 0; i<nr_squares_on_board; i++)
  {
    t_e[i] = get_walk_of_piece_on_square(boardnum[i]);
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

    switch (t_e[i])
    {
      case Empty:
        empty_square(sq2);
        break;

      case Invalid:
        block_square(sq2);
        break;

      default:
        occupy_square(sq2,t_e[i],t_spec[i]);
        break;
    }

    if (sq1==t_rb)
      king_square[White] = sq2;
    if (sq1==t_rn)
      king_square[Black] = sq2;
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

  if (indexx>TwinningCount)
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

  if (is_square_empty(sq1))
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
            is_piece_neutral(spec[sq1])
            ? 'n'
            : TSTFLAG(spec[sq1], White) ? 'w' : 's',
            LaTeXPiece(get_walk_of_piece_on_square(sq1)),
            'a'-nr_files_on_board+sq1%onerow,
            '1'-nr_rows_on_board+sq1/onerow);
    strcat(ActTwinning, GlobalStr);
  }

  WriteSpec(spec[sq1],get_walk_of_piece_on_square(sq1),!is_square_empty(sq1));
  WritePiece(get_walk_of_piece_on_square(sq1));
  WriteSquare(sq1);
  if (indexx == TwinningExchange) {
    StdString("<-->");
    WriteSpec(spec[sq2], get_walk_of_piece_on_square(sq2),!is_square_empty(sq2));
    WritePiece(get_walk_of_piece_on_square(sq2));
    if (LaTeXout) {
      strcat(ActTwinning, "{\\lra}");
      sprintf(GlobalStr, "\\%c%s ",
              is_piece_neutral(spec[sq2])
              ? 'n'
              : TSTFLAG(spec[sq2], White) ? 'w' : 's',
              LaTeXPiece(get_walk_of_piece_on_square(sq2)));
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

  {
    PieNam const p = get_walk_of_piece_on_square(sq2);
    Flags const sp = spec[sq2];

    occupy_square(sq2,get_walk_of_piece_on_square(sq1),spec[sq1]);

    if (indexx==TwinningMove)
    {
      empty_square(sq1);

      if (sq1 == king_square[White])
        king_square[White]= sq2;
      else if (sq2 == king_square[White])
        king_square[White] = initsquare;

      if (sq1 == king_square[Black])
        king_square[Black]= sq2;
      else if (sq2 == king_square[Black])
        king_square[Black]= initsquare;
    }
    else
    {
      occupy_square(sq1,p,sp);

      if (sq1 == king_square[White])
        king_square[White]= sq2;
      else if (sq2 == king_square[White])
        king_square[White]= sq1;

      if (sq1 == king_square[Black])
        king_square[Black]= sq2;
      else if (sq2 == king_square[Black])
        king_square[Black]= sq1;
    }
  }

  return ReadNextTokStr();

} /* ParseTwinningMove */

static void MovePieceFromTo(square from, square to)
{
  PieNam const piece = get_walk_of_piece_on_square(from);

  switch (piece)
  {
    case Empty:
      empty_square(to);
      break;

    case Invalid:
      block_square(to);
      empty_square(from);
      break;

    default:
      occupy_square(to,piece,spec[from]);
      empty_square(from);
      if (from == king_square[White])
        king_square[White]= to;
      if (from == king_square[Black])
        king_square[Black]= to;
      break;
  }
} /* MovePieceFromTo */

static char *ParseTwinningShift(void)
{
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

  for (bnp= boardnum; *bnp; bnp++)
    if (!is_square_empty(*bnp))
    {
      if (*bnp/onerow < minrank)
        minrank= *bnp/onerow;
      if (*bnp/onerow > maxrank)
        maxrank= *bnp/onerow;
      if (*bnp%onerow < mincol)
        mincol= *bnp%onerow;
      if (*bnp%onerow > maxcol)
        maxcol= *bnp%onerow;
    }

  if ( maxcol+diffcol > 15
       || mincol+diffcol <  8
       || maxrank+diffrank > 15
       || minrank+diffrank <  8)
    ErrorMsg(PieceOutside);
  else
  {
    /* move along columns */
    if (diffrank > 0)
    {
      for (c= 8; c <= 15; c++)
        for (r= maxrank; r >= minrank; r--)
          MovePieceFromTo(onerow*r+c, onerow*(r+diffrank)+c);
    }
    else if (diffrank < 0)
    {
      for (c= 8; c <= 15; c++)
        for (r= minrank; r <= maxrank; r++)
          MovePieceFromTo(onerow*r+c, onerow*(r+diffrank)+c);
    }

    /* move along ranks */
    if (diffcol > 0)
    {
      for (c= maxcol; c >= mincol; c--)
        for (r= 8; r <= 15; r++)
          MovePieceFromTo(onerow*r+c, onerow*r+c+diffcol);
    }
    else if (diffcol < 0)
    {
      for (c= mincol; c <= maxcol; c++)
        for (r= 8; r <= 15; r++)
          MovePieceFromTo(onerow*r+c, onerow*r+c+diffcol);
    }
  }

  /* read next1 token */
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

    if (get_walk_of_piece_on_square(sq) < King) {
      WriteSquare(sq);
      StdString(": ");
      Message(NothingToRemove);
    }
    else {
      if (LaTeXout) {
        strcat(ActTwinning, " --");
        strcat(ActTwinning,
               is_piece_neutral(spec[sq])
               ? "\\n"
               : TSTFLAG(spec[sq], White) ? "\\w" : "\\s");
        strcat(ActTwinning,
               LaTeXPiece(get_walk_of_piece_on_square(sq)));
        sprintf(GlobalStr, " %c%c",
                'a'-nr_files_on_board+sq%onerow,
                '1'-nr_rows_on_board+sq/onerow);
        strcat(ActTwinning, GlobalStr);
      }

      StdString(" -");
      WriteSpec(spec[sq], get_walk_of_piece_on_square(sq),!is_square_empty(sq));
      WritePiece(get_walk_of_piece_on_square(sq));
      WriteSquare(sq);
      empty_square(sq);
      if (sq == king_square[White])
        king_square[White]= initsquare;
      if (sq == king_square[Black])
        king_square[Black]= initsquare;
    }
    tok += 2;
  }

  return ReadNextTokStr();
} /* ParseTwinningRemove */

static char *ParseTwinningPolish(void)
{
  {
    square const king_square_white = king_square[White];
    king_square[White] = king_square[Black];
    king_square[Black] = king_square_white;
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
      if (!is_piece_neutral(spec[*bnp]) && !is_square_empty(*bnp))
      {
        Flags flags = spec[*bnp];
        piece_change_side(&flags);
        occupy_square(*bnp,get_walk_of_piece_on_square(*bnp),flags);
      }
  }

  StdString(TwinningTab[TwinningPolish]);

  if (LaTeXout)
    strcat(ActTwinning, TwinningTab[TwinningPolish]);

  return ReadNextTokStr();
}

static char *ParseTwinningSubstitute(void)
{
  PieNam p_old, p_new;
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

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
      if (get_walk_of_piece_on_square(*bnp)==p_old)
        replace_piece(*bnp,p_new);
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

static char *ParseTwinning(slice_index root_slice_hook)
{
  char  *tok = ReadNextTokStr();
  boolean continued= false;
  boolean TwinningRead= false;

  ++TwinNumber;
  OptFlag[noboard]= true;

  while (true)
  {
    TwinningType twinning = 0;
    Token tk = StringToToken(tok);

    if (twinning>=TwinningCount
        || tk==TwinProblem
        || tk==NextProblem
        || tk==EndProblem)
    {
      Message(NewLine);
      if (LaTeXout)
        strcat(ActTwinning, "{\\newline}");
      return tok;
    }

    twinning = GetUniqIndex(TwinningCount,TwinningTab,tok);
    if (twinning>=TwinningCount)
      return tok;
    else
      switch (twinning)
      {
        case TwinningContinued:
          if (TwinningRead == true)
            Message(ContinuedFirst);
          else
            continued= true;
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
        {
          slice_index const next = slices[root_slice_hook].next1;
          pipe_unlink(root_slice_hook);
          dealloc_slices(next);
        }
        tok = ParseStip(root_slice_hook);

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
        {
          slice_index const next = slices[root_slice_hook].next1;
          pipe_unlink(root_slice_hook);
          dealloc_slices(next);
        }
        tok = ParseStructuredStip(root_slice_hook);

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
        tok = ParsePieces(piece_addition_twinning);
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

  TraceText("ParseTwinning() returns\n");
} /* ParseTwinning */

/***** twinning *****  end  *****/

/* new conditions: PromOnly, AprilChess */
char *ReadPieces(int condition)
{
  PieNam tmp_piece;
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
      if (!piece_read && condition != football) {
        CondFlag[condition]= false;
        IoErrorMsg(WrongPieceName,0);
      }
      return tok;
    }
    if (!tmp_piece && condition != football) {
      IoErrorMsg(WrongPieceName,0);
      break;
    }
    switch (condition) {
    case promotiononly:
      promonly[tmp_piece]= true;
      break;
    case football:
      is_football_substitute[tmp_piece]= true;
      football_are_substitutes_limited = true;
      break;
    case april:
      is_april_kind[tmp_piece]= true;
      break;
    default:
      /* Never mind ... */
      break;
    }
  }
  return tok;
}

static void ReadRemark(void)
{
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
}

Token ReadTwin(Token tk, slice_index root_slice_hook)
{
  char *tok;

  /* open mode for protocol and/or TeX file; overwrite existing file(s)
   * if we are doing a regression test */
  char const *open_mode = flag_regression ? "w" : "a";

  TraceValue("ReadTwin() - %u\n",tk);

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
    TraceText("-> ParseTwinning()\n");
    tok = ParseTwinning(root_slice_hook);
    TraceValue("ParseTwinning() -> %s\n",tok);

    while (true)
    {
      tk = StringToToken(tok);
      if (tk>TokenCount)
      {
        IoErrorMsg(ComNotUniq,0);
        tok = ReadNextTokStr();
      }
      else
        switch (tk)
        {
          case TwinProblem:
            if (slices[root_slice_hook].next1!=no_slice)
              return tk;
            else
            {
              IoErrorMsg(NoStipulation,0);
              tok = ReadNextTokStr();
              break;
            }

          case NextProblem:
          case EndProblem:
            if (root_slice_hook!=no_slice)
              return tk;
            else
            {
              IoErrorMsg(NoStipulation,0);
              tok = ReadNextTokStr();
              break;
            }

          case RemToken:
            ReadRemark();
            tok = ReadNextTokStr();
            break;

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
      if (tk>TokenCount)
      {
        IoErrorMsg(ComNotUniq,0);
        tok = ReadNextTokStr();
      }
      else
        switch (tk)
        {
          case TokenCount:
            IoErrorMsg(ComNotKnown,0);
            tok = ReadNextTokStr();
            break;

          case BeginProblem:
            tok = ReadNextTokStr();
            break;

          case TwinProblem:
            if (TwinNumber==1)
              TwinStorePosition();

            if (slices[root_slice_hook].next1!=no_slice)
              return tk;
            else
            {
              IoErrorMsg(NoStipulation,0);
              tok = ReadNextTokStr();
              break;
            }

          case NextProblem:
          case EndProblem:
            if (slices[root_slice_hook].next1!=no_slice)
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
            if (slices[root_slice_hook].next1==no_slice)
            {
              *AlphaStip='\0';
              tok = ParseStip(root_slice_hook);
            }
            else
            {
              IoErrorMsg(UnrecStip,0);
              tok = ReadNextTokStr();
            }
            break;

          case StructStipToken:
            *AlphaStip='\0';
            tok = ParseStructuredStip(root_slice_hook);
            break;

          case Author:
            ReadToEndOfLine();
            strcat(ActAuthor,InputLine);
            strcat(ActAuthor,"\n");
            tok = ReadNextTokStr();
            break;

          case Award:
            ReadToEndOfLine();
            strcpy(ActAward,InputLine);
            strcat(ActAward, "\n");
            tok = ReadNextTokStr();
            break;

          case Origin:
            ReadToEndOfLine();
            strcat(ActOrigin,InputLine);
            strcat(ActOrigin,"\n");
            tok = ReadNextTokStr();
            break;

          case TitleToken:
            ReadToEndOfLine();
            strcat(ActTitle,InputLine);
            strcat(ActTitle,"\n");
            tok = ReadNextTokStr();
            break;

          case PieceToken:
            tok = ParsePieces(piece_addition_initial);
            break;

          case CondToken:
            tok = ParseCond();
            break;

          case OptToken:
            tok = ParseOpt(root_slice_hook);
            break;

          case RemToken:
            ReadRemark();
            tok = ReadNextTokStr();
            break;

          case InputToken:
            ReadToEndOfLine();
            PushInput(InputLine);
            tok = ReadNextTokStr();
            break;

          case TraceToken:
            if (TraceFile!=NULL)
              fclose(TraceFile);

            ReadToEndOfLine();
            TraceFile = fopen(InputLine,open_mode);
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

            ReadToEndOfLine();
            LaTeXFile= fopen(InputLine,open_mode);
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
                occupy_square(boardnum[i],PAS[i],BIT(PAS_sides[i]));
              king_square[White] = square_e1;
              king_square[Black] = square_e8;
            }
            break;

          case Forsyth:
            tok = ParseForsyth(false);
            break;

          default:
            FtlMsg(InternalError);
            break;
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

static boolean is_square_occupied_by_imitator(square s)
{
  boolean result = false;
  unsigned int imi_idx;

  for (imi_idx = 0; imi_idx<number_of_imitators; ++imi_idx)
    if (s==isquare[imi_idx])
    {
      result = true;
      break;
    }

  return result;
}

void WritePosition()
{
  int nBlack, nWhite, nNeutr;
  square square, square_a;
  int row, file;
  char    HLine1[40];
  char    HLine2[40];
  char    PieCnts[20];
  char    StipOptStr[300];
  PieSpec sp;
  char    ListSpec[PieSpCount-nr_sides][256];
  unsigned int SpecCount[PieSpCount-nr_sides] = { 0 };
  FILE    *OrigSolFile= SolFile;

  static char BorderL[]="+---a---b---c---d---e---f---g---h---+\n";
  static char HorizL[]="%c   .   .   .   .   .   .   .   .   %c\n";
  static char BlankL[]="|                                   |\n";

  unsigned int const fileWidth = 4;

  SolFile= NULL;

  for (sp = nr_sides; sp<PieSpCount; ++sp)
    strcpy(ListSpec[sp-nr_sides], PieSpString[UserLanguage][sp-nr_sides]);

  StdChar('\n');
  MultiCenter(ActAuthor);
  MultiCenter(ActOrigin);
  MultiCenter(ActAward);
  MultiCenter(ActTitle);

  nBlack= nWhite= nNeutr= 0;
  StdChar('\n');
  StdString(BorderL);
  StdString(BlankL);

  for (row=1, square_a = square_a8;
       row<=nr_rows_on_board;
       row++, square_a += dir_down)
  {
    char const *digits="87654321";
    sprintf(HLine1, HorizL, digits[row-1], digits[row-1]);

    strcpy(HLine2,BlankL);

    for (file= 1, square= square_a;
         file <= nr_files_on_board;
         file++, square += dir_right)
    {
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

      if (is_square_occupied_by_imitator(square))
        *h1= 'I';
      else if (is_square_blocked(square))
        /* this is a hole ! */
        *h1= ' ';
      else if (is_square_empty(square))
      {
        /* nothing */
      }
      else
      {
        PieNam const pp = get_walk_of_piece_on_square(square);
        for (sp= nr_sides; sp<PieSpCount; ++sp)
          if (TSTFLAG(spec[square],sp)
              && !(sp==Royal && (pp==King || pp==Poseidon)))
          {
            AddSquare(ListSpec[sp-nr_sides], square);
            ++SpecCount[sp-nr_sides];
          }

        if (pp<Hunter0 || pp>=Hunter0+maxnrhuntertypes)
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
          char *n1 = HLine2 + (h1-HLine1); /* current position on next1 line */

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

        if (is_piece_neutral(spec[square]))
        {
          nNeutr++;
          *h1= '=';
        }
        else if (TSTFLAG(spec[square],Black))
        {
          nBlack++;
          *h1= '-';
        }
        else
          nWhite++;
      }
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

  if (SpecCount[Royal-nr_sides]>0)
    CenterLine(ListSpec[Royal-nr_sides]);

  for (sp = nr_sides; sp<PieSpCount; sp++)
    if (TSTFLAG(some_pieces_flags,sp))
      if (sp!=Royal
          && !(sp==Patrol && CondFlag[patrouille])
          && !(sp==Volage && CondFlag[volage])
          && !(sp==Beamtet && CondFlag[beamten]))
        CenterLine(ListSpec[sp-nr_sides]);

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
        || max_solutions_reached()
        || was_max_nr_solutions_per_target_position_reached()
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
  boolean is_piece_on_side[PieceCount][nr_sides+1];
  char ListSpec[PieSpCount-nr_sides][256];
  unsigned int SpecCount[PieSpCount-nr_sides] = { 0 };
  char    HolesSqList[256] = "";
  square const *bnp;

  for (sp= nr_sides; sp<PieSpCount; ++sp)
    strcpy(ListSpec[sp-nr_sides], PieSpString[UserLanguage][sp-nr_sides]);

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

  fprintf(LaTeXFile, " \\pieces{");

  {
    PieNam p;
    for (p = Empty; p < PieceCount; ++p)
    {
      is_piece_on_side[p][White] = false;
      is_piece_on_side[p][Black] = false;
      is_piece_on_side[p][nr_sides] = false;
    }
  }

  for (bnp= boardnum; *bnp; bnp++) {
    if (is_square_blocked(*bnp))
    {
      /* holes */
      if (holess)
        strcat(HolesSqList, ", ");
      else
        holess= true;
      AddSquare(HolesSqList, *bnp);
    }
    else if (!is_square_empty(*bnp))
    {
      PieNam const p = get_walk_of_piece_on_square(*bnp);
      if (!firstpiece)
        fprintf(LaTeXFile, ", ");
      else
        firstpiece= false;

      fprintf(LaTeXFile, "%c%s%c%c",
              is_piece_neutral(spec[*bnp]) ? 'n' :
              TSTFLAG(spec[*bnp], White)   ? 'w' : 's',
              LaTeXPiece(p),
              *bnp%onerow-200%onerow+'a',
              *bnp/onerow-200/onerow+'1');

      if (p>Bishop && (LaTeXPiecesAbbr[p] != NULL))
      {
        fairypieces= true;

        if (is_piece_neutral(spec[*bnp])) {
          is_piece_on_side[p][nr_sides] = true;
        }
        else if (TSTFLAG(spec[*bnp], White)) {
          is_piece_on_side[p][White] = true;
        }
        else {
          is_piece_on_side[p][Black] = true;
        }
      }

      for (sp= nr_sides; sp<PieSpCount; ++sp)
      {
        if (TSTFLAG(spec[*bnp], sp)
            && !(sp==Royal && (p==King || p==Poseidon)))
        {
          AddSquare(ListSpec[sp-nr_sides], *bnp);
          ++SpecCount[sp-nr_sides];
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

  for (sp= nr_sides; sp<PieSpCount; ++sp)
    if (SpecCount[sp-nr_sides]>0
        && !(sp==Patrol && CondFlag[patrouille])
        && !(sp==Volage && CondFlag[volage])
        && !(sp==Beamtet && CondFlag[beamten]))
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
    switch (grid_type)
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
  if (fairypieces || holess || modifiedpieces)
  {
    PieNam p;
    boolean firstline= true;

    fprintf(LaTeXFile, " \\remark{");
    for (p = Bishop+1; p < PieceCount; ++p)
    {
      PieNam q;
      if (is_piece_on_side[p][White] || is_piece_on_side[p][Black] || is_piece_on_side[p][nr_sides])
      {
        for (q = Bishop+1; q < p; q++) {
          if ((is_piece_on_side[q][White]
               || is_piece_on_side[q][Black]
               || is_piece_on_side[q][nr_sides])
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
        if (is_piece_on_side[p][White])
          fprintf(LaTeXFile, "\\w%s ", LaTeXPiecesAbbr[p]);
        if (is_piece_on_side[p][ Black])
          fprintf(LaTeXFile, "\\s%s ", LaTeXPiecesAbbr[p]);
        if (is_piece_on_side[p][nr_sides])
          fprintf(LaTeXFile, "\\n%s ", LaTeXPiecesAbbr[p]);
        fprintf(LaTeXFile, "=%s}", LaTeXPiecesFull[p]);
        firstline= false;
      }
    }

    if (modifiedpieces)
    {
      for (sp = nr_sides; sp<PieSpCount; ++sp)
        if (SpecCount[sp-nr_sides]>0)
        {
          if (!firstline)
            fprintf(LaTeXFile, "{\\newline}\n    ");
          fprintf(LaTeXFile, "%s\n", ListSpec[sp-nr_sides]);
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

void WritePiece(PieNam p) {
  if (p<Hunter0 || p>= (Hunter0 + maxnrhuntertypes))
  {
    char const p1 = PieceTab[p][1];
    StdChar(UPCASE(PieceTab[p][0]));
    if (p1!=' ')
      StdChar(UPCASE(p1));
  }
  else
  {
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

void InitMetaData(void)
{
  ActTitle[0] = '\0';
  ActAuthor[0] = '\0';
  ActOrigin[0] = '\0';
  ActTwinning[0] = '\0';
  ActAward[0] = '\0';
  ActStip[0] = '\0';
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
