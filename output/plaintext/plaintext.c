#include "output/plaintext/plaintext.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/pieces.h"
#include "pymsg.h"
#include "conditions/conditions.h"
#include "conditions/republican.h"
#include "conditions/bgl.h"
#include "conditions/kobul.h"
#include "conditions/imitator.h"
#include "conditions/singlebox/type1.h"
#include "pieces/walks/pawns/en_passant.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>
#include <stdlib.h>

char GlobalStr[4000];

FILE *TraceFile;

typedef struct
{
    char const * closing_sequence;
    PieNam moving;
    Flags flags;
    square target_square;
} move_context;

static void context_open(move_context *context,
                         char const *opening_sequence,
                         char const *closing_sequence)
{
  StdString(opening_sequence);

  context->moving = Empty;
  context->flags = 0;
  context->target_square = initsquare;
  context->closing_sequence = closing_sequence;
}

static void context_close(move_context *context)
{
  StdString(context->closing_sequence);
}

static void next_context(move_context *context,
                         char const *opening_sequence,
                         char const *closing_sequence)
{
  context_close(context);
  context_open(context,opening_sequence,closing_sequence);
}

static void context_set_target_square(move_context *context, square s)
{
  context->target_square = s;
}

static void context_set_moving_piece(move_context *context, PieNam p)
{
  context->moving = p;
}

static void context_set_flags(move_context *context, Flags flags)
{
  context->flags = flags;
}

static void context_set_from_piece_movement(move_context *context, move_effect_journal_index_type movement)
{
  context_set_target_square(context,move_effect_journal[movement].u.piece_movement.to);
  context_set_moving_piece(context,move_effect_journal[movement].u.piece_movement.moving);
  context_set_flags(context,move_effect_journal[movement].u.piece_movement.movingspec);
}

static void write_capture(move_context *context,
                          move_effect_journal_index_type capture,
                          move_effect_journal_index_type movement)
{
  square const sq_capture = move_effect_journal[capture].u.piece_removal.from;

  if (WriteSpec(move_effect_journal[movement].u.piece_movement.movingspec,
                move_effect_journal[movement].u.piece_movement.moving,
                false)
      || move_effect_journal[movement].u.piece_movement.moving!=Pawn)
    WritePiece(move_effect_journal[movement].u.piece_movement.moving);

  WriteSquare(move_effect_journal[movement].u.piece_movement.from);

  StdChar('*');
  if (sq_capture==move_effect_journal[movement].u.piece_movement.to)
    WriteSquare(move_effect_journal[movement].u.piece_movement.to);
  else if (move_effect_journal[capture].reason==move_effect_reason_ep_capture)
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
  if (WriteSpec(move_effect_journal[movement].u.piece_movement.movingspec,
                move_effect_journal[movement].u.piece_movement.moving,
                false)
      || move_effect_journal[movement].u.piece_movement.moving!=Pawn)
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
  WritePiece(get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.from));
  WriteSquare(move_effect_journal[movement].u.piece_exchange.to);
  StdString("<->");
  WritePiece(get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.to));
  WriteSquare(move_effect_journal[movement].u.piece_exchange.from);
}

static void write_singlebox_promotion(move_effect_journal_index_type promotion)
{
  WriteSquare(move_effect_journal[promotion].u.piece_change.on);
  StdString("=");
  WritePiece(move_effect_journal[promotion].u.piece_change.to);
}

static void write_singlebox_type3_promotion(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const sb3_prom = capture-1;

  if (move_effect_journal[sb3_prom].type==move_effect_piece_change
      && move_effect_journal[sb3_prom].reason==move_effect_reason_singlebox_promotion)
  {
    move_context context;
    context_open(&context,"[","]");
    write_singlebox_promotion(sb3_prom);
    context_close(&context);
  }
}

static void write_regular_move(move_context *context)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_type const capture_type = move_effect_journal[capture].type;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  move_effect_type const movement_type = move_effect_journal[movement].type;

  assert(capture_type==move_effect_no_piece_removal
         || capture_type==move_effect_piece_removal
         || capture_type==move_effect_none);

  if (capture_type==move_effect_piece_removal)
  {
    assert(move_effect_journal[movement].type==move_effect_piece_movement);
    assert(move_effect_journal[movement].reason==move_effect_reason_moving_piece_movement);
    write_capture(context,capture,movement);
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
        write_no_capture(context,movement);
      else
        write_castling(movement);
    }
    else
    {
      assert(move_effect_journal[movement].reason==move_effect_reason_exchange_castling_exchange
             || move_effect_journal[movement].reason==move_effect_reason_messigny_exchange);
      write_exchange(movement);
    }
  }
}

static void write_complete_piece(Flags spec, PieNam piece, square on)
{
  WriteSpec(spec,piece,true);
  WritePiece(piece);
  WriteSquare(on);
}

static void write_transformed_piece(Flags spec, PieNam piece)
{
  WriteSpec(spec,piece,false);
  WritePiece(piece);
}

