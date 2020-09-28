#include "output/plaintext/plaintext.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/position.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/tree/tree.h"
#include "output/plaintext/line/line.h"
#include "output/plaintext/end_of_phase_writer.h"
#include "output/plaintext/language_dependant.h"
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

#include <ctype.h>
#include <stdlib.h>

output_engine_type const output_plaintext_engine =
{
    &protocol_fputc,
    &protocol_vfprintf,
    &protocol_fprintf,
    &protocol_fflush
};

output_symbol_table_type const output_plaintext_symbol_table =
{
    "->",
    "<->",
    "0-0-0",
    "0-0"
};

boolean output_plaintext_check_indication_disabled = false;

/* Context are used to visually group the output related to one or more effects,
 * i.e. the flags, walk and rebirth square of a Circe rebirth ("[Sb8]").
 */
typedef struct
{
    output_engine_type const *engine;
    FILE *file;
    output_symbol_table_type const *symbol_table;
    move_effect_journal_index_type start;
    char const * closing_sequence;
} output_plaintext_move_context_type;

static void context_open(output_plaintext_move_context_type *context,
                         output_engine_type const *engine,
                         FILE *file,
                         output_symbol_table_type const *symbol_table,
                         move_effect_journal_index_type start,
                         char const *opening_sequence,
                         char const *closing_sequence)
{
  context->engine = engine;
  context->file = file;
  context->symbol_table = symbol_table;
  context->start = start;
  context->closing_sequence = closing_sequence;

  (*context->engine->fprintf)(context->file,"%s",opening_sequence);
}

static void context_close(output_plaintext_move_context_type *context)
{
  (*context->engine->fprintf)(context->file,"%s",context->closing_sequence);
  context->start = move_effect_journal_index_null;
}

static void next_context(output_plaintext_move_context_type *context,
                         move_effect_journal_index_type start,
                         char const *opening_sequence,
                         char const *closing_sequence)
{
  context_close(context);
  context_open(context,
               context->engine,
               context->file,
               context->symbol_table,
               start,
               opening_sequence,
               closing_sequence);
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

static void write_departing_piece(output_plaintext_move_context_type *context,
                                  move_effect_journal_index_type movement)
{
  if (WriteSpec(context->engine,context->file,
                move_effect_journal[movement].u.piece_movement.movingspec,
                move_effect_journal[movement].u.piece_movement.moving,
                false)
      || move_effect_journal[movement].u.piece_movement.moving!=Pawn)
    WriteWalk(context->engine,context->file,move_effect_journal[movement].u.piece_movement.moving);

  WriteSquare(context->engine,context->file,move_effect_journal[movement].u.piece_movement.from);
}

static void write_departure(output_plaintext_move_context_type *context,
                            move_effect_journal_index_type movement)
{
  move_effect_journal_index_type const phantom_movement = find_pre_move_effect(move_effect_piece_movement,
                                                                               move_effect_reason_phantom_movement);

  if (phantom_movement==move_effect_journal_index_null)
    write_departing_piece(context,movement);
  else
  {
    write_departing_piece(context,phantom_movement);
    (*context->engine->fputc)('-',context->file);
    WriteSquare(context->engine,context->file,move_effect_journal[movement].u.piece_movement.from);
  }
}

static void write_castling(output_plaintext_move_context_type *context,
                           move_effect_journal_index_type movement)
{
  if (CondFlag[castlingchess] || CondFlag[rokagogo])
  {
    WriteWalk(context->engine,context->file,move_effect_journal[movement].u.piece_movement.moving);
    WriteSquare(context->engine,context->file,move_effect_journal[movement].u.piece_movement.from);
    (*context->engine->fputc)('-',context->file);
    WriteSquare(context->engine,context->file,move_effect_journal[movement].u.piece_movement.to);
  }
  else
  {
    square const to = move_effect_journal[movement].u.piece_movement.to;
    output_symbol_type const symbol = to==square_g1 || to==square_g8 ? output_symbol_king_side_castling : output_symbol_queen_side_castling;
    (*context->engine->fprintf)(context->file,"%s",(*context->symbol_table)[symbol]);
  }
}

static void write_capture(output_plaintext_move_context_type *context,
                          move_effect_journal_index_type capture,
                          move_effect_journal_index_type movement)
{
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
  move_effect_reason_type const movement_reason = move_effect_journal[movement].reason;

  assert(movement_reason==move_effect_reason_moving_piece_movement
         || movement_reason==move_effect_reason_castling_king_movement);

  if (movement_reason==move_effect_reason_moving_piece_movement)
    write_departure(context,movement);
  else
    /* e.g. Make&Take */
    write_castling(context,movement);

  (*context->engine->fputc)('*',context->file);

  if (sq_capture==move_effect_journal[movement].u.piece_movement.to)
    WriteSquare(context->engine,context->file,move_effect_journal[movement].u.piece_movement.to);
  else if (move_effect_journal[capture].reason==move_effect_reason_ep_capture)
  {
    WriteSquare(context->engine,context->file,move_effect_journal[movement].u.piece_movement.to);
    (*context->engine->fprintf)(context->file,"%s"," ep.");
  }
  else
  {
    WriteSquare(context->engine,context->file,sq_capture);

    if (movement_reason!=move_effect_reason_castling_king_movement)
    {
      (*context->engine->fputc)('-',context->file);
      WriteSquare(context->engine,context->file,move_effect_journal[movement].u.piece_movement.to);
    }
  }
}

static void write_no_capture(output_plaintext_move_context_type *context,
                             move_effect_journal_index_type movement)
{
  write_departure(context,movement);
  (*context->engine->fputc)('-',context->file);
  WriteSquare(context->engine,context->file,move_effect_journal[movement].u.piece_movement.to);
}

static void write_exchange(output_plaintext_move_context_type *context,
                           move_effect_journal_index_type movement)
{
  WriteWalk(context->engine,context->file,get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.from));
  WriteSquare(context->engine,context->file,move_effect_journal[movement].u.piece_exchange.to);
  (*context->engine->fprintf)(context->file,"%s",(*context->symbol_table)[output_symbol_left_right_arrow]);
  WriteWalk(context->engine,context->file,get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.to));
  WriteSquare(context->engine,context->file,move_effect_journal[movement].u.piece_exchange.from);
}

