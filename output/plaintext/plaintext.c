#include "output/plaintext/plaintext.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/position.h"
#include "output/plaintext/message.h"
#include "output/plaintext/tree/tree.h"
#include "output/plaintext/line/line.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "conditions/conditions.h"
#include "conditions/republican.h"
#include "conditions/bgl.h"
#include "conditions/kobul.h"
#include "conditions/imitator.h"
#include "conditions/singlebox/type1.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/classification.h"
#include "pieces/pieces.h"
#include "stipulation/pipe.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/slice_insertion.h"
#include "solving/machinery/twin.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <stdlib.h>

FILE *TraceFile;

void output_plaintext_context_open(FILE *file,
                                   output_plaintext_move_context_type *context,
                                   move_effect_journal_index_type start,
                                   char const *opening_sequence,
                                   char const *closing_sequence)
{
  fputs(opening_sequence,file);

  context->start = start;
  context->closing_sequence = closing_sequence;
  context->file = file;
}

void output_plaintext_context_close(output_plaintext_move_context_type *context)
{
  fprintf(context->file,context->closing_sequence);
  context->start = move_effect_journal_index_null;
}

void output_plaintext_next_context(output_plaintext_move_context_type *context,
                                   move_effect_journal_index_type start,
                                   char const *opening_sequence,
                                   char const *closing_sequence)
{
  FILE * const file = context->file;
  output_plaintext_context_close(context);
  output_plaintext_context_open(file,context,start,opening_sequence,closing_sequence);
}

static move_effect_journal_index_type find_pre_move_effect(move_effect_type type,
                                                           move_effect_reason_type reason)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type i;

  for (i = base; i!=capture; ++i)
    if (move_effect_journal[i].type==type
        && move_effect_journal[i].reason==reason)
      return i;

  return move_effect_journal_index_null;
}

static void write_departing_piece(FILE *file,
                                  move_effect_journal_index_type movement)
{
  if (WriteSpec(file,
                move_effect_journal[movement].u.piece_movement.movingspec,
                move_effect_journal[movement].u.piece_movement.moving,
                false)
      || move_effect_journal[movement].u.piece_movement.moving!=Pawn)
    WritePiece(file,move_effect_journal[movement].u.piece_movement.moving);

  WriteSquare(file,move_effect_journal[movement].u.piece_movement.from);
}


static void write_departure(FILE *file,
                            move_effect_journal_index_type movement)
{
  move_effect_journal_index_type const phantom_movement = find_pre_move_effect(move_effect_piece_movement,
                                                                               move_effect_reason_phantom_movement);

  if (phantom_movement==move_effect_journal_index_null)
    write_departing_piece(file,movement);
  else
  {
    write_departing_piece(file,phantom_movement);
    fputc('-',file);
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.from);
  }
}

void output_plaintext_write_capture(FILE *file,
                                    output_plaintext_move_context_type *context,
                                    move_effect_journal_index_type capture,
                                    move_effect_journal_index_type movement)
{
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;

  write_departure(file,movement);
  fputc('*',file);
  if (sq_capture==move_effect_journal[movement].u.piece_movement.to)
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.to);
  else if (move_effect_journal[capture].reason==move_effect_reason_ep_capture)
  {
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.to);
    fputs(" ep.",file);
  }
  else
  {
    WriteSquare(file,sq_capture);
    fputc('-',file);
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.to);
  }
}

void output_plaintext_write_no_capture(FILE *file,
                                       output_plaintext_move_context_type *context,
                                       move_effect_journal_index_type movement)
{
  write_departure(file,movement);
  fputc('-',file);
  WriteSquare(file,move_effect_journal[movement].u.piece_movement.to);
}

