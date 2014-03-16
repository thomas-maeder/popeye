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
#include "conditions/circe/circe.h"
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
#include "conditions/marscirce/phantom.h"
#include "conditions/protean.h"
#include "conditions/republican.h"
#include "conditions/sat.h"
#include "conditions/sentinelles.h"
#include "conditions/singlebox/type1.h"
#include "conditions/transmuting_kings/vaulting_kings.h"
#include "conditions/woozles.h"
#include "debugging/assert.h"

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

  result += append_to_CondLine(line,pos+result,"%s"," ");

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

static unsigned int append_circe_variants(circe_variant_type const *variant,
                                          char (*CondLine)[256],
                                          unsigned int written,
                                          CirceVariantType rex_default)
{
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_pwc)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantPWC]);
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_symmetry)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantSymmetry]);
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_diagram)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantDiagramm]);
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_cage)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantCage]);
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_rank)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantRank]);
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_file)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantFile]);
  if (variant->relevant_piece==circe_relevant_piece_other)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantCouscous]);
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_antipodes)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantAntipodes]);
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_take_and_make)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantTakeAndMake]);
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_super)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantSuper]);
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_april)
  {
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantApril]);

    {
      PieNam pp;
      for (pp = Empty; pp!=PieceCount; ++pp)
        if (is_april_kind[pp])
        {
          written += append_to_CondLine(CondLine,written,"%c",' ');
          written += append_to_CondLine_walk(CondLine,written,pp);
        }
    }
  }
  if (variant->determine_rebirth_square==circe_determine_rebirth_square_equipollents)
  {
    if (variant->relevant_capture==circe_relevant_capture_thismove)
      written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantEquipollents]);
    else if (variant->is_mirror)
      written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantContraParrain]);
    else
      written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantParrain]);
  }
  if (variant->is_mirror)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantMirror]);
  if (variant->on_occupied_rebirth_square==circe_on_occupied_rebirth_square_assassinate)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantAssassin]);
  if (variant->on_occupied_rebirth_square==circe_on_occupied_rebirth_square_strict)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantStrict]);
  if (variant->on_occupied_rebirth_square==circe_on_occupied_rebirth_square_relaxed)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantRelaxed]);
  if (variant->on_occupied_rebirth_square==circe_on_occupied_rebirth_square_parachute)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantParachute]);
  if (variant->on_occupied_rebirth_square==circe_on_occupied_rebirth_square_volcanic)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantVolcanic]);
  if (variant->is_diametral)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantDiametral]);
  if (variant->reborn_walk_adapter==circe_reborn_walk_adapter_clone)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantClone]);
  if (variant->reborn_walk_adapter==circe_reborn_walk_adapter_chameleon)
  {
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantChameleon]);
    if (!chameleon_circe_is_squence_implicit)
      written += append_to_CondLine_chameleon_sequence(CondLine,written,
                                                       chameleon_circe_walk_sequence);
  }
  if (variant->is_turncoat)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantTurncoats]);
  if (variant->is_frischauf)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantFrischauf]);

  /* AntiCirceTypeCalvet is default in AntiCirce */
  if (variant->anticirce_type==anticirce_type_cheylan)
    written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[CirceVariantCheylan]);

  {
    CirceVariantType const rex_type = (variant->is_rex_inclusive
                                       ? CirceVariantRexInclusive
                                       : CirceVariantRexExclusive);
    if (rex_type!=rex_default)
      written += append_to_CondLine(CondLine,written," %s",CirceVariantTypeTab[rex_type]);
  }

  return written;
}

static unsigned int append_mummer_strictness(Side side,
                                             char (*CondLine)[256],
                                             unsigned int written)
{
  switch (mummer_strictness[side])
  {
    case mummer_strictness_none:
    case mummer_strictness_regular:
      break;
    case mummer_strictness_exact:
      written += append_to_CondLine(CondLine,written,"  %s",mummer_strictness_tab[mummer_strictness_exact]);
      break;
    case mummer_strictness_ultra:
      written += append_to_CondLine(CondLine,written,"  %s",mummer_strictness_tab[mummer_strictness_ultra]);
      break;
    default:
      assert(0);
      break;
  }

  return written;
}