static void write_regular_move(output_plaintext_move_context_type *context)
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
    write_capture(context,capture,movement);
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
        write_no_capture(context,movement);
      else
        write_castling(context,movement);
    }
    else if (movement_type==move_effect_piece_exchange)
    {
      assert(move_effect_journal[movement].reason==move_effect_reason_exchange_castling_exchange
             || move_effect_journal[movement].reason==move_effect_reason_messigny_exchange);
      write_exchange(context,movement);
    }
    else
      /* null move */
      (*context->engine->fprintf)(context->file,"%s"," ...");
  }
}

static void write_complete_piece(output_plaintext_move_context_type *context,
                                 Flags spec,
                                 piece_walk_type piece,
                                 square on)
{
  WriteSpec(context->engine,context->file,spec,piece,true);
  WriteWalk(context->engine,context->file,piece);
  WriteSquare(context->engine,context->file,on);
}

static piece_walk_type find_piece_walk(output_plaintext_move_context_type const *context,
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
        if (move_effect_journal[m].u.piece_addition.added.on==on)
          return move_effect_journal[m].u.piece_addition.added.walk;
        else
          break;

      default:break;
    }

  assert(0);
  return 0;
}

static void write_flags_change(output_plaintext_move_context_type *context,
                               move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_pawn_promotion:
      (*context->engine->fputc)('=',context->file);
      WriteSpec(context->engine,context->file,
                move_effect_journal[curr].u.flags_change.to,
                find_piece_walk(context,curr,move_effect_journal[curr].u.flags_change.on),
                false);
      break;

    case move_effect_reason_kobul_king:
      if (move_effect_journal[curr-1].type!=move_effect_walk_change
          || move_effect_journal[curr-1].reason!=move_effect_reason_kobul_king)
        /* otherwise the flags are written with the changed walk */
      {
        next_context(context,curr,"[","]");
        WriteSquare(context->engine,context->file,move_effect_journal[curr].u.flags_change.on);
        (*context->engine->fputc)('=',context->file);
        WriteSpec(context->engine,context->file,
                  move_effect_journal[curr].u.flags_change.to,
                  get_walk_of_piece_on_square(move_effect_journal[curr].u.flags_change.on),
                  false);
      }
      break;

    default:
      break;
  }
}