void output_plaintext_write_castling(FILE *file,
                                     move_effect_journal_index_type movement)
{
  if (CondFlag[castlingchess])
  {
    WritePiece(file,move_effect_journal[movement].u.piece_movement.moving);
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.from);
    fputc('-',file);
    WriteSquare(file,move_effect_journal[movement].u.piece_movement.to);
  }
  else
  {
    square const to = move_effect_journal[movement].u.piece_movement.to;
    if (to==square_g1 || to==square_g8)
      fputs("0-0",file);
    else
      fputs("0-0-0",file);
  }
}

static void write_exchange(FILE *file, move_effect_journal_index_type movement)
{
  WritePiece(file,get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.from));
  WriteSquare(file,move_effect_journal[movement].u.piece_exchange.to);
  fputs("<->",file);
  WritePiece(file,get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.to));
  WriteSquare(file,move_effect_journal[movement].u.piece_exchange.from);
}

static void write_singlebox_promotion(FILE *file,
                                      move_effect_journal_index_type curr)
{
  WriteSquare(file,move_effect_journal[curr].u.piece_change.on);
  fputs("=",file);
  WritePiece(file,move_effect_journal[curr].u.piece_change.to);
}

void output_plaintext_write_singlebox_type3_promotion(FILE *file)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const sb3_prom = find_pre_move_effect(move_effect_piece_change,
                                                                       move_effect_reason_singlebox_promotion);

  if (sb3_prom!=move_effect_journal_index_null)
  {
    output_plaintext_move_context_type context;
    output_plaintext_context_open(file,&context,base,"[","]");
    write_singlebox_promotion(file,sb3_prom);
    output_plaintext_context_close(&context);
  }
}

void output_plaintext_write_regular_move(FILE *file,
                                         output_plaintext_move_context_type *context)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_type const capture_type = move_effect_journal[capture].type;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  move_effect_type const movement_type = move_effect_journal[movement].type;

  assert(capture_type==move_effect_no_piece_removal
         || capture_type==move_effect_piece_removal
         || capture_type==move_effect_none);

  output_plaintext_context_open(file,context,base,"","");

  if (capture_type==move_effect_piece_removal)
  {
    assert(move_effect_journal[movement].type==move_effect_piece_movement);
    assert(move_effect_journal[movement].reason==move_effect_reason_moving_piece_movement);
    output_plaintext_write_capture(file,context,capture,movement);
  }
  else if (capture_type==move_effect_no_piece_removal)
  {
    assert(movement_type==move_effect_piece_movement
           || movement_type==move_effect_piece_exchange
           || movement_type==move_effect_none);
    if (movement_type==move_effect_piece_movement)
    {
      move_effect_reason_type const movement_reason = move_effect_journal[movement].reason;

      assert(movement_reason==move_effect_reason_moving_piece_movement
             || movement_reason==move_effect_reason_castling_king_movement);

      if (movement_reason==move_effect_reason_moving_piece_movement)
        output_plaintext_write_no_capture(file,context,movement);
      else
        output_plaintext_write_castling(file,movement);
    }
    else if (movement_type==move_effect_piece_exchange)
    {
      assert(move_effect_journal[movement].reason==move_effect_reason_exchange_castling_exchange
             || move_effect_journal[movement].reason==move_effect_reason_messigny_exchange);
      write_exchange(file,movement);
    }
  }
}

void output_plaintext_write_complete_piece(FILE *file,
                                           Flags spec,
                                           piece_walk_type piece,
                                           square on)
{
  WriteSpec(file,spec,piece,true);
  WritePiece(file,piece);
  WriteSquare(file,on);
}

static Flags find_piece_walk(output_plaintext_move_context_type const *context,
                             move_effect_journal_index_type curr,
                             square on)
{
  move_effect_journal_index_type m;

  for (m = curr-1; m>=context->start; --m)
    switch (move_effect_journal[m].type)
    {
      case move_effect_piece_movement:
        if (move_effect_journal[m].u.piece_movement.to==on)
          return move_effect_journal[m].u.piece_movement.moving;
        else
          break;

      case move_effect_piece_readdition:
        if (move_effect_journal[m].u.piece_addition.on==on)
          return move_effect_journal[m].u.piece_addition.walk;
        else
          break;

      default:break;
    }

  assert(0);
  return 0;
}

