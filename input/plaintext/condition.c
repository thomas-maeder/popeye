#include "input/plaintext/condition.h"
#include "input/plaintext/pieces.h"
#include "input/plaintext/geometry/square.h"
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
#include "conditions/bolero.h"
#include "conditions/breton.h"
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
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static char *ParseSquareLastCapture(char *tok)
{
  tok = ParseSquare(tok,&retro_capture.on);
  if (retro_capture.on==initsquare || tok[0]!=0)
    output_plaintext_error_message(WrongSquareList);

  return ReadNextTokStr();
}

static char *ParsePieceWalkAndSquareLastCapture(char *tok)
{
  tok = ParsePieceWalk(tok,&retro_capture.walk);

  if (retro_capture.walk!=nr_piece_walks)
  {
    if (tok[0]==0)
      tok = ReadNextTokStr();
    tok = ParseSquareLastCapture(tok);
  }
  else
  {
    output_plaintext_input_error_message(WrongPieceName);
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

  *endptr = inptr;
  while (**endptr && strchr("0123456789.,-", **endptr))
    /* isdigit((unsigned char)**endptr) || **endptr == '.' || **endptr == ',' || **endptr == '-')) */
    (*endptr)++;

  {
    size_t const len = (size_t)(*endptr-inptr);
    assert(*endptr>=inptr);

    if (len>11)
      return BGL_infinity;
    else
    {
      char buf[12];
      strncpy(buf,inptr,len);
      buf[len]= '\0';

      if (len==1 && buf[0]=='-')
        return BGL_infinity;
      else
      {
        char* dpp;
        for (dpp = buf; *dpp; dpp++)
          if (*dpp==',')  /* allow 3,45 notation */
            *dpp = '.';

        for (dpp = buf; *dpp && *dpp!='.'; dpp++)
        {
        }

        {
          size_t const dp = len-(size_t)(dpp-buf);
          long int tmp;
          if (dp==0)
          {
            tmp = strtol(buf, NULL, 10);
            return (((tmp >= (LONG_MIN / 100)) && (tmp <= (LONG_MAX / 100))) ? (100 * tmp) : BGL_infinity);
          }
          else
          {
            while ((size_t)(dpp-buf)<len)
            {
              *dpp=*(dpp+1);
              dpp++;
            }

            for (dpp = buf; *dpp; dpp++)
              if (*dpp=='.')
                return BGL_infinity;  /* 2 d.p. characters */

            switch (dp) /* N.B> d.p. is part of count */
            {
              case 1 :
                tmp = strtol(buf, NULL, 10);
                return (((tmp >= (LONG_MIN / 100)) && (tmp <= (LONG_MAX / 100))) ? (100 * tmp) : BGL_infinity);
              case 2 :
                tmp = strtol(buf, NULL, 10);
                return (((tmp >= (LONG_MIN / 10)) && (tmp <= (LONG_MAX / 10))) ? (10 * tmp) : BGL_infinity);
              case 3 :
                return strtol(buf, NULL, 10);
              default :
                return BGL_infinity;
            }
          }
        }
      }
    }
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
    tok = ParsePieceWalkToken(tok,&walk);

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

static char *ReadChameleonSequence(char *tok,
                                   twin_id_type *is_explicit,
                                   chameleon_sequence_type *sequence)
{
  piece_walk_type first_from;
  tok = ParsePieceWalkToken(tok,&first_from);

  if (first_from!=nr_piece_walks)
  {
    piece_walk_type from = first_from;

    while (true)
    {
      piece_walk_type to;
      tok = ParsePieceWalkToken(tok,&to);

      if (to==nr_piece_walks)
      {
        if (from!=first_from)
          /* user input forgot to close sequence */
          chameleon_set_successor_walk_explicit(is_explicit,sequence,from,first_from);
        break;
      }
      else
      {
        chameleon_set_successor_walk_explicit(is_explicit,sequence,from,to);
        from = to;
      }
    }
  }

  return tok;
}

static char *ParseCirceVariants(char *tok, circe_variant_type *variant)
{
  while (true)
  {
    unsigned int const index = GetUniqIndex(CirceVariantCount,CirceVariantTypeTab,tok);

    if (index==CirceVariantCount)
      break;
    else if (index>CirceVariantCount)
    {
      output_plaintext_input_error_message(CondNotUniq);
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
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantDiametral:
          if (!circe_override_rebirth_square_adapter(variant,circe_rebirth_square_adapter_diametral))
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantVerticalMirror:
          if (!circe_override_rebirth_square_adapter(variant,circe_rebirth_square_adapter_verticalmirror))
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantAssassin:
          variant->on_occupied_rebirth_square = circe_on_occupied_rebirth_square_assassinate;
          break;

        case CirceVariantClone:
          if (!circe_override_reborn_walk_adapter(variant,circe_reborn_walk_adapter_clone))
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantEinstein:
          if (!circe_override_reborn_walk_adapter(variant,circe_reborn_walk_adapter_einstein))
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantReverseEinstein:
          if (!circe_override_reborn_walk_adapter(variant,circe_reborn_walk_adapter_reversaleinstein))
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantChameleon:
          if (circe_override_reborn_walk_adapter(variant,circe_reborn_walk_adapter_chameleon))
          {
            tok = ReadChameleonSequence(tok,
                                        &variant->explicit_chameleon_squence_set_in_twin,
                                        &variant->chameleon_walk_sequence);
            variant->is_chameleon_sequence_explicit = variant->explicit_chameleon_squence_set_in_twin==twin_id;
          }
          else
            output_plaintext_input_error_message(NonsenseCombination);
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
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantParrain:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_equipollents))
          {
            variant->relevant_capture = circe_relevant_capture_lastmove;
            variant->is_promotion_possible = true;
          }
          else
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantContraParrain:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_equipollents)
              && circe_override_relevant_side_overrider(variant,circe_relevant_side_overrider_mirror))
          {
            variant->relevant_capture = circe_relevant_capture_lastmove;
            variant->is_promotion_possible = true;
          }
          else
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantCage:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_cage))
          {
            variant->is_promotion_possible = true;
            variant->rebirth_reason = move_effect_reason_rebirth_choice;
          }
          else
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantRank:
          if (!circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_rank)
              || !circe_override_rebirth_square_adapter(variant,circe_rebirth_square_adapter_rank)
              || !circe_override_relevant_side_overrider(variant,circe_relevant_side_overrider_rank))
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantFile:
          if (!circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_file))
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantSymmetry:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_symmetry))
            variant->is_promotion_possible = true;
          else
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantVerticalSymmetry:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_vertical_symmetry))
            variant->is_promotion_possible = true;
          else
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantHorizontalSymmetry:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_horizontal_symmetry))
            variant->is_promotion_possible = false;
          else
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantDiagramm:
          if (!circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_diagram))
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantPWC:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_pwc))
            variant->is_promotion_possible = true;
          else
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantAntipodes:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_antipodes))
            variant->is_promotion_possible = true;
          else
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantSuper:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_super))
          {
            variant->is_promotion_possible = true;
            variant->rebirth_reason = move_effect_reason_rebirth_choice;
          }
          else
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantTakeAndMake:
          if (circe_override_determine_rebirth_square(variant,circe_determine_rebirth_square_take_and_make))
          {
            variant->is_promotion_possible = true;
            variant->rebirth_reason = move_effect_reason_rebirth_choice;
          }
          else
            output_plaintext_input_error_message(NonsenseCombination);
          break;

        case CirceVariantApril:
        {
          unsigned int nr_walks_read;
          tok = ReadWalks(tok,&variant->is_walk_affected,&nr_walks_read);
          if (nr_walks_read==0)
            output_plaintext_input_error_message(WrongPieceName);
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
            output_plaintext_input_error_message(NonsenseCombination);
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

        default:
          assert(0);
          break;
      }
    }
  }

  return tok;
}

