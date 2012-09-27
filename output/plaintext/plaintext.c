#include "output/plaintext/plaintext.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "debugging/trace.h"
#include "conditions/republican.h"
#include "conditions/bgl.h"
#include "conditions/kobul.h"
#include "conditions/imitator.h"
#include "pieces/side_change.h"
#include "solving/en_passant.h"

#include <assert.h>
#include <stdlib.h>

typedef struct
{
    char const * closing_sequence;
    piece moving;
    Flags non_side_flags;
    square target_square;
} editcoup_context;

static void context_open(editcoup_context *context,
                         char const *opening_sequence,
                         char const *closing_sequence)
{
  StdString(opening_sequence);

  context->moving = vide;
  context->non_side_flags = 0;
  context->target_square = initsquare;
  context->closing_sequence = closing_sequence;
}

static void context_close(editcoup_context *context)
{
  StdString(context->closing_sequence);
}

static void context_set_target_square(editcoup_context *context, square s)
{
  context->target_square = s;
}

static void context_set_moving_piece(editcoup_context *context, piece p)
{
  context->moving = p;
}

static void context_set_non_side_flags(editcoup_context *context, Flags flags)
{
  Flags const side_flag_mask = BIT(White)|BIT(Black);
  context->non_side_flags = flags&~side_flag_mask;
}