static void write_side_change(output_plaintext_move_context_type *context,
                              move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_andernach_chess:
    case move_effect_reason_volage_side_change:
    case move_effect_reason_magic_square:
    case move_effect_reason_circe_turncoats:
      (*context->engine->fputc)('=',context->file);
      (*context->engine->fputc)(tolower((unsigned char)ColourTab[move_effect_journal[curr].u.side_change.to][0]),context->file);
      break;

    case move_effect_reason_magic_piece:
    case move_effect_reason_masand:
    case move_effect_reason_hurdle_colour_changing:
    case move_effect_reason_breton:
      next_context(context,curr,"[","]");
      WriteSquare(context->engine,context->file,move_effect_journal[curr].u.side_change.on);
      (*context->engine->fputc)('=',context->file);
      (*context->engine->fputc)(tolower((unsigned char)ColourTab[move_effect_journal[curr].u.side_change.to][0]),context->file);
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
        if (move_effect_journal[m].u.piece_addition.added.on==on)
          return move_effect_journal[m].u.piece_addition.added.flags;
        else
          break;

      default:
        break;
    }

  assert(0);
  return 0;
}

static void write_singlebox_promotion(output_plaintext_move_context_type *context,
                                      move_effect_journal_index_type curr)
{
  WriteSquare(context->engine,context->file,move_effect_journal[curr].u.piece_walk_change.on);
  (*context->engine->fputc)('=',context->file);
  WriteWalk(context->engine,context->file,move_effect_journal[curr].u.piece_walk_change.to);
}

static void write_walk_change(output_plaintext_move_context_type *context,
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
          move_effect_journal[curr].u.piece_walk_change.to
          !=move_effect_journal[curr].u.piece_walk_change.from)
      {
        square const on = move_effect_journal[curr].u.piece_walk_change.on;
        Flags const flags = find_piece_flags(context,curr,on);
        (*context->engine->fputc)('=',context->file);
        WriteSpec(context->engine,context->file,flags,move_effect_journal[curr].u.piece_walk_change.to,false);
        WriteWalk(context->engine,context->file,move_effect_journal[curr].u.piece_walk_change.to);
      }
      break;

    case move_effect_reason_singlebox_promotion:
      /* type 3 is already dealt with, so this is type 2 */
      next_context(context,curr,"[","]");
      write_singlebox_promotion(context,curr);
      break;

    case move_effect_reason_kobul_king:
      next_context(context,curr,"[","]");

      WriteSquare(context->engine,context->file,move_effect_journal[curr].u.piece_walk_change.on);
      (*context->engine->fputc)('=',context->file);

      {
        Flags flags;

        if (move_effect_journal[curr+1].type==move_effect_flags_change
            && move_effect_journal[curr+1].reason==move_effect_reason_kobul_king)
          flags = move_effect_journal[curr+1].u.flags_change.to;
        else
          flags = BIT(Royal);

        WriteSpec(context->engine,context->file,flags,move_effect_journal[curr].u.piece_walk_change.to,false);
      }

      WriteWalk(context->engine,context->file,move_effect_journal[curr].u.piece_walk_change.to);
      break;

    case move_effect_reason_snek:
    case move_effect_reason_influencer:
    {
      square const on = move_effect_journal[curr].u.piece_walk_change.on;
      piece_walk_type const to = move_effect_journal[curr].u.piece_walk_change.to;

      next_context(context,curr,"[","]");

      WriteSquare(context->engine,context->file,on);
      (*context->engine->fputc)('=',context->file);
      WriteSpec(context->engine,context->file,being_solved.spec[on],to,false);
      WriteWalk(context->engine,context->file,to);
      break;
    }

    case move_effect_reason_einstein_chess:
    case move_effect_reason_football_chess_substitution:
    case move_effect_reason_king_transmutation:
      (*context->engine->fputc)('=',context->file);
      WriteWalk(context->engine,context->file,move_effect_journal[curr].u.piece_walk_change.to);
      break;

    default:
      break;
  }
}

