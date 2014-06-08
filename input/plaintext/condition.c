#include "input/plaintext/condition.h"
#include "input/plaintext/pieces.h"
#include "output/output.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/message.h"
#include "pieces/pieces.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "pieces/attributes/chameleon.h"
#include "pieces/attributes/neutral/neutral.h"
#include "conditions/annan.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/cheylan.h"
#include "conditions/bgl.h"
#include "conditions/circe/april.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/reborn_piece.h"
#include "conditions/circe/rebirth_square.h"
#include "conditions/circe/relevant_side.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/circe/chameleon.h"
#include "conditions/circe/assassin.h"
#include "conditions/circe/parrain.h"
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
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/phantom.h"
#include "conditions/messigny.h"
#include "conditions/oscillating_kings.h"
#include "conditions/protean.h"
#include "conditions/republican.h"
#include "conditions/sat.h"
#include "conditions/sentinelles.h"
#include "conditions/singlebox/type1.h"
#include "conditions/transmuting_kings/vaulting_kings.h"
#include "conditions/woozles.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/castling.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *ParseSquareLastCapture(char *tok)
{
  retro_capture.on = SquareNum(tok[0],tok[1]);
  if (retro_capture.on==initsquare || tok[2]!=0)
  {
    ErrorMsg(WrongSquareList);
    return tok;
  }
  else
    return ReadNextTokStr();
}

static char *ParsePieceWalkAndSquareLastCapture(char *tok)
{
  tok = ParsePieceName(tok,&retro_capture.walk);

  if (retro_capture.walk>=King)
  {
    if (tok[0]==0)
      tok = ReadNextTokStr();
    tok = ParseSquareLastCapture(tok);
  }
  else
  {
    IoErrorMsg(WrongPieceName,0);
    tok = ReadNextTokStr();
  }

  return tok;
}