void output_plaintext_write_flags_change(output_plaintext_move_context_type *context,
                                         move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_pawn_promotion:
      fputc('=',context->file);
      WriteSpec(context->file,
                move_effect_journal[curr].u.flags_change.to,
                find_piece_walk(context,curr,move_effect_journal[curr].u.flags_change.on),
                false);
      break;

    case move_effect_reason_kobul_king:
      if (move_effect_journal[curr-1].type!=move_effect_piece_change
          || move_effect_journal[curr-1].reason!=move_effect_reason_kobul_king)
        /* otherwise the flags are written with the changed piece */
      {
        output_plaintext_next_context(context,curr,"[","]");
        WriteSquare(context->file,move_effect_journal[curr].u.flags_change.on);
        fputs("=",context->file);
        WriteSpec(context->file,
                  move_effect_journal[curr].u.flags_change.to,
                  being_solved.board[move_effect_journal[curr].u.flags_change.on],
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

void output_plaintext_write_side_change(output_plaintext_move_context_type *context,
                                        move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_andernach_chess:
    case move_effect_reason_volage_side_change:
    case move_effect_reason_magic_square:
    case move_effect_reason_circe_turncoats:
      fputc('=',context->file);
      fputc(side_shortcut(move_effect_journal[curr].u.side_change.to),
            context->file);
      break;

    case move_effect_reason_magic_piece:
    case move_effect_reason_masand:
    case move_effect_reason_hurdle_colour_changing:
      output_plaintext_next_context(context,curr,"[","]");
      WriteSquare(context->file,move_effect_journal[curr].u.side_change.on);
      fputc('=',context->file);
      fputc(side_shortcut(move_effect_journal[curr].u.side_change.to),
            context->file);
      break;

    default:
      break;
  }
}

static Flags find_piece_flags(output_plaintext_move_context_type const *context,
                              move_effect_journal_index_type curr,
                              square on)
{
  move_effect_journal_index_type m;

  for (m = curr-1; m>=context->start; --m)
    switch (move_effect_journal[m].type)
    {
      case move_effect_piece_movement:
        if (move_effect_journal[m].u.piece_movement.to==on)
          return move_effect_journal[m].u.piece_movement.movingspec;
        else
          break;

      case move_effect_piece_readdition:
        if (move_effect_journal[m].u.piece_addition.on==on)
          return move_effect_journal[m].u.piece_addition.flags;
        else
          break;

      default:break;
    }

  assert(0);
  return 0;
}

void output_plaintext_write_piece_change(output_plaintext_move_context_type *context,
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
        square const on = move_effect_journal[curr].u.piece_change.on;
        Flags const flags = find_piece_flags(context,curr,on);
        fputc('=',context->file);
        WriteSpec(context->file,flags,move_effect_journal[curr].u.piece_change.to,false);
        WritePiece(context->file,move_effect_journal[curr].u.piece_change.to);
      }
      break;

    case move_effect_reason_singlebox_promotion:
      /* type 3 is already dealt with, so this is type 2 */
      output_plaintext_next_context(context,curr,"[","]");
      write_singlebox_promotion(context->file,curr);
      break;

    case move_effect_reason_kobul_king:
      output_plaintext_next_context(context,curr,"[","]");

      WriteSquare(context->file,move_effect_journal[curr].u.piece_change.on);
      fputc('=',context->file);

      {
        Flags flags;

        if (move_effect_journal[curr+1].type==move_effect_flags_change
            && move_effect_journal[curr+1].reason==move_effect_reason_kobul_king)
          flags = move_effect_journal[curr+1].u.flags_change.to;
        else
          flags = BIT(Royal);

        WriteSpec(context->file,
                  flags,
                  move_effect_journal[curr].u.piece_change.to,
                  false);
      }

      WritePiece(context->file,move_effect_journal[curr].u.piece_change.to);
      break;

    case move_effect_reason_einstein_chess:
    case move_effect_reason_football_chess_substitution:
    case move_effect_reason_king_transmutation:
      fputc('=',context->file);
      WritePiece(context->file,move_effect_journal[curr].u.piece_change.to);
      break;

    default:
      break;
  }
}