static void write_piece_movement(output_plaintext_move_context_type *context,
                                 move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_moving_piece_movement:
      /* write_capture() and write_no_capture() have dealt with this */
      assert(0);
      break;

    case move_effect_reason_castling_partner:
      if (CondFlag[castlingchess] || CondFlag[rokagogo])
      {
        (*context->engine->fputc)('/',context->file);
        write_complete_piece(context,
                             move_effect_journal[curr].u.piece_movement.movingspec,
                             move_effect_journal[curr].u.piece_movement.moving,
                             move_effect_journal[curr].u.piece_movement.from);
        (*context->engine->fputc)('-',context->file);
        WriteSquare(context->engine,context->file,move_effect_journal[curr].u.piece_movement.to);
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

static
move_effect_journal_index_type
find_piece_removal(output_plaintext_move_context_type const *context,
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
  next_context(context,removal,"[","]");

  write_complete_piece(context,
                       move_effect_journal[removal].u.piece_removal.flags,
                       move_effect_journal[removal].u.piece_removal.walk,
                       move_effect_journal[removal].u.piece_removal.on);

  (*context->engine->fprintf)(context->file,"%s",(*context->symbol_table)[output_symbol_right_arrow]);

  if (move_effect_journal[removal].u.piece_removal.flags
      !=move_effect_journal[addition].u.piece_addition.added.flags
      || (TSTFLAG(move_effect_journal[addition].u.piece_addition.added.flags,Royal)
          && is_king(move_effect_journal[removal].u.piece_removal.walk)
          && !is_king(move_effect_journal[addition].u.piece_addition.added.walk)))
  {
    WriteSpec(context->engine,context->file,
               move_effect_journal[addition].u.piece_addition.added.flags,
               move_effect_journal[addition].u.piece_addition.added.walk,
               false);
    WriteWalk(context->engine,context->file,move_effect_journal[addition].u.piece_addition.added.walk);
  }
  else if (move_effect_journal[removal].u.piece_removal.walk
           !=move_effect_journal[addition].u.piece_addition.added.walk)
    WriteWalk(context->engine,context->file,move_effect_journal[addition].u.piece_addition.added.walk);

  WriteSquare(context->engine,context->file,move_effect_journal[addition].u.piece_addition.added.on);
}

static void write_piece_creation(output_plaintext_move_context_type *context,
                                 move_effect_journal_index_type curr)
{
  next_context(context,curr,"[+","]");
  write_complete_piece(context,
                       move_effect_journal[curr].u.piece_addition.added.flags,
                       move_effect_journal[curr].u.piece_addition.added.walk,
                       move_effect_journal[curr].u.piece_addition.added.on);
}

static void write_piece_readdition(output_plaintext_move_context_type *context,
                                   move_effect_journal_index_type curr)
{
  if (move_effect_journal[curr].reason==move_effect_reason_volcanic_remember)
    (*context->engine->fprintf)(context->file,"%s","->v");
  else
  {
    PieceIdType const id_added = GetPieceId(move_effect_journal[curr].u.piece_addition.added.flags);
    move_effect_journal_index_type const removal = find_piece_removal(context,
                                                                      curr,
                                                                      id_added);
    if (removal==move_effect_journal_index_null)
      write_piece_creation(context,curr);
    else
      write_transfer(context,removal,curr);
  }
}

static void write_piece_removal(output_plaintext_move_context_type *context,
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
      /* dealt with at the end of the transfer */
      break;

    case move_effect_reason_kamikaze_capturer:
    case move_effect_reason_zeroed_in:
    case move_effect_reason_breton:
      next_context(context,curr,"[-","]");
      write_complete_piece(context,
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

static void write_piece_exchange(output_plaintext_move_context_type *context,
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
      next_context(context,curr,"[","]");
      WriteWalk(context->engine,context->file,get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.from));
      WriteSquare(context->engine,context->file,move_effect_journal[curr].u.piece_exchange.to);
      (*context->engine->fprintf)(context->file,"%s",(*context->symbol_table)[output_symbol_left_right_arrow]);
      WriteWalk(context->engine,context->file,get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.to));
      WriteSquare(context->engine,context->file,move_effect_journal[curr].u.piece_exchange.from);
      break;

    default:
      write_exchange(context,curr);
      break;
  }
}

static void write_half_neutral_deneutralisation(output_plaintext_move_context_type *context,
                                                move_effect_journal_index_type curr)
{
  (*context->engine->fputc)('=',context->file);
  (*context->engine->fputc)(tolower((unsigned char)ColourTab[move_effect_journal[curr].u.half_neutral_phase_change.side][0]),context->file);
  (*context->engine->fputc)('h',context->file);
}

static void write_half_neutral_neutralisation(output_plaintext_move_context_type *context,
                                              move_effect_journal_index_type curr)
{
  (*context->engine->fprintf)(context->file,"%s","=nh");
}

static void write_imitator_addition(output_plaintext_move_context_type *context)
{
  (*context->engine->fprintf)(context->file,"=I");
}

static void write_imitator_movement(output_plaintext_move_context_type *context,
                                    move_effect_journal_index_type curr)
{
  unsigned int const nr_moved = move_effect_journal[curr].u.imitator_movement.nr_moved;
  unsigned int icount;

  (*context->engine->fprintf)(context->file,"[I");

  for (icount = 0; icount<nr_moved; ++icount)
  {
    WriteSquare(context->engine,context->file,being_solved.isquare[icount]);
    if (icount+1<nr_moved)
      (*context->engine->fputc)(',',context->file);
  }

  (*context->engine->fputc)(']',context->file);
}

static void write_bgl_status(output_plaintext_move_context_type *context,
                             move_effect_journal_index_type curr)
{
  char buf[12];

  if (BGL_global)
  {
    if (move_effect_journal[curr].u.bgl_adjustment.side==White)
    {
      next_context(context,curr," (",")");
      WriteBGLNumber(buf,BGL_values[White]);
      (*context->engine->fprintf)(context->file,"%s",buf);
    }
  }
  else
  {
    next_context(context,curr," (",")");
    WriteBGLNumber(buf,BGL_values[White]);
    (*context->engine->fprintf)(context->file,"%s",buf);
    (*context->engine->fputc)('/',context->file);
    WriteBGLNumber(buf,BGL_values[Black]);
    (*context->engine->fprintf)(context->file,"%s",buf);
  }
}

static void write_revelation(output_plaintext_move_context_type *context,
                             move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_revelation_of_invisible:
    {
      square const on = move_effect_journal[curr].u.piece_addition.added.on;
      piece_walk_type const walk = move_effect_journal[curr].u.piece_addition.added.walk;
      Flags const flags = move_effect_journal[curr].u.piece_addition.added.flags;
      next_context(context,curr,"[","]");
      WriteSquare(context->engine,context->file,on);
      (*context->engine->fputc)('=',context->file);
      WriteSpec(context->engine,context->file,flags,walk,true);
      WriteWalk(context->engine,context->file,walk);
      break;
    }

    default:
      assert(0);
      break;
  }
}