static char *ParseLastCapturedPiece(char *tok)
{
  boolean const is_colour_mandatory = true;

  retro_capture.walk = Empty;
  retro_capture.flags = ParseColour(tok,is_colour_mandatory);

  if (retro_capture.flags>0)
  {
    if (is_piece_neutral(retro_capture.flags))
      SETFLAGMASK(some_pieces_flags,NeutralMask);

    {
      Flags nonColourFlags = 0;
      tok = ParsePieceFlags(&nonColourFlags);
      retro_capture.flags |= nonColourFlags;
      some_pieces_flags |= nonColourFlags;
    }

    tok = ParsePieceWalkAndSquareLastCapture(tok);
  }

  return tok;
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

static char *ParseRexIncl(char *tok, boolean *is_rexincl, CirceVariantType what)
{
  if (what==GetUniqIndex(CirceVariantCount,CirceVariantTypeTab,tok))
  {
    *is_rexincl = what==CirceVariantRexInclusive;
    tok = ReadNextTokStr();
  }

  return tok;
}

static char *ReadWalks(char *tok,
                       boolean (*walks)[nr_piece_walks],
                       unsigned int *nr_walks_read)
{
  *nr_walks_read = 0;

  {
    piece_walk_type p;
    for (p = Empty; p!=nr_piece_walks; ++p)
      (*walks)[p] = false;
  }

  while (true)
  {
    piece_walk_type walk;
    tok = ParseSingleWalk(tok,&walk);

    TraceWalk(walk);TraceValue("%s",tok);TraceEOL();

    if (walk==nr_piece_walks)
      break;
    else
    {
      ++*nr_walks_read;
      (*walks)[walk] = true;
    }
  }

  return tok;
}

static boolean handle_chameleon_reborn_piece(twin_number_type *is_explicit,
                                             chameleon_sequence_type* sequence,
                                             piece_walk_type from, piece_walk_type to,
                                             char const *tok)
{
  boolean result;

  if (to==Empty)
  {
    IoErrorMsg(WrongPieceName,0);
    result = false;
  }
  else
  {
    if (from!=Empty)
      chameleon_set_successor_walk_explicit(is_explicit,sequence,from,to);
    result = true;
  }

  return result;
}

static char *ReadChameleonSequence(char *tok,
                                   twin_number_type *is_explicit,
                                   chameleon_sequence_type* sequence)
{
  piece_walk_type from = Empty;

  while (true)
  {
    piece_walk_type to;
    tok = ParseSingleWalk(tok,&to);

    if (to==nr_piece_walks)
      break;
    else if (handle_chameleon_reborn_piece(is_explicit,sequence,from,to,tok))
      from = to;
    else
      break;
  }

  return tok;
}

static char *ParseCirceVariants(circe_variant_type *variant)
{
  char *tok = ReadNextTokStr();

  while (true)
  {
    unsigned int const index = GetUniqIndex(CirceVariantCount,CirceVariantTypeTab,tok);

    if (index==CirceVariantCount)
      break;
    else if (index>CirceVariantCount)
    {
      IoErrorMsg(CondNotUniq,0);
      break;
    }
    else
    {
      tok = ReadNextTokStr();

      switch (index)
      {
        case CirceVariantRexInclusive:
          variant->is_rex_inclusive = true;
          break;

        case CirceVariantRexExclusive:
          variant->is_rex_inclusive = false;
          break;

        case CirceVariantMirror:
          if (!circe_override_relevant_side_overrider(variant,circe_relevant_side_overrider_mirror))
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantDiametral:
          if (!circe_override_rebirth_square_adapter(variant,circe_rebirth_square_adapter_diametral))
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantVerticalMirror:
          if (!circe_override_rebirth_square_adapter(variant,circe_rebirth_square_adapter_verticalmirror))
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantAssassin:
          variant->on_occupied_rebirth_square = circe_on_occupied_rebirth_square_assassinate;
          break;

        case CirceVariantClone:
          if (!circe_override_reborn_walk_adapter(variant,circe_reborn_walk_adapter_clone))
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantEinstein:
          if (!circe_override_reborn_walk_adapter(variant,circe_reborn_walk_adapter_einstein))
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantReverseEinstein:
          if (!circe_override_reborn_walk_adapter(variant,circe_reborn_walk_adapter_reversaleinstein))
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantChameleon:
          if (circe_override_reborn_walk_adapter(variant,circe_reborn_walk_adapter_chameleon))
            tok = ReadChameleonSequence(tok,
                                        &variant->chameleon_is_walk_squence_explicit,
                                        &variant->chameleon_walk_sequence);
          else
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantTurncoats:
          variant->is_turncoat = true;
          break;

        case CirceVariantCouscous:
          variant->actual_relevant_piece = 1-variant->default_relevant_piece;
          variant->is_promotion_possible = true;
          break;

        case CirceVariantLastMove:
          variant->relevant_capture = circe_relevant_capture_lastmove;
          break;

        case CirceVariantEquipollents:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_equipollents))
            variant->is_promotion_possible = true;
          else
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantParrain:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_equipollents))
          {
            variant->relevant_capture = circe_relevant_capture_lastmove;
            variant->is_promotion_possible = true;
          }
          else
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantContraParrain:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_equipollents)
              && circe_override_relevant_side_overrider(variant,circe_relevant_side_overrider_mirror))
          {
            variant->relevant_capture = circe_relevant_capture_lastmove;
            variant->is_promotion_possible = true;
          }
          else
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantCage:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_cage))
          {
            variant->is_promotion_possible = true;
            variant->rebirth_reason = move_effect_reason_rebirth_choice;
          }
          else
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantRank:
          if (!circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_rank)
              || !circe_override_rebirth_square_adapter(variant,circe_rebirth_square_adapter_rank)
              || !circe_override_relevant_side_overrider(variant,circe_relevant_side_overrider_rank))
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantFile:
          if (!circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_file))
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantSymmetry:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_symmetry))
            variant->is_promotion_possible = true;
          else
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantDiagramm:
          if (!circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_diagram))
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantPWC:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_pwc))
            variant->is_promotion_possible = true;
          else
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantAntipodes:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_antipodes))
            variant->is_promotion_possible = true;
          else
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantSuper:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_super))
          {
            variant->is_promotion_possible = true;
            variant->rebirth_reason = move_effect_reason_rebirth_choice;
          }
          else
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantTakeAndMake:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_take_and_make))
          {
            variant->is_promotion_possible = true;
            variant->rebirth_reason = move_effect_reason_rebirth_choice;
          }
          else
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantApril:
        {
          unsigned int nr_walks_read;
          tok = ReadWalks(tok,&variant->is_walk_affected,&nr_walks_read);
          if (nr_walks_read==0)
            IoErrorMsg(WrongPieceName,0);
          else
          {
            variant->is_restricted_to_walks = true;
            variant->is_promotion_possible = true;
            variant->rebirth_reason = move_effect_reason_rebirth_choice;
          }
          break;
        }

        case CirceVariantFrischauf:
          if (!circe_override_rebirth_square_adapter(variant,circe_rebirth_square_adapter_frischauf))
            IoErrorMsg(NonsenseCombination,0);
          break;

        case CirceVariantCalvet:
          variant->anticirce_type = anticirce_type_calvet;
          break;

        case CirceVariantCheylan:
          variant->anticirce_type = anticirce_type_cheylan;
          break;

        case CirceVariantStrict:
          variant->on_occupied_rebirth_square = circe_on_occupied_rebirth_square_strict;
          break;

        case CirceVariantRelaxed:
          variant->on_occupied_rebirth_square = circe_on_occupied_rebirth_square_relaxed;
          break;

        case CirceVariantVolcanic:
          variant->on_occupied_rebirth_square = circe_on_occupied_rebirth_square_volcanic;
          break;

        case CirceVariantParachute:
          variant->on_occupied_rebirth_square = circe_on_occupied_rebirth_square_parachute;
          break;
      }
    }
  }

  return tok;
}

