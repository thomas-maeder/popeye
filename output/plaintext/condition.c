#include "output/plaintext/condition.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "input/plaintext/condition.h"
#include "pieces/pieces.h"
#include "pieces/walks/pawns/promotion.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/annan.h"
#include "conditions/bgl.h"
#include "conditions/circe/april.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/anticirce/anticirce.h"
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

static unsigned int WritePieces(char *pos, PieNam const *p)
{
  unsigned int result = 0;

  while (*p)
  {
    if (PieceTab[*p][1]==' ')
      result += sprintf(pos+result, " %c",toupper(PieceTab[*p][0]));
    else
      result += sprintf(pos+result," %c%c",toupper(PieceTab[*p][0]),toupper(PieceTab[*p][1]));

    p++;
  }

  return result;
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

static unsigned int AppendSquareToCond(char *pos, square s)
{
  *pos++ = ' ';
  *pos++ = 'a' - nr_files_on_board + s%onerow;
  *pos++ = '1' - nr_rows_on_board + s/onerow;
  *pos = '\0';
  return 3;
}

boolean WriteConditions(void (*WriteCondition)(char const CondLine[], boolean is_first))
{
  Cond  cond;
  boolean CondPrinted= false;
  char CondLine[256] = { '\0' };

  for (cond = 1; cond<CondCount; ++cond)
  {
    unsigned int written = 0;

    if (!CondFlag[cond])
      continue;

    if (cond == rexexcl)
      continue;

    if (cond == volage && CondFlag[hypervolage])
      continue;

    if (cond == chinoises && CondFlag[leofamily])
      continue;

    if ((cond==gridchess || cond==koeko) && CondFlag[contactgrid])
      continue;

    if (cond==tibet && CondFlag[dbltibet])
      continue;

    if (cond==holes)
      continue;

    if (cond==couscous && CondFlag[couscousmirror])
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
    written += snprintf(CondLine+written, (sizeof CondLine)-written, "%s", CondTab[cond]);

    if ((cond == blmax || cond == whmax) && ExtraCondFlag[maxi])
      written = snprintf(CondLine,sizeof CondLine,"%s", ExtraCondTab[maxi]);

    if ((cond==blackultraschachzwang || cond==whiteultraschachzwang)
        && ExtraCondFlag[ultraschachzwang])
      written = snprintf(CondLine,sizeof CondLine, "%s", ExtraCondTab[ultraschachzwang]);

    if (cond == sentinelles && flagparasent)
      written = snprintf(CondLine,sizeof CondLine,"Para%s",CondTab[cond]);

    if (cond == koeko || cond == antikoeko)
    {
      PieNam koekop = King;
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

      written = 0;

      if (koekop!=King)
      {
        if (PieceTab[koekop][1]==' ')
          written += snprintf(CondLine+written, (sizeof CondLine)-written, " %c-",toupper(PieceTab[koekop][0]));
        else
          written += snprintf(CondLine+written, (sizeof CondLine)-written, "%c%c-",toupper(PieceTab[koekop][0]),toupper(PieceTab[koekop][1]));
      }

      written += snprintf(CondLine+written, (sizeof CondLine)-written, "%s", CondTab[cond]);
    }

    if (cond == BGL)
    {
      char buf1[12];
      char buf2[12];
      if (BGL_global)
      {
        WriteBGLNumber(buf1, BGL_values[White]);
        written += snprintf(CondLine+written, (sizeof CondLine)-written, " %s", buf1);
      }
      else
      {
        WriteBGLNumber(buf1, BGL_values[White]);
        WriteBGLNumber(buf2, BGL_values[Black]);
        written += snprintf(CondLine+written, (sizeof CondLine)-written, " %s/%s", buf1,buf2);
      }
    }

    if ( cond == kobulkings )
    {
      if (!kobulking[White])
        written += snprintf(CondLine+written, (sizeof CondLine)-written, " Black");
      if (!kobulking[Black])
        written += snprintf(CondLine+written, (sizeof CondLine)-written, " White");
    }

    if ( cond == whvault_king || cond == vault_king)
    {
      if (king_vaulters[White][0] != EquiHopper || king_vaulters[White][1] != Empty)
        written += WritePieces(CondLine+written,king_vaulters[White]);
      if (vaulting_kings_transmuting[White])
        written += snprintf(CondLine+written, (sizeof CondLine)-written,  " -%c",toupper(PieceTab[King][0]));
    }

    if ( cond == blvault_king )
    {
      if (king_vaulters[Black][0] != EquiHopper || king_vaulters[Black][1] != Empty)
        written += WritePieces(CondLine+written,king_vaulters[Black]);
      if (vaulting_kings_transmuting[Black])
        written += snprintf(CondLine+written, (sizeof CondLine)-written,  " -%c",toupper(PieceTab[King][0]));
    }

    if (cond == promotiononly)
    {
      PieNam pp = Empty;
      while (true)
      {
        pp = pieces_pawns_promotee_chain[pieces_pawns_promotee_chain_orthodox][pp];
        if (pp==Empty)
          break;

        if (PieceTab[pp][1]==' ')
          written += snprintf(CondLine+written, (sizeof CondLine)-written,  " %c",toupper(PieceTab[pp][0]));
        else
          written += snprintf(CondLine+written, (sizeof CondLine)-written, " %c%c",toupper(PieceTab[pp][0]),toupper(PieceTab[pp][1]));
      }
      if (strlen(CondLine) <= strlen(CondTab[promotiononly])) {
        /* due to zeroposition, where pieces_pawns_promotee_chain is not */
        /* set (it's set in verifieposition), I suppress  */
        /* output of promotiononly for now.  */
        continue;
      }
    }

    if (cond == football)
    {
      PieNam pp= Empty;
      while (true)
      {
        pp = next_football_substitute[pp];
        if (pp==Empty)
          break;

        if (PieceTab[pp][1]==' ')
          written += snprintf(CondLine+written, (sizeof CondLine)-written,  " %c",toupper(PieceTab[pp][0]));
        else
          written += snprintf(CondLine+written, (sizeof CondLine)-written, " %c%c",toupper(PieceTab[pp][0]),toupper(PieceTab[pp][1]));
      }
      if (strlen(CondLine) <= strlen(CondTab[football])) {
        /* due to zeroposition, where pieces_pawns_promotee_chain is not */
        /* set (it's set in verifieposition), I suppress  */
        /* output of promotiononly for now.  */
        continue;
      }
    }

    if (cond == april)
    {
      PieNam pp;
      for (pp = Empty; pp!=PieceCount; ++pp)
        if (is_april_kind[pp])
        {
          if (PieceTab[pp][1]==' ')
            written += snprintf(CondLine+written, (sizeof CondLine)-written,  " %c",toupper(PieceTab[pp][0]));
          else
            written += snprintf(CondLine+written, (sizeof CondLine)-written, " %c%c",toupper(PieceTab[pp][0]),toupper(PieceTab[pp][1]));
        }
    }

    if (cond == imitators)
    {
      unsigned int imi_idx;
      for (imi_idx = 0; imi_idx<number_of_imitators; imi_idx++)
        written += AppendSquareToCond(CondLine+written,isquare[imi_idx]);
    }

    if (cond == noiprom && !CondFlag[imitators])
      continue;

    if (cond == magicsquare) {
      square  i;
      if (magic_square_type==magic_square_type2)
        written += snprintf(CondLine+written, (sizeof CondLine)-written,  " %s", VariantTypeString[UserLanguage][Type2]);

      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], MagicSq))
          written += AppendSquareToCond(CondLine+written,i);
      }
    }
    if (cond == whforsqu || cond == whconforsqu)
    {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], WhForcedSq))
          written += AppendSquareToCond(CondLine+written,i);
      }
    }
    if (cond == blforsqu || cond == blconforsqu) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], BlForcedSq))
          written += AppendSquareToCond(CondLine+written,i);
      }
    }

    if (cond == whprom_sq) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], WhPromSq))
          written += AppendSquareToCond(CondLine+written,i);
      }
    }
    if (cond == blprom_sq) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], BlPromSq))
          written += AppendSquareToCond(CondLine+written,i);
      }
    }

    if (cond == blroyalsq)
      written += AppendSquareToCond(CondLine+written,royal_square[Black]);

    if (cond == whroyalsq)
      written += AppendSquareToCond(CondLine+written,royal_square[White]);

    if (cond==wormholes)
    {
      square i;
      for (i = square_a1; i<=square_h8; ++i)
        if (TSTFLAG(sq_spec[i],Wormhole))
          written += AppendSquareToCond(CondLine+written,i);
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
      written += snprintf(CondLine+written, (sizeof CondLine)-written, " %s",CondTab[rexincl]);

    if ((messigny_rex_exclusive && cond == messigny)
        || (woozles_rex_exclusive
            && (cond==woozles || cond==biwoozles
                || cond==heffalumps || cond==biheffalumps)))
      written += snprintf(CondLine+written, (sizeof CondLine)-written, " %s",CondTab[rexexcl]);

    if (protean_is_rex_exclusive && cond==protean)
      written += snprintf(CondLine+written, (sizeof CondLine)-written, " %s",CondTab[rexexcl]);

    if (cond==chamcirce && ChameleonSequence[0])
      written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",ChameleonSequence);

    if (cond==isardam && IsardamB)
      written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][TypeB]);

    if (cond == annan)
    {
      switch (annan_type)
      {
        case annan_type_A:
          break;
        case annan_type_B:
          written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][TypeB]);
          break;
        case annan_type_C:
          written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][TypeC]);
          break;
        case annan_type_D:
          written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][TypeD]);
          break;
      }
    }

    if (cond == gridchess && OptFlag[suppressgrid])
    {
      switch (grid_type)
      {
        case grid_normal:
          /* nothing */
          break;
        case grid_vertical_shift:
          written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",VariantTypeString[UserLanguage][ShiftRank]);
          break;
        case grid_horizontal_shift:
          written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",VariantTypeString[UserLanguage][ShiftFile]);
          break;
        case grid_diagonal_shift:
          written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",VariantTypeString[UserLanguage][ShiftRankFile]);
          break;
        case grid_orthogonal_lines:
          written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",VariantTypeString[UserLanguage][Orthogonal]);
          /* to do - write lines */
          break;
        case grid_irregular:
          written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",VariantTypeString[UserLanguage][Irregular]);
          /* to do - write squares */
          break;
      }
    }

    if (cond==white_oscillatingKs && OscillatingKingsTypeB[White])
      written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][TypeB]);

    if (cond==black_oscillatingKs && OscillatingKingsTypeB[Black])
      written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][TypeB]);

    if (cond==white_oscillatingKs
        && OscillatingKingsTypeC[White]
        && ! CondFlag[swappingkings])
      written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",VariantTypeString[UserLanguage][TypeC]);

    if (cond==black_oscillatingKs
        && OscillatingKingsTypeC[Black]
        && !CondFlag[swappingkings])
      written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",VariantTypeString[UserLanguage][TypeC]);

    if (cond==singlebox)
    {
      if (SingleBoxType==singlebox_type1)
        written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][Type1]);
      if (SingleBoxType==singlebox_type2)
        written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][Type2]);
      if (SingleBoxType==singlebox_type3)
        written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][Type3]);
    }

    if (cond == republican)
    {
      if (RepublicanType==republican_type1)
        written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][Type1]);
      if (RepublicanType==republican_type2)
        written += snprintf(CondLine+written, (sizeof CondLine)-written, "    %s",VariantTypeString[UserLanguage][Type2]);
    }

    if (cond == sentinelles)
    {
      if (sentinelle == BerolinaPawn)
        written += snprintf(CondLine+written, (sizeof CondLine)-written, " Berolina");
      if (SentPionAdverse)
        written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",VariantTypeString[UserLanguage][PionAdverse]);
      if (SentPionNeutral)
        written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",VariantTypeString[UserLanguage][PionNeutral]);
      if (sentinelles_max_nr_pawns[Black] !=8 || sentinelles_max_nr_pawns[White] != 8)
        written += snprintf(CondLine+written, (sizeof CondLine)-written, " %u/%u",sentinelles_max_nr_pawns[White],sentinelles_max_nr_pawns[Black]);
      if (sentinelles_max_nr_pawns_total != 16)
        written += snprintf(CondLine+written, (sizeof CondLine)-written, " //%u", sentinelles_max_nr_pawns_total);
    }

    if ((cond==SAT || cond==strictSAT)
        && (SAT_max_nr_allowed_flights[White]!=1
            || SAT_max_nr_allowed_flights[Black]!=1)) {
      char const roman[][9] = {"","I","II","III","IV","V","VI","VII","VIII"};
      if (SAT_max_nr_allowed_flights[White] == SAT_max_nr_allowed_flights[Black])
        written += snprintf(CondLine+written, (sizeof CondLine)-written, " %s", roman[SAT_max_nr_allowed_flights[White]-1]);
      else
        written += snprintf(CondLine+written, (sizeof CondLine)-written, " %s/%s", roman[SAT_max_nr_allowed_flights[White]-1], roman[SAT_max_nr_allowed_flights[Black]-1]);
    }

    switch (cond)
    {
      case anti:
      case antispiegel:
      case antidiagramm:
      case antifile:
      case antisymmetrie:
      case antispiegelfile:
      case antiantipoden:
      case antiequipollents:
      case antisuper:
        /* AntiCirceTypeCalvet is default in AntiCirce */
        if (AntiCirceType!=AntiCirceTypeCalvet)
          written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",VariantTypeString[UserLanguage][AntiCirceType]);
        break;

      case blmax:
      case blmin:
      case blcapt:
        if (mummer_strictness[Black]>mummer_strictness_regular)
        {
          if (mummer_strictness[Black]==mummer_strictness_ultra)
            written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",mummer_strictness_tab[mummer_strictness_ultra]);
          else
            written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",mummer_strictness_tab[mummer_strictness_exact]);
        }
        break;

      case whmax:
      case whmin:
      case whcapt:
        if (mummer_strictness[White]>mummer_strictness_regular)
        {
          if (mummer_strictness[White]==mummer_strictness_ultra)
            written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",mummer_strictness_tab[mummer_strictness_ultra]);
          else
            written += snprintf(CondLine+written, (sizeof CondLine)-written, "  %s",mummer_strictness_tab[mummer_strictness_exact]);
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