static void write_revelation_of_placed(output_plaintext_move_context_type *context,
                                       move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_revelation_of_invisible:
    {
      square const on = move_effect_journal[curr].u.revelation_of_placed_piece.on;
      piece_walk_type const walk = move_effect_journal[curr].u.revelation_of_placed_piece.walk_revealed;
      Flags const flags = move_effect_journal[curr].u.revelation_of_placed_piece.flags_revealed;
      next_context(context,curr,"[","]");
      WriteSquare(context->engine,context->file,on);
      (*context->engine->fputc)('=',context->file);
      WriteSpec(context->engine,context->file,flags,walk,true);
      WriteWalk(context->engine,context->file,walk);
      break;
    }

    default:
      assert(0);
      break;
  }
}

static void write_other_effects(output_plaintext_move_context_type *context,
                                move_effect_journal_index_type offset)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr = move_effect_journal_base[nbply];

  for (curr += offset; curr!=top; ++curr)
  {
    switch (move_effect_journal[curr].type)
    {
      case move_effect_flags_change:
        write_flags_change(context,curr);
        break;

      case move_effect_side_change:
        write_side_change(context,curr);
        break;

      case move_effect_walk_change:
        write_walk_change(context,curr);
        break;

      case move_effect_piece_movement:
        write_piece_movement(context,curr);
        break;

      case move_effect_piece_creation:
        write_piece_creation(context,curr);
        break;

      case move_effect_piece_readdition:
        write_piece_readdition(context,curr);
        break;

      case move_effect_piece_removal:
        write_piece_removal(context,curr);
        break;

      case move_effect_piece_exchange:
        write_piece_exchange(context,curr);
        break;

      case move_effect_imitator_addition:
        write_imitator_addition(context);
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

      case move_effect_revelation_of_castling_partner:
      case move_effect_revelation_of_new_invisible:
        write_revelation(context,curr);
        break;

      case move_effect_revelation_of_placed_invisible:
        write_revelation_of_placed(context,curr);
        break;

      default:
        break;
    }
  }
}

