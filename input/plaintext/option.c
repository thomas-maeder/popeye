#include "input/plaintext/option.h"
#include "input/plaintext/geometry/square.h"
#include "input/plaintext/condition.h"
#include "input/plaintext/token.h"
#include "output/output.h"
#include "options/options.h"
#include "options/maxsolutions/maxsolutions.h"
#include "options/movenumbers.h"
#include "options/maxthreatlength.h"
#include "options/degenerate_tree.h"
#include "options/maxflightsquares.h"
#include "options/nontrivial.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "output/plaintext/language_dependant.h"
#include "output/plaintext/message.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "solving/castling.h"
#include "solving/battle_play/try.h"
#include "stipulation/pipe.h"
#include "platform/beep.h"
#include "platform/maxtime.h"

#include <stdlib.h>
#include <string.h>

static void ReadMutuallyExclusiveCastling(void)
{
  char *tok = ReadNextTokStr();
  square white_rook_square;
  square black_rook_square;

  tok = ParseSquare(tok,&white_rook_square);
  tok = ParseSquare(tok,&black_rook_square);

  if (game_array.board[white_rook_square]==Rook
      && game_array.board[black_rook_square]==Rook
      && tok[0]==0)
  {
    square const white_castling = (white_rook_square==square_a1
                                   ? queenside_castling
                                   : kingside_castling);
    castling_rights_type const white_flag = (white_rook_square==square_a1
                                             ? ra_cancastle
                                             : rh_cancastle);
    square const black_castling = (black_rook_square==square_a8
                                   ? queenside_castling
                                   : kingside_castling);
    castling_rights_type const black_flag = (black_rook_square==square_a8
                                             ? ra_cancastle
                                             : rh_cancastle);
    castling_mutual_exclusive[White][white_castling-min_castling] |= black_flag;
    castling_mutual_exclusive[Black][black_castling-min_castling] |= white_flag;
  }
  else
    output_plaintext_error_message(MissngSquareList);
}

static void HandleEpSquare(square sq, void *dummy)
{
  if (en_passant_nr_retro_squares==en_passant_retro_capacity)
    output_plaintext_message(TooManyEpKeySquares);
  else
    en_passant_retro_squares[en_passant_nr_retro_squares++] = sq;
}

static void HandleNoCastlingSquare(square sq, void *dummy)
{
  switch (sq)
  {
    case square_a1:
      CLRFLAGMASK(castling_flags_no_castling,ra_cancastle<<(White*black_castling_rights_offset));
      break;

    case square_e1:
      CLRFLAGMASK(castling_flags_no_castling,k_cancastle<<(White*black_castling_rights_offset));
      break;

    case square_h1:
      CLRFLAGMASK(castling_flags_no_castling,rh_cancastle<<(White*black_castling_rights_offset));
      break;

    case square_a8:
      CLRFLAGMASK(castling_flags_no_castling,ra_cancastle<<(Black*black_castling_rights_offset));
      break;

    case square_e8:
      CLRFLAGMASK(castling_flags_no_castling,k_cancastle<<(Black*black_castling_rights_offset));
      break;

    case square_h8:
      CLRFLAGMASK(castling_flags_no_castling,rh_cancastle<<(Black*black_castling_rights_offset));
      break;

    default:
      break;
  }
}

char *ParseOpt(char *tok, slice_index start)
{
  Opt indexx;
  unsigned int OptCnt = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",start);
  TraceFunctionParamListEnd();

  for (indexx = GetUniqIndex(OptCount,OptTab,tok);
       indexx<OptCount;
       indexx = GetUniqIndex(OptCount,OptTab,tok))
  {
    TraceValue("%u",indexx);TraceEOL();

    if (indexx>OptCount)
    {
      output_plaintext_input_error_message(OptNotUniq,0);
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
        if (*end!=0 || value==0 || value>UINT_MAX)
        {
          OptFlag[maxtime]= false;
          output_plaintext_input_error_message(WrongInt, 0);
          indexx = OptCount;
        }
        else
          setOptionMaxtime((maxtime_type)value);
        break;
      }

      case enpassant:
      {
        char * const squares_tok = ReadNextTokStr();

        tok = ParseSquareList(squares_tok,&HandleEpSquare,0);
        if (tok==squares_tok)
        {
          output_plaintext_input_error_message(MissngSquareList,0);
          indexx = OptCount+1;
        }
        else if (*tok!=0)
          output_plaintext_error_message(WrongSquareList);

        break;
      }

      case maxsols:
        tok = ReadNextTokStr();
        if (!read_max_solutions(tok))
        {
          OptFlag[maxsols] = false;
          output_plaintext_input_error_message(WrongInt,0);
          indexx = OptCount;
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
          output_plaintext_input_error_message(WrongInt,0);
          indexx = OptCount;
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
          output_plaintext_input_error_message(WrongInt,0);
          indexx = OptCount;
        }
        break;

      case solflights:
        tok = ReadNextTokStr();
        if (!read_max_flights(tok))
        {
          OptFlag[solflights] = false;
          output_plaintext_input_error_message(WrongInt,0);
          indexx = OptCount;
        }
        break;

      case soltout:
        tok = ReadNextTokStr();
        if (!read_max_nr_refutations(tok))
        {
          OptFlag[soltout] = false;
          output_plaintext_input_error_message(WrongInt,0);
          indexx = OptCount;
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
            output_plaintext_input_error_message(WrongInt, 0);
            indexx = OptCount;
          }
        }
        else
        {
          OptFlag[nontrivial] = false;
          output_plaintext_input_error_message(WrongInt, 0);
          indexx = OptCount;
        }
        break;
      }

      case postkeyplay:
        OptFlag[solvariantes]= true;
        break;

      case nocastling:
      {
        char * const squares_tok = ReadNextTokStr();

        castling_flags_no_castling = bl_castlings|wh_castlings;

        tok = ParseSquareList(squares_tok,&HandleNoCastlingSquare,0);
        if (tok==squares_tok)
          output_plaintext_input_error_message(MissngSquareList,0);
        else if (*tok!=0)
          output_plaintext_error_message(WrongSquareList);

        break;
      }

      case mutuallyexclusivecastling:
        ReadMutuallyExclusiveCastling();
        break;

      case duplex:
        if (input_is_instrumented_with_duplex(start))
          output_plaintext_input_error_message(InconsistentDuplexOption,0);
        else
          input_instrument_duplex(start,STInputDuplex);
        break;
      case halfduplex:
        if (input_is_instrumented_with_duplex(start))
          output_plaintext_input_error_message(InconsistentDuplexOption,0);
        else
          input_instrument_duplex(start,STInputHalfduplex);
        break;

      default:
        /* no extra action required */
        break;
    }

    if (indexx<=OptCount)
      tok = ReadNextTokStr();
  }

  if (OptCnt==0)
    output_plaintext_input_error_message(UnrecOption,0);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}

void InitOpt(void)
{
  {
    Side side;
    square castling;
    for (side = White; side<=Black; ++side)
      for (castling = min_castling; castling<=max_castling; ++castling)
        castling_mutual_exclusive[side][castling-min_castling] = 0;
  }

  castling_flags_no_castling = bl_castlings|wh_castlings;

  en_passant_forget_multistep();
  en_passant_nr_retro_squares = 0;

  resetOptionMaxtime();

  reset_max_flights();
  set_max_nr_refutations(0);
  reset_restart_number();
  reset_max_threat_length();
  reset_nontrivial_settings();

  {
    unsigned int i;
    for (i = 0; i<OptCount; i++)
      OptFlag[i] = false;
  }
}
