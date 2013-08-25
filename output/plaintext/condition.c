#include "output/plaintext/condition.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "input/plaintext/condition.h"
#include "pieces/pieces.h"
#include "pieces/walks/pawns/promotion.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/annan.h"
#include "conditions/bgl.h"
#include "conditions/circe/april.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/football.h"
#include "conditions/geneva.h"
#include "conditions/grid.h"
#include "conditions/imitator.h"
#include "conditions/immune.h"
#include "conditions/isardam.h"
#include "conditions/kobul.h"
#include "conditions/koeko/anti.h"
#include "conditions/koeko/koeko.h"
#include "conditions/madrasi.h"
#include "conditions/magic_square.h"
#include "conditions/messigny.h"
#include "conditions/oscillating_kings.h"
#include "conditions/phantom.h"
#include "conditions/protean.h"
#include "conditions/republican.h"
#include "conditions/sat.h"
#include "conditions/sentinelles.h"
#include "conditions/singlebox/type1.h"
#include "conditions/vaulting_kings.h"
#include "conditions/woozles.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void WritePieces(PieNam const *p, char* CondLine)
{
      /* due to a Borland C++ 4.5 bug we have to use LocalBuf ... */
  char LocalBuf[4];
  while (*p) {
      if (PieceTab[*p][1] != ' ')
      sprintf(LocalBuf, " %c%c",
              toupper(PieceTab[*p][0]),
              toupper(PieceTab[*p][1]));
    else
      sprintf(LocalBuf, " %c",
              toupper(PieceTab[*p][0]));
      strcat(CondLine, LocalBuf);
    p++;
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

boolean WriteConditions(void (*WriteCondition)(char const CondLine[], boolean is_first))
{
  Cond  cond;
  boolean CondPrinted= false;
  char CondLine[256];

  for (cond= 1; cond < CondCount; cond++)
  {
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
                toupper(PieceTab[koekop][0]),
                toupper(PieceTab[koekop][1]));
      else
        sprintf(LocalBuf, " %c-",
                toupper(PieceTab[koekop][0]));

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
          toupper(PieceTab[King][0]));
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
          toupper(PieceTab[King][0]));
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
                  toupper(PieceTab[pp][0]),
                  toupper(PieceTab[pp][1]));
        else
          sprintf(LocalBuf, " %c",
                  toupper(PieceTab[pp][0]));
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
                  toupper(PieceTab[pp][0]),
                  toupper(PieceTab[pp][1]));
        else
          sprintf(LocalBuf, " %c",
                  toupper(PieceTab[pp][0]));
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
                    toupper(PieceTab[pp][0]),
                    toupper(PieceTab[pp][1]));
          else
            sprintf(LocalBuf, " %c",
                    toupper(PieceTab[pp][0]));
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
      republican_write_diagram_caption(CondLine, sizeof CondLine);

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

    (*WriteCondition)(CondLine,!CondPrinted);

    CondPrinted= true;
  }

  return CondPrinted;
}