static void write_flags_change(move_context *context,
                               move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_pawn_promotion:
      StdChar('=');
      WriteSpec(move_effect_journal[curr].u.flags_change.to,
                context->moving,
                false);
      context_set_flags(context,move_effect_journal[curr].u.flags_change.to);
      break;

    case move_effect_reason_kobul_king:
      if (move_effect_journal[curr-1].type!=move_effect_piece_change
          || move_effect_journal[curr-1].reason!=move_effect_reason_kobul_king)
        /* otherwise the flags are written with the changed piece */
      {
        next_context(context,"[","]");
        context_set_target_square(context,
                                  move_effect_journal[curr].u.flags_change.on);
        WriteSquare(move_effect_journal[curr].u.flags_change.on);
        StdString("=");
        WriteSpec(move_effect_journal[curr].u.flags_change.to,
                  e[context->target_square],
                  false);
      }
      break;

    default:
      break;
  }
}

static char side_shortcut(Side side)
{
  message_id_t const side_name_id = side==White ? WhiteColor : BlackColor;
  return GetMsgString(side_name_id)[0];
}

static void write_side_change(move_context *context,
                              move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_andernach_chess:
    case move_effect_reason_volage_side_change:
    case move_effect_reason_magic_square:
    case move_effect_reason_circe_turncoats:
      StdChar('=');
      StdChar(side_shortcut(move_effect_journal[curr].u.side_change.to));
      break;

    case move_effect_reason_magic_piece:
    case move_effect_reason_masand:
    case move_effect_reason_hurdle_colour_changing:
      next_context(context,"[","]");
      WriteSquare(move_effect_journal[curr].u.side_change.on);
      StdChar('=');
      StdChar(side_shortcut(move_effect_journal[curr].u.side_change.to));
      context_set_target_square(context,move_effect_journal[curr].u.side_change.on);
      break;

    default:
      break;
  }
}

static void write_piece_change(move_context *context,
                                   move_effect_journal_index_type curr)
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
        write_transformed_piece(context->flags,
                                move_effect_journal[curr].u.piece_change.to);
      }
      break;

    case move_effect_reason_singlebox_promotion:
      /* type 3 is already dealt with, so this is type 2 */
      next_context(context,"[","]");
      write_singlebox_promotion(curr);
      break;

    case move_effect_reason_kobul_king:
      next_context(context,"[","]");
      context_set_moving_piece(context,
                               move_effect_journal[curr].u.piece_change.to);
      context_set_target_square(context,
                                move_effect_journal[curr].u.piece_change.on);

      WriteSquare(move_effect_journal[curr].u.piece_change.on);
      StdChar('=');

      {
        Flags flags;

        if (move_effect_journal[curr+1].type==move_effect_flags_change
            && move_effect_journal[curr+1].reason==move_effect_reason_kobul_king)
          flags = move_effect_journal[curr+1].u.flags_change.to;
        else
          flags = BIT(Royal);

        WriteSpec(flags,move_effect_journal[curr].u.piece_change.to,false);
      }

      WritePiece(move_effect_journal[curr].u.piece_change.to);
      break;

    case move_effect_reason_einstein_chess:
    case move_effect_reason_football_chess_substitution:
    case move_effect_reason_king_transmutation:
      StdChar('=');
      WritePiece(move_effect_journal[curr].u.piece_change.to);
      break;

    default:
      break;
  }
}

static void write_piece_movement(move_context *context,
                                 move_effect_journal_index_type curr)
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
        write_complete_piece(move_effect_journal[curr].u.piece_movement.movingspec,
                             move_effect_journal[curr].u.piece_movement.moving,
                             move_effect_journal[curr].u.piece_movement.from);
        StdChar('-');
        WriteSquare(move_effect_journal[curr].u.piece_movement.to);
      }
      else
      {
        /* implicitly written in castling symbols */
      }
      break;

    default:
      break;
  }

  context_set_from_piece_movement(context,curr);
}

static void write_piece_addition(move_context *context,
                                 move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_rebirth_no_choice:
    case move_effect_reason_rebirth_choice:
    case move_effect_reason_republican_king_insertion:
    case move_effect_reason_sentinelles:
    case move_effect_reason_summon_ghost:
      next_context(context,"[+","]");
      write_complete_piece(move_effect_journal[curr].u.piece_addition.addedspec,
                           move_effect_journal[curr].u.piece_addition.added,
                           move_effect_journal[curr].u.piece_addition.on);
      break;

    case move_effect_reason_transfer_no_choice:
    case move_effect_reason_transfer_choice:
      StdString("->");
      WriteSquare(move_effect_journal[curr].u.piece_addition.on);
      break;

    default:
      break;
  }

  context_set_target_square(context,
                            move_effect_journal[curr].u.piece_addition.on);
  context_set_moving_piece(context,
                           move_effect_journal[curr].u.piece_addition.added);
  context_set_flags(context,
                             move_effect_journal[curr].u.piece_addition.addedspec);
}

