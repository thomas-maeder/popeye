#include "output/plaintext/plaintext.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "debugging/trace.h"
#include "conditions/republican.h"
#include "conditions/bgl.h"
#include "conditions/kobul.h"
#include "conditions/imitator.h"
#include "solving/en_passant.h"

#include <assert.h>
#include <stdlib.h>

typedef struct
{
    char const * closing_sequence;
    piece moving;
    Flags non_side_flags;
    square target_square;
} move_context;

static void context_open(move_context *context,
                         char const *opening_sequence,
                         char const *closing_sequence)
{
  StdString(opening_sequence);

  context->moving = vide;
  context->non_side_flags = 0;
  context->target_square = initsquare;
  context->closing_sequence = closing_sequence;
}

static void context_close(move_context *context)
{
  StdString(context->closing_sequence);
}

static void context_set_target_square(move_context *context, square s)
{
  context->target_square = s;
}

static void context_set_moving_piece(move_context *context, piece p)
{
  context->moving = p;
}

static void context_set_non_side_flags(move_context *context, Flags flags)
{
  Flags const side_flag_mask = BIT(White)|BIT(Black);
  context->non_side_flags = flags&~side_flag_mask;
}

void output_plaintext_write_move(void)
{
  char const side_shortcut[nr_sides] = { GetMsgString(WhiteColor)[0], GetMsgString(BlackColor)[0] };
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;
  move_effect_journal_index_type capture = move_effect_journal_index_null;
  move_effect_journal_index_type castling = move_effect_journal_index_null;
  move_context context = { "", vide, 0, initsquare };

#ifdef _SE_DECORATE_SOLUTION_
  se_move(mov);
#endif

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
              context_set_non_side_flags(&context,
                                         move_effect_journal[curr].u.flags_change.to);
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

          case move_effect_reason_kobul_king:
          {
            WriteSquare(move_effect_journal[curr].u.flags_change.on);
            StdString("=");
            WriteSpec(move_effect_journal[curr].u.flags_change.to,
                      context.moving,
                      false);
            WritePiece(context.moving);
            break;
          }

          case move_effect_reason_summon_ghost:
          {
            WriteSpec(move_effect_journal[curr].u.flags_change.to,
                      context.moving,
                      true);
            WritePiece(context.moving);
            WriteSquare(move_effect_journal[curr].u.flags_change.on);
            break;
          }

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
              StdChar(side_shortcut[move_effect_journal[curr].u.side_change.to]);
            }
            break;

          case move_effect_reason_magic_piece:
          case move_effect_reason_masand:
          case move_effect_reason_hurdle_colour_changing:
          {
            context_close(&context);

            context_open(&context," [","]");
            WriteSquare(move_effect_journal[curr].u.side_change.on);
            StdChar('=');
            StdChar(side_shortcut[move_effect_journal[curr].u.side_change.to]);
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
                WriteSpec(context.non_side_flags,
                          move_effect_journal[curr].u.piece_change.to,
                          false);
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

          case move_effect_reason_kobul_king:
          {
            context_close(&context);

            context_open(&context," [","]");
            context_set_target_square(&context,
                                      move_effect_journal[curr].u.piece_change.on);
            context_set_moving_piece(&context,
                                     move_effect_journal[curr].u.piece_change.to);
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

          case move_effect_reason_summon_ghost:
          {
            context_close(&context);

            context_open(&context,"[+","]");
            context_set_moving_piece(&context,
                                     move_effect_journal[curr].u.piece_change.to);
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
          case move_effect_reason_sentinelles:
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
        switch (move_effect_journal[curr].reason)
        {
          case move_effect_reason_oscillating_kings:
          {
            context_close(&context);

            context_open(&context,"[","]");
            WritePiece(e[move_effect_journal[curr].u.piece_exchange.from]);
            WriteSquare(move_effect_journal[curr].u.piece_exchange.to);
            StdString("<->");
            WritePiece(e[move_effect_journal[curr].u.piece_exchange.to]);
            WriteSquare(move_effect_journal[curr].u.piece_exchange.from);
            break;
          }

          default:
          {
            WritePiece(e[move_effect_journal[curr].u.piece_exchange.from]);
            WriteSquare(move_effect_journal[curr].u.piece_exchange.to);
            StdString("<->");
            WritePiece(e[move_effect_journal[curr].u.piece_exchange.to]);
            WriteSquare(move_effect_journal[curr].u.piece_exchange.from);
            break;
          }
        }

        break;
      }

      case move_effect_imitator_addition:
      {
        StdString("=I");
        break;
      }

      case move_effect_imitator_movement:
      {
        context_close(&context);

        context_open(&context,"[","]");

        StdChar('I');
        {
          unsigned int icount;
          for (icount = 0; icount<number_of_imitators; ++icount)
          {
            WriteSquare(isquare[icount]);
            if (icount+1<number_of_imitators)
              StdChar(',');
          }
        }
        break;
      }

      default:
        break;
    }

  context_close(&context);

  if (CondFlag[BGL])
  {
    char buf[12];

    StdString(" (");
    WriteBGLNumber(buf,BGL_values[White][nbply]);
    StdString(buf);

    if (!BGL_global)
    {
      StdString("/");
      WriteBGLNumber(buf,BGL_values[Black][nbply]);
      StdString(buf);
    }

    StdString(")");
  }
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