char *ReadSquares(SquareListContext context)
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
          being_solved.isquare[nr_squares_read] = sq;
          break;

        case ReadHoles:
          block_square(sq);
          break;

        case ReadEpSquares:
          if (en_passant_nr_retro_squares==en_passant_retro_capacity)
            Message(TooManyEpKeySquares);
          else
            en_passant_retro_squares[en_passant_nr_retro_squares++] = sq;
          break;

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
    being_solved.number_of_imitators = nr_squares_read;

  return tok;
}

static char *ParseKobulSides(boolean (*variant)[nr_sides])
{
  char *tok = ReadNextTokStr();

  do
  {
    KobulVariantType const type = GetUniqIndex(KobulVariantCount,KobulVariantTypeTab,tok);

    if (type>KobulVariantCount)
      IoErrorMsg(CondNotUniq,0);
    else if (type==KobulWhiteOnly)
      (*variant)[Black] = false;
    else if (type==KobulBlackOnly)
      (*variant)[White] = false;
    else
      break;

    tok = ReadNextTokStr();
  } while (tok);

  return tok;
}

static char *ParseMaximumPawn(unsigned int *result,
                              unsigned int defaultVal,
                              unsigned int boundary)
{
  char *tok = ReadNextTokStr();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",defaultVal);
  TraceFunctionParam("%u",boundary);
  TraceFunctionParamListEnd();

  {
    char *end;
    unsigned long tmp = strtoul(tok,&end,10);
    if (tok==end || tmp>boundary)
      *result = defaultVal;
    else
    {
      *result = tmp;
      tok = ReadNextTokStr();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseSentinellesVariants(void)
{
  char *tok;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  tok = ReadNextTokStr();

  sentinelles_pawn_mode = sentinelles_pawn_propre;

  do
  {
    SentinellesVariantType const type = GetUniqIndex(SentinellesVariantCount,SentinellesVariantTypeTab,tok);

    if (type>SentinellesVariantCount)
      IoErrorMsg(CondNotUniq,0);
    else if (type==SentinellesVariantPionAdverse)
    {
      sentinelles_pawn_mode = sentinelles_pawn_adverse;
      tok = ReadNextTokStr();
    }
    else if (type==SentinellesVariantPionNeutral)
    {
      sentinelles_pawn_mode = sentinelles_pawn_neutre;
      tok = ReadNextTokStr();
    }
    else if (type==SentinellesVariantPionNoirMaximum)
      tok = ParseMaximumPawn(&sentinelles_max_nr_pawns[Black],8,64);
    else if (type==SentinellesVariantPionBlancMaximum)
      tok = ParseMaximumPawn(&sentinelles_max_nr_pawns[White],8,64);
    else if (type==SentinellesVariantPionTotalMaximum)
      tok = ParseMaximumPawn(&sentinelles_max_nr_pawns_total,16,64);
    else if (type==SentinellesVariantPara)
    {
      sentinelles_is_para = true;
      tok = ReadNextTokStr();
    }
    else if (type==SentinellesVariantBerolina)
    {
      sentinelle_walk = BerolinaPawn;
      tok = ReadNextTokStr();
    }
    else
      break;
  } while (tok);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseGridVariant(void)
{
  char *tok = ReadNextTokStr();

  do
  {
    GridVariantType type = GetUniqIndex(GridVariantCount,GridVariantTypeTab,tok);

    if (type==GridVariantCount)
      break;
    else if (type>GridVariantCount)
      IoErrorMsg(CondNotUniq,0);
    else if (type==GridVariantShiftRank)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 8)/2)+4*((*bnp/24-7)/2)) << Grid;
      }
      grid_type = grid_vertical_shift;
    }
    else if (type==GridVariantShiftFile)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 7)/2)+5*((*bnp/24-8)/2)) << Grid;
      }
      grid_type = grid_horizontal_shift;
    }
    else if (type==GridVariantShiftRankFile)
    {
      square const *bnp;
      for (bnp= boardnum; *bnp; bnp++)
      {
        ClearGridNum(*bnp);
        sq_spec[*bnp] += (((*bnp%24 - 7)/2)+5*((*bnp/24-7)/2)) << Grid;
      }
      grid_type = grid_diagonal_shift;
    }
    else if (type==GridVariantOrthogonal)
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
    else if (type==GridVariantIrregular)
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
    else if (type==GridVariantExtraGridLines)
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
    else
      return tok;

    tok = ReadNextTokStr();
  } while (tok);

  return tok;
}