static boolean anything_to_write(Cond cond)
{
  switch (cond)
  {
    case rexexcl:
      return false;

    case volage:
      return !CondFlag[hypervolage];

    case chinoises:
      return !CondFlag[leofamily];

    case gridchess:
    case koeko:
      return !CondFlag[contactgrid];

    case tibet:
      return !CondFlag[dbltibet];

    case holes:
      /* depicted in diagram */
      return false;

    /* these values have been kept for backward compatibility */
    case circemirror:
    case circecouscous:
    case circecouscousmirror:
    case circefilemirror:
    case circeclonemirror:
    case circeassassin:
    case circediametral:
    case circeclone:
    case circechameleon:
    case circeturncoats:
    case circedoubleagents:
    case circeparrain:
    case circecontraparrain:
    case circeequipollents:
    case circecage:
    case circerank:
    case circefile:
    case circesymmetry:
    case circediagramm:
    case pwc:
    case circeantipoden:
    case circetakeandmake:
    case supercirce:
    case april:
    case frischauf:
    case antisuper:
    case antidiagramm:
    case antifile:
    case antisymmetrie:
    case antimirror:
    case antimirrorfile:
    case antiantipoden:
    case antiequipollents:
    case anticlonecirce:
    case immunmirror:
    case immunfile:
    case immundiagramm:
    case immunmirrorfile:
    case immunsymmetry:
    case immunantipoden:
    case immunequipollents:
      return false;

    case white_oscillatingKs:
      return !(CondFlag[swappingkings] && OscillatingKings[White]==ConditionTypeC);

    case black_oscillatingKs:
      return !(CondFlag[swappingkings] && OscillatingKings[Black]==ConditionTypeC);

    case whitealphabetic:
    case blackalphabetic:
      return !CondFlag[alphabetic];

    case whvault_king:
    case blvault_king:
      return !CondFlag[vault_king];

    case whtrans_king:
    case bltrans_king:
      return !CondFlag[trans_king];

    default:
      return true;
  }
}

