#include "output/plaintext/condition.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/pieces.h"
#include "input/plaintext/condition.h"
#include "pieces/pieces.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/annan.h"
#include "conditions/bgl.h"
#include "conditions/circe/april.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/circe/chameleon.h"
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
#include "conditions/transmuting_kings/vaulting_kings.h"
#include "conditions/woozles.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static unsigned int WriteWalks(char *pos, PieNam const walks[], unsigned int nr_walks)
{
  unsigned int i;
  unsigned int result = 0;

  for (i = 0; i!=nr_walks; ++i)
  {
    PieNam const walk = walks[i];
    if (PieceTab[walk][1]==' ')
      result += sprintf(pos+result, " %c",toupper(PieceTab[walk][0]));
    else
      result += sprintf(pos+result," %c%c",toupper(PieceTab[walk][0]),toupper(PieceTab[walk][1]));
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

#define append_to_CondLine(line,pos,format,value) snprintf(*(line)+(pos), (sizeof *(line))-(pos),(format),(value))

static int append_to_CondLine_walk(char (*line)[256], int pos, PieNam walk)
{
  int result = append_to_CondLine(line,pos,"%c",(char)toupper(PieceTab[walk][0]));

  if (PieceTab[walk][1]!=' ')
    result += append_to_CondLine(line,pos+result,"%c",(char)toupper(PieceTab[walk][1]));

  return result;
}

static int append_to_CondLine_square(char (*line)[256], int pos, square s)
{
  return snprintf(*line+pos, sizeof *line - pos,
                  " %c%c",
                  'a' - nr_files_on_board + s%onerow,
                  '1' - nr_rows_on_board + s/onerow);
}

static int append_to_CondLine_chameleon_sequence(char (*line)[256],
                                                 int pos,
                                                 chameleon_sequence_type sequence)
{
  boolean already_written[PieceCount] = { false };
  PieNam p;
  int result = 0;

  result += append_to_CondLine(line,pos+result,"%s","    ");

  for (p = King; p<PieceCount; ++p)
    if (!already_written[p] && sequence[p]!=p)
    {
      PieNam q = p;

      result += append_to_CondLine_walk(line,pos+result,p);

      do
      {
        q = sequence[q];
        result += append_to_CondLine(line,pos+result,"%s","->");
        result += append_to_CondLine_walk(line,pos+result,q);
        already_written[q] = true;
      } while (q!=p);
    }

  return result;
}

boolean WriteConditions(void (*WriteCondition)(char const CondLine[], boolean is_first))
{
  Cond  cond;
  boolean CondPrinted= false;
  char CondLine[256] = { '\0' };

  for (cond = 1; cond<CondCount; ++cond)
  {
    unsigned int written;

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

    if (CondFlag[couscousmirror])
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
    written = append_to_CondLine(&CondLine,0,"%s", CondTab[cond]);

    if ((cond == blmax || cond == whmax) && ExtraCondFlag[maxi])
      written = append_to_CondLine(&CondLine,0,"%s", ExtraCondTab[maxi]);

    if ((cond==blackultraschachzwang || cond==whiteultraschachzwang)
        && ExtraCondFlag[ultraschachzwang])
      written = append_to_CondLine(&CondLine,0, "%s", ExtraCondTab[ultraschachzwang]);

    if (cond == sentinelles && flagparasent)
      written = append_to_CondLine(&CondLine,0,"Para%s",CondTab[cond]);

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
        written += append_to_CondLine_walk(&CondLine,written,koekop);
        written += append_to_CondLine(&CondLine,written,"%c",'-');
      }

      written += append_to_CondLine(&CondLine,written,"%s", CondTab[cond]);
    }

    if (cond == BGL)
    {
      char buf[12];

      WriteBGLNumber(buf, BGL_values[White]);
      written += append_to_CondLine(&CondLine,written," %s", buf);

      if (!BGL_global)
      {
        WriteBGLNumber(buf, BGL_values[Black]);
        written += append_to_CondLine(&CondLine,written,"/%s",buf);
      }
    }

    if (cond==kobulkings)
    {
      if (!kobulking[White])
        written += append_to_CondLine(&CondLine,written," %s","Black");
      if (!kobulking[Black])
        written += append_to_CondLine(&CondLine,written," %s","White");
    }

    if (cond==whvault_king || cond==vault_king)
    {
      if (nr_king_vaulters[White]!=1 || king_vaulters[White][0]!=EquiHopper)
        written += WriteWalks(CondLine+written,king_vaulters[White],nr_king_vaulters[White]);
      if (vaulting_kings_transmuting[White])
      {
        written += append_to_CondLine(&CondLine,written,"%c",'-');
        written += append_to_CondLine_walk(&CondLine,written,King);
      }
    }

    if (cond==blvault_king)
    {
      if (nr_king_vaulters[Black]!=1 || king_vaulters[Black][0]!=EquiHopper)
        written += WriteWalks(CondLine+written,king_vaulters[Black],nr_king_vaulters[Black]);
      if (vaulting_kings_transmuting[Black])
      {
        written += append_to_CondLine(&CondLine,written,"%c",'-');
        written += append_to_CondLine_walk(&CondLine,written,King);
      }
    }

    if (cond==promotiononly)
    {
      PieNam pp = Empty;
      while (true)
      {
        pp = pieces_pawns_promotee_sequence[pieces_pawns_promotee_chain_orthodox][pp];
        if (pp==Empty)
          break;
        else
        {
          written += append_to_CondLine(&CondLine,written,"%c",' ');
          written += append_to_CondLine_walk(&CondLine,written,pp);
        }
      }

      if (strlen(CondLine) <= strlen(CondTab[promotiononly])) {
        /* due to zeroposition, where pieces_pawns_promotee_sequence is not */
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
        else
        {
          written += append_to_CondLine(&CondLine,written,"%c",' ');
          written += append_to_CondLine_walk(&CondLine,written,pp);
        }
      }
      if (strlen(CondLine) <= strlen(CondTab[football])) {
        /* due to zeroposition, where pieces_pawns_promotee_sequence is not */
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
          written += append_to_CondLine(&CondLine,written,"%c",' ');
          written += append_to_CondLine_walk(&CondLine,written,pp);
        }
    }

    if (cond == imitators)
    {
      unsigned int imi_idx;
      for (imi_idx = 0; imi_idx<number_of_imitators; imi_idx++)
        written += append_to_CondLine_square(&CondLine,written,isquare[imi_idx]);
    }

    if (cond == noiprom && !CondFlag[imitators])
      continue;

    if (cond == magicsquare) {
      square  i;
      if (magic_square_type==magic_square_type2)
        written += append_to_CondLine(&CondLine,written, " %s", VariantTypeString[UserLanguage][Type2]);

      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], MagicSq))
          written += append_to_CondLine_square(&CondLine,written,i);
      }
    }
    if (cond == whforsqu || cond == whconforsqu)
    {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], WhForcedSq))
          written += append_to_CondLine_square(&CondLine,written,i);
      }
    }
    if (cond == blforsqu || cond == blconforsqu) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], BlForcedSq))
          written += append_to_CondLine_square(&CondLine,written,i);
      }
    }

    if (cond == whprom_sq) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], WhPromSq))
          written += append_to_CondLine_square(&CondLine,written,i);
      }
    }
    if (cond == blprom_sq) {
      square  i;
      for (i= square_a1; i <= square_h8; i++) {
        if (TSTFLAG(sq_spec[i], BlPromSq))
          written += append_to_CondLine_square(&CondLine,written,i);
      }
    }

    if (cond == blroyalsq)
      written += append_to_CondLine_square(&CondLine,written,royal_square[Black]);

    if (cond == whroyalsq)
      written += append_to_CondLine_square(&CondLine,written,royal_square[White]);

    if (cond==wormholes)
    {
      square i;
      for (i = square_a1; i<=square_h8; ++i)
        if (TSTFLAG(sq_spec[i],Wormhole))
          written += append_to_CondLine_square(&CondLine,written,i);
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
      written += append_to_CondLine(&CondLine,written," %s",CondTab[rexincl]);

    if ((messigny_rex_exclusive && cond == messigny)
        || (woozles_rex_exclusive
            && (cond==woozles || cond==biwoozles
                || cond==heffalumps || cond==biheffalumps)))
      written += append_to_CondLine(&CondLine,written," %s",CondTab[rexexcl]);

    if (protean_is_rex_exclusive && cond==protean)
      written += append_to_CondLine(&CondLine,written," %s",CondTab[rexexcl]);

    if (cond==chamcirce && !chameleon_circe_is_squence_implicit)
      written += append_to_CondLine_chameleon_sequence(&CondLine,written,
                                                       chameleon_circe_walk_sequence);

    if ((cond==chameleonsequence || cond==chamchess)
        && !chameleon_is_squence_implicit)
      written += append_to_CondLine_chameleon_sequence(&CondLine,written,
                                                       chameleon_walk_sequence);

    if (cond==isardam && IsardamB)
      written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][TypeB]);

    if (cond == annan)
    {
      switch (annan_type)
      {
        case annan_type_A:
          break;
        case annan_type_B:
          written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][TypeB]);
          break;
        case annan_type_C:
          written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][TypeC]);
          break;
        case annan_type_D:
          written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][TypeD]);
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
          written += append_to_CondLine(&CondLine,written,"  %s",VariantTypeString[UserLanguage][ShiftRank]);
          break;
        case grid_horizontal_shift:
          written += append_to_CondLine(&CondLine,written,"  %s",VariantTypeString[UserLanguage][ShiftFile]);
          break;
        case grid_diagonal_shift:
          written += append_to_CondLine(&CondLine,written,"  %s",VariantTypeString[UserLanguage][ShiftRankFile]);
          break;
        case grid_orthogonal_lines:
          written += append_to_CondLine(&CondLine,written,"  %s",VariantTypeString[UserLanguage][Orthogonal]);
          /* to do - write lines */
          break;
        case grid_irregular:
          written += append_to_CondLine(&CondLine,written,"  %s",VariantTypeString[UserLanguage][Irregular]);
          /* to do - write squares */
          break;
      }
    }

    if (cond==white_oscillatingKs && OscillatingKingsTypeB[White])
      written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][TypeB]);

    if (cond==black_oscillatingKs && OscillatingKingsTypeB[Black])
      written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][TypeB]);

    if (cond==white_oscillatingKs
        && OscillatingKingsTypeC[White]
        && ! CondFlag[swappingkings])
      written += append_to_CondLine(&CondLine,written,"  %s",VariantTypeString[UserLanguage][TypeC]);

    if (cond==black_oscillatingKs
        && OscillatingKingsTypeC[Black]
        && !CondFlag[swappingkings])
      written += append_to_CondLine(&CondLine,written,"  %s",VariantTypeString[UserLanguage][TypeC]);

    if (cond==singlebox)
    {
      if (SingleBoxType==singlebox_type1)
        written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][Type1]);
      if (SingleBoxType==singlebox_type2)
        written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][Type2]);
      if (SingleBoxType==singlebox_type3)
        written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][Type3]);
    }

    if (cond == republican)
    {
      if (RepublicanType==republican_type1)
        written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][Type1]);
      if (RepublicanType==republican_type2)
        written += append_to_CondLine(&CondLine,written,"    %s",VariantTypeString[UserLanguage][Type2]);
    }

    if (cond == sentinelles)
    {
      if (sentinelle == BerolinaPawn)
        written += append_to_CondLine(&CondLine,written," %s","Berolina");
      if (SentPionAdverse)
        written += append_to_CondLine(&CondLine,written,"  %s",VariantTypeString[UserLanguage][PionAdverse]);
      if (SentPionNeutral)
        written += append_to_CondLine(&CondLine,written,"  %s",VariantTypeString[UserLanguage][PionNeutral]);
      if (sentinelles_max_nr_pawns[Black] !=8 || sentinelles_max_nr_pawns[White] != 8)
      {
        written += append_to_CondLine(&CondLine,written," %u",sentinelles_max_nr_pawns[White]);
        written += append_to_CondLine(&CondLine,written,"/%u",sentinelles_max_nr_pawns[Black]);
      }
      if (sentinelles_max_nr_pawns_total != 16)
        written += append_to_CondLine(&CondLine,written," //%u", sentinelles_max_nr_pawns_total);
    }

    if ((cond==SAT || cond==strictSAT)
        && (SAT_max_nr_allowed_flights[White]!=1
            || SAT_max_nr_allowed_flights[Black]!=1)) {
      char const roman[][9] = {"","I","II","III","IV","V","VI","VII","VIII"};
      written += append_to_CondLine(&CondLine,written," %s", roman[SAT_max_nr_allowed_flights[White]-1]);
      if (SAT_max_nr_allowed_flights[White] != SAT_max_nr_allowed_flights[Black])
        written += append_to_CondLine(&CondLine,written,"/%s",roman[SAT_max_nr_allowed_flights[Black]-1]);
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
          written += append_to_CondLine(&CondLine,written,"  %s",VariantTypeString[UserLanguage][AntiCirceType]);
        break;

      case blmax:
      case blmin:
      case blcapt:
        if (mummer_strictness[Black]>mummer_strictness_regular)
        {
          if (mummer_strictness[Black]==mummer_strictness_ultra)
            written += append_to_CondLine(&CondLine,written,"  %s",mummer_strictness_tab[mummer_strictness_ultra]);
          else
            written += append_to_CondLine(&CondLine,written,"  %s",mummer_strictness_tab[mummer_strictness_exact]);
        }
        break;

      case whmax:
      case whmin:
      case whcapt:
        if (mummer_strictness[White]>mummer_strictness_regular)
        {
          if (mummer_strictness[White]==mummer_strictness_ultra)
            written += append_to_CondLine(&CondLine,written,"  %s",mummer_strictness_tab[mummer_strictness_ultra]);
          else
            written += append_to_CondLine(&CondLine,written,"  %s",mummer_strictness_tab[mummer_strictness_exact]);
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
