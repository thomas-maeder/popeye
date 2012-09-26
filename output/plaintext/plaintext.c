#include "output/plaintext/plaintext.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "debugging/trace.h"
#include "conditions/republican.h"
#include "conditions/bgl.h"
#include "conditions/kobul.h"
#include "conditions/einstein/einstein.h"
#include "conditions/imitator.h"
#include "pieces/side_change.h"
#include "solving/en_passant.h"

#include <assert.h>
#include <stdlib.h>

static void editcoup(coup const *mov)
{
  char    BlackChar= *GetMsgString(BlackColor);
  char    WhiteChar= *GetMsgString(WhiteColor);
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;
  move_effect_journal_index_type capture = move_effect_journal_index_null;
  move_effect_journal_index_type castling = move_effect_journal_index_null;
  Flags active_non_side_flags = 0;
  piece active_piece = vide;
  square active_piece_pos = initsquare;

#ifdef _SE_DECORATE_SOLUTION_
  se_move(mov);
#endif

  if (mov->cazz==nullsquare) return;

  for (curr = move_effect_journal_top[nbply-1]; curr!=top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_flags_change:
      {
        switch (move_effect_journal[curr].reason)
        {
          case move_effect_reason_pawn_promotion:
            if (active_piece_pos==move_effect_journal[curr].u.flags_change.on)
            {
              StdChar('=');
              WriteSpec(move_effect_journal[curr].u.flags_change.to,
                        active_piece,
                        false);

              {
                Flags const side_flag_mask = BIT(White)|BIT(Black);
                active_non_side_flags = move_effect_journal[curr].u.flags_change.to&~side_flag_mask;
              }
            }
            break;

          case move_effect_reason_half_neutral_neutralisation:
          case move_effect_reason_half_neutral_deneutralisation:
            if (active_piece_pos==move_effect_journal[curr].u.flags_change.on)
            {
              StdChar('=');
              WriteSpec(move_effect_journal[curr].u.flags_change.to,
                        active_piece,
                        true);
            }
            break;

          default:
            break;
        }
        break;
      }

      case move_effect_side_change:
      {
        switch (move_effect_journal[curr].reason)
        {
          case move_effect_reason_andernach_chess:
          case move_effect_reason_volage_side_change:
          case move_effect_reason_magic_square:
            if (active_piece_pos==move_effect_journal[curr].u.side_change.on)
            {
              StdChar('=');
              StdChar(move_effect_journal[curr].u.side_change.to==White ? WhiteChar : BlackChar);
            }
            break;

          case move_effect_reason_magic_piece:
          case move_effect_reason_masand:
          {
            StdString(" [");

            WriteSquare(move_effect_journal[curr].u.side_change.on);
            StdChar('=');
            StdChar(move_effect_journal[curr].u.side_change.to==White ? WhiteChar : BlackChar);

            while (curr+1!=top
                   && move_effect_journal[curr+1].type==move_effect_side_change
                   && move_effect_journal[curr+1].reason==move_effect_journal[curr].reason)
            {
              ++curr;
              StdString(", ");
              WriteSquare(move_effect_journal[curr].u.side_change.on);
              StdChar('=');
              StdChar(move_effect_journal[curr].u.side_change.to==White ? WhiteChar : BlackChar);
            }

            StdString("]");
            break;
          }

          default:
            break;
        }
        break;
      }

      case move_effect_piece_change:
      {
        switch (move_effect_journal[curr].reason)
        {
          case move_effect_reason_pawn_promotion:
          case move_effect_reason_protean_adjustment:
          case move_effect_reason_chameleon_movement:
          case move_effect_reason_degradierung:
          case move_effect_reason_norsk_chess:
            if (active_piece_pos==move_effect_journal[curr].u.piece_change.on
                /* regular promotion doesn't test whether the "promotion" is
                 * into pawn (e.g. in SingleBox); it's more efficient to test here */
                && (move_effect_journal[curr].u.piece_change.to
                    !=move_effect_journal[curr].u.piece_change.from))
            {
              StdChar('=');
              if (active_non_side_flags!=0)
              {
                WriteSpec(active_non_side_flags,move_effect_journal[curr].u.piece_change.to,false);
                active_non_side_flags = 0;
              }
              WritePiece(move_effect_journal[curr].u.piece_change.to);
            }
            break;

          case move_effect_reason_singlebox_promotion:
          {
            StdString(" [");
            WriteSquare(move_effect_journal[curr].u.piece_change.on);
            StdString("=");
            WritePiece(move_effect_journal[curr].u.piece_change.to);
            StdString("]");
            break;
          }

          case move_effect_reason_einstein_chess:
          case move_effect_reason_football_chess_substituition:
          case move_effect_reason_king_transmutation:
          {
            StdChar('=');
            WritePiece(move_effect_journal[curr].u.piece_change.to);
            break;
          }

          default:
            break;
        }
        break;
      }

      case move_effect_piece_movement:
      {
        switch (move_effect_journal[curr].reason)
        {
          case move_effect_reason_moving_piece_movement:
          {
            piece const moving = move_effect_journal[curr].u.piece_movement.moving;
            square const sq_arrival = move_effect_journal[curr].u.piece_movement.to;
            if (WriteSpec(move_effect_journal[curr].u.piece_movement.movingspec,moving,false)
                || (moving!=pb && moving!=pn))
              WritePiece(moving);

            WriteSquare(move_effect_journal[curr].u.piece_movement.from);

            if (capture==move_effect_journal_index_null)
            {
              /* TODO better modeling for antimars */
              if (anyantimars)
              {
                StdString("->");
                WriteSquare(mov->mren);
              }
              StdChar('-');
              WriteSquare(sq_arrival);
            }
            else
            {
              move_effect_journal_index_type const capturing_movement = move_effect_journal[capture].u.piece_removal.capturing_movement;
              if (capturing_movement==curr)
              {
                square const sq_capture = move_effect_journal[capture].u.piece_removal.from;
                StdChar('*');
                if (sq_capture==sq_arrival)
                  WriteSquare(sq_arrival);
                else
                {
                  /* TODO better modeling for e.p.? */
                  if (is_pawn(move_effect_journal[capture].u.piece_removal.removed)
                      && is_pawn(moving)
                      && (sq_arrival==ep[parent_ply[nbply]]
                          || sq_arrival==ep2[parent_ply[nbply]]))
                  {
                    WriteSquare(sq_arrival);
                    StdString(" ep.");
                  }
                  else
                  {
                    WriteSquare(sq_capture);
                    StdChar('-');
                    WriteSquare(sq_arrival);
                  }
                }

                capture = move_effect_journal_index_null;
              }
              else
              {
                StdChar('-');
                WriteSquare(sq_arrival);
              }
            }
            break;
          }

          case move_effect_reason_castling_king_movement:
          {
            square const to = move_effect_journal[curr].u.piece_movement.to;
            if (to==square_g1 || to==square_g8)
              StdString("0-0");
            else
              StdString("0-0-0");
            castling = curr;
            break;
          }

          case move_effect_reason_castling_partner_movement:
          {
            /* TODO model a connection between
             * move_effect_reason_castling_king_movement
             * /move_effect_reason_moving_piece_movement
             * and move_effect_reason_castling_partner_movement? */
            if (castling==move_effect_journal_index_null)
            {
              StdChar('/');
              WriteSpec(move_effect_journal[curr].u.piece_movement.movingspec,
                        move_effect_journal[curr].u.piece_movement.moving,
                        true);
              WritePiece(move_effect_journal[curr].u.piece_movement.moving);
              WriteSquare(move_effect_journal[curr].u.piece_movement.from);
              StdChar('-');
              WriteSquare(move_effect_journal[curr].u.piece_movement.to);

              castling = move_effect_journal_index_null;
            }
            break;
          }

          default:
            break;
        }

        {
          Flags const side_flag_mask = BIT(White)|BIT(Black);
          active_non_side_flags = move_effect_journal[curr].u.piece_movement.movingspec&~side_flag_mask;
        }

        active_piece = move_effect_journal[curr].u.piece_movement.moving;
        active_piece_pos = move_effect_journal[curr].u.piece_movement.to;

        break;
      }

      case move_effect_piece_addition:
      {
        active_piece_pos = initsquare; /* move_effect_journal[curr].u.piece_addition.on; */
        break;
      }

      case move_effect_piece_removal:
      {
        switch (move_effect_journal[curr].reason)
        {
          case move_effect_reason_regular_capture:
          {
            capture = curr;
            break;
          }

          default:
            break;
        }
        break;
      }

      case move_effect_piece_exchange:
      {
        WritePiece(e[move_effect_journal[curr].u.piece_exchange.from]);
        WriteSquare(move_effect_journal[curr].u.piece_exchange.to);
        StdString("<->");
        WritePiece(e[move_effect_journal[curr].u.piece_exchange.to]);
        WriteSquare(move_effect_journal[curr].u.piece_exchange.from);
        break;
      }

      case move_effect_imitator_addition:
      {
        StdString("=I");
        break;
      }

      default:
        break;
    }

  if (mov->sqren != initsquare) {
    piece   p= CondFlag[antieinstein]
        ? einstein_increase_piece(mov->ppri)
        : CondFlag[parrain]
        ? mov->ren_parrain
        : CondFlag[chamcirce]
        ? ChamCircePiece(mov->ppri)
        : (anyclone && abs(mov->pjzz) != roib)
        ? -mov->pjzz
        : (anytraitor && abs(mov->ppri) >= roib)
        ? -mov->ppri
        : mov->ppri;
    StdString(" [+");
    WriteSpec(mov->ren_spec,p, p!=vide);
    WritePiece(p);

    WriteSquare(mov->sqren);
    if (mov->cir_prom!=Empty)
    {
      Flags written_spec = mov->ren_spec;
      if (mov->bool_cir_cham_prom)
        SETFLAG(written_spec,Chameleon);
      StdChar('=');
      WriteSpec(written_spec,mov->cir_prom,p!=vide);
      WritePiece(mov->cir_prom);
    }

    if (TSTFLAG(mov->ren_spec, Volage)
        && SquareCol(mov->cpzz) != SquareCol(mov->sqren))
    {
      sprintf(GlobalStr, "=(%c)",
              (mov->tr == White) ? WhiteChar : BlackChar);
      StdString(GlobalStr);
    }
    StdChar(']');
  }

  if (CondFlag[republican])
    write_republican_king_placement(mov);

  if (mov->renkam!=initsquare)
  {
    StdChar('[');
    WriteSpec(mov->new_spec,mov->pjazz, mov->pjazz != vide);
    WritePiece(mov->pjazz);
    WriteSquare(mov->cazz);
    StdString("->");
    WriteSquare(mov->renkam);
    if (mov->norm_prom!=Empty && mov->norm_prom!=abs(mov->pjazz))
    {
      StdChar('=');
      WriteSpec(mov->new_spec,mov->norm_prom, true);
      WritePiece(mov->norm_prom);
    }
    StdChar(']');
  }
  if (mov->bool_senti) {
    StdString("[+");
    StdChar((!SentPionNeutral || !TSTFLAG(mov->speci, Neutral))
            ?  ((mov->tr==White) != SentPionAdverse
                ? WhiteChar
                : BlackChar)
            : 'n');
    WritePiece(sentinelb); WriteSquare(mov->cdzz);
    StdChar(']');
  }
  if (TSTFLAG(mov->speci, ColourChange)
      && (abs(e[mov->hurdle])>roib))
  {
    Side hc= e[mov->hurdle] < vide ? Black : White;
    StdString("[");
    WriteSquare(mov->hurdle);
    StdString("=");
    StdChar(hc == White ? WhiteChar : BlackChar);
    StdString("]");
  }
  if (CondFlag[kobulkings])
  {
    if (mov->tr == Black && abs(e[king_square[White]]) != kobul[White][nbply])
    {
        StdString(" [");
        WriteSpec(spec[king_square[White]],kobul[White][nbply], true);
        WritePiece(kobul[White][nbply]);
        StdString("=");
        WriteSpec(spec[king_square[White]],e[king_square[White]], false);
        WritePiece(e[king_square[White]]);
        StdString("]");
    }
    if (mov->tr == White && -abs(e[king_square[Black]]) != kobul[Black][nbply])
    {
        StdString(" [");
        WriteSpec(spec[king_square[Black]],kobul[Black][nbply], true);
        WritePiece(kobul[Black][nbply]);
        StdString("=");
        WriteSpec(spec[king_square[Black]],e[king_square[Black]], false);
        WritePiece(e[king_square[Black]]);
        StdString("]");
    }
  }

  if (mov->numi && CondFlag[imitators])
  {
    unsigned int icount;
    int const diff = im0-isquare[0];
    StdChar('[');
    for (icount = 1; icount<=mov->numi; icount++)
    {
      StdChar('I');
      WriteSquare(isquare[icount-1] + mov->sum + diff);
      if (icount<mov->numi)
        StdChar(',');
    }
    StdChar(']');
  }
  if (mov->osc) {
    StdString("[");
    StdChar(WhiteChar);
    WritePiece(roib);
    StdString("<>");
    StdChar(BlackChar);
    WritePiece(roib);
    StdString("]");
  }

  if ((CondFlag[ghostchess] || CondFlag[hauntedchess])
      && mov->ghost_piece!=vide)
  {
    StdString("[+");
    WriteSpec(mov->ghost_flags,mov->ghost_piece, mov->ghost_piece != vide);
    WritePiece(mov->ghost_piece);
    WriteSquare(mov->cdzz);
    StdString("]");
  }

  if (CondFlag[BGL])
  {
    char s[30], buf1[12], buf2[12];
    if (BGL_global)
      sprintf(s, " (%s)", WriteBGLNumber(buf1, mov->bgl_wh));
    else
      sprintf(s, " (%s/%s)",
              WriteBGLNumber(buf1, mov->bgl_wh),
              WriteBGLNumber(buf2, mov->bgl_bl));
    StdString(s);
  }
} /* editcoup */

void output_plaintext_write_move(void)
{
  coup mov;
  current(&mov);
  editcoup(&mov);
}

/* Determine whether a goal writer slice should replace the check writer slice
 * which would normally following the possible check deliverd by the move just
 * played (i.e. if both a possible check and the symbol for the reached goal
 * should be written).
 * @param goal goal written by goal writer
 * @return true iff the check writer should be replaced by the goal writer
 */
boolean output_plaintext_goal_writer_replaces_check_writer(goal_type goal)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_mate:
    case goal_check:
    case goal_doublemate:
    case goal_countermate:
    case goal_mate_or_stale:
    case goal_stale:
    case goal_dblstale:
    case goal_autostale:
      result = true;
      break;

    default:
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