static void editcoup(coup const *mov)
{
  char    BlackChar= *GetMsgString(BlackColor);
  char    WhiteChar= *GetMsgString(WhiteColor);
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;
  move_effect_journal_index_type capture = move_effect_journal_index_null;
  move_effect_journal_index_type castling = move_effect_journal_index_null;
  editcoup_context context = { "", vide, 0, initsquare };

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
            if (context.target_square==move_effect_journal[curr].u.flags_change.on)
            {
              StdChar('=');
              WriteSpec(move_effect_journal[curr].u.flags_change.to,
                        context.moving,
                        false);
              context_set_non_side_flags(&context,move_effect_journal[curr].u.flags_change.to);
            }
            break;

          case move_effect_reason_half_neutral_neutralisation:
          case move_effect_reason_half_neutral_deneutralisation:
            if (context.target_square==move_effect_journal[curr].u.flags_change.on)
            {
              StdChar('=');
              WriteSpec(move_effect_journal[curr].u.flags_change.to,
                        context.moving,
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
            if (context.target_square==move_effect_journal[curr].u.side_change.on)
            {
              StdChar('=');
              StdChar(move_effect_journal[curr].u.side_change.to==White ? WhiteChar : BlackChar);
            }
            break;

          case move_effect_reason_magic_piece:
          case move_effect_reason_masand:
          {
            context_close(&context);

            context_open(&context," [","]");
            WriteSquare(move_effect_journal[curr].u.side_change.on);
            StdChar('=');
            StdChar(move_effect_journal[curr].u.side_change.to==White ? WhiteChar : BlackChar);
            context_set_target_square(&context,move_effect_journal[curr].u.side_change.on);
            context_set_moving_piece(&context,-context.moving);
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
          case move_effect_reason_promotion_of_reborn:
            if (context.target_square==move_effect_journal[curr].u.piece_change.on
                /* regular promotion doesn't test whether the "promotion" is
                 * into pawn (e.g. in SingleBox); it's more efficient to test here */
                && (move_effect_journal[curr].u.piece_change.to
                    !=move_effect_journal[curr].u.piece_change.from))
            {
              StdChar('=');
              if (context.non_side_flags!=0)
                WriteSpec(context.non_side_flags,move_effect_journal[curr].u.piece_change.to,false);
              WritePiece(move_effect_journal[curr].u.piece_change.to);
            }
            break;

          case move_effect_reason_singlebox_promotion:
          {
            context_close(&context);

            context_open(&context," [","]");
            WriteSquare(move_effect_journal[curr].u.piece_change.on);
            StdString("=");
            WritePiece(move_effect_journal[curr].u.piece_change.to);
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
            context_close(&context);

            context_open(&context,"","");

            if (WriteSpec(move_effect_journal[curr].u.piece_movement.movingspec,
                          move_effect_journal[curr].u.piece_movement.moving,
                          false)
                || (move_effect_journal[curr].u.piece_movement.moving!=pb
                    && move_effect_journal[curr].u.piece_movement.moving!=pn))
              WritePiece(move_effect_journal[curr].u.piece_movement.moving);

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
              WriteSquare(move_effect_journal[curr].u.piece_movement.to);
            }
            else
            {
              move_effect_journal_index_type const capturing_movement = move_effect_journal[capture].u.piece_removal.capturing_movement;
              if (capturing_movement==curr)
              {
                square const sq_capture = move_effect_journal[capture].u.piece_removal.from;
                StdChar('*');
                if (sq_capture==move_effect_journal[curr].u.piece_movement.to)
                  WriteSquare(move_effect_journal[curr].u.piece_movement.to);
                else
                {
                  /* TODO better modeling for e.p.? */
                  if (is_pawn(move_effect_journal[capture].u.piece_removal.removed)
                      && is_pawn(move_effect_journal[curr].u.piece_movement.moving)
                      && (move_effect_journal[curr].u.piece_movement.to==ep[parent_ply[nbply]]
                          || move_effect_journal[curr].u.piece_movement.to==ep2[parent_ply[nbply]]))
                  {
                    WriteSquare(move_effect_journal[curr].u.piece_movement.to);
                    StdString(" ep.");
                  }
                  else
                  {
                    WriteSquare(sq_capture);
                    StdChar('-');
                    WriteSquare(move_effect_journal[curr].u.piece_movement.to);
                  }
                }

                capture = move_effect_journal_index_null;
              }
              else
              {
                StdChar('-');
                WriteSquare(move_effect_journal[curr].u.piece_movement.to);
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

        context_set_target_square(&context,move_effect_journal[curr].u.piece_movement.to);
        context_set_moving_piece(&context,move_effect_journal[curr].u.piece_movement.moving);
        context_set_non_side_flags(&context,move_effect_journal[curr].u.piece_movement.movingspec);

        break;
      }

      case move_effect_piece_addition:
      {
        switch (move_effect_journal[curr].reason)
        {
          case move_effect_reason_circe_rebirth:
          {
            context_close(&context);

            context_open(&context," [+","]");
            WriteSpec(move_effect_journal[curr].u.piece_addition.addedspec,
                      move_effect_journal[curr].u.piece_addition.added,
                      true);
            WritePiece(move_effect_journal[curr].u.piece_addition.added);
            WriteSquare(move_effect_journal[curr].u.piece_addition.on);
            break;
          }

          case move_effect_reason_republican_king_insertion:
          {
            context_close(&context);

            context_open(&context,"[+","]");
            WriteSpec(move_effect_journal[curr].u.piece_addition.addedspec,
                      move_effect_journal[curr].u.piece_addition.added,
                      true);
            WritePiece(move_effect_journal[curr].u.piece_addition.added);
            WriteSquare(move_effect_journal[curr].u.piece_addition.on);
            break;
          }

          case move_effect_reason_anticirce_rebirth:
          {
            StdString("->");
            WriteSquare(move_effect_journal[curr].u.piece_addition.on);
//            if (mov->norm_prom!=Empty && mov->norm_prom!=abs(mov->pjazz))
//            {
//              StdChar('=');
//              WriteSpec(move_effect_journal[curr].u.piece_addition.addedspec,
//                        mov->norm_prom,
//                        true);
//              WritePiece(mov->norm_prom);
//            }
            break;
          }

          default:
            break;
        }

        context_set_target_square(&context,
                                  move_effect_journal[curr].u.piece_addition.on);
        context_set_moving_piece(&context,
                                 move_effect_journal[curr].u.piece_addition.added);
        context_set_non_side_flags(&context,
                                   move_effect_journal[curr].u.piece_addition.addedspec);
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

          case move_effect_reason_anticirce_rebirth:
          {
            context_close(&context);

            context_open(&context,"[","]");
            WriteSpec(move_effect_journal[curr].u.piece_removal.removedspec,
                      move_effect_journal[curr].u.piece_removal.removed,
                      true);
            WritePiece(move_effect_journal[curr].u.piece_removal.removed);
            WriteSquare(move_effect_journal[curr].u.piece_removal.from);
            break;
          }

          case move_effect_reason_kamikaze_capturer:
          {
            context_close(&context);

            context_open(&context,"[-","]");
            WriteSpec(move_effect_journal[curr].u.piece_removal.removedspec,
                      move_effect_journal[curr].u.piece_removal.removed,
                      true);
            WritePiece(move_effect_journal[curr].u.piece_removal.removed);
            WriteSquare(move_effect_journal[curr].u.piece_removal.from);
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

  context_close(&context);

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
