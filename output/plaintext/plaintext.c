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
#include "conditions/einstein/en_passant.h"

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

static void context_set_from_piece_movement(move_context *context, move_effect_journal_index_type movement)
{
  context_set_target_square(context,move_effect_journal[movement].u.piece_movement.to);
  context_set_moving_piece(context,move_effect_journal[movement].u.piece_movement.moving);
  context_set_non_side_flags(context,move_effect_journal[movement].u.piece_movement.movingspec);
}

static void write_capture(move_context *context,
                          move_effect_journal_index_type capture,
                          move_effect_journal_index_type movement)
{
  square const sq_capture = move_effect_journal[capture].u.piece_removal.from;

  context_close(context);

  context_open(context,"","");

  if (WriteSpec(move_effect_journal[movement].u.piece_movement.movingspec,
                move_effect_journal[movement].u.piece_movement.moving,
                false)
      || (move_effect_journal[movement].u.piece_movement.moving!=pb
          && move_effect_journal[movement].u.piece_movement.moving!=pn))
    WritePiece(move_effect_journal[movement].u.piece_movement.moving);

  WriteSquare(move_effect_journal[movement].u.piece_movement.from);

  StdChar('*');
  if (sq_capture==move_effect_journal[movement].u.piece_movement.to)
    WriteSquare(move_effect_journal[movement].u.piece_movement.to);
  else if (is_pawn(abs(move_effect_journal[movement].u.piece_movement.moving))
           && is_pawn(abs(move_effect_journal[capture].u.piece_removal.removed))
           && move_effect_journal[capture].reason==move_effect_reason_ep_capture)
  {
    WriteSquare(move_effect_journal[movement].u.piece_movement.to);
    StdString(" ep.");
  }
  else
  {
    WriteSquare(sq_capture);
    StdChar('-');
    WriteSquare(move_effect_journal[movement].u.piece_movement.to);
  }

  context_set_from_piece_movement(context,movement);
}

static void write_no_capture(move_context *context,
                             move_effect_journal_index_type movement)
{
  context_close(context);

  context_open(context,"","");

  if (WriteSpec(move_effect_journal[movement].u.piece_movement.movingspec,
                move_effect_journal[movement].u.piece_movement.moving,
                false)
      || (move_effect_journal[movement].u.piece_movement.moving!=pb
          && move_effect_journal[movement].u.piece_movement.moving!=pn))
    WritePiece(move_effect_journal[movement].u.piece_movement.moving);

  WriteSquare(move_effect_journal[movement].u.piece_movement.from);
  StdChar('-');
  WriteSquare(move_effect_journal[movement].u.piece_movement.to);

  context_set_from_piece_movement(context,movement);
}

static void write_castling(move_effect_journal_index_type movement)
{
  if (CondFlag[castlingchess])
  {
    WritePiece(move_effect_journal[movement].u.piece_movement.moving);
    WriteSquare(move_effect_journal[movement].u.piece_movement.from);
    StdChar('-');
    WriteSquare(move_effect_journal[movement].u.piece_movement.to);
  }
  else
  {
    square const to = move_effect_journal[movement].u.piece_movement.to;
    if (to==square_g1 || to==square_g8)
      StdString("0-0");
    else
      StdString("0-0-0");
  }
}

static void write_exchange(move_effect_journal_index_type movement)
{
  WritePiece(e[move_effect_journal[movement].u.piece_exchange.from]);
  WriteSquare(move_effect_journal[movement].u.piece_exchange.to);
  StdString("<->");
  WritePiece(e[move_effect_journal[movement].u.piece_exchange.to]);
  WriteSquare(move_effect_journal[movement].u.piece_exchange.from);
}

static void write_singlebox_promotion(move_context *context,
                                      move_effect_journal_index_type promotion)
{
  context_close(context);

  context_open(context,"[","]");
  WriteSquare(move_effect_journal[promotion].u.piece_change.on);
  StdString("=");
  WritePiece(move_effect_journal[promotion].u.piece_change.to);
}