static nocontactfunc_t *nocontactfunc_parsed;

static char *ParseKoekoVariant(void)
{
  char *tok = ReadNextTokStr();

  unsigned int const type = GetUniqIndex(1,KoekoVariantTypeTab,tok);

  if (type==1)
  {
    /* nothing */
  }
  else if (type>1)
    IoErrorMsg(CondNotUniq,0);
  else
  {
    piece_walk_type tmp_piece;
    tok = ParseSingleWalk(ReadNextTokStr(),&tmp_piece);

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

  return tok;
}

static char *ParseLetteredType(ConditionLetteredVariantType *variant, ConditionLetteredVariantType max_letter)
{
  char *tok = ReadNextTokStr();
  ConditionLetteredVariantType const type_read = GetUniqIndex(ConditionLetteredVariantTypeCount,ConditionLetteredVariantTypeTab,tok);

  *variant = ConditionTypeA;

  if (type_read==ConditionLetteredVariantTypeCount)
  {
   /* nothing */
  }
  else if (type_read>ConditionLetteredVariantTypeCount)
    IoErrorMsg(CondNotUniq,0);
  else
  {
    ConditionLetteredVariantType type;
    for (type = ConditionTypeA; type<=max_letter; ++type)
      if (type_read==type)
      {
        *variant = type;
        tok = ReadNextTokStr();
      }
  }

  return tok;
}

static char *ParseNumberedType(ConditionNumberedVariantType *variant,
                               ConditionNumberedVariantType default_number,
                               ConditionNumberedVariantType max_number)
{
  char *tok = ReadNextTokStr();
  ConditionNumberedVariantType const type_read = GetUniqIndex(ConditionNumberedVariantTypeCount,ConditionNumberedVariantTypeTab,tok);

  *variant = default_number;

  if (type_read==ConditionNumberedVariantTypeCount)
  {
    /* nothing */
  }
  else if (type_read>ConditionNumberedVariantTypeCount)
    IoErrorMsg(CondNotUniq,0);
  else
  {
    ConditionNumberedVariantType type;
    for (type = ConditionType1; type<=max_number; ++type)
      if (type_read==type)
      {
        *variant = type;
        tok = ReadNextTokStr();
      }
  }

  return tok;
}

static char *ParseAnticirceVariant(anticirce_type_type *variant)
{
  char *tok = ReadNextTokStr();

  anticirce_type_type const type = GetUniqIndex(anticirce_type_count,AntiCirceVariantTypeTab,tok);

  *variant = anticirce_type_calvet;

  if (type==anticirce_type_count)
    return tok;
  else if (type>anticirce_type_count)
  {
    IoErrorMsg(CondNotUniq,0);
    return tok;
  }
  else if (type==anticirce_type_cheylan || type==anticirce_type_calvet)
  {
    *variant = type;
    return ReadNextTokStr();
  }
  else
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
  char *tok = ReadNextTokStr();

  while (true)
  {
    piece_walk_type p;
    tok = ParseSingleWalk(tok,&p);

    if (p==nr_piece_walks)
    {
      if (GetUniqIndex(1,VaultingVariantTypeTab,tok)==0)
      {
        if (side!=Black)
          vaulting_kings_transmuting[White] = true;
        if (side!=White)
          vaulting_kings_transmuting[Black] = true;
      }
      else
        break;
    }
    else
    {
      if (side!=Black)
        append_king_vaulter(White,p);
      if (side!=White)
        append_king_vaulter(Black,p);
    }
  }

  return tok;
}

char *ParseCond(void)
{
  char *tok = ReadNextTokStr();
  unsigned int CondCnt = 0;

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
        magic_square_type = ConditionType1;
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
      case circediametral:
        CondFlag[circe] = true;
        circe_variant.rebirth_square_adapter = circe_rebirth_square_adapter_diametral;
        break;
      case circemirrorvertical:
        circe_variant.rebirth_square_adapter = circe_rebirth_square_adapter_verticalmirror;
        CondFlag[circe] = true;
        break;
      case circechameleon:
        CondFlag[circe] = true;
        circe_variant.reborn_walk_adapter = circe_reborn_walk_adapter_chameleon;
        break;
      case circeturncoats:
        CondFlag[circe] = true;
        circe_variant.is_turncoat = true;
        break;
      case circerank:
        CondFlag[circe] = true;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_rank;
        circe_variant.rebirth_square_adapter = circe_rebirth_square_adapter_rank;
        circe_variant.relevant_side_overrider = circe_relevant_side_overrider_rank;
        break;
      case circediagramm:
        CondFlag[circe] = true;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_diagram;
        break;
      case circefile:
        CondFlag[circe] = true;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
        break;
      case circecouscous:
      case circecouscousmirror:
        CondFlag[circe] = true;
        circe_variant.is_promotion_possible= true;
        circe_variant.actual_relevant_piece = circe_relevant_piece_capturer;
        break;
      case circeequipollents:
        CondFlag[circe] = true;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_equipollents;
        circe_variant.is_promotion_possible= true;
        break;
      case circesymmetry:
        CondFlag[circe] = true;
        circe_variant.is_promotion_possible= true;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_symmetry;
        break;
      case pwc:
        CondFlag[circe] = true;
        circe_variant.is_promotion_possible= true;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_pwc;
        break;
      case circeantipoden:
        CondFlag[circe] = true;
        circe_variant.is_promotion_possible= true;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_antipodes;
        break;
      case circetakeandmake:
        CondFlag[circe] = true;
        circe_variant.is_promotion_possible= true;
        circe_variant.rebirth_reason = move_effect_reason_rebirth_choice;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_take_and_make;
        break;
      case supercirce:
        CondFlag[circe] = true;
        circe_variant.is_promotion_possible= true;
        circe_variant.rebirth_reason = move_effect_reason_rebirth_choice;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_super;
        break;
      case circecage:
        CondFlag[circe] = true;
        circe_variant.is_promotion_possible= true;
        circe_variant.rebirth_reason = move_effect_reason_rebirth_choice;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_cage;
        break;
      case circeparrain:
        CondFlag[circe] = true;
        circe_variant.relevant_capture = circe_relevant_capture_lastmove;
        circe_variant.is_promotion_possible= true;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_equipollents;
        break;
      case circecontraparrain:
        CondFlag[circe] = true;
        circe_variant.relevant_capture = circe_relevant_capture_lastmove;
        circe_variant.is_promotion_possible= true;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_equipollents;
        circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        break;

      case anticirce:
        break;
      case antimirror:
        CondFlag[anticirce] = true;
        anticirce_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        break;
      case anticlonecirce:
        CondFlag[anticirce] = true;
        anticirce_variant.reborn_walk_adapter = circe_reborn_walk_adapter_clone;
        break;
      case antiequipollents:
        CondFlag[anticirce] = true;
        anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_equipollents;
        anticirce_variant.is_promotion_possible = true;
        break;
      case antiantipoden:
        CondFlag[anticirce] = true;
        anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_antipodes;
        anticirce_variant.is_promotion_possible = true;
        break;
      case antimirrorfile:
        CondFlag[anticirce] = true;
        anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
        anticirce_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        break;
      case antisymmetrie:
        CondFlag[anticirce] = true;
        anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_symmetry;
        anticirce_variant.is_promotion_possible = true;
        break;
      case antifile:
        CondFlag[anticirce] = true;
        anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
        break;
      case antidiagramm:
        CondFlag[anticirce] = true;
        anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_diagram;
        break;
      case antisuper:
        CondFlag[anticirce] = true;
        anticirce_variant.rebirth_reason = move_effect_reason_rebirth_choice;
        anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_super;
        anticirce_variant.is_promotion_possible = true;
        break;

        /* different types of immunchess */
      case immunmirror:
        CondFlag[immun] = true;
        immune_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        break;
      case immunfile:
        CondFlag[immun] = true;
        immune_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
        break;
      case immundiagramm:
        CondFlag[immun] = true;
        immune_variant.determine_rebirth_square = circe_determine_rebirth_square_diagram;
        break;
      case immunmirrorfile:
        CondFlag[immun] = true;
        immune_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
        immune_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        break;
      case immunsymmetry:
        CondFlag[immun] = true;
        immune_variant.determine_rebirth_square = circe_determine_rebirth_square_symmetry;
        break;
      case immunantipoden:
        CondFlag[immun] = true;
        immune_variant.determine_rebirth_square = circe_determine_rebirth_square_antipodes;
        break;
      case immunequipollents:
        CondFlag[immun] = true;
        immune_variant.determine_rebirth_square = circe_determine_rebirth_square_equipollents;
        break;

        /* different types of mars circe */
      case marsmirror:
        CondFlag[mars] = true;
        marscirce_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        break;
      case antimarsmirror:
        CondFlag[antimars] = true;
        antimars_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        break;
      case antimarsantipodean:
        CondFlag[antimars] = true;
        antimars_variant.determine_rebirth_square = circe_determine_rebirth_square_antipodes;
        break;

      case whprom_sq:
        ReadSquares(WhPromSq);
        break;
      case blprom_sq:
        ReadSquares(BlPromSq);
        break;

      default:
        break;
    }

    switch (indexx)
    {
      case messigny:
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&messigny_rex_inclusive, CirceVariantRexExclusive);
        break;
      case woozles:
      case biwoozles:
      case heffalumps:
      case biheffalumps:
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&woozles_rex_inclusive, CirceVariantRexExclusive);
        break;
      case immun:
        tok = ParseCirceVariants(&immune_variant);
        break;
      case immunmirror:
      case immundiagramm:
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&immune_variant.is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case circechameleon:
        tok = ReadNextTokStr();
        tok = ReadChameleonSequence(tok,
                                    &circe_variant.chameleon_is_walk_squence_explicit,
                                    &circe_variant.chameleon_walk_sequence);
        break;
      case chameleonsequence:
      case chamchess:
        tok = ReadNextTokStr();
        tok = ReadChameleonSequence(tok,
                                    &chameleon_is_squence_explicit,
                                    &chameleon_walk_sequence);
        break;
      case circemirror:
        CondFlag[circe] = true;
        circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case circefilemirror:
        CondFlag[circe] = true;
        circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        circe_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case circeclonemirror:
        CondFlag[circe] = true;
        circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        circe_variant.reborn_walk_adapter = circe_reborn_walk_adapter_clone;
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case circecouscousmirror:
        CondFlag[circecouscous] = true;
        circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case circedoubleagents:
        CondFlag[circe] = true;
        circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
        circe_variant.is_turncoat = true;
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case circeassassin:
        CondFlag[circe] = true;
        circe_variant.on_occupied_rebirth_square = circe_on_occupied_rebirth_square_assassinate;
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case circeclone:
        CondFlag[circe] = true;
        circe_variant.reborn_walk_adapter = circe_reborn_walk_adapter_clone;
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case frischauf:
        CondFlag[circe] = true;
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
        circe_variant.rebirth_square_adapter = circe_rebirth_square_adapter_frischauf;
        break;
      case circefile:
      case circeequipollents:
      case circediagramm:
      case circesymmetry:
      case circeantipoden:
      case circemirrorvertical:
      case circediametral:
      case circerank:
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case circe:
        tok = ParseCirceVariants(&circe_variant);
        break;
      case mars:
        tok = ParseCirceVariants(&marscirce_variant);
        break;
      case marsmirror:
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&marscirce_variant.is_rex_inclusive, CirceVariantRexExclusive);
        break;
      case antimars:
        tok = ParseCirceVariants(&antimars_variant);
        break;
      case antimarsantipodean:
      case antimarsmirror:
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&antimars_variant.is_rex_inclusive, CirceVariantRexExclusive);
        break;
      case protean:
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&protean_is_rex_inclusive, CirceVariantRexExclusive);
        break;
      case phantom:
        tok = ParseCirceVariants(&phantom_variant);
        break;
      case madras:
        tok = ReadNextTokStr();
        tok = ParseRexIncl(tok,&madrasi_is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case isardam:
        tok = ParseLetteredType(&isardam_variant,ConditionTypeB);
        tok = ParseRexIncl(tok,&madrasi_is_rex_inclusive, CirceVariantRexInclusive);
        break;
      case annan:
        tok = ParseLetteredType(&annan_type,ConditionTypeD);
        break;
      case kobulkings:
        kobul_who[White] = true;
        kobul_who[Black] = true;
        tok = ParseKobulSides(&kobul_who);
        break;
      case sentinelles:
        tok = ParseSentinellesVariants();
        break;

        /*****  exact-maxis  *****/
      case blmax:
        tok = ParseMummerStrictness(&mummer_strictness[Black]);
        break;
      case whmax:
        tok = ParseMummerStrictness(&mummer_strictness[White]);
        break;
      case blmin:
        tok = ParseMummerStrictness(&mummer_strictness[Black]);
        break;
      case whmin:
        tok = ParseMummerStrictness(&mummer_strictness[White]);
        break;
      case blcapt:
        tok = ParseMummerStrictness(&mummer_strictness[Black]);
        break;
      case whcapt:
        tok = ParseMummerStrictness(&mummer_strictness[White]);
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
      case anticirce:
        tok = ParseCirceVariants(&anticirce_variant);
        break;
      case antimirror:
      case antidiagramm:
      case antifile:
      case antisymmetrie:
      case antimirrorfile:
      case antiantipoden:
      case antiequipollents:
      case antisuper:
        tok = ParseAnticirceVariant(&anticirce_variant.anticirce_type);
        break;
      case singlebox:
        tok = ParseNumberedType(&SingleBoxType,ConditionType1,ConditionType3);
        break;
      case republican:
        tok = ParseNumberedType(&RepublicanType,ConditionType2,ConditionType2);
        break;
      case magicsquare:
        tok = ParseNumberedType(&magic_square_type,ConditionType1,ConditionType2);
        break;
      case promotiononly:
      {
        unsigned int nr_walks_read;
        tok = ReadNextTokStr();
        tok = ReadWalks(tok,&promonly,&nr_walks_read);
        if (nr_walks_read==0)
        {
          CondFlag[promotiononly] = false;
          IoErrorMsg(WrongPieceName,0);
        }
        break;
      }
      case football:
      {
        unsigned int nr_walks_read;
        football_are_substitutes_limited = false;
        tok = ReadNextTokStr();
        tok = ReadWalks(tok,&is_football_substitute,&nr_walks_read);
        football_are_substitutes_limited = nr_walks_read>0;
        init_football_substitutes();
        break;
      }
      case april:
      {
        unsigned int nr_walks_read;
        tok = ReadNextTokStr();
        tok = ReadWalks(tok,&circe_variant.is_walk_affected,&nr_walks_read);
        if (nr_walks_read==0)
        {
          CondFlag[april] = false;
          IoErrorMsg(WrongPieceName,0);
        }
        else
        {
          CondFlag[circe] = true;
          circe_variant.is_restricted_to_walks = true;
          circe_variant.is_promotion_possible= true;
          circe_variant.rebirth_reason = move_effect_reason_rebirth_choice;
          circe_variant.on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_strict;
          circe_variant.determine_rebirth_square = circe_determine_rebirth_square_super;
        }
        break;
      }
      case koeko:
        koeko_nocontact= &nokingcontact;
        nocontactfunc_parsed= &koeko_nocontact;
        tok = ParseKoekoVariant();
        break;
      case antikoeko:
        antikoeko_nocontact= nokingcontact;
        nocontactfunc_parsed= &antikoeko_nocontact;
        tok = ParseKoekoVariant();
        break;
      case white_oscillatingKs:
        tok = ParseLetteredType(&OscillatingKings[White],ConditionTypeC);
        break;
      case black_oscillatingKs:
        tok = ParseLetteredType(&OscillatingKings[Black],ConditionTypeC);
        break;
      case swappingkings:
        CondFlag[white_oscillatingKs]= true;
        OscillatingKings[White]= ConditionTypeC;
        CondFlag[black_oscillatingKs]= true;
        OscillatingKings[Black]= ConditionTypeC;
        tok = ReadNextTokStr();
        break;
      case SAT:
      case strictSAT:
      {
        char *ptr;
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
      }
      case BGL:
      {
        char *ptr;
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
      }
      case geneva:
        tok = ParseCirceVariants(&geneva_variant);
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
        tok = ParseGridVariant();
        break;
      case lastcapture:
        tok = ReadNextTokStr();
        retro_capture.on = initsquare;
        tok = ParseLastCapturedPiece(tok);
        CondFlag[lastcapture] = retro_capture.on!=initsquare;
        break;
      default:
        tok = ReadNextTokStr();
        break;
    }
  }

  if (CondCnt==0)
    IoErrorMsg(UnrecCondition,0);

  if (CondFlag[black_oscillatingKs] && OscillatingKings[White]==ConditionTypeC
      && CondFlag[white_oscillatingKs] && OscillatingKings[White]==ConditionTypeC)
    CondFlag[swappingkings] = true;

  return tok;
}