static void write_pre_capture_effect(output_engine_type const *engine,
                                     FILE *file,
                                     output_symbol_table_type const *symbol_table)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  switch (move_effect_journal[base].type)
  {
    case move_effect_walk_change:
    {
      output_plaintext_move_context_type context;
      context_open(&context,engine,file,symbol_table,base,"[","]");
      write_singlebox_promotion(&context,base);
      context_close(&context);
      break;
    }

    case move_effect_piece_creation:
    case move_effect_piece_readdition:
      if (move_effect_journal[base].u.piece_addition.added.on<capture_by_invisible)
      {
        output_plaintext_move_context_type context;
        context_open(&context,engine,file,symbol_table,move_effect_journal_base[nbply],"","");
        write_piece_creation(&context,base);
        context_close(&context);
      }
      break;

    default:
      break;
  }
}

void output_plaintext_write_move(output_engine_type const *engine,
                                 FILE *file,
                                 output_symbol_table_type const *symbol_table)
{
  output_plaintext_move_context_type context;

#ifdef _SE_DECORATE_SOLUTION_
  se_move(mov);
#endif

  if (move_effect_journal_index_offset_capture==1)
    write_pre_capture_effect(engine,file,symbol_table);

  context_open(&context,engine,file,symbol_table,move_effect_journal_base[nbply],"","");
  write_regular_move(&context);
  write_other_effects(&context,move_effect_journal_index_offset_other_effects);
  context_close(&context);
}

/* Write the effects of a dummy move (e.g. if the black "any" move has some
 * effects such as removal of white lost pieces
 */
void output_plaintext_write_dummy_move_effects(output_engine_type const *engine,
                                               FILE *file,
                                               output_symbol_table_type const *symbol_table)
{
  output_plaintext_move_context_type context;
  context_open(&context,engine,file,symbol_table,move_effect_journal_base[nbply],"","");
  write_other_effects(&context,0);
  context_close(&context);
}

static void visit_output_mode_selector(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_U(si).output_mode_selector.mode==output_mode_line)
    solving_insert_output_plaintext_line_slices(si);
  else
    solving_insert_output_plaintext_tree_slices(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/*
 *  Instrument the solving machinery with slices that write the solution in
 * plain text
 */
void output_plaintext_instrument_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_plaintext_check_indication_disabled = false;

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STOutputModeSelector,
                                           &visit_output_mode_selector);
  stip_traverse_structure(si,&st);

  {
    slice_index const prototypes[] =
    {
        alloc_output_plaintext_end_of_phase_writer_slice()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  solving_insert_move_inversion_counter_slices(si);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void output_plaintext_instrument_solving_builder_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] = {
        alloc_pipe(STOutputPlainTextInstrumentSolvers)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void output_plaintext_writer_solve(slice_index si)
{
  output_plaintext_print_version_info(stdout);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STOutputPlaintextProblemWriter)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert(si,prototypes,nr_prototypes);
  }

  pipe_solve_delegate(si);
}

slice_index output_plaintext_alloc_writer(FILE *file)
{
  slice_index const result = alloc_pipe(STOutputPlainTextWriter);
  SLICE_U(result).writer.file = file;
  return result;
}
