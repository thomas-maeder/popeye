#include "output/plaintext/plaintext.h"
#include "output/plaintext/condition.h"
#include "output/plaintext/pieces.h"
#include "output/plaintext/position.h"
#include "output/plaintext/protocol.h"
#include "output/plaintext/tree/tree.h"
#include "output/plaintext/line/line.h"
#include "output/plaintext/illegal_selfcheck_writer.h"
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

static void context_open(output_plaintext_move_context_type *context,
                         move_effect_journal_index_type start,
                         char const *opening_sequence,
                         char const *closing_sequence)
{
  protocol_fprintf(stdout,"%s",opening_sequence);

  context->start = start;
  context->closing_sequence = closing_sequence;
}

static void context_close(output_plaintext_move_context_type *context)
{
  protocol_fprintf(stdout,"%s",context->closing_sequence);
  context->start = move_effect_journal_index_null;
}

static void next_context(output_plaintext_move_context_type *context,
                         move_effect_journal_index_type start,
                         char const *opening_sequence,
                         char const *closing_sequence)
{
  context_close(context);
  context_open(context,start,opening_sequence,closing_sequence);
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

static void write_departing_piece(move_effect_journal_index_type movement)
{
  if (WriteSpec1(move_effect_journal[movement].u.piece_movement.movingspec,
                 move_effect_journal[movement].u.piece_movement.moving,
                 false)
      || move_effect_journal[movement].u.piece_movement.moving!=Pawn)
    WritePiece1(move_effect_journal[movement].u.piece_movement.moving);

  WriteSquare1(move_effect_journal[movement].u.piece_movement.from);
}

static void write_departure(move_effect_journal_index_type movement)
{
  move_effect_journal_index_type const phantom_movement = find_pre_move_effect(move_effect_piece_movement,
                                                                               move_effect_reason_phantom_movement);

  if (phantom_movement==move_effect_journal_index_null)
    write_departing_piece(movement);
  else
  {
    write_departing_piece(phantom_movement);
    protocol_fputc('-',stdout);
    WriteSquare1(move_effect_journal[movement].u.piece_movement.from);
  }
}

static void write_capture(output_plaintext_move_context_type *context,
                          move_effect_journal_index_type capture,
                          move_effect_journal_index_type movement)
{
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;

  write_departure(movement);
  protocol_fputc('*',stdout);
  if (sq_capture==move_effect_journal[movement].u.piece_movement.to)
    WriteSquare1(move_effect_journal[movement].u.piece_movement.to);
  else if (move_effect_journal[capture].reason==move_effect_reason_ep_capture)
  {
    WriteSquare1(move_effect_journal[movement].u.piece_movement.to);
    protocol_fprintf(stdout,"%s"," ep.");
  }
  else
  {
    WriteSquare1(sq_capture);
    protocol_fputc('-',stdout);
    WriteSquare1(move_effect_journal[movement].u.piece_movement.to);
  }
}

static void write_no_capture(output_plaintext_move_context_type *context,
                             move_effect_journal_index_type movement)
{
  write_departure(movement);
  protocol_fputc('-',stdout);
  WriteSquare1(move_effect_journal[movement].u.piece_movement.to);
}

static void write_castling(output_plaintext_move_context_type *context,
                           move_effect_journal_index_type movement)
{
  if (CondFlag[castlingchess])
  {
    WritePiece1(move_effect_journal[movement].u.piece_movement.moving);
    WriteSquare1(move_effect_journal[movement].u.piece_movement.from);
    protocol_fputc('-',stdout);
    WriteSquare1(move_effect_journal[movement].u.piece_movement.to);
  }
  else
  {
    square const to = move_effect_journal[movement].u.piece_movement.to;
    if (to==square_g1 || to==square_g8)
      protocol_fprintf(stdout,"%s","0-0");
    else
      protocol_fprintf(stdout,"%s","0-0-0");
  }
}

static void write_exchange(move_effect_journal_index_type movement)
{
  WritePiece1(get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.from));
  WriteSquare1(move_effect_journal[movement].u.piece_exchange.to);
  protocol_fprintf(stdout,"%s","<->");
  WritePiece1(get_walk_of_piece_on_square(move_effect_journal[movement].u.piece_exchange.to));
  WriteSquare1(move_effect_journal[movement].u.piece_exchange.from);
}

static void write_singlebox_promotion(move_effect_journal_index_type curr)
{
  WriteSquare1(move_effect_journal[curr].u.piece_change.on);
  protocol_fputc('=',stdout);
  WritePiece1(move_effect_journal[curr].u.piece_change.to);
}