void output_plaintext_write_move(void)
{
  char const side_shortcut[nr_sides] = { GetMsgString(WhiteColor)[0], GetMsgString(BlackColor)[0] };
  move_context context = { "", vide, 0, initsquare };
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr = move_effect_journal_top[nbply-1];

  move_effect_journal_index_type const capture = curr+move_effect_journal_index_offset_capture;
  move_effect_type const capture_type = move_effect_journal[capture].type;
  move_effect_journal_index_type const movement = curr+move_effect_journal_index_offset_movement;
  move_effect_type const movement_type = move_effect_journal[movement].type;

#ifdef _SE_DECORATE_SOLUTION_
  se_move(mov);
#endif

  if (CondFlag[singlebox] && SingleBoxType==singlebox_type3)
  {
    move_effect_journal_index_type const sb3_prom = capture-1;
    if (move_effect_journal[sb3_prom].type==move_effect_piece_change
        && move_effect_journal[sb3_prom].reason==move_effect_reason_singlebox_promotion)
      write_singlebox_promotion(&context,sb3_prom);
  }

  assert(capture_type==move_effect_no_piece_removal
         || capture_type==move_effect_piece_removal
         || capture_type==move_effect_none);

  if (capture_type==move_effect_piece_removal)
  {
    assert(move_effect_journal[movement].type==move_effect_piece_movement);
    assert(move_effect_journal[movement].reason==move_effect_reason_moving_piece_movement);
    write_capture(&context,capture,movement);
  }
  else if (capture_type==move_effect_no_piece_removal)
  {
    assert(movement_type==move_effect_piece_movement
           || movement_type==move_effect_piece_exchange);
    if (movement_type==move_effect_piece_movement)
    {
      move_effect_reason_type const movement_reason = move_effect_journal[movement].reason;

      assert(movement_reason==move_effect_reason_moving_piece_movement
             || movement_reason==move_effect_reason_castling_king_movement);

      if (movement_reason==move_effect_reason_moving_piece_movement)
        write_no_capture(&context,movement);
      else
        write_castling(movement);
    }
    else
    {
      move_effect_reason_type const exchange_reason = move_effect_journal[movement].reason;
      assert(exchange_reason==move_effect_reason_exchange_castling_exchange
             || exchange_reason==move_effect_reason_messigny_exchange);
      write_exchange(movement);
    }
  }

  for (curr += move_effect_journal_index_offset_other_effects; curr!=top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_flags_change:
      {
        switch (move_effect_journal[curr].reason)
        {
          case move_effect_reason_pawn_promotion:
            StdChar('=');
            WriteSpec(move_effect_journal[curr].u.flags_change.to,
                      context.moving,
                      false);
            context_set_non_side_flags(&context,
                                       move_effect_journal[curr].u.flags_change.to);
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
            WriteSquare(move_effect_journal[curr].u.flags_change.on);
            StdString("=");
            WriteSpec(move_effect_journal[curr].u.flags_change.to,
                      context.moving,
                      false);
            WritePiece(context.moving);
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
          case move_effect_reason_circe_turncoats:
            StdChar('=');
            StdChar(side_shortcut[move_effect_journal[curr].u.side_change.to]);
            break;

          case move_effect_reason_magic_piece:
          case move_effect_reason_masand:
          case move_effect_reason_hurdle_colour_changing:
          {
            context_close(&context);

            context_open(&context,"[","]");
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
            if (/* regular promotion doesn't test whether the "promotion" is
                 * into pawn (e.g. in SingleBox); it's more efficient to test here */
                move_effect_journal[curr].u.piece_change.to
                !=move_effect_journal[curr].u.piece_change.from)
            {
              StdChar('=');
              WriteSpec(context.non_side_flags,
                        move_effect_journal[curr].u.piece_change.to,
                        false);
              WritePiece(move_effect_journal[curr].u.piece_change.to);
            }
            break;

          case move_effect_reason_singlebox_promotion:
            /* type 3 is already dealth with, so this is type 2 */
            write_singlebox_promotion(&context,curr);
            break;

          case move_effect_reason_kobul_king:
          {
            context_close(&context);

            context_open(&context,"[","]");
            context_set_target_square(&context,
                                      move_effect_journal[curr].u.piece_change.on);
            context_set_moving_piece(&context,
                                     move_effect_journal[curr].u.piece_change.to);
            break;
          }

          case move_effect_reason_einstein_chess:
          case move_effect_reason_football_chess_substitution:
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
            /* write_capture() and write_no_capture() have dealt with this */
            assert(0);
            break;

          case move_effect_reason_castling_king_movement:
            /* write_castling() has dealt with this */
            assert(0);
            break;

          case move_effect_reason_castling_partner_movement:
            if (CondFlag[castlingchess])
            {
              StdChar('/');
              WriteSpec(move_effect_journal[curr].u.piece_movement.movingspec,
                        move_effect_journal[curr].u.piece_movement.moving,
                        true);
              WritePiece(move_effect_journal[curr].u.piece_movement.moving);
              WriteSquare(move_effect_journal[curr].u.piece_movement.from);
              StdChar('-');
              WriteSquare(move_effect_journal[curr].u.piece_movement.to);
            }
            break;

          default:
            break;
        }

        context_set_from_piece_movement(&context,curr);

        break;
      }

      case move_effect_piece_addition:
      {
        switch (move_effect_journal[curr].reason)
        {
          case move_effect_reason_circe_rebirth:
          case move_effect_reason_supercirce_rebirth:
          case move_effect_reason_republican_king_insertion:
          case move_effect_reason_sentinelles:
          case move_effect_reason_summon_ghost:
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
          case move_effect_reason_antisupercirce_rebirth:
          case move_effect_reason_wormhole_transfer:
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
          case move_effect_no_reason:
          case move_effect_reason_regular_capture:
          case move_effect_reason_ep_capture:
            /* write_capture() has dealt with these */
            assert(0);
            break;

          case move_effect_reason_anticirce_rebirth:
          case move_effect_reason_antisupercirce_rebirth:
          case move_effect_reason_wormhole_transfer:
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
          case move_effect_reason_exchange_castling_exchange:
          case move_effect_reason_messigny_exchange:
            /* already dealt with */
            assert(0);
            break;

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
            write_exchange(curr);
            break;
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