static void HandleImitatorPosition(square pos, void *param)
{
  unsigned int * const number_of_imitators = param;

  being_solved.isquare[(*number_of_imitators)++] = pos;
}

static void HandleGridCell(square cell, void *param)
{
  unsigned int * const currentgridnum = param;

  ClearGridNum(cell);
  sq_spec(cell) += *currentgridnum << Grid;
}

static void HandleSquaresWithFlag(square sq, void *param)
{
  SquareFlags * const flag = param;
  SETFLAG(sq_spec(sq),*flag);
}

static char *ParseSquaresWithFlag(char *tok, SquareFlags flag)
{
  char * const squares_tok = tok;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  tok = ParseSquareList(squares_tok,&HandleSquaresWithFlag,&flag);
  if (tok==squares_tok)
    output_plaintext_input_error_message(MissngSquareList);
  else if (*tok!=0)
    output_plaintext_error_message(WrongSquareList);

  tok = ReadNextTokStr();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static void HandleHole(square sq, void *dummy)
{
  block_square(sq);
}

static char *ParseRoyalSquare(char *tok, Side side)
{
  square sq;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  tok = ParseSquare(tok,&sq);
  if (sq==initsquare || tok[0]!=0)
    output_plaintext_input_error_message(WrongSquareList);
  else
    royal_square[side] = sq;

  tok = ReadNextTokStr();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseKobulSides(char *tok, boolean (*variant)[nr_sides])
{
  do
  {
    KobulVariantType const type = GetUniqIndex(KobulVariantCount,KobulVariantTypeTab,tok);

    if (type>KobulVariantCount)
      output_plaintext_input_error_message(CondNotUniq);
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
    unsigned int tmp = (unsigned int)strtoul(tok,&end,10);
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

static char *ParseSentinellesVariants(char *tok)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  sentinelles_pawn_mode = sentinelles_pawn_propre;

  do
  {
    SentinellesVariantType const type = GetUniqIndex(SentinellesVariantCount,SentinellesVariantTypeTab,tok);

    if (type>SentinellesVariantCount)
      output_plaintext_input_error_message(CondNotUniq);
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

static char *ParseBretonVariants(char *tok)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  breton_mode = breton_propre;
  breton_chromaticity = breton_nonchromatic;

  do
  {
    BretonVariantType const type = GetUniqIndex(BretonVariantCount,BretonVariantTypeTab,tok);

    if (type>BretonVariantCount)
      output_plaintext_input_error_message(CondNotUniq);
    else if (type==BretonAdverse)
    {
      breton_mode = breton_adverse;
      tok = ReadNextTokStr();
    }
    else if (type==BretonChromatique)
    {
      breton_chromaticity = breton_chromatic;
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

/* parse the orthogonal grid lines from the current token
 * @param tok current token
 * @param file_numbers where to store file numbers
 * @param row_numbers where to store row numbers
 * @return position where parsing ends; end of token after successful parsing
 */
static char *ParseOrthogonalGridLines(char *tok,
                                      unsigned int file_numbers[],
                                      unsigned int row_numbers[])
{
  assert(*tok!=0); /* we are at the start of a token */

  {
    unsigned int i;
    for (i = 0; i<nr_files_on_board; i++)
      file_numbers[i] = 0;
  }

  {
    unsigned int i;
    for (i = 0; i<nr_rows_on_board; i++)
      row_numbers[i] = 0;
  }

  do
  {
    char const c = (char)tolower((unsigned char)*tok);
    if (c>='1' && c<='8')
    {
      unsigned int i;
      for (i = (unsigned int)(c-'1')+1; i<nr_rows_on_board; ++i)
        ++row_numbers[i];
    }
    else if (c>='a' && c<='h')
    {
      unsigned int i;
      for (i = (unsigned int)(c-'a')+1; i<nr_files_on_board; ++i)
        ++file_numbers[i];
    }
    else
      /* return position within token to indicate failure */
      break;
  } while (*++tok);

  return tok;
}

static void InitOrthogonalGridLines(unsigned int const file_numbers[],
                                    unsigned int const row_numbers[])
{
  unsigned int const rows_worth = file_numbers[nr_files_on_board-1]+1;

  square const *bnp;
  for (bnp = boardnum; *bnp; bnp++)
  {
    unsigned int const file = (unsigned int)(*bnp%onerow-nr_of_slack_files_left_of_board);
    unsigned int const rank = (unsigned int)(*bnp/onerow-nr_of_slack_rows_below_board);
    ClearGridNum(*bnp);
    sq_spec(*bnp) += (file_numbers[file]+rows_worth*row_numbers[rank]) << Grid;
  }
}

static char *ParseGridVariant(char *tok)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  do
  {
    GridVariantType const type = GetUniqIndex(GridVariantCount,GridVariantTypeTab,tok);
    TraceValue("%u",type);
    TraceEOL();

    if (type==GridVariantCount)
      break;
    else
    {
      tok = ReadNextTokStr();

      switch (type)
      {
        case GridVariantShiftRank:
        {
          square const *bnp;
          for (bnp = boardnum; *bnp; bnp++)
          {
            unsigned int const file = (unsigned int)(*bnp%onerow-nr_of_slack_files_left_of_board);
            unsigned int const row = (unsigned int)(*bnp/onerow-nr_of_slack_rows_below_board);
            unsigned int const rows_worth = nr_rows_on_board/2;
            ClearGridNum(*bnp);
            sq_spec(*bnp) += (file/2 + rows_worth*(row+1)/2)  <<  Grid;
          }
          grid_type = grid_vertical_shift;
          break;
        }
        case GridVariantShiftFile:
        {
          square const *bnp;
          for (bnp = boardnum; *bnp; bnp++)
          {
            unsigned int const file = (unsigned int)(*bnp%onerow-nr_of_slack_files_left_of_board);
            unsigned int const row = (unsigned int)(*bnp/onerow-nr_of_slack_rows_below_board);
            unsigned int const rows_worth = nr_rows_on_board/2 + 1;
            ClearGridNum(*bnp);
            sq_spec(*bnp) += ((file+1)/2 + rows_worth*(row/2))  <<  Grid;
          }
          grid_type = grid_horizontal_shift;
          break;
        }
        case GridVariantShiftRankFile:
        {
          square const *bnp;
          for (bnp = boardnum; *bnp; bnp++)
          {
            unsigned int const file = (unsigned int)(*bnp%onerow-nr_of_slack_files_left_of_board);
            unsigned int const rank = (unsigned int)(*bnp/onerow-nr_of_slack_rows_below_board);
            unsigned int const rows_worth = nr_rows_on_board/2 + 1;
            ClearGridNum(*bnp);
            sq_spec(*bnp) += ((file+1)/2 + rows_worth*(rank+1)/2) << Grid;
          }
          grid_type = grid_diagonal_shift;
          break;
        }
        case GridVariantOrthogonal:
        {
          unsigned int file_numbers[nr_files_on_board];
          unsigned int row_numbers[nr_rows_on_board];

          if (*ParseOrthogonalGridLines(tok,file_numbers,row_numbers)==0)
          {
            InitOrthogonalGridLines(file_numbers,row_numbers);
            grid_type = grid_orthogonal_lines;
            tok = ReadNextTokStr();
          }
          else
            output_plaintext_input_error_message(CondNotUniq);

          break;
        }
        case GridVariantIrregular:
        {
          unsigned int currentgridnum = 1;
          square const *bnp;
          for (bnp = boardnum; *bnp; bnp++)
            ClearGridNum(*bnp);
          grid_type = grid_irregular;

          do {
            char * const save_tok = tok;
            tok = ParseSquareList(tok,&HandleGridCell,&currentgridnum);
            if (*tok==0)
            {
              ++currentgridnum;
              tok = ReadNextTokStr();
            }
            else
            {
              tok = save_tok;
              break;
            }
          } while (true);

          break;
        }
        case GridVariantExtraGridLines:
        {
          IntialiseIrregularGridLines();

          while (true)
            if (strlen(tok)==4)
            {
              char const dir_char = (char)tolower((unsigned char)tok[0]);
              char const file_char = (char)tolower((unsigned char)tok[1]);
              char const row_char = tok[2];
              char const length_char = tok[3];

              if ((dir_char=='h' || dir_char=='v')
                  && (file_char>='a' && file_char<='h')
                  && (row_char>='1' && row_char<='8')
                  && (length_char>='1' && length_char<='8'))
              {
                unsigned int const file = (unsigned int)(file_char-'a');
                unsigned int const row = (unsigned int)(row_char-'1');
                unsigned int const length = (unsigned int)(length_char-'0');
                gridline_direction const dir = dir_char=='h' ? gridline_horizonal : gridline_vertical;

                if (PushIrregularGridLine(file,row,length,dir))
                  tok = ReadNextTokStr();
                else
                  break;
              }
              else
                break;
            }
            else
              break;

          break;
        }
        default:
          output_plaintext_input_error_message(CondNotUniq);
          break;
      }
    }
  } while (true);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static nocontactfunc_t *nocontactfunc_parsed;

static char *ParseKoekoVariant(char *tok)
{
  unsigned int const type = GetUniqIndex(1,KoekoVariantTypeTab,tok);

  if (type==1)
  {
    /* nothing */
  }
  else if (type>1)
    output_plaintext_input_error_message(CondNotUniq);
  else
  {
    piece_walk_type tmp_piece;
    tok = ParsePieceWalkToken(ReadNextTokStr(),&tmp_piece);

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
        output_plaintext_input_error_message(WrongPieceName);
        break;
    }
  }

  return tok;
}

static char *ParseLetteredType(char *tok,
                               ConditionLetteredVariantType *variant,
                               ConditionLetteredVariantType max_letter)
{
  ConditionLetteredVariantType const type_read = GetUniqIndex(ConditionLetteredVariantTypeCount,ConditionLetteredVariantTypeTab,tok);

  *variant = ConditionTypeA;

  if (type_read==ConditionLetteredVariantTypeCount)
  {
   /* nothing */
  }
  else if (type_read>ConditionLetteredVariantTypeCount)
    output_plaintext_input_error_message(CondNotUniq);
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

static char *ParseNumberedType(char *tok,
                               ConditionNumberedVariantType *variant,
                               ConditionNumberedVariantType default_number,
                               ConditionNumberedVariantType max_number)
{
  ConditionNumberedVariantType const type_read = GetUniqIndex(ConditionNumberedVariantTypeCount,ConditionNumberedVariantTypeTab,tok);

  *variant = default_number;

  if (type_read==ConditionNumberedVariantTypeCount)
  {
    /* nothing */
  }
  else if (type_read>ConditionNumberedVariantTypeCount)
    output_plaintext_input_error_message(CondNotUniq);
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

static char *ParseAnticirceVariant(char *tok, anticirce_type_type *variant)
{
  anticirce_type_type const type = GetUniqIndex(anticirce_type_count,AntiCirceVariantTypeTab,tok);

  *variant = anticirce_type_calvet;

  if (type==anticirce_type_count)
    return tok;
  else if (type>anticirce_type_count)
  {
    output_plaintext_input_error_message(CondNotUniq);
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

static char *ParseMummerStrictness(char *tok, mummer_strictness_type *strictness)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

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

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

static char *ParseVaultingPieces(char *tok, Side side)
{
  while (true)
  {
    piece_walk_type p;
    tok = ParsePieceWalkToken(tok,&p);

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

char *ParseCond(char *tok)
{
  unsigned int CondCnt = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  do
  {
    Cond const cond = GetUniqIndex(CondCount,CondTab,tok);
    TraceValue("%s",tok);TraceValue("%u",cond);TraceEOL();
    if (cond==CondCount)
    {
      ExtraCond const extra = GetUniqIndex(ExtraCondCount,ExtraCondTab,tok);
      if (extra>ExtraCondCount)
      {
        output_plaintext_input_error_message(CondNotUniq);
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
            tok = ReadNextTokStr();
            tok = ParseMummerStrictness(tok,&mummer_strictness_default_side);
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
      }
    }
    else if (cond>CondCount)
    {
      output_plaintext_input_error_message(CondNotUniq);
      tok = ReadNextTokStr();
    }
    else
    {
      CondFlag[cond]= true;

      ++CondCnt;

      tok = ReadNextTokStr();

      switch (cond)
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
        {
          char * const squares_tok = tok;

          being_solved.number_of_imitators = 0;
          tok = ParseSquareList(squares_tok,
                                &HandleImitatorPosition,
                                &being_solved.number_of_imitators);
          if (tok==squares_tok)
            output_plaintext_input_error_message(MissngSquareList);
          else if (*tok!=0)
            output_plaintext_error_message(WrongSquareList);

          tok = ReadNextTokStr();
          break;
        }
        case blroyalsq:
          tok = ParseRoyalSquare(tok,Black);
          break;
        case whroyalsq:
          tok = ParseRoyalSquare(tok,White);
          break;
        case magicsquare:
          magic_square_type = ConditionType1;
          tok = ParseSquaresWithFlag(tok,MagicSq);
          tok = ParseNumberedType(tok,&magic_square_type,ConditionType1,ConditionType2);
          break;
        case wormholes:
          tok = ParseSquaresWithFlag(tok,Wormhole);
          break;
        case dbltibet:
          CondFlag[tibet]= true;
          break;
        case holes:
        {
          char * const squares_tok = tok;

          tok = ParseSquareList(squares_tok,&HandleHole,0);
          if (tok==squares_tok)
            output_plaintext_input_error_message(MissngSquareList);
          else if (*tok!=0)
            output_plaintext_error_message(WrongSquareList);

          tok = ReadNextTokStr();
          break;
        }
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
          mummer_strictness[White] = mummer_strictness_regular;
          break;
        case blsupertrans_king:
          calc_reflective_king[Black]= true;
          mummer_strictness[Black] = mummer_strictness_regular;
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
          tok = ParseVaultingPieces(tok,no_side);
          break;
        case whvault_king:
          calc_reflective_king[White]= true;
          vaulting_kings_transmuting[White] = false;
          tok = ParseVaultingPieces(tok,White);
          break;
        case blvault_king:
          calc_reflective_king[Black]= true;
          vaulting_kings_transmuting[Black] = false;
          tok = ParseVaultingPieces(tok,Black);
          break;

        /*****  exact-maxis  *****/
        case blmax:
        case blmin:
        case blcapt:
          tok = ParseMummerStrictness(tok,&mummer_strictness[Black]);
          break;
        case whmax:
        case whmin:
        case whcapt:
          tok = ParseMummerStrictness(tok,&mummer_strictness[White]);
          break;

        case whforsqu:
          tok = ParseSquaresWithFlag(tok,WhForcedSq);
          mummer_strictness[White] = mummer_strictness_regular;
          break;
        case blforsqu:
          mummer_strictness[Black] = mummer_strictness_regular;
          tok = ParseSquaresWithFlag(tok,BlForcedSq);
          break;
        case whconforsqu:
          mummer_strictness[White] = mummer_strictness_ultra;
          tok = ParseSquaresWithFlag(tok,WhForcedSq);
          break;
        case blconforsqu:
          mummer_strictness[Black] = mummer_strictness_ultra;
          tok = ParseSquaresWithFlag(tok,BlForcedSq);
          break;

        case blfollow:
        case blackalphabetic:
        case blacksynchron:
        case blackantisynchron:
        case schwarzschacher:
          mummer_strictness[Black] = mummer_strictness_regular;
          break;

        case whfollow:
        case whitealphabetic:
        case whitesynchron:
        case whiteantisynchron:
          mummer_strictness[White] = mummer_strictness_regular;
          break;

        case alphabetic:
          mummer_strictness[Black] = mummer_strictness_regular;
          mummer_strictness[White] = mummer_strictness_regular;
          break;

        case duellist:
          mummer_strictness[Black] = mummer_strictness_regular;
          mummer_strictness[White] = mummer_strictness_regular;
          break;

        case losingchess:
          mummer_strictness[Black] = mummer_strictness_regular;
          mummer_strictness[White] = mummer_strictness_regular;
          OptFlag[sansrn] = true;
          OptFlag[sansrb] = true;
          break;

        /* different types of circe */
          /* just Circe - but maybe with many variants */
        case circe:
          tok = ParseCirceVariants(tok,&circe_variant);
          break;
          /* Circe variant encoded in specific input tokens: */
        case circediametral:
          CondFlag[circe] = true;
          circe_variant.rebirth_square_adapter = circe_rebirth_square_adapter_diametral;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circemirrorvertical:
          circe_variant.rebirth_square_adapter = circe_rebirth_square_adapter_verticalmirror;
          CondFlag[circe] = true;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circechameleon:
          CondFlag[circe] = true;
          circe_variant.reborn_walk_adapter = circe_reborn_walk_adapter_chameleon;
          tok = ReadChameleonSequence(tok,
                                      &circe_variant.explicit_chameleon_squence_set_in_twin,
                                      &circe_variant.chameleon_walk_sequence);
          circe_variant.is_chameleon_sequence_explicit = circe_variant.explicit_chameleon_squence_set_in_twin==twin_id;
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
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circediagramm:
          CondFlag[circe] = true;
          circe_variant.determine_rebirth_square = circe_determine_rebirth_square_diagram;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circefile:
          CondFlag[circe] = true;
          circe_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circecouscous:
          CondFlag[circe] = true;
          circe_variant.is_promotion_possible= true;
          circe_variant.actual_relevant_piece = circe_relevant_piece_capturer;
          break;
        case circecouscousmirror:
          CondFlag[circe] = true;
          CondFlag[circecouscous] = true;
          circe_variant.is_promotion_possible= true;
          circe_variant.actual_relevant_piece = circe_relevant_piece_capturer;
          circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circeequipollents:
          CondFlag[circe] = true;
          circe_variant.determine_rebirth_square = circe_determine_rebirth_square_equipollents;
          circe_variant.is_promotion_possible= true;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circesymmetry:
          CondFlag[circe] = true;
          circe_variant.is_promotion_possible= true;
          circe_variant.determine_rebirth_square = circe_determine_rebirth_square_symmetry;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circeverticalsymmetry:
          CondFlag[circe] = true;
          circe_variant.is_promotion_possible= true;
          circe_variant.determine_rebirth_square = circe_determine_rebirth_square_vertical_symmetry;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circehorizontalsymmetry:
          CondFlag[circe] = true;
          circe_variant.is_promotion_possible= false;
          circe_variant.determine_rebirth_square = circe_determine_rebirth_square_horizontal_symmetry;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
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
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
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
        case circemirror:
          CondFlag[circe] = true;
          circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circefilemirror:
          CondFlag[circe] = true;
          circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
          circe_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circeclonemirror:
          CondFlag[circe] = true;
          circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
          circe_variant.reborn_walk_adapter = circe_reborn_walk_adapter_clone;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circedoubleagents:
          CondFlag[circe] = true;
          circe_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
          circe_variant.is_turncoat = true;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circeassassin:
          CondFlag[circe] = true;
          circe_variant.on_occupied_rebirth_square = circe_on_occupied_rebirth_square_assassinate;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case circeclone:
          CondFlag[circe] = true;
          circe_variant.reborn_walk_adapter = circe_reborn_walk_adapter_clone;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case frischauf:
          CondFlag[circe] = true;
          tok = ParseRexIncl(tok,&circe_variant.is_rex_inclusive, CirceVariantRexInclusive);
          circe_variant.rebirth_square_adapter = circe_rebirth_square_adapter_frischauf;
          break;
        case april:
        {
          unsigned int nr_walks_read;
          tok = ReadWalks(tok,&circe_variant.is_walk_affected,&nr_walks_read);
          if (nr_walks_read==0)
          {
            CondFlag[april] = false;
            output_plaintext_input_error_message(WrongPieceName);
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
        case geneva:
          tok = ParseCirceVariants(tok,&geneva_variant);
          break;

        /* different types of Anticirce */
          /* just Anticirce - but maybe with many variants */
        case anticirce:
          tok = ParseCirceVariants(tok,&anticirce_variant);
          break;
          /* Anticirce variant encoded in specific input tokens: */
        case antimirror:
          CondFlag[anticirce] = true;
          anticirce_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
          tok = ParseAnticirceVariant(tok,&anticirce_variant.anticirce_type);
          break;
        case anticlonecirce:
          CondFlag[anticirce] = true;
          anticirce_variant.reborn_walk_adapter = circe_reborn_walk_adapter_clone;
          break;
        case antiequipollents:
          CondFlag[anticirce] = true;
          anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_equipollents;
          anticirce_variant.is_promotion_possible = true;
          tok = ParseAnticirceVariant(tok,&anticirce_variant.anticirce_type);
          break;
        case antiantipoden:
          CondFlag[anticirce] = true;
          anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_antipodes;
          anticirce_variant.is_promotion_possible = true;
          tok = ParseAnticirceVariant(tok,&anticirce_variant.anticirce_type);
          break;
        case antimirrorfile:
          CondFlag[anticirce] = true;
          anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
          anticirce_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
          tok = ParseAnticirceVariant(tok,&anticirce_variant.anticirce_type);
          break;
        case antisymmetrie:
          CondFlag[anticirce] = true;
          anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_symmetry;
          anticirce_variant.is_promotion_possible = true;
          tok = ParseAnticirceVariant(tok,&anticirce_variant.anticirce_type);
          break;
        case antiverticalsymmetrie:
          CondFlag[anticirce] = true;
          anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_vertical_symmetry;
          anticirce_variant.is_promotion_possible = true;
          tok = ParseAnticirceVariant(tok,&anticirce_variant.anticirce_type);
          break;
        case antihorizontalsymmetrie:
          CondFlag[anticirce] = true;
          anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_horizontal_symmetry;
          anticirce_variant.is_promotion_possible = false;
          tok = ParseAnticirceVariant(tok,&anticirce_variant.anticirce_type);
          break;
        case antifile:
          CondFlag[anticirce] = true;
          anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
          tok = ParseAnticirceVariant(tok,&anticirce_variant.anticirce_type);
          break;
        case antidiagramm:
          CondFlag[anticirce] = true;
          anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_diagram;
          tok = ParseAnticirceVariant(tok,&anticirce_variant.anticirce_type);
          break;
        case antisuper:
          CondFlag[anticirce] = true;
          anticirce_variant.rebirth_reason = move_effect_reason_rebirth_choice;
          anticirce_variant.determine_rebirth_square = circe_determine_rebirth_square_super;
          anticirce_variant.is_promotion_possible = true;
          tok = ParseAnticirceVariant(tok,&anticirce_variant.anticirce_type);
          break;

        /* different types of immunchess */
          /* just immunchess - but maybe with many variants */
        case immun:
          tok = ParseCirceVariants(tok,&immune_variant);
          break;
          /* immunchess variant encoded in specific input tokens: */
        case immunmirror:
          CondFlag[immun] = true;
          immune_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
          tok = ParseRexIncl(tok,&immune_variant.is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case immunfile:
          CondFlag[immun] = true;
          immune_variant.determine_rebirth_square = circe_determine_rebirth_square_file;
          break;
        case immundiagramm:
          CondFlag[immun] = true;
          immune_variant.determine_rebirth_square = circe_determine_rebirth_square_diagram;
          tok = ParseRexIncl(tok,&immune_variant.is_rex_inclusive, CirceVariantRexInclusive);
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
        case immunverticalsymmetry:
          CondFlag[immun] = true;
          immune_variant.determine_rebirth_square = circe_determine_rebirth_square_vertical_symmetry;
          break;
        case immunhorizontalsymmetry:
          CondFlag[immun] = true;
          immune_variant.determine_rebirth_square = circe_determine_rebirth_square_horizontal_symmetry;
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
          /* just mars circe - but maybe with many variants */
        case mars:
          tok = ParseCirceVariants(tok,&marscirce_variant);
          break;
          /* mars circe variant encoded in specific input tokens: */
        case marsmirror:
          CondFlag[mars] = true;
          marscirce_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
          tok = ParseRexIncl(tok,&marscirce_variant.is_rex_inclusive, CirceVariantRexExclusive);
          break;
        case antimars:
          tok = ParseCirceVariants(tok,&antimars_variant);
          break;
        case antimarsmirror:
          CondFlag[antimars] = true;
          antimars_variant.relevant_side_overrider = circe_relevant_side_overrider_mirror;
          tok = ParseRexIncl(tok,&antimars_variant.is_rex_inclusive, CirceVariantRexExclusive);
          break;
        case antimarsantipodean:
          CondFlag[antimars] = true;
          antimars_variant.determine_rebirth_square = circe_determine_rebirth_square_antipodes;
          tok = ParseRexIncl(tok,&antimars_variant.is_rex_inclusive, CirceVariantRexExclusive);
          break;

        case whprom_sq:
          tok = ParseSquaresWithFlag(tok,WhPromSq);
          break;
        case blprom_sq:
          tok = ParseSquaresWithFlag(tok,BlPromSq);
          break;

        case messigny:
          tok = ParseRexIncl(tok,&messigny_rex_inclusive, CirceVariantRexExclusive);
          break;

        case woozles:
        case biwoozles:
        case heffalumps:
        case biheffalumps:
          tok = ParseRexIncl(tok,&woozles_rex_inclusive, CirceVariantRexExclusive);
          break;

        case chamchess:
          tok = ReadChameleonSequence(tok,
                                      &explicit_chameleon_squence_set_in_twin,
                                      &chameleon_walk_sequence);
          CondFlag[chameleonsequence] = explicit_chameleon_squence_set_in_twin==twin_id;
          break;

        case chameleonsequence:
          tok = ReadChameleonSequence(tok,
                                      &explicit_chameleon_squence_set_in_twin,
                                      &chameleon_walk_sequence);
          break;

        case protean:
          tok = ParseRexIncl(tok,&protean_is_rex_inclusive, CirceVariantRexExclusive);
          break;

        case phantom:
          tok = ParseCirceVariants(tok,&phantom_variant);
          break;

        case madras:
          tok = ParseRexIncl(tok,&madrasi_is_rex_inclusive, CirceVariantRexInclusive);
          break;
        case isardam:
          tok = ParseLetteredType(tok,&isardam_variant,ConditionTypeB);
          tok = ParseRexIncl(tok,&madrasi_is_rex_inclusive, CirceVariantRexInclusive);
          break;

        case annan:
        case nanna:
          tok = ParseLetteredType(tok,&annan_type,ConditionTypeD);
          break;

        case kobulkings:
          kobul_who[White] = true;
          kobul_who[Black] = true;
          tok = ParseKobulSides(tok,&kobul_who);
          break;

        case sentinelles:
          tok = ParseSentinellesVariants(tok);
          break;

        case breton:
          tok = ParseBretonVariants(tok);
          break;

        case dynasty:
        case extinction:
          OptFlag[sansrn] = true;
          OptFlag[sansrb] = true;
          break;

        case singlebox:
          tok = ParseNumberedType(tok,&SingleBoxType,ConditionType1,ConditionType3);
          break;

        case republican:
          tok = ParseNumberedType(tok,&RepublicanType,ConditionType2,ConditionType2);
          break;

        case promotiononly:
        {
          unsigned int nr_walks_read;
          tok = ReadWalks(tok,&promonly,&nr_walks_read);
          if (nr_walks_read==0)
          {
            CondFlag[promotiononly] = false;
            output_plaintext_input_error_message(WrongPieceName);
          }
          break;
        }

        case football:
        {
          unsigned int nr_walks_read;
          football_are_substitutes_limited = false;
          tok = ReadWalks(tok,&is_football_substitute,&nr_walks_read);
          football_are_substitutes_limited = nr_walks_read>0;
          init_football_substitutes();
          break;
        }

        case koeko:
          koeko_nocontact= &nokingcontact;
          nocontactfunc_parsed= &koeko_nocontact;
          tok = ParseKoekoVariant(tok);
          break;
        case antikoeko:
          antikoeko_nocontact= nokingcontact;
          nocontactfunc_parsed= &antikoeko_nocontact;
          tok = ParseKoekoVariant(tok);
          break;

        case white_oscillatingKs:
          tok = ParseLetteredType(tok,&OscillatingKings[White],ConditionTypeC);
          break;
        case black_oscillatingKs:
          tok = ParseLetteredType(tok,&OscillatingKings[Black],ConditionTypeC);
          break;

        case swappingkings:
          CondFlag[white_oscillatingKs]= true;
          OscillatingKings[White]= ConditionTypeC;
          CondFlag[black_oscillatingKs]= true;
          OscillatingKings[Black]= ConditionTypeC;
          break;

        case SAT:
        case strictSAT:
        {
          char *ptr;
          SAT_max_nr_allowed_flights[White] = (unsigned int)strtoul(tok,&ptr,10) + 1;
          if (tok == ptr) {
            SAT_max_nr_allowed_flights[White]= 1;
            SAT_max_nr_allowed_flights[Black]= 1;
            break;
          }
          tok = ReadNextTokStr();
          SAT_max_nr_allowed_flights[Black] = (unsigned int)strtoul(tok,&ptr,10) + 1;
          if (tok == ptr)
            SAT_max_nr_allowed_flights[Black]= SAT_max_nr_allowed_flights[White];
          break;
        }

        case BGL:
        {
          char *ptr;
          BGL_global= false;
          BGL_values[White] = ReadBGLNumber(tok,&ptr);
          if (tok == ptr)
          {
            BGL_values[White] = BGL_infinity;
            BGL_values[Black] = BGL_infinity;
          }
          else
          {
            tok = ReadNextTokStr();
            BGL_values[Black] = ReadBGLNumber(tok,&ptr);
            if (tok == ptr)
            {
              BGL_values[Black] = BGL_values[White];
              BGL_global= true;
            }
            else
              tok = ReadNextTokStr();
          }
          break;
        }

        case gridchess:
          tok = ParseGridVariant(tok);
          break;

        case lastcapture:
          retro_capture.on = initsquare;
          tok = ParseLastCapturedPiece(tok);
          CondFlag[lastcapture] = retro_capture.on!=initsquare;
          break;

        case lostpieces:
          break;

        case bolero:
        case bolero_inverse:
          tok = ParseRexIncl(tok,&bolero_is_rex_inclusive,CirceVariantRexInclusive);
          break;

        default:
          break;
      }
    }
  }
  while (true);

  if (CondCnt==0)
    output_plaintext_input_error_message(UnrecCondition);

  if (CondFlag[black_oscillatingKs] && OscillatingKings[Black]==ConditionTypeC
      && CondFlag[white_oscillatingKs] && OscillatingKings[White]==ConditionTypeC)
    CondFlag[swappingkings] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
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
  bolero_is_rex_inclusive = false;

  sentinelles_max_nr_pawns[Black] = 8;
  sentinelles_max_nr_pawns[White] = 8;
  sentinelles_max_nr_pawns_total = 16;
  sentinelle_walk = Pawn;

  grid_type = grid_normal;

  {
    PieceIdType id;
    for (id = MinPieceId; id<=MaxPieceId; ++id)
      PiecePositionsInDiagram[id] = initsquare;
  }

  for (bnp= boardnum; *bnp; bnp++) {
    int const file= *bnp%onerow - nr_of_slack_files_left_of_board;
    int const row= *bnp/onerow - nr_of_slack_rows_below_board;

    CLEARFL(sq_spec(*bnp));
    sq_num(*bnp)= (int)(bnp-boardnum);

    /* initialise sq_spec and set grid number */
    sq_spec(*bnp) += ((file/2)+4*(row/2)) << Grid;
    if (file!=0 && file!=nr_files_on_board-1
        && row!=0 && row!=nr_rows_on_board-1)
      SETFLAG(sq_spec(*bnp), NoEdgeSq);
  }

  for (i= square_a1; i < square_h8; i+= onerow)
  {
    if (i > square_a1)
      if (!TSTFLAG(sq_spec(i+dir_down), SqColor))
        SETFLAG(sq_spec(i), SqColor);
    for (j= i+1; j < i+nr_files_on_board; j++)
      if (!TSTFLAG(sq_spec(j+dir_left), SqColor))
        SETFLAG(sq_spec(j), SqColor);
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

void conditions_resetter_solve(slice_index si)
{
  InitCond();
  pipe_solve_delegate(si);
}