boolean WriteConditions(void (*WriteCondition)(char const CondLine[], boolean is_first))
{
  Cond cond;
  boolean result = false;

  for (cond = 1; cond<CondCount; ++cond)
    if (CondFlag[cond] && anything_to_write(cond))
    {
      char CondLine[256] = { '\0' };
      unsigned int written = append_to_CondLine(&CondLine,0,"%s", CondTab[cond]);

      switch (cond)
      {
        case blmax:
          if (ExtraCondFlag[maxi])
            written = append_to_CondLine(&CondLine,0,"%s", ExtraCondTab[maxi]);
          written = append_mummer_strictness(Black,&CondLine,written);
          break;

        case blmin:
        case blcapt:
          written = append_mummer_strictness(Black,&CondLine,written);
          break;

        case whmax:
          if (ExtraCondFlag[maxi])
            written = append_to_CondLine(&CondLine,0,"%s", ExtraCondTab[maxi]);
          written = append_mummer_strictness(White,&CondLine,written);
          break;

        case whmin:
        case whcapt:
          written = append_mummer_strictness(White,&CondLine,written);
          break;

        case blackultraschachzwang:
        case whiteultraschachzwang:
          if (ExtraCondFlag[ultraschachzwang])
            written = append_to_CondLine(&CondLine,0, "%s", ExtraCondTab[ultraschachzwang]);
          break;

        case sentinelles:
          if (sentinelles_is_para)
            written = append_to_CondLine(&CondLine,0,"Para%s",CondTab[cond]);
          if (sentinelle_walk == BerolinaPawn)
            written += append_to_CondLine(&CondLine,written," %s","Berolina");
          if (sentinelles_pawn_mode==sentinelles_pawn_adverse)
            written += append_to_CondLine(&CondLine,written,"  %s",SentinellesVariantTypeString[UserLanguage][SentinellesVariantPionAdverse]);
          if (sentinelles_pawn_mode==sentinelles_pawn_neutre)
            written += append_to_CondLine(&CondLine,written,"  %s",SentinellesVariantTypeString[UserLanguage][SentinellesVariantPionNeutral]);
          if (sentinelles_max_nr_pawns[Black] !=8 || sentinelles_max_nr_pawns[White] != 8)
          {
            written += append_to_CondLine(&CondLine,written," %u",sentinelles_max_nr_pawns[White]);
            written += append_to_CondLine(&CondLine,written,"/%u",sentinelles_max_nr_pawns[Black]);
          }
          if (sentinelles_max_nr_pawns_total != 16)
            written += append_to_CondLine(&CondLine,written," //%u", sentinelles_max_nr_pawns_total);
          break;

        case koeko:
        case antikoeko:
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
          break;
        }

        case BGL:
        {
          char buf[12];

          WriteBGLNumber(buf, BGL_values[White]);
          written += append_to_CondLine(&CondLine,written," %s", buf);

          if (!BGL_global)
          {
            WriteBGLNumber(buf, BGL_values[Black]);
            written += append_to_CondLine(&CondLine,written,"/%s",buf);
          }
          break;
        }

        case kobulkings:
        {
          if (!kobul_who[White])
            written += append_to_CondLine(&CondLine,written," %s","Black");
          if (!kobul_who[Black])
            written += append_to_CondLine(&CondLine,written," %s","White");
          break;
        }

        case whvault_king:
        case vault_king:
        {
          if (nr_king_vaulters[White]!=1 || king_vaulters[White][0]!=EquiHopper)
            written += WriteWalks(CondLine+written,king_vaulters[White],nr_king_vaulters[White]);
          if (vaulting_kings_transmuting[White])
          {
            written += append_to_CondLine(&CondLine,written,"%c",'-');
            written += append_to_CondLine_walk(&CondLine,written,King);
          }
          break;
        }
        case blvault_king:
        {
          if (nr_king_vaulters[Black]!=1 || king_vaulters[Black][0]!=EquiHopper)
            written += WriteWalks(CondLine+written,king_vaulters[Black],nr_king_vaulters[Black]);
          if (vaulting_kings_transmuting[Black])
          {
            written += append_to_CondLine(&CondLine,written,"%c",'-');
            written += append_to_CondLine_walk(&CondLine,written,King);
          }
          break;
        }

        case promotiononly:
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

          if (strlen(CondLine) <= strlen(CondTab[promotiononly]))
            /* due to zeroposition, where pieces_pawns_promotee_sequence is not */
            /* set (it's set in verifieposition), I suppress  */
            /* output of promotiononly for now.  */
            continue;
          else
            break;
        }

        case football:
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
          if (strlen(CondLine) <= strlen(CondTab[football]))
            /* due to zeroposition, where pieces_pawns_promotee_sequence is not */
            /* set (it's set in verifieposition), I suppress  */
            /* output of promotiononly for now.  */
            continue;
          else
            break;
        }

        case imitators:
        {
          unsigned int imi_idx;
          for (imi_idx = 0; imi_idx<number_of_imitators; imi_idx++)
            written += append_to_CondLine_square(&CondLine,written,isquare[imi_idx]);
          break;
        }

        case noiprom:
          if (!CondFlag[imitators])
            continue;
          break;

        case magicsquare:
        {
          square i;
          if (magic_square_type==ConditionType2)
            written += append_to_CondLine(&CondLine,written, " %s", ConditionNumberedVariantTypeString[UserLanguage][ConditionType2]);

          for (i= square_a1; i <= square_h8; i++) {
            if (TSTFLAG(sq_spec[i], MagicSq))
              written += append_to_CondLine_square(&CondLine,written,i);
          }
          break;
        }

        case whforsqu:
        case whconforsqu:
        {
          square  i;
          for (i= square_a1; i <= square_h8; i++) {
            if (TSTFLAG(sq_spec[i], WhForcedSq))
              written += append_to_CondLine_square(&CondLine,written,i);
          }
          break;
        }
        case blforsqu:
        case blconforsqu:
        {
          square  i;
          for (i= square_a1; i <= square_h8; i++) {
            if (TSTFLAG(sq_spec[i], BlForcedSq))
              written += append_to_CondLine_square(&CondLine,written,i);
          }
          break;
        }

        case whprom_sq:
        {
          square  i;
          for (i= square_a1; i <= square_h8; i++) {
            if (TSTFLAG(sq_spec[i], WhPromSq))
              written += append_to_CondLine_square(&CondLine,written,i);
          }
          break;
        }
        case blprom_sq:
        {
          square  i;
          for (i= square_a1; i <= square_h8; i++) {
            if (TSTFLAG(sq_spec[i], BlPromSq))
              written += append_to_CondLine_square(&CondLine,written,i);
          }
          break;
        }

        case blroyalsq:
          written += append_to_CondLine_square(&CondLine,written,royal_square[Black]);
          break;
        case whroyalsq:
          written += append_to_CondLine_square(&CondLine,written,royal_square[White]);
          break;

        case wormholes:
        {
          square i;
          for (i = square_a1; i<=square_h8; ++i)
            if (TSTFLAG(sq_spec[i],Wormhole))
              written += append_to_CondLine_square(&CondLine,written,i);
          break;
        }

        case madras:
          if (madrasi_is_rex_inclusive)
            written += append_to_CondLine(&CondLine,written," %s",CondTab[rexincl]);
          break;

        case geneva:
          written = append_circe_variants(&geneva_variant,&CondLine,written,CirceVariantRexExclusive);
          break;

        case immun:
          written = append_circe_variants(&immune_variant,&CondLine,written,CirceVariantRexExclusive);
          break;

        case circe:
          written = append_circe_variants(&circe_variant,&CondLine,written,CirceVariantRexExclusive);
          break;

        case messigny:
          if (messigny_rex_exclusive)
            written += append_to_CondLine(&CondLine,written," %s",CondTab[rexexcl]);
          break;

        case woozles:
        case biwoozles:
        case heffalumps:
        case biheffalumps:
          if (woozles_rex_exclusive)
            written += append_to_CondLine(&CondLine,written," %s",CondTab[rexexcl]);
          break;

        case protean:
          if (protean_is_rex_exclusive)
            written += append_to_CondLine(&CondLine,written," %s",CondTab[rexexcl]);
          break;

        case chameleonsequence:
        case chamchess:
          if (!chameleon_is_squence_implicit)
            written += append_to_CondLine_chameleon_sequence(&CondLine,written,
                                                             chameleon_walk_sequence);
          break;

        case isardam:
          if (isardam_variant==ConditionTypeB)
            written += append_to_CondLine(&CondLine,written,"    %s",ConditionLetteredVariantTypeString[UserLanguage][ConditionTypeB]);
          break;

        case annan:
          switch (annan_type)
          {
            case ConditionTypeA:
              break;
            case ConditionTypeB:
              written += append_to_CondLine(&CondLine,written,"    %s",ConditionLetteredVariantTypeString[UserLanguage][ConditionTypeB]);
              break;
            case ConditionTypeC:
              written += append_to_CondLine(&CondLine,written,"    %s",ConditionLetteredVariantTypeString[UserLanguage][ConditionTypeC]);
              break;
            case ConditionTypeD:
              written += append_to_CondLine(&CondLine,written,"    %s",ConditionLetteredVariantTypeString[UserLanguage][ConditionTypeD]);
              break;

            default:
              assert(0);
              break;
          }
          break;

        case gridchess:
          if (OptFlag[suppressgrid])
            switch (grid_type)
            {
              case grid_normal:
                /* nothing */
                break;
              case grid_vertical_shift:
                written += append_to_CondLine(&CondLine,written,"  %s",GridVariantTypeString[UserLanguage][GridVariantShiftRank]);
                break;
              case grid_horizontal_shift:
                written += append_to_CondLine(&CondLine,written,"  %s",GridVariantTypeString[UserLanguage][GridVariantShiftFile]);
                break;
              case grid_diagonal_shift:
                written += append_to_CondLine(&CondLine,written,"  %s",GridVariantTypeString[UserLanguage][GridVariantShiftRankFile]);
                break;
              case grid_orthogonal_lines:
                written += append_to_CondLine(&CondLine,written,"  %s",GridVariantTypeString[UserLanguage][GridVariantOrthogonal]);
                /* to do - write lines */
                break;
              case grid_irregular:
                written += append_to_CondLine(&CondLine,written,"  %s",GridVariantTypeString[UserLanguage][GridVariantIrregular]);
                /* to do - write squares */
                break;
            }
          break;

        case white_oscillatingKs:
          if (OscillatingKings[White]==ConditionTypeB)
            written += append_to_CondLine(&CondLine,written,"    %s",ConditionLetteredVariantTypeString[UserLanguage][ConditionTypeB]);
          if (OscillatingKings[White]==ConditionTypeC && !CondFlag[swappingkings])
            written += append_to_CondLine(&CondLine,written,"  %s",ConditionLetteredVariantTypeString[UserLanguage][ConditionTypeC]);
          break;

        case black_oscillatingKs:
          if (OscillatingKings[Black]==ConditionTypeB)
            written += append_to_CondLine(&CondLine,written,"    %s",ConditionLetteredVariantTypeString[UserLanguage][ConditionTypeB]);
          if (OscillatingKings[Black]==ConditionTypeC && !CondFlag[swappingkings])
            written += append_to_CondLine(&CondLine,written,"  %s",ConditionLetteredVariantTypeString[UserLanguage][ConditionTypeC]);
          break;

        case singlebox:
          if (SingleBoxType==ConditionType1)
            written += append_to_CondLine(&CondLine,written,"    %s",ConditionNumberedVariantTypeString[UserLanguage][ConditionType1]);
          if (SingleBoxType==ConditionType2)
            written += append_to_CondLine(&CondLine,written,"    %s",ConditionNumberedVariantTypeString[UserLanguage][ConditionType2]);
          if (SingleBoxType==ConditionType3)
            written += append_to_CondLine(&CondLine,written,"    %s",ConditionNumberedVariantTypeString[UserLanguage][ConditionType3]);
          break;

        case republican:
          if (RepublicanType==ConditionType1)
            written += append_to_CondLine(&CondLine,written,"    %s",ConditionNumberedVariantTypeString[UserLanguage][ConditionType1]);
          if (RepublicanType==ConditionType2)
            written += append_to_CondLine(&CondLine,written,"    %s",ConditionNumberedVariantTypeString[UserLanguage][ConditionType2]);
          break;

        case SAT:
        case strictSAT:
          if (SAT_max_nr_allowed_flights[White]!=1
              || SAT_max_nr_allowed_flights[Black]!=1)
          {
            char const roman[][9] = {"","I","II","III","IV","V","VI","VII","VIII"};
            written += append_to_CondLine(&CondLine,written," %s", roman[SAT_max_nr_allowed_flights[White]-1]);
            if (SAT_max_nr_allowed_flights[White] != SAT_max_nr_allowed_flights[Black])
              written += append_to_CondLine(&CondLine,written,"/%s",roman[SAT_max_nr_allowed_flights[Black]-1]);
          }
          break;

        case anticirce:
          written = append_circe_variants(&anticirce_variant,&CondLine,written,CirceVariantRexInclusive);
          break;

        default:
          break;
      }

      (*WriteCondition)(CondLine,!result);

      result = true;
    }

  return result;
}