void output_plaintext_write_piece_movement(output_plaintext_move_context_type *context,
                                           move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_moving_piece_movement:
      /* output_plaintext_write_capture() and output_plaintext_write_no_capture() have dealt with this */
      assert(0);
      break;

    case move_effect_reason_castling_king_movement:
      /* output_plaintext_write_castling() has dealt with this */
      assert(0);
      break;

    case move_effect_reason_castling_partner_movement:
      if (CondFlag[castlingchess])
      {
        fputc('/',context->file);
        output_plaintext_write_complete_piece(context->file,
                             move_effect_journal[curr].u.piece_movement.movingspec,
                             move_effect_journal[curr].u.piece_movement.moving,
                             move_effect_journal[curr].u.piece_movement.from);
        fputc('-',context->file);
        WriteSquare(context->file,move_effect_journal[curr].u.piece_movement.to);
      }
      else
      {
        /* implicitly written in castling symbols */
      }
      break;

    default:
      break;
  }
}

move_effect_journal_index_type output_plaintext_find_piece_removal(output_plaintext_move_context_type const *context,
                                                                   move_effect_journal_index_type curr,
                                                                   PieceIdType id_added)
{
  move_effect_journal_index_type m;

  for (m = curr-1; m>=context->start; --m)
    if (move_effect_journal[m].type==move_effect_piece_removal
        && move_effect_journal[m].reason!=move_effect_reason_regular_capture
        && move_effect_journal[m].reason!=move_effect_reason_ep_capture
        && GetPieceId(move_effect_journal[m].u.piece_removal.flags)==id_added)
      return m;

  return move_effect_journal_index_null;
}

static void write_transfer(output_plaintext_move_context_type *context,
                           move_effect_journal_index_type removal,
                           move_effect_journal_index_type addition)
{
  output_plaintext_next_context(context,removal,"[","]");

  output_plaintext_write_complete_piece(context->file,
                       move_effect_journal[removal].u.piece_removal.flags,
                       move_effect_journal[removal].u.piece_removal.walk,
                       move_effect_journal[removal].u.piece_removal.on);

  fputs("->",context->file);

  if (move_effect_journal[removal].u.piece_removal.flags
      !=move_effect_journal[addition].u.piece_addition.flags
      || (TSTFLAG(move_effect_journal[addition].u.piece_addition.flags,Royal)
          && is_king(move_effect_journal[removal].u.piece_removal.walk)
          && !is_king(move_effect_journal[addition].u.piece_addition.walk)))
  {
    WriteSpec(context->file,
              move_effect_journal[addition].u.piece_addition.flags,
              move_effect_journal[addition].u.piece_addition.walk,
              false);
    WritePiece(context->file,move_effect_journal[addition].u.piece_addition.walk);
  }
  else if (move_effect_journal[removal].u.piece_removal.walk
           !=move_effect_journal[addition].u.piece_addition.walk)
    WritePiece(context->file,move_effect_journal[addition].u.piece_addition.walk);

  WriteSquare(context->file,move_effect_journal[addition].u.piece_addition.on);
}

void output_plaintext_write_piece_creation(output_plaintext_move_context_type *context,
                                           move_effect_journal_index_type curr)
{
  output_plaintext_next_context(context,curr,"[+","]");
  output_plaintext_write_complete_piece(context->file,
                       move_effect_journal[curr].u.piece_addition.flags,
                       move_effect_journal[curr].u.piece_addition.walk,
                       move_effect_journal[curr].u.piece_addition.on);
}