void InitCond(void)
{
  square const *bnp;
  square i, j;

  mummer_strictness[White] = mummer_strictness_none;
  mummer_strictness[Black] = mummer_strictness_none;

  circe_reset_variant(&circe_variant);
  anticirce_reset_variant(&anticirce_variant);
  immune_reset_variant(&immune_variant);
  geneva_reset_variant(&geneva_variant);
  marscirce_reset_variant(&marscirce_variant);
  marscirce_reset_variant(&antimars_variant);
  phantom_reset_variant(&phantom_variant);

  royal_square[White] = initsquare;
  royal_square[Black] = initsquare;

  sentinelles_is_para= false;
  madrasi_is_rex_inclusive = false;
  phantom_variant.is_rex_inclusive = false;
  messigny_rex_inclusive = true;
  woozles_rex_inclusive = true;
  protean_is_rex_inclusive = true;

  sentinelles_max_nr_pawns[Black] = 8;
  sentinelles_max_nr_pawns[White] = 8;
  sentinelles_max_nr_pawns_total = 16;
  sentinelle_walk = Pawn;

  grid_type = grid_normal;
  numgridlines = 0;

  {
    PieceIdType id;
    for (id = MinPieceId; id<=MaxPieceId; ++id)
      PiecePositionsInDiagram[id] = initsquare;
  }

  for (bnp= boardnum; *bnp; bnp++) {
    int const file= *bnp%onerow - nr_of_slack_files_left_of_board;
    int const row= *bnp/onerow - nr_of_slack_rows_below_board;

    CLEARFL(sq_spec[*bnp]);
    sq_num[*bnp]= (int)(bnp-boardnum);

    /* initialise sq_spec and set grid number */
    sq_spec[*bnp] += ((file/2)+4*(row/2)) << Grid;
    if (file!=0 && file!=nr_files_on_board-1
        && row!=0 && row!=nr_rows_on_board-1)
      SETFLAG(sq_spec[*bnp], NoEdgeSq);
  }

  for (i= square_a1; i < square_h8; i+= onerow)
  {
    if (i > square_a1)
      if (!TSTFLAG(sq_spec[i+dir_down], SqColor))
        SETFLAG(sq_spec[i], SqColor);
    for (j= i+1; j < i+nr_files_on_board; j++)
      if (!TSTFLAG(sq_spec[j+dir_left], SqColor))
        SETFLAG(sq_spec[j], SqColor);
  }

  for (i= 0; i < CondCount; ++i)
    CondFlag[i] = false;

  for (i= 0; i < ExtraCondCount; ++i)
    ExtraCondFlag[i] = false;

  being_solved.number_of_imitators = 0;

  koeko_nocontact= nokingcontact;
  antikoeko_nocontact= nokingcontact;
  OscillatingKings[White]= ConditionTypeA;
  OscillatingKings[Black]= ConditionTypeA;

  BGL_values[White] = BGL_infinity;
  BGL_values[Black] = BGL_infinity;
  BGL_global= false;

  calc_reflective_king[White] = false;
  calc_reflective_king[Black] = false;

  reset_king_vaulters();

  kobul_who[White] = false;
  kobul_who[Black] = false;
} /* InitCond */