static void write_piece_removal(move_context *context,
                                move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_no_reason:
    case move_effect_reason_regular_capture:
    case move_effect_reason_ep_capture:
      /* write_capture() has dealt with these */
      assert(0);
      break;

    case move_effect_reason_transfer_no_choice:
    case move_effect_reason_transfer_choice:
      next_context(context,"[","]");
      write_complete_piece(move_effect_journal[curr].u.piece_removal.removedspec,
                           move_effect_journal[curr].u.piece_removal.removed,
                           move_effect_journal[curr].u.piece_removal.from);
      break;

    case move_effect_reason_kamikaze_capturer:
      next_context(context,"[-","]");
      write_complete_piece(move_effect_journal[curr].u.piece_removal.removedspec,
                           move_effect_journal[curr].u.piece_removal.removed,
                           move_effect_journal[curr].u.piece_removal.from);
      break;

    case move_effect_reason_assassin_circe_rebirth:
      /* no output for the removal of an assassinated piece ... */
    case move_effect_reason_pawn_promotion:
      /* ... nor for the removal of a pawn promoted to imitator */
      break;

    default:
      assert(0);
      break;
  }
}

static void write_piece_exchange(move_context *context,
                                 move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_exchange_castling_exchange:
    case move_effect_reason_messigny_exchange:
      /* already dealt with */
      assert(0);
      break;

    case move_effect_reason_oscillating_kings:
      next_context(context,"[","]");
      WritePiece(get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.from));
      WriteSquare(move_effect_journal[curr].u.piece_exchange.to);
      StdString("<->");
      WritePiece(get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.to));
      WriteSquare(move_effect_journal[curr].u.piece_exchange.from);
      break;

    default:
      write_exchange(curr);
      break;
  }
}

static void write_half_neutral_deneutralisation(move_context *context,
                                                move_effect_journal_index_type curr)
{
  StdChar('=');
  StdChar(side_shortcut(move_effect_journal[curr].u.half_neutral_phase_change.side));
  StdChar('h');
}

static void write_half_neutral_neutralisation(move_context *context,
                                              move_effect_journal_index_type curr)
{
  StdString("=nh");
}

static void write_imitator_addition(void)
{
  StdString("=I");
}

static void write_imitator_movement(move_context *context,
                                    move_effect_journal_index_type curr)
{
  unsigned int const nr_moved = move_effect_journal[curr].u.imitator_movement.nr_moved;
  unsigned int icount;

  StdString("[I");

  for (icount = 0; icount<nr_moved; ++icount)
  {
    WriteSquare(isquare[icount]);
    if (icount+1<nr_moved)
      StdChar(',');
  }

  StdChar(']');
}

static void write_bgl_status(move_context *context,
                             move_effect_journal_index_type curr)
{
  char buf[12];

  if (BGL_global)
  {
    if (move_effect_journal[curr].u.bgl_adjustment.side==White)
    {
      next_context(context," (",")");
      WriteBGLNumber(buf,BGL_values[White]);
      StdString(buf);
    }
  }
  else
  {
    next_context(context," (",")");
    WriteBGLNumber(buf,BGL_values[White]);
    StdString(buf);
    StdString("/");
    WriteBGLNumber(buf,BGL_values[Black]);
    StdString(buf);
  }
}

static void write_other_effects(move_context *context)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr = move_effect_journal_base[nbply];

  for (curr += move_effect_journal_index_offset_other_effects; curr!=top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_flags_change:
        write_flags_change(context,curr);
        break;

      case move_effect_side_change:
        write_side_change(context,curr);
        break;

      case move_effect_piece_change:
        write_piece_change(context,curr);
        break;

      case move_effect_piece_movement:
        write_piece_movement(context,curr);
        break;

      case move_effect_piece_readdition:
      case move_effect_piece_creation:
        write_piece_addition(context,curr);
        break;

      case move_effect_piece_removal:
        write_piece_removal(context,curr);
        break;

      case move_effect_piece_exchange:
        write_piece_exchange(context,curr);
        break;

      case move_effect_imitator_addition:
        write_imitator_addition();
        break;

      case move_effect_imitator_movement:
        write_imitator_movement(context,curr);
        break;

      case move_effect_half_neutral_deneutralisation:
        write_half_neutral_deneutralisation(context,curr);
        break;

      case move_effect_half_neutral_neutralisation:
        write_half_neutral_neutralisation(context,curr);
        break;

      case move_effect_bgl_adjustment:
        write_bgl_status(context,curr);
        break;

      default:
        break;
    }
}

void output_plaintext_write_move(void)
{
  move_context context;

#ifdef _SE_DECORATE_SOLUTION_
  se_move(mov);
#endif

  if (CondFlag[singlebox] && SingleBoxType==singlebox_type3)
    write_singlebox_type3_promotion();

  context_open(&context,"","");
  write_regular_move(&context);
  write_other_effects(&context);
  context_close(&context);
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