void output_plaintext_write_piece_readdition(output_plaintext_move_context_type *context,
                                             move_effect_journal_index_type curr)
{
  if (move_effect_journal[curr].reason==move_effect_reason_volcanic_remember)
    fputs("->v",context->file);
  else
  {
    PieceIdType const id_added = GetPieceId(move_effect_journal[curr].u.piece_addition.flags);
    move_effect_journal_index_type const removal = output_plaintext_find_piece_removal(context,
                                                                      curr,
                                                                      id_added);
    if (removal==move_effect_journal_index_null)
      output_plaintext_write_piece_creation(context,curr);
    else
      write_transfer(context,removal,curr);
  }
}

void output_plaintext_write_piece_removal(output_plaintext_move_context_type *context,
                                          move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_no_reason:
    case move_effect_reason_regular_capture:
    case move_effect_reason_ep_capture:
      /* output_plaintext_write_capture() has dealt with these */
      assert(0);
      break;

    case move_effect_reason_transfer_no_choice:
    case move_effect_reason_transfer_choice:
      /* dealt with at the end of the transfer */
      break;

    case move_effect_reason_kamikaze_capturer:
      output_plaintext_next_context(context,curr,"[-","]");
      output_plaintext_write_complete_piece(context->file,
                           move_effect_journal[curr].u.piece_removal.flags,
                           move_effect_journal[curr].u.piece_removal.walk,
                           move_effect_journal[curr].u.piece_removal.on);
      break;

    case move_effect_reason_assassin_circe_rebirth:
      /* no output for the removal of an assassinated piece ... */
    case move_effect_reason_pawn_promotion:
      /* ... nor for the removal of a pawn promoted to imitator ... */
    case move_effect_reason_volcanic_remember:
      break;

    default:
      assert(0);
      break;
  }
}

void output_plaintext_write_piece_exchange(output_plaintext_move_context_type *context,
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
      output_plaintext_next_context(context,curr,"[","]");
      WritePiece(context->file,
                 get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.from));
      WriteSquare(context->file,move_effect_journal[curr].u.piece_exchange.to);
      fputs("<->",context->file);
      WritePiece(context->file,
                 get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.to));
      WriteSquare(context->file,move_effect_journal[curr].u.piece_exchange.from);
      break;

    default:
      write_exchange(context->file,curr);
      break;
  }
}

void output_plaintext_write_half_neutral_deneutralisation(output_plaintext_move_context_type *context,
                                                          move_effect_journal_index_type curr)
{
  fputc('=',context->file);
  fputc(side_shortcut(move_effect_journal[curr].u.half_neutral_phase_change.side),context->file);
  fputc('h',context->file);
}

void output_plaintext_write_half_neutral_neutralisation(output_plaintext_move_context_type *context,
                                                        move_effect_journal_index_type curr)
{
  fputs("=nh",context->file);
}

void output_plaintext_write_imitator_addition(output_plaintext_move_context_type *context)
{
  fputs("=I",context->file);
}

void output_plaintext_write_imitator_movement(output_plaintext_move_context_type *context,
                                              move_effect_journal_index_type curr)
{
  unsigned int const nr_moved = move_effect_journal[curr].u.imitator_movement.nr_moved;
  unsigned int icount;

  fputs("[I",context->file);

  for (icount = 0; icount<nr_moved; ++icount)
  {
    WriteSquare(context->file,being_solved.isquare[icount]);
    if (icount+1<nr_moved)
      fputc(',',context->file);
  }

  fputc(']',context->file);
}

void output_plaintext_write_bgl_status(output_plaintext_move_context_type *context,
                                       move_effect_journal_index_type curr)
{
  char buf[12];

  if (BGL_global)
  {
    if (move_effect_journal[curr].u.bgl_adjustment.side==White)
    {
      output_plaintext_next_context(context,curr," (",")");
      WriteBGLNumber(buf,BGL_values[White]);
      fprintf(context->file,buf);
    }
  }
  else
  {
    output_plaintext_next_context(context,curr," (",")");
    WriteBGLNumber(buf,BGL_values[White]);
    fprintf(context->file,buf);
    fputs("/",context->file);
    WriteBGLNumber(buf,BGL_values[Black]);
    fprintf(context->file,buf);
  }
}

