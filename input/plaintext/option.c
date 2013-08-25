#include "input/plaintext/option.h"
#include "input/plaintext/pieces.h"
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
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "solving/castling.h"
#include "solving/battle_play/try.h"
#include "platform/beep.h"
#include "platform/maxtime.h"
#include "pymsg.h"

#include <stdlib.h>
#include <string.h>

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

char *ParseOpt(slice_index root_slice_hook)
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

  move_effect_journal_reset_retro_capture();
}