static void write_singlebox_type3_promotion(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const sb3_prom = find_pre_move_effect(move_effect_piece_change,
                                                                       move_effect_reason_singlebox_promotion);

  if (sb3_prom!=move_effect_journal_index_null)
  {
    output_plaintext_move_context_type context;
    context_open(&context,base,"[","]");
    write_singlebox_promotion(sb3_prom);
    context_close(&context);
  }
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

  context_open(context,base,"","");

  if (capture_type==move_effect_piece_removal)
  {
    assert(move_effect_journal[movement].type==move_effect_piece_movement);
    assert(move_effect_journal[movement].reason==move_effect_reason_moving_piece_movement);
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
      write_exchange(movement);
    }
  }
}

static void write_complete_piece(Flags spec,
                                 piece_walk_type piece,
                                 square on)
{
  WriteSpec1(spec,piece,true);
  WritePiece1(piece);
  WriteSquare1(on);
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

static void write_flags_change(output_plaintext_move_context_type *context,
                               move_effect_journal_index_type curr)
{
  switch (move_effect_journal[curr].reason)
  {
    case move_effect_reason_pawn_promotion:
      protocol_fputc('=',stdout);
      WriteSpec1(move_effect_journal[curr].u.flags_change.to,
                 find_piece_walk(context,curr,move_effect_journal[curr].u.flags_change.on),
                 false);
      break;

    case move_effect_reason_kobul_king:
      if (move_effect_journal[curr-1].type!=move_effect_piece_change
          || move_effect_journal[curr-1].reason!=move_effect_reason_kobul_king)
        /* otherwise the flags are written with the changed piece */
      {
        next_context(context,curr,"[","]");
        WriteSquare1(move_effect_journal[curr].u.flags_change.on);
        protocol_fputc('=',stdout);
        WriteSpec1(move_effect_journal[curr].u.flags_change.to,
                   being_solved.board[move_effect_journal[curr].u.flags_change.on],
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
      protocol_fputc('=',stdout);
      protocol_fputc(tolower(ColourTab[move_effect_journal[curr].u.side_change.to][0]),stdout);
      break;

    case move_effect_reason_magic_piece:
    case move_effect_reason_masand:
    case move_effect_reason_hurdle_colour_changing:
      next_context(context,curr,"[","]");
      WriteSquare1(move_effect_journal[curr].u.side_change.on);
      protocol_fputc('=',stdout);
      protocol_fputc(tolower(ColourTab[move_effect_journal[curr].u.side_change.to][0]),stdout);
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

static void write_piece_change(output_plaintext_move_context_type *context,
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
        protocol_fputc('=',stdout);
        WriteSpec1(flags,move_effect_journal[curr].u.piece_change.to,false);
        WritePiece1(move_effect_journal[curr].u.piece_change.to);
      }
      break;

    case move_effect_reason_singlebox_promotion:
      /* type 3 is already dealt with, so this is type 2 */
      next_context(context,curr,"[","]");
      write_singlebox_promotion(curr);
      break;

    case move_effect_reason_kobul_king:
      next_context(context,curr,"[","]");

      WriteSquare1(move_effect_journal[curr].u.piece_change.on);
      protocol_fputc('=',stdout);

      {
        Flags flags;

        if (move_effect_journal[curr+1].type==move_effect_flags_change
            && move_effect_journal[curr+1].reason==move_effect_reason_kobul_king)
          flags = move_effect_journal[curr+1].u.flags_change.to;
        else
          flags = BIT(Royal);

        WriteSpec1(flags,move_effect_journal[curr].u.piece_change.to,false);
      }

      WritePiece1(move_effect_journal[curr].u.piece_change.to);
      break;

    case move_effect_reason_einstein_chess:
    case move_effect_reason_football_chess_substitution:
    case move_effect_reason_king_transmutation:
      protocol_fputc('=',stdout);
      WritePiece1(move_effect_journal[curr].u.piece_change.to);
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

    case move_effect_reason_castling_king_movement:
      /* write_castling() has dealt with this */
      assert(0);
      break;

    case move_effect_reason_castling_partner_movement:
      if (CondFlag[castlingchess])
      {
        protocol_fputc('/',stdout);
        write_complete_piece(move_effect_journal[curr].u.piece_movement.movingspec,
                                              move_effect_journal[curr].u.piece_movement.moving,
                                              move_effect_journal[curr].u.piece_movement.from);
        protocol_fputc('-',stdout);
        WriteSquare1(move_effect_journal[curr].u.piece_movement.to);
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

  write_complete_piece(move_effect_journal[removal].u.piece_removal.flags,
                                        move_effect_journal[removal].u.piece_removal.walk,
                                        move_effect_journal[removal].u.piece_removal.on);

  protocol_fprintf(stdout,"%s","->");

  if (move_effect_journal[removal].u.piece_removal.flags
      !=move_effect_journal[addition].u.piece_addition.flags
      || (TSTFLAG(move_effect_journal[addition].u.piece_addition.flags,Royal)
          && is_king(move_effect_journal[removal].u.piece_removal.walk)
          && !is_king(move_effect_journal[addition].u.piece_addition.walk)))
  {
    WriteSpec1(move_effect_journal[addition].u.piece_addition.flags,
               move_effect_journal[addition].u.piece_addition.walk,
               false);
    WritePiece1(move_effect_journal[addition].u.piece_addition.walk);
  }
  else if (move_effect_journal[removal].u.piece_removal.walk
           !=move_effect_journal[addition].u.piece_addition.walk)
    WritePiece1(move_effect_journal[addition].u.piece_addition.walk);

  WriteSquare1(move_effect_journal[addition].u.piece_addition.on);
}

static void write_piece_creation(output_plaintext_move_context_type *context,
                                 move_effect_journal_index_type curr)
{
  next_context(context,curr,"[+","]");
  write_complete_piece(move_effect_journal[curr].u.piece_addition.flags,
                       move_effect_journal[curr].u.piece_addition.walk,
                       move_effect_journal[curr].u.piece_addition.on);
}

static void write_piece_readdition(output_plaintext_move_context_type *context,
                                   move_effect_journal_index_type curr)
{
  if (move_effect_journal[curr].reason==move_effect_reason_volcanic_remember)
    protocol_fprintf(stdout,"%s","->v");
  else
  {
    PieceIdType const id_added = GetPieceId(move_effect_journal[curr].u.piece_addition.flags);
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
      next_context(context,curr,"[-","]");
      write_complete_piece(move_effect_journal[curr].u.piece_removal.flags,
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
      WritePiece1(get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.from));
      WriteSquare1(move_effect_journal[curr].u.piece_exchange.to);
      protocol_fprintf(stdout,"%s","<->");
      WritePiece1(get_walk_of_piece_on_square(move_effect_journal[curr].u.piece_exchange.to));
      WriteSquare1(move_effect_journal[curr].u.piece_exchange.from);
      break;

    default:
      write_exchange(curr);
      break;
  }
}

static void write_half_neutral_deneutralisation(output_plaintext_move_context_type *context,
                                                move_effect_journal_index_type curr)
{
  protocol_fputc('=',stdout);
  protocol_fputc(tolower(ColourTab[move_effect_journal[curr].u.half_neutral_phase_change.side][0]),stdout);
  protocol_fputc('h',stdout);
}

static void write_half_neutral_neutralisation(output_plaintext_move_context_type *context,
                                              move_effect_journal_index_type curr)
{
  protocol_fprintf(stdout,"%s","=nh");
}

static void write_imitator_addition(output_plaintext_move_context_type *context)
{
  protocol_fprintf(stdout,"=I");
}

static void write_imitator_movement(output_plaintext_move_context_type *context,
                                    move_effect_journal_index_type curr)
{
  unsigned int const nr_moved = move_effect_journal[curr].u.imitator_movement.nr_moved;
  unsigned int icount;

  protocol_fprintf(stdout,"[I");

  for (icount = 0; icount<nr_moved; ++icount)
  {
    WriteSquare1(being_solved.isquare[icount]);
    if (icount+1<nr_moved)
      protocol_fputc(',',stdout);
  }

  protocol_fputc(']',stdout);
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
      protocol_fprintf(stdout,"%s",buf);
    }
  }
  else
  {
    next_context(context,curr," (",")");
    WriteBGLNumber(buf,BGL_values[White]);
    protocol_fprintf(stdout,"%s",buf);
    protocol_fputc('/',stdout);
    WriteBGLNumber(buf,BGL_values[Black]);
    protocol_fprintf(stdout,"%s",buf);
  }
}

static void write_other_effects(output_plaintext_move_context_type *context)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr = move_effect_journal_base[nbply];

  for (curr += move_effect_journal_index_offset_other_effects; curr!=top; ++curr)
  {
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

      default:
        break;
    }
  }
}

void output_plaintext_write_move(void)
{
  output_plaintext_move_context_type context;

#ifdef _SE_DECORATE_SOLUTION_
  se_move(mov);
#endif

  if (CondFlag[singlebox] && SingleBoxType==ConditionType3)
    write_singlebox_type3_promotion();

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

static void write_position(slice_index si)
{
  switch (find_unique_goal(si).type)
  {
    case goal_atob:
      WritePositionAtoB(slices[si].starter);
      break;

    case goal_proofgame:
      WritePositionProofGame();
      break;

    default:
      WritePositionRegular();
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
      write_position(si);
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