static void write_other_effects(FILE *FILE, output_plaintext_move_context_type *context)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr = move_effect_journal_base[nbply];

  for (curr += move_effect_journal_index_offset_other_effects; curr!=top; ++curr)
  {
    switch (move_effect_journal[curr].type)
    {
      case move_effect_flags_change:
        output_plaintext_write_flags_change(context,curr);
        break;

      case move_effect_side_change:
        output_plaintext_write_side_change(context,curr);
        break;

      case move_effect_piece_change:
        output_plaintext_write_piece_change(context,curr);
        break;

      case move_effect_piece_movement:
        output_plaintext_write_piece_movement(context,curr);
        break;

      case move_effect_piece_creation:
        output_plaintext_write_piece_creation(context,curr);
        break;

      case move_effect_piece_readdition:
        output_plaintext_write_piece_readdition(context,curr);
        break;

      case move_effect_piece_removal:
        output_plaintext_write_piece_removal(context,curr);
        break;

      case move_effect_piece_exchange:
        output_plaintext_write_piece_exchange(context,curr);
        break;

      case move_effect_imitator_addition:
        output_plaintext_write_imitator_addition(context);
        break;

      case move_effect_imitator_movement:
        output_plaintext_write_imitator_movement(context,curr);
        break;

      case move_effect_half_neutral_deneutralisation:
        output_plaintext_write_half_neutral_deneutralisation(context,curr);
        break;

      case move_effect_half_neutral_neutralisation:
        output_plaintext_write_half_neutral_neutralisation(context,curr);
        break;

      case move_effect_bgl_adjustment:
        output_plaintext_write_bgl_status(context,curr);
        break;

      default:
        break;
    }
  }
}

void output_plaintext_write_move(FILE *file)
{
  output_plaintext_move_context_type context;

#ifdef _SE_DECORATE_SOLUTION_
  se_move(mov);
#endif

  if (CondFlag[singlebox] && SingleBoxType==ConditionType3)
    output_plaintext_write_singlebox_type3_promotion(file);

  output_plaintext_write_regular_move(file,&context);
  write_other_effects(file,&context);
  output_plaintext_context_close(&context);
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

static void select_output_mode(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.output_mode_selector.mode==output_mode_line)
    solving_insert_output_plaintext_line_slices(si);
  else
  {
    boolean const is_setplay = st->level==structure_traversal_level_setplay;
    solving_insert_output_plaintext_tree_slices(si,is_setplay);
  }

  if (twin_duplex_type!=twin_is_duplex)
  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STOutputPlaintextTwinningWriter)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  {
    slice_index const prototypes[] =
    {
        alloc_illegal_selfcheck_writer_slice(),
        alloc_output_plaintext_end_of_phase_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void write_position(FILE *file, slice_index si)
{
  switch (find_unique_goal(si).type)
  {
    case goal_atob:
      WritePositionAtoB(file,slices[si].starter);
      break;

    case goal_proofgame:
      WritePositionProofGame(file);
      break;

    default:
      WritePositionRegular(file);
      break;
  }
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void output_plaintext_write_position(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (twin_stage)
  {
    case twin_original_position_no_twins:
    case twin_zeroposition:
    case twin_initial:
      write_position(stdout,si);
      if (TraceFile)
        write_position(TraceFile,si);
      break;

    case twin_regular:
    case twin_last:
      break;

    default:
      assert(0);
      break;
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solving machinery with slices that write the solution in
 * plain text
 */
void output_plaintext_instrument_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STOutputModeSelector,
                                           &select_output_mode);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
