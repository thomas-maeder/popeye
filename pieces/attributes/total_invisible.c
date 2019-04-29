#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "position/position.h"
#include "position/move_diff_code.h"
#include "position/effects/piece_readdition.h"
#include "position/effects/piece_movement.h"
#include "position/effects/null_move.h"
#include "position/effects/walk_change.h"
#include "position/effects/flags_change.h"
#include "position/effects/king_square.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/goals/slice_insertion.h"
#include "solving/check.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/machinery/slack_length.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/move_effect_journal.h"
#include "solving/castling.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/pieces.h"
#include "optimisations/orthodox_square_observation.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

#include <string.h>

unsigned int total_invisible_number;

static unsigned int nr_total_invisibles_left;

static ply top_ply_of_regular_play;
static slice_index tester_slice;

static ply ply_replayed;

static stip_length_type combined_result;

static boolean end_of_iteration;

static unsigned int taboo[maxsquare];

static enum
{
  play_regular,
  play_rewinding,
  play_detecting_revelations,
  play_validating_mate,
  play_testing_mate,
  play_unwinding
} play_phase = play_regular;

typedef enum
{
  no_mate,
  mate_attackable,
  mate_defendable_by_interceptors,
  mate_with_2_uninterceptable_doublechecks,
  mate_unvalidated
} mate_validation_type;

static mate_validation_type mate_validation_result;
static mate_validation_type combined_validation_result;

static square sq_mating_piece_to_be_attacked = initsquare;

typedef struct
{
    square pos;
    piece_walk_type walk;
    Flags spec;
} revelation_status_type;

static boolean revelation_status_is_uninitialised;
static unsigned int nr_potential_revelations;
static revelation_status_type revelation_status[nr_squares_on_board];

static ply flesh_out_move_highwater = ply_retro_move;

static PieceIdType next_invisible_piece_id;

static boolean has_revelation_been_violated;

static boolean is_rider_check_uninterceptable_on_vector(Side side_checking, square king_pos,
                                                        vec_index_type k, piece_walk_type rider_walk)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceValue("%u",k);
  TraceWalk(rider_walk);
  TraceFunctionParamListEnd();

  {
    square s = king_pos+vec[k];
    while (is_square_empty(s) && taboo[s]>0 && taboo[s]>0)
      s += vec[k];

    {
      piece_walk_type const walk = get_walk_of_piece_on_square(s);
      result = ((walk==rider_walk || walk==Queen)
                && TSTFLAG(being_solved.spec[s],side_checking));
    }
    TraceSquare(s);
    TraceValue("%u",is_square_empty(s));
    TraceValue("%u",taboo[s]);
    TraceValue("%u",taboo[s]);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static vec_index_type is_rider_check_uninterceptable(Side side_checking, square king_pos,
                                                     vec_index_type kanf, vec_index_type kend, piece_walk_type rider_walk)
{
  vec_index_type result = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceValue("%u",kanf);
  TraceValue("%u",kend);
  TraceWalk(rider_walk);
  TraceFunctionParamListEnd();

  {
    vec_index_type k;
    for (k = kanf; !result && k<=kend; k++)
      if (is_rider_check_uninterceptable_on_vector(side_checking,king_pos,k,rider_walk))
      {
        result = k;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static vec_index_type is_square_uninterceptably_attacked(Side side_under_attack, square sq_attacked)
{
  vec_index_type result = 0;
  Side const side_checking = advers(side_under_attack);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_under_attack);
  TraceSquare(sq_attacked);
  TraceFunctionParamListEnd();

  if (being_solved.king_square[side_under_attack]!=initsquare)
  {
    if (!result && being_solved.number_of_pieces[side_checking][King]>0)
      result = king_check_ortho(side_checking,sq_attacked);

    if (!result && being_solved.number_of_pieces[side_checking][Pawn]>0)
      result = pawn_check_ortho(side_checking,sq_attacked);

    if (!result && being_solved.number_of_pieces[side_checking][Knight]>0)
      result = knight_check_ortho(side_checking,sq_attacked);

    if (!result && being_solved.number_of_pieces[side_checking][Rook]+being_solved.number_of_pieces[side_checking][Queen]>0)
      result = is_rider_check_uninterceptable(side_checking,sq_attacked, vec_rook_start,vec_rook_end, Rook);

    if (!result && being_solved.number_of_pieces[side_checking][Bishop]+being_solved.number_of_pieces[side_checking][Queen]>0)
      result = is_rider_check_uninterceptable(side_checking,sq_attacked, vec_bishop_start,vec_bishop_end, Bishop);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static vec_index_type is_square_attacked_by_uninterceptable(Side side_under_attack, square sq_attacked)
{
  vec_index_type result = 0;
  Side const side_checking = advers(side_under_attack);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_under_attack);
  TraceSquare(sq_attacked);
  TraceFunctionParamListEnd();

  if (being_solved.king_square[side_under_attack]!=initsquare)
  {
    if (!result && being_solved.number_of_pieces[side_checking][King]>0)
      result = king_check_ortho(side_checking,sq_attacked);

    if (!result && being_solved.number_of_pieces[side_checking][Pawn]>0)
      result = pawn_check_ortho(side_checking,sq_attacked);

    if (!result && being_solved.number_of_pieces[side_checking][Knight]>0)
      result = knight_check_ortho(side_checking,sq_attacked);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void do_revelation_of_new_invisible(move_effect_reason_type reason,
                                           square on,
                                           piece_walk_type walk,
                                           Flags spec)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_revelation_of_new_invisible,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(walk);
  TraceFunctionParam("%x",spec);
  TraceFunctionParamListEnd();

  assert(play_phase==play_regular);

  entry->u.piece_addition.added.on = on;
  entry->u.piece_addition.added.walk = walk;
  entry->u.piece_addition.added.flags = spec;

  if (TSTFLAG(spec,White))
    ++being_solved.number_of_pieces[White][walk];
  if (TSTFLAG(spec,Black))
    ++being_solved.number_of_pieces[Black][walk];
  occupy_square(on,walk,spec);

  if (TSTFLAG(spec,Royal) && walk==King)
  {
    Side const side = TSTFLAG(spec,White) ? White : Black;
    being_solved.king_square[side] = on;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_revelation_of_new_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (play_phase)
  {
    case play_regular:
      if (TSTFLAG(spec,White))
        --being_solved.number_of_pieces[White][walk];
      if (TSTFLAG(spec,Black))
        --being_solved.number_of_pieces[Black][walk];

      empty_square(on);

      if (TSTFLAG(spec,Royal) && walk==King)
      {
        Side const side = TSTFLAG(spec,White) ? White : Black;
        being_solved.king_square[side] = initsquare;
      }
      break;

    case play_rewinding:
      /* nothing since we are leaving the revealed piece on the board */
      break;

    case play_detecting_revelations:
    case play_validating_mate:
    case play_testing_mate:
      assert(!is_square_empty(on));
      ++nr_total_invisibles_left;
      break;

    case play_unwinding:
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_revelation_of_new_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (play_phase)
  {
    case play_regular:
      assert(is_square_empty(on));
      if (TSTFLAG(spec,White))
        ++being_solved.number_of_pieces[White][walk];
      if (TSTFLAG(spec,Black))
        ++being_solved.number_of_pieces[Black][walk];
      occupy_square(on,walk,spec);

      if (TSTFLAG(spec,Royal) && walk==King)
      {
        Side const side = TSTFLAG(spec,White) ? White : Black;
        being_solved.king_square[side] = on;
      }
      break;

    case play_unwinding:
      /* nothing since we are leaving the revealed piece on the board */
      break;

    case play_rewinding:
      assert(0);
      break;

    case play_detecting_revelations:
    case play_validating_mate:
    case play_testing_mate:
    {
      TraceSquare(on);
      TraceWalk(walk);
      TraceWalk(get_walk_of_piece_on_square(on));
      TraceValue("%u",nr_total_invisibles_left);
      TraceEOL();
      assert(!is_square_empty(on));

      if (nr_total_invisibles_left==0)
        // TODO this is ugly: we temporarily accept an integer underflow
        has_revelation_been_violated = true;
      else if (get_walk_of_piece_on_square(on)==walk)
      {
        /* go on */
      }
      else
        has_revelation_been_violated = true;

      --nr_total_invisibles_left;
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_revelation_of_castling_partner(move_effect_reason_type reason,
                                              square on,
                                              piece_walk_type walk,
                                              Flags spec)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_revelation_of_castling_partner,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(walk);
  TraceFunctionParam("%x",spec);
  TraceFunctionParamListEnd();

  assert(play_phase==play_regular);

  entry->u.piece_addition.added.on = on;
  entry->u.piece_addition.added.walk = walk;
  entry->u.piece_addition.added.flags = spec;

  assert(get_walk_of_piece_on_square(on)==Rook);
  assert(walk==Rook);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_revelation_of_castling_partner(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (play_phase)
  {
    case play_regular:
    case play_rewinding:
      TraceSquare(on);
      TraceValue("%x",being_solved.spec[on]);
      TraceEOL();
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      SETFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_detecting_revelations:
    case play_validating_mate:
    case play_testing_mate:
      break;

    case play_unwinding:
      assert(0);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_revelation_of_castling_partner(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(on);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
    case play_unwinding:
      TraceSquare(on);
      TraceValue("%x",being_solved.spec[on]);
      TraceEOL();
      assert(TSTFLAG(being_solved.spec[on],Chameleon));
      CLRFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_rewinding:
      assert(0);
      break;

    case play_detecting_revelations:
    case play_validating_mate:
      break;

    case play_testing_mate:
      assert(!is_square_empty(on));
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void taint_history_of_placed_piece(move_effect_journal_index_type idx)
{
  move_effect_journal_index_type const total_base = move_effect_journal_base[ply_retro_move+1];
  square pos = move_effect_journal[idx].u.piece_addition.added.on;
  PieceIdType const id = GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags);
  piece_walk_type const walk_to = move_effect_journal[idx].u.piece_addition.added.walk;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx);
  TraceFunctionParamListEnd();

  TraceSquare(pos);
  TraceWalk(walk_to);
  TraceValue("%u",id);
  TraceEOL();

  assert(move_effect_journal[idx].type==move_effect_revelation_of_placed_invisible);

  while (idx>=total_base)
  {
    TraceValue("%u",idx);
    TraceValue("%u",move_effect_journal[idx].type);
    TraceEOL();

    switch (move_effect_journal[idx].type)
    {
      case move_effect_piece_movement:
        TraceValue("%u",GetPieceId(move_effect_journal[idx].u.piece_movement.movingspec));
        TraceSquare(move_effect_journal[idx].u.piece_movement.to);
        TraceEOL();
        if (id==GetPieceId(move_effect_journal[idx].u.piece_movement.movingspec))
        {
          assert(pos==move_effect_journal[idx].u.piece_movement.to);
          TraceSquare(move_effect_journal[idx].u.piece_movement.from);
          TraceWalk(move_effect_journal[idx].u.piece_movement.moving);
          TraceEOL();
          assert(move_effect_journal[idx].u.piece_movement.moving==Dummy);
          pos = move_effect_journal[idx].u.piece_movement.from;
          move_effect_journal[idx].u.piece_movement.moving = walk_to;
          CLRFLAG(move_effect_journal[idx].u.piece_movement.movingspec,Chameleon);
        }
        else
          assert(pos!=move_effect_journal[idx].u.piece_movement.to);
        break;

      case move_effect_piece_readdition:
        TraceValue("%u",GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags));
        TraceSquare(move_effect_journal[idx].u.piece_addition.added.on);
        TraceEOL();
        if (id==GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags))
        {
          assert(pos==move_effect_journal[idx].u.piece_addition.added.on);
          if (pos>=capture_by_invisible)
          {
            TraceWalk(move_effect_journal[idx].u.piece_addition.added.walk);
            TraceEOL();
            move_effect_journal[idx].u.piece_addition.added.walk = walk_to;
            CLRFLAG(move_effect_journal[idx].u.piece_addition.added.flags,Chameleon);
          }
          idx = 1;
        }
        else
          assert(pos!=move_effect_journal[idx].u.piece_addition.added.on);
        break;

      default:
        break;
    }
    --idx;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void untaint_history_of_placed_piece(move_effect_journal_index_type idx)
{
  move_effect_journal_index_type const total_base = move_effect_journal_base[ply_retro_move+1];
  square pos = move_effect_journal[idx].u.piece_addition.added.on;
  PieceIdType const id = GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx);
  TraceFunctionParamListEnd();

  TraceSquare(pos);
  TraceWalk(move_effect_journal[idx].u.piece_addition.added.walk);
  TraceValue("%u",id);
  TraceEOL();

  assert(move_effect_journal[idx].type==move_effect_revelation_of_placed_invisible);

  while (idx>=total_base)
  {
    switch (move_effect_journal[idx].type)
    {
      case move_effect_piece_movement:
        TraceValue("%u",GetPieceId(move_effect_journal[idx].u.piece_movement.movingspec));
        TraceSquare(move_effect_journal[idx].u.piece_movement.to);
        TraceEOL();
        if (id==GetPieceId(move_effect_journal[idx].u.piece_movement.movingspec))
        {
          assert(pos==move_effect_journal[idx].u.piece_movement.to);
          pos = move_effect_journal[idx].u.piece_movement.from;
          move_effect_journal[idx].u.piece_movement.moving = Dummy;
          SETFLAG(move_effect_journal[idx].u.piece_movement.movingspec,Chameleon);
        }
        else
          assert(pos!=move_effect_journal[idx].u.piece_movement.to);
        break;

      case move_effect_piece_readdition:
        TraceValue("%u",GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags));
        TraceSquare(move_effect_journal[idx].u.piece_addition.added.on);
        TraceEOL();
        if (id==GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags))
        {
          assert(pos==move_effect_journal[idx].u.piece_addition.added.on);
          move_effect_journal[idx].u.piece_addition.added.walk = Dummy;
          SETFLAG(move_effect_journal[idx].u.piece_addition.added.flags,Chameleon);
          idx = 1;
        }
        else
          assert(pos!=move_effect_journal[idx].u.piece_addition.added.on);
        break;

      default:
        break;
    }

    --idx;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_revelation_of_placed_invisible(move_effect_reason_type reason,
                                              square on,
                                              piece_walk_type walk,
                                              Flags spec)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_revelation_of_placed_invisible,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(walk);
  TraceFunctionParam("%x",spec);
  TraceFunctionParamListEnd();

  assert(play_phase==play_regular);

  entry->u.piece_addition.added.on = on;
  entry->u.piece_addition.added.walk = walk;
  entry->u.piece_addition.added.flags = spec;

  if (TSTFLAG(spec,Royal) && walk==King)
  {
    Side const side = TSTFLAG(spec,White) ? White : Black;
    being_solved.king_square[side] = on;
  }

  replace_walk(on,walk);

  if (TSTFLAG(being_solved.spec[on],White))
    ++being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(being_solved.spec[on],Black))
    ++being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_revelation_of_placed_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",play_phase);
  TraceSquare(on);
  TraceWalk(walk);
  TraceWalk(get_walk_of_piece_on_square(on));
  TraceValue("%x",spec);
  TraceValue("%x",being_solved.spec[on]);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
      assert(!is_square_empty(on));

      if (TSTFLAG(spec,Royal) && walk==King)
      {
        Side const side = TSTFLAG(spec,White) ? White : Black;
        being_solved.king_square[side] = initsquare;
      }

      replace_walk(on,Dummy);

      if (TSTFLAG(being_solved.spec[on],White))
        --being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
      if (TSTFLAG(being_solved.spec[on],Black))
        --being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      SETFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_rewinding:
      taint_history_of_placed_piece(entry-&move_effect_journal[0]);
      assert(!is_square_empty(on));
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      break;

    case play_detecting_revelations:
    case play_validating_mate:
    case play_testing_mate:
      assert(!is_square_empty(on));
      // either get_walk_of_piece_on_square(on) is the revealed walk or iterations have
      // been canceled by redo_revelation_of_placed_invisible()
      assert(has_revelation_been_violated || get_walk_of_piece_on_square(on)==walk);
      break;

    case play_unwinding:
      assert(0);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_revelation_of_placed_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",play_phase);
  TraceSquare(on);
  TraceWalk(being_solved.board[on]);
  TraceWalk(walk);
  TraceValue("%x",being_solved.spec[on]);
  TraceValue("%x",spec);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
      assert(get_walk_of_piece_on_square(on)==Dummy);

      if (TSTFLAG(spec,Royal) && walk==King)
      {
        Side const side = TSTFLAG(spec,White) ? White : Black;
        being_solved.king_square[side] = on;
      }

      replace_walk(on,walk);

      if (TSTFLAG(being_solved.spec[on],White))
        ++being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
      if (TSTFLAG(being_solved.spec[on],Black))
        ++being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

      assert(TSTFLAG(being_solved.spec[on],Chameleon));
      CLRFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_rewinding:
      assert(0);
      break;

    case play_detecting_revelations:
    case play_validating_mate:
    case play_testing_mate:
      assert(!is_square_empty(on));
      if (get_walk_of_piece_on_square(on)!=walk)
      {
        TraceText("the revelation has been violated - terminating redoing effects with this ply");
        has_revelation_been_violated = true;
      }
      break;

    case play_unwinding:
      assert(!is_square_empty(on));
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      untaint_history_of_placed_piece(entry-&move_effect_journal[0]);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void add_revelation_effect(square s, piece_walk_type walk, Flags spec)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceWalk(walk);
  TraceFunctionParam("%x",spec);
  TraceFunctionParamListEnd();

  assert(TSTFLAG(spec,Chameleon));
  CLRFLAG(spec,Chameleon);

  if (is_square_empty(s))
  {
    TraceValue("%u",nbply);
    TraceValue("%u",nr_total_invisibles_left);
    TraceText("revelation of a hitherto unplaced invisible (typically a king)\n");
    SetPieceId(spec,++next_invisible_piece_id);
    do_revelation_of_new_invisible(move_effect_reason_revelation_of_invisible,
                                   s,walk,spec);
  }
  else
  {
    assert(TSTFLAG(being_solved.spec[s],Chameleon));
    CLRFLAG(being_solved.spec[s],Chameleon);

    if (move_effect_journal[base].type==move_effect_piece_readdition
        && move_effect_journal[base].reason==move_effect_reason_castling_partner
        && (GetPieceId(move_effect_journal[base].u.piece_addition.added.flags)
            ==GetPieceId(spec)))
    {
      TraceText("pseudo revelation of a castling partner\n");
      assert(GetPieceId(spec)==GetPieceId(being_solved.spec[s]));
      do_revelation_of_castling_partner(move_effect_reason_revelation_of_invisible,
                                        s,walk,spec);
    }
    else
    {
      TraceText("revelation of a placed invisible\n");
      SetPieceId(spec,GetPieceId(being_solved.spec[s]));
      do_revelation_of_placed_invisible(move_effect_reason_revelation_of_invisible,
                                        s,walk,spec);
    }
  }

  assert(!TSTFLAG(being_solved.spec[s],Chameleon));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void setup_revelations(void)
{
  square const *s;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_potential_revelations = 0;

  for (s = boardnum; *s; ++s)
    if (is_square_empty(*s) || TSTFLAG(being_solved.spec[*s],Chameleon))
    {
      revelation_status[nr_potential_revelations].pos = *s;
      ++nr_potential_revelations;
    }

  revelation_status_is_uninitialised = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void initialise_revelations(void)
{
  unsigned int i = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (i!=nr_potential_revelations)
  {
    square const s = revelation_status[i].pos;
    piece_walk_type const walk = get_walk_of_piece_on_square(s);
    if (walk==Empty)
    {
      memmove(&revelation_status[i],&revelation_status[i+1],
              (nr_potential_revelations-i-1)*sizeof revelation_status[0]);
      --nr_potential_revelations;
    }
    else
    {
      TraceSquare(s);
      TraceWalk(walk);
      TraceValue("%x",being_solved.spec[s]);
      TraceEOL();
      revelation_status[i].walk = walk;
      revelation_status[i].spec = being_solved.spec[s];
      ++i;
    }
  }

  revelation_status_is_uninitialised = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_revelations(void)
{
  unsigned int i = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (i!=nr_potential_revelations)
  {
    square const s = revelation_status[i].pos;
    if (get_walk_of_piece_on_square(s)!=revelation_status[i].walk
        || (being_solved.spec[s]&PieSpMask)!=(revelation_status[i].spec&PieSpMask))
    {
      TraceSquare(s);
      TraceWalk(get_walk_of_piece_on_square(s));
      TraceValue("%x",being_solved.spec[s]);
      TraceEOL();
      memmove(&revelation_status[i],&revelation_status[i+1],
              (nr_potential_revelations-i-1)*sizeof revelation_status[0]);
      --nr_potential_revelations;
    }
    else
      ++i;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void evaluate_revelations(void)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_potential_revelations; ++i)
  {
    square const s = revelation_status[i].pos;
    TraceSquare(s);TraceWalk(revelation_status[i].walk);TraceEOL();
    if (revelation_status[i].walk!=Empty)
      add_revelation_effect(s,revelation_status[i].walk,revelation_status[i].spec);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void play_with_placed_invisibles(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TracePosition(being_solved.board,being_solved.spec);

  mate_validation_result = mate_unvalidated;

  pipe_solve_delegate(tester_slice);

  if (solve_result>combined_result)
    combined_result = solve_result;

  switch (play_phase)
  {
    case play_regular:
      assert(0);
      break;

    case play_validating_mate:
      if (mate_validation_result<combined_validation_result)
        combined_validation_result = mate_validation_result;
      if (mate_validation_result<=mate_attackable)
        end_of_iteration = true;
      break;

    case play_testing_mate:
      /* This:
       * assert(solve_result>=previous_move_has_solved);
       * held surprisingly long, especially since it's wrong.
       * E.g. mate by castling: if we attack the rook, the castling is not
       * evan playable */
      if (solve_result==previous_move_has_not_solved)
        end_of_iteration = true;
      break;

    default:
      assert(0);
      break;
  }

  TraceValue("%u",end_of_iteration);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_validating_king_placements(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TracePosition(being_solved.board,being_solved.spec);

  if (play_phase==play_detecting_revelations)
  {
    if (revelation_status_is_uninitialised)
      initialise_revelations();
    else
      update_revelations();

    if (nr_potential_revelations==0)
      end_of_iteration = true;
  }
  else
  {
    do
    {
      --nbply;
      undo_move_effects();
    }
    while (nbply-1!=ply_retro_move);

    ply_replayed = nbply;
    nbply = top_ply_of_regular_play;

    play_with_placed_invisibles();

    nbply = ply_replayed;

    while (nbply<=top_ply_of_regular_play)
    {
      redo_move_effects();
      ++nbply;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_iteration(void);

static void restart_from_scratch(void)
{
  ply const save_nbply = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (nbply!=ply_retro_move+1)
  {
    --nbply;

    {
      numecoup const curr = CURRMOVE_OF_PLY(nbply);
      move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
      square const sq_departure = move_gen_top->departure;

      --taboo[sq_departure];
    }

    undo_move_effects();
  }

  TraceValue("%u",nbply);TraceEOL();
  start_iteration();

  while (nbply!=save_nbply)
  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
    square const sq_departure = move_gen_top->departure;

    redo_move_effects();

    ++taboo[sq_departure];

    ++nbply;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void nominate_king_invisible_by_invisible(void)
{
  Side const side_to_be_mated = Black;
  Side const side_mating = White;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    square const *s;
    TraceText("Try to make one of the placed invisibles the king to be mated\n");
    for (s = boardnum; *s && !end_of_iteration; ++s)
      if (get_walk_of_piece_on_square(*s)==Dummy
          && TSTFLAG(being_solved.spec[*s],side_to_be_mated))
      {
        Flags const save_flags = being_solved.spec[*s];
        CLRFLAG(being_solved.spec[*s],side_mating);
        SETFLAG(being_solved.spec[*s],Royal);
        ++being_solved.number_of_pieces[side_to_be_mated][King];
        being_solved.board[*s] = King;
        being_solved.king_square[side_to_be_mated] = *s;

        TraceSquare(*s);TraceEOL();
        restart_from_scratch();

        being_solved.board[*s] = Dummy;
        --being_solved.number_of_pieces[side_to_be_mated][King];
        being_solved.spec[*s] = save_flags;
      }

    being_solved.king_square[side_to_be_mated] = initsquare;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_king_placements(void)
{
  Side const side_to_be_mated = Black;
  Side const side_mating = White;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEnumerator(Side,side_to_be_mated);
  TraceSquare(being_solved.king_square[side_to_be_mated]);
  TraceValue("%u",nr_total_invisibles_left);
  TraceEnumerator(Side,side_mating);
  TraceSquare(being_solved.king_square[side_mating]);
  TraceEOL();
  TracePosition(being_solved.board,being_solved.spec);

  if (being_solved.king_square[side_to_be_mated]==initsquare)
  {
    if (nr_total_invisibles_left==0)
      nominate_king_invisible_by_invisible();
    else
    {
      TraceText("The king to be mated can be anywhere\n");

      switch (play_phase)
      {
        case play_detecting_revelations:
          if (revelation_status_is_uninitialised)
            initialise_revelations();
          else
            update_revelations();

          if (nr_potential_revelations==0)
            end_of_iteration = true;

          break;

        case play_validating_mate:
          combined_validation_result = no_mate;
          combined_result = previous_move_has_not_solved;
          end_of_iteration = true;
          break;

        default:
          combined_result = previous_move_has_not_solved;
          end_of_iteration = true;
          break;
      }
    }
  }
  else if (being_solved.king_square[side_mating]==initsquare
           && nr_total_invisibles_left==0)
    combined_result = previous_move_is_illegal;
  else
    done_validating_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void recurse_into_child_ply(void)
{
  ply const save_nbply = nbply;
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_departure = move_gen_top->departure;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].u.piece_movement.from==move_by_invisible
         || GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec)!=NullPieceId);

  ++taboo[sq_departure];

  has_revelation_been_violated = false;

  redo_move_effects();

  if (!has_revelation_been_violated)
  {
    if (nbply<=flesh_out_move_highwater)
    {
      ++nbply;
      TraceValue("%u",nbply);TraceEOL();
      start_iteration();
      nbply = save_nbply;
    }
    else
    {
      ply const save_highwater = flesh_out_move_highwater;
      flesh_out_move_highwater = nbply;
      ++nbply;
      TraceValue("%u",nbply);TraceEOL();
      start_iteration();
      nbply = save_nbply;
      flesh_out_move_highwater = save_highwater;
    }
  }

  undo_move_effects();

  --taboo[sq_departure];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_adapted_move_effects(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const to = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  if (move_effect_journal[capture].type==move_effect_no_piece_removal)
  {
    if (is_square_empty(to))
      /* no need for adaptation */
      recurse_into_child_ply();
    else
    {
      if (TSTFLAG(being_solved.spec[to],advers(trait[nbply])))
      {
        TraceText("capture of a total invisible that happened to land on the arrival square\n");

        /* if the piece to be captured is royal, then our tests for self check have failed */
        assert(!TSTFLAG(being_solved.spec[to],Royal));
        move_effect_journal[capture].type = move_effect_piece_removal;
        move_effect_journal[capture].reason = move_effect_reason_regular_capture;
        move_effect_journal[capture].u.piece_removal.on = to;
        move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
        move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];
        recurse_into_child_ply();
        move_effect_journal[capture].type = move_effect_no_piece_removal;
      }
      else
      {
        TraceText("move blocked by a random TI move\n");
      }
    }
  }
  else if (move_effect_journal[base].type==move_effect_piece_readdition
           && move_effect_journal[base].u.piece_addition.added.on==to)
  {
    if (is_square_empty(to))
    {
      if (nbply<=flesh_out_move_highwater)
      {
        TraceText("placed an invisible as victim in previous iteration\n");
        recurse_into_child_ply();
      }
      else if (nr_total_invisibles_left>0)
      {
        /* victim to be created - no need for adaptation, but for bookkeeping */
        TraceText("place an invisible as victim\n");
        --nr_total_invisibles_left;
        TraceValue("%u",nr_total_invisibles_left);TraceEOL();
        recurse_into_child_ply();
        ++nr_total_invisibles_left;
        TraceValue("%u",nr_total_invisibles_left);TraceEOL();
      }
      else
      {
        TraceText("no invisible left to be used as victim\n");
      }
    }
    else
    {
      assert(move_effect_journal[movement].u.piece_movement.moving==Pawn);
      TraceText("capture of a total invisible (created for this purpose) by a pawn, "
                "but another total invisible has moved to the arrival square\n");

      if (TSTFLAG(being_solved.spec[to],advers(trait[nbply])))
      {
        piece_walk_type const walk_victim_orig = move_effect_journal[capture].u.piece_removal.walk;
        /* if the piece to be captured is royal, then our tests for self check have failed */
        assert(!TSTFLAG(being_solved.spec[to],Royal));
        move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
        move_effect_journal[base].type = move_effect_none;
        recurse_into_child_ply();
        move_effect_journal[base].type = move_effect_piece_readdition;
        move_effect_journal[capture].u.piece_removal.walk = walk_victim_orig;
      }
      else
      {
        TraceText("move is now blocked\n");
      }
    }
  }
  else if (is_square_empty(to))
  {
    TraceText("original capture victim was captured by a TI that has since left\n");
    move_effect_journal[capture].type = move_effect_no_piece_removal;
    recurse_into_child_ply();
    move_effect_journal[capture].type = move_effect_piece_removal;
  }
  else
  {
    piece_walk_type const orig_walk_removed = move_effect_journal[capture].u.piece_removal.walk;
    Flags const orig_flags_removed = move_effect_journal[capture].u.piece_removal.flags;

    TraceText("adjusting capture to current piece that occupies the capture square\n");

    move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
    move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];
    recurse_into_child_ply();
    move_effect_journal[capture].u.piece_removal.walk = orig_walk_removed;
    move_effect_journal[capture].u.piece_removal.flags = orig_flags_removed;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_fleshing_out_move_by_invisible(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  recurse_into_child_ply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static void flesh_out_move_by_invisible_pawn(square s)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  Side const side = trait[nbply];
  int const dir = side==White ? dir_up : dir_down;

  numecoup const currmove = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  {
    square const sq_singlestep = move_effect_journal[movement].u.piece_movement.from+dir;
    TraceSquare(sq_singlestep);TraceEOL();
    if (is_square_empty(sq_singlestep))
    {
      if (taboo[sq_singlestep]==0)
      {
        move_effect_journal[movement].u.piece_movement.to = sq_singlestep;
        move_generation_stack[currmove].arrival = sq_singlestep;
        done_fleshing_out_move_by_invisible();
      }

      if (!end_of_iteration)
      {
        SquareFlags const doublstepsq = side==White ? WhPawnDoublestepSq : BlPawnDoublestepSq;
        if (TSTFLAG(sq_spec[move_effect_journal[movement].u.piece_movement.from],doublstepsq))
        {
          square const sq_doublestep = sq_singlestep+dir;
          TraceSquare(sq_doublestep);TraceEOL();
          if (is_square_empty(sq_doublestep))
          {
            if (taboo[sq_doublestep]==0)
            {
              move_effect_journal[movement].u.piece_movement.to = sq_doublestep;
              move_generation_stack[currmove].arrival = sq_doublestep;
              done_fleshing_out_move_by_invisible();
            }
          }
        }
      }
    }
  }


  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_move_by_invisible_rider(square s,
                                              vec_index_type kstart,
                                              vec_index_type kend)
{
  vec_index_type k;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  numecoup const currmove = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceValue("%u",kstart);
  TraceValue("%u",kend);
  TraceFunctionParamListEnd();

  TraceWalk(get_walk_of_piece_on_square(s));TraceEOL();

  assert(kstart<=kend);
  for (k = kstart; k<=kend && !end_of_iteration; ++k)
  {
    square sq_arrival;
    for (sq_arrival = move_effect_journal[movement].u.piece_movement.from+vec[k];
         is_square_empty(sq_arrival) && !end_of_iteration;
         sq_arrival += vec[k])
    {
      TraceSquare(sq_arrival);TraceEOL();
      if (taboo[sq_arrival]==0)
      {
        move_effect_journal[movement].u.piece_movement.to = sq_arrival;
        move_generation_stack[currmove].arrival = sq_arrival;
        done_fleshing_out_move_by_invisible();
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_move_by_invisible_leaper(square s,
                                               vec_index_type kstart,
                                               vec_index_type kend)
{
  vec_index_type k;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

  numecoup const currmove = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceValue("%u",kstart);
  TraceValue("%u",kend);
  TraceFunctionParamListEnd();

  TraceWalk(get_walk_of_piece_on_square(s));TraceEOL();

  assert(kstart<=kend);
  for (k = kstart; k<=kend && !end_of_iteration; ++k)
  {
    square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.from+vec[k];
    TraceSquare(sq_arrival);TraceEOL();
    if (is_square_empty(sq_arrival))
    {
      if (taboo[sq_arrival]==0)
      {
        move_effect_journal[movement].u.piece_movement.to = sq_arrival;
        move_generation_stack[currmove].arrival = sq_arrival;

        if (TSTFLAG(being_solved.spec[sq_departure],Royal))
        {
          assert(move_effect_journal[movement+1].type==move_effect_none);
          move_effect_journal[movement+1].type = move_effect_king_square_movement;
          move_effect_journal[movement+1].u.king_square_movement.from = sq_departure;
          move_effect_journal[movement+1].u.king_square_movement.to = sq_arrival;
          move_effect_journal[movement+1].u.king_square_movement.side = trait[nbply];
          done_fleshing_out_move_by_invisible();
          move_effect_journal[movement+1].type = move_effect_none;
        }
        else
          done_fleshing_out_move_by_invisible();
      }
    }
    else
      ;// TODO
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_move_by_specific_invisible(square s)
{
  piece_walk_type const walk_on_square = get_walk_of_piece_on_square(s);
  Flags const flags_on_square = being_solved.spec[s];
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  piece_walk_type const walk_moving = move_effect_journal[movement].u.piece_movement.moving;
  Flags const flags_moving = move_effect_journal[movement].u.piece_movement.movingspec;

  numecoup const currmove = CURRMOVE_OF_PLY(nbply);
  square const currmove_departure = move_generation_stack[currmove].departure;
  square const currmove_arrival = move_generation_stack[currmove].arrival;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  TraceWalk(walk_on_square);
  TraceValue("%x",flags_on_square);
  TraceWalk(walk_moving);
  TraceValue("%x",flags_moving);
  TraceEOL();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  assert(move_effect_journal[movement].u.piece_movement.from==move_by_invisible);
  move_effect_journal[movement].u.piece_movement.from = s;

  assert(currmove_arrival==move_by_invisible);
  move_generation_stack[currmove].departure = s;

  // TODO use a sibling ply and the regular move generation machinery?

  move_effect_journal[movement].u.piece_movement.moving = being_solved.board[s];
  CLRFLAG(being_solved.spec[s],advers(trait[nbply]));
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[s];

  if (walk_on_square==Dummy)
  {
    Side const side_under_attack = advers(trait[nbply]);
    square const king_pos = being_solved.king_square[side_under_attack];

    assert(play_phase==play_validating_mate);

    if (!end_of_iteration)
    {
      Side const side = trait[nbply];
      if (being_solved.king_square[side]==initsquare)
      {
        being_solved.king_square[side] = s;
        ++being_solved.number_of_pieces[trait[nbply]][King];
        being_solved.board[s] = King;
        SETFLAG(being_solved.spec[s],Royal);
        flesh_out_move_by_invisible_leaper(s,vec_queen_start,vec_queen_end);
        CLRFLAG(being_solved.spec[s],Royal);
        --being_solved.number_of_pieces[trait[nbply]][King];
        being_solved.king_square[side] = initsquare;
      }
    }

    if (!end_of_iteration)
    {
      Side const side = trait[nbply];
      SquareFlags const promsq = side==White ? WhPromSq : BlPromSq;
      SquareFlags const basesq = side==White ? WhBaseSq : BlBaseSq;
      if (!(TSTFLAG(sq_spec[s],basesq) || TSTFLAG(sq_spec[s],promsq)))
      {
        ++being_solved.number_of_pieces[trait[nbply]][Pawn];
        being_solved.board[s] = Pawn;
        if (!(king_pos!=initsquare && pawn_check_ortho(trait[nbply],king_pos)))
          flesh_out_move_by_invisible_pawn(s);
        --being_solved.number_of_pieces[trait[nbply]][Pawn];
      }
    }

    if (!end_of_iteration)
    {
      ++being_solved.number_of_pieces[trait[nbply]][Knight];
      being_solved.board[s] = Knight;
      if (!(king_pos!=initsquare && knight_check_ortho(trait[nbply],king_pos)))
        flesh_out_move_by_invisible_leaper(s,vec_knight_start,vec_knight_end);
      --being_solved.number_of_pieces[trait[nbply]][Knight];
    }

    if (!end_of_iteration)
    {
      ++being_solved.number_of_pieces[trait[nbply]][Bishop];
      being_solved.board[s] = Bishop;
      if (!is_rider_check_uninterceptable(trait[nbply],king_pos,
                                          vec_bishop_start,vec_bishop_end,
                                          Bishop))
        flesh_out_move_by_invisible_rider(s,vec_bishop_start,vec_bishop_end);
      --being_solved.number_of_pieces[trait[nbply]][Bishop];
    }

    if (!end_of_iteration)
    {
      ++being_solved.number_of_pieces[trait[nbply]][Rook];
      being_solved.board[s] = Rook;
      if (!is_rider_check_uninterceptable(trait[nbply],king_pos,
                                          vec_rook_start,vec_rook_end,
                                          Rook))
      flesh_out_move_by_invisible_rider(s,vec_rook_start,vec_rook_end);
      --being_solved.number_of_pieces[trait[nbply]][Rook];
    }

    if (!end_of_iteration)
    {
      ++being_solved.number_of_pieces[trait[nbply]][Queen];
      being_solved.board[s] = Queen;
      if (!is_rider_check_uninterceptable(trait[nbply],king_pos,
                                          vec_queen_start,vec_queen_end,
                                          Queen))
        flesh_out_move_by_invisible_rider(s,vec_queen_start,vec_queen_end);
      --being_solved.number_of_pieces[trait[nbply]][Queen];
    }
  }
  else
  {
    switch (walk_on_square)
    {
      case King:
        flesh_out_move_by_invisible_leaper(s,vec_queen_start,vec_queen_end);
        break;

      case Queen:
        flesh_out_move_by_invisible_rider(s,vec_queen_start,vec_queen_end);
        break;

      case Rook:
        flesh_out_move_by_invisible_rider(s,vec_rook_start,vec_rook_end);
        break;

      case Bishop:
        flesh_out_move_by_invisible_rider(s,vec_bishop_start,vec_bishop_end);
        break;

      case Knight:
        flesh_out_move_by_invisible_leaper(s,vec_knight_start,vec_knight_end);
        break;

      case Pawn:
        flesh_out_move_by_invisible_pawn(s);
        break;

      default:
        break;
    }
  }

  being_solved.board[s] = walk_on_square;
  being_solved.spec[s] = flags_on_square;

  move_generation_stack[currmove].departure = currmove_departure;
  move_generation_stack[currmove].arrival = currmove_arrival;

  move_effect_journal[movement].u.piece_movement.from = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.to = move_by_invisible;
  move_effect_journal[movement].u.piece_movement.moving = walk_moving;
  move_effect_journal[movement].u.piece_movement.movingspec = flags_moving;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_move_by_invisible(void)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt * const move_gen_top = move_generation_stack+curr;
  Side const side_playing = trait[nbply];
  square const *s;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side_playing);
  TraceEOL();

  for (s = boardnum; *s && !end_of_iteration; ++s)
    if (TSTFLAG(being_solved.spec[*s],Chameleon)
        && TSTFLAG(being_solved.spec[*s],side_playing))
    {
      Flags const save_flags = being_solved.spec[*s];
      CLRFLAG(being_solved.spec[*s],advers(side_playing));
      move_gen_top->departure = *s;
      flesh_out_move_by_specific_invisible(*s);
      move_gen_top->departure = move_by_invisible;
      being_solved.spec[*s] = save_flags;
    }

  if (nr_total_invisibles_left>0)
  {
    // TODO avoid random moves by 1 unplaced invisible for both sides
    TraceText("random move by unplaced invisible\n");
    done_fleshing_out_move_by_invisible();
  }
  else if (nbply==6)
  {
    // TODO
    TraceText("random move by invisible to its placement in first ply\n");
    done_fleshing_out_move_by_invisible();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_specific_invisible(piece_walk_type walk_capturing,
                                                    square from)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt * const move_gen_top = move_generation_stack+curr;
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const precapture = effects_base;
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  piece_walk_type const save_removed_walk = move_effect_journal[capture].u.piece_removal.walk;
  Flags const save_removed_spec = move_effect_journal[capture].u.piece_removal.flags;
  Flags const save_moving_spec = move_effect_journal[movement].u.piece_movement.movingspec;
  square const sq_created_on = move_effect_journal[movement].u.piece_movement.from;
  square const sq_capture = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  assert(move_gen_top->departure==sq_created_on);

  assert(!TSTFLAG(being_solved.spec[from],advers(trait[nbply])));

  move_effect_journal[precapture].type = move_effect_none;

  move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(sq_capture);
  move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[sq_capture];

  move_effect_journal[movement].u.piece_movement.from = from;
  move_effect_journal[movement].u.piece_movement.moving = walk_capturing;
  move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[from];

  move_gen_top->departure = from;

  recurse_into_child_ply();

  move_gen_top->departure = sq_created_on;

  move_effect_journal[movement].u.piece_movement.from = sq_created_on;
  move_effect_journal[movement].u.piece_movement.moving = Dummy;
  move_effect_journal[movement].u.piece_movement.movingspec = save_moving_spec;

  move_effect_journal[capture].u.piece_removal.walk = save_removed_walk;
  move_effect_journal[capture].u.piece_removal.flags = save_removed_spec;

  move_effect_journal[precapture].type = move_effect_piece_readdition;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_inserted_invisible(piece_walk_type walk_capturing,
                                                    square from)
{
  Side const side_playing = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  if (taboo[from]==0)
  {
    TraceValue("%u",nr_total_invisibles_left);TraceEOL();
    if (nr_total_invisibles_left>0)
    {
      /* insert the capturer with the flags from the journal, which may have been
       * tainted if the piece was eventually revealed
       */
      move_effect_journal_index_type const base = move_effect_journal_base[nbply];
      move_effect_journal_index_type const pre_capture_effect = base;
      Flags const flags = move_effect_journal[pre_capture_effect].u.piece_addition.added.flags;
      Side const side_in_check = trait[nbply-1];
      square const king_pos = being_solved.king_square[side_in_check];

      assert(move_effect_journal[pre_capture_effect].type==move_effect_piece_readdition);

      --nr_total_invisibles_left;
      TraceValue("%u",nr_total_invisibles_left);TraceEOL();

      occupy_square(from,walk_capturing,flags);

      ++taboo[from];

      ++being_solved.number_of_pieces[side_playing][walk_capturing];

      if (!is_square_uninterceptably_attacked(side_in_check,king_pos))
        flesh_out_capture_by_specific_invisible(walk_capturing,from);

      --being_solved.number_of_pieces[side_playing][walk_capturing];

      --taboo[from];

      empty_square(from);

      ++nr_total_invisibles_left;
      TraceValue("%u",nr_total_invisibles_left);TraceEOL();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_existing_invisible(piece_walk_type walk_capturing,
                                                    square from)
{
  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  TraceValue("%u",TSTFLAG(being_solved.spec[from],Chameleon));
  TraceValue("%u",TSTFLAG(being_solved.spec[from],trait[nbply]));
  TraceWalk(get_walk_of_piece_on_square(from));
  TraceEOL();

  if (TSTFLAG(being_solved.spec[from],Chameleon)
      && TSTFLAG(being_solved.spec[from],trait[nbply]))
  {
    if (get_walk_of_piece_on_square(from)==walk_capturing)
      flesh_out_capture_by_specific_invisible(walk_capturing,from);
    else if (get_walk_of_piece_on_square(from)==Dummy)
    {
      Side const side_in_check = trait[nbply-1];
      square const king_pos = being_solved.king_square[side_in_check];
      Flags const flags = being_solved.spec[from];
      ++being_solved.number_of_pieces[trait[nbply]][walk_capturing];
      being_solved.board[from] = walk_capturing;
      CLRFLAG(being_solved.spec[from],advers(trait[nbply]));
      if (!is_square_uninterceptably_attacked(side_in_check,king_pos))
        flesh_out_capture_by_specific_invisible(walk_capturing,from);
      being_solved.spec[from] = flags;
      being_solved.board[from] = Dummy;
      --being_solved.number_of_pieces[trait[nbply]][walk_capturing];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_rider(piece_walk_type walk_rider,
                                                  vec_index_type kcurr, vec_index_type kend)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[capture].type==move_effect_piece_removal);

  TraceSquare(sq_capture);TraceEOL();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square s;
    for (s = sq_capture+vec[kcurr];
         is_square_empty(s) && !end_of_iteration;
         s += vec[kcurr])
    {
      flesh_out_capture_by_inserted_invisible(walk_rider,s);
      ++taboo[s];
    }

    if (!end_of_iteration)
      flesh_out_capture_by_existing_invisible(walk_rider,s);

    for (s -= vec[kcurr]; s!=sq_capture; s -= vec[kcurr])
      --taboo[s];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_leaper(piece_walk_type walk_leaper,
                                                   vec_index_type kcurr, vec_index_type kend)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square const s = sq_capture+vec[kcurr];
    if (is_square_empty(s))
      flesh_out_capture_by_inserted_invisible(walk_leaper,s);
    else
      flesh_out_capture_by_existing_invisible(walk_leaper,s);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_pawn(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
  int const dir_vert = trait[nbply]==White ? -dir_up : -dir_down;
  SquareFlags const promsq = trait[nbply]==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!end_of_iteration)
  {
    square s = sq_capture+dir_vert+dir_left;
    if (is_square_empty(s) && !TSTFLAG(sq_spec[s],basesq) && !TSTFLAG(sq_spec[s],promsq))
      flesh_out_capture_by_inserted_invisible(Pawn,s);
  }

  if (!end_of_iteration)
  {
    square s = sq_capture+dir_vert+dir_right;
    if (is_square_empty(s) && !TSTFLAG(sq_spec[s],basesq) && !TSTFLAG(sq_spec[s],promsq))
      flesh_out_capture_by_inserted_invisible(Pawn,s);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible_walk_by_walk(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  // TODO King
  flesh_out_capture_by_invisible_pawn();
  flesh_out_capture_by_invisible_leaper(Knight,vec_knight_start,vec_knight_end);
  flesh_out_capture_by_invisible_rider(Bishop,vec_bishop_start,vec_bishop_end);
  flesh_out_capture_by_invisible_rider(Rook,vec_rook_start,vec_rook_end);
  flesh_out_capture_by_invisible_rider(Queen,vec_queen_start,vec_queen_end);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_invisible(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (is_square_empty(sq_capture))
  {
    // TODO we shouldn't arrive here, but currently we do:
    // 1 TI was revealed on capture square
    // 2 a move with a TI capturing the revealed piece was generated (and is being played)
    // 3 the revealed piece has left before that in a TI~ move
    // 4 there is nothing to remove when the move generated in 2 is played
    // this case is illegal because 3 shouldn't occur
    TraceText("capture by invisible, but invisible has left\n");
  }
  else
    flesh_out_capture_by_invisible_walk_by_walk();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_intercepting_illegal_checks(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (nbply<=top_ply_of_regular_play)
  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt const * const move_gen_top = move_generation_stack+curr;

    TraceValue("%u",nbply);
    TraceValue("%u",top_ply_of_regular_play);
    TraceSquare(move_gen_top->departure);
    TraceValue("%u",move_gen_top->departure);
    TraceValue("%u",capture_by_invisible);
    TraceValue("%u",flesh_out_move_highwater);
    TraceSquare(move_gen_top->arrival);
    TraceValue("%u",move_gen_top->arrival);
    TraceValue("%u",move_by_invisible);
    TraceEOL();

    if (nbply<=flesh_out_move_highwater)
      redo_adapted_move_effects();
    else if (move_gen_top->departure>=capture_by_invisible
             && is_on_board(move_gen_top->arrival))
      flesh_out_capture_by_invisible();
    else if (move_gen_top->departure==move_by_invisible
             && move_gen_top->arrival==move_by_invisible)
      flesh_out_move_by_invisible();
    else
      redo_adapted_move_effects();
  }
  else
    validate_king_placements();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor_any_walk(Side side,
                                      square pos,
                                      piece_walk_type walk)
{
  Flags spec = BIT(side)|BIT(Chameleon);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  ++being_solved.number_of_pieces[side][walk];
  SetPieceId(spec,++next_invisible_piece_id);
  occupy_square(pos,walk,spec);
  if (walk==King)
  {
    SETFLAG(being_solved.spec[pos],Royal);
    {
      Side const side_attacked = side;
      // TODO use pos instead of king_pos (or assert that they are equal)
      square const king_pos = being_solved.king_square[side_attacked];
      vec_index_type const k = is_square_uninterceptably_attacked(side_attacked,
                                                                  king_pos);
      if (k==0)
        restart_from_scratch();
    }
  }
  else
  {
    Side const side_attacked = advers(side);
    square const king_pos = being_solved.king_square[side_attacked];
    vec_index_type const k = is_square_uninterceptably_attacked(side_attacked,
                                                                king_pos);
    if (k==0 || king_pos+vec[k]!=pos)
      restart_from_scratch();
  }
  TraceWalk(get_walk_of_piece_on_square(pos));
  TraceWalk(walk);
  TraceEOL();
  assert(get_walk_of_piece_on_square(pos)==walk);
  empty_square(pos);
  --next_invisible_piece_id;
  --being_solved.number_of_pieces[side][walk];

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor_pawn(Side side, square pos)
{
  SquareFlags const promsq = side==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = side==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  if (!(TSTFLAG(sq_spec[pos],basesq) || TSTFLAG(sq_spec[pos],promsq)))
    walk_interceptor_any_walk(side,pos,Pawn);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor_king(Side side, square pos)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  being_solved.king_square[side] = pos;
  walk_interceptor_any_walk(side,pos,King);
  being_solved.king_square[side] = initsquare;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor(Side side, square pos)
{
  piece_walk_type walk;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  TraceSquare(pos);
  TraceSquare(being_solved.king_square[side]);
  TraceEOL();
  assert(is_square_empty(pos));

  if (being_solved.king_square[side]==initsquare)
    walk_interceptor_king(side,pos);

  if (!end_of_iteration)
    walk_interceptor_pawn(side,pos);

  for (walk = Queen; walk<=Bishop && !end_of_iteration; ++walk)
    walk_interceptor_any_walk(side,pos,walk);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void colour_interceptor(Side preferred_side, square pos)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,preferred_side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  /* taboo equal to 1 is ok: this is "my" taboo! */
  if (taboo[pos]==1)
  {
    walk_interceptor(preferred_side,pos);

    if (!end_of_iteration)
      walk_interceptor(advers(preferred_side),pos);
  }

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef void intercept_checks_fct(vec_index_type kcurr);

static void place_interceptor_on_square(vec_index_type kcurr,
                                        square s,
                                        piece_walk_type const walk_at_end,
                                        intercept_checks_fct *recurse)
{
  Side const side_in_check = trait[nbply-1];
  Side const side_checking = advers(side_in_check);
  square const king_pos = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceSquare(s);
  TraceWalk(walk_at_end);
  TraceFunctionParamListEnd();

  assert(!is_rider_check_uninterceptable_on_vector(side_checking,king_pos,kcurr,walk_at_end));
  TraceSquare(s);TraceEnumerator(Side,side_in_check);TraceEOL();

  assert(nr_total_invisibles_left>0);

  ++taboo[s];

  --nr_total_invisibles_left;
  TraceValue("%u",nr_total_invisibles_left);TraceEOL();

  if (play_phase==play_validating_mate)
  {
    Flags spec = BIT(White)|BIT(Black)|BIT(Chameleon);
    SetPieceId(spec,++next_invisible_piece_id);
    occupy_square(s,Dummy,spec);
    (*recurse)(kcurr+1);
    empty_square(s);
    --next_invisible_piece_id;
  }
  else
    colour_interceptor(side_in_check,s);

  ++nr_total_invisibles_left;
  TraceValue("%u",nr_total_invisibles_left);TraceEOL();

  --taboo[s];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_line(vec_index_type kcurr,
                                      piece_walk_type const walk_at_end,
                                      intercept_checks_fct *recurse)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceWalk(walk_at_end);
  TraceFunctionParamListEnd();

  {
    square s;
    for (s = king_pos+vec[kcurr];
         is_square_empty(s) && !end_of_iteration;
         s += vec[kcurr])
    {
      TraceSquare(s);
      TraceValue("%u",taboo[s]);
      TraceValue("%u",taboo[s]);
      TraceEOL();
      if (taboo[s]==0)
        place_interceptor_on_square(kcurr,s,walk_at_end,recurse);
    }
    TraceSquare(s);
    TraceValue("%u",end_of_iteration);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_line_if_check(vec_index_type kcurr,
                                    piece_walk_type walk_rider,
                                    intercept_checks_fct *recurse)
{
  Side const side_in_check = trait[nbply-1];
  Side const side_checking = advers(side_in_check);
  square const king_pos = being_solved.king_square[side_in_check];
  int const dir = vec[kcurr];
  square const sq_end = find_end_of_line(king_pos,dir);
  piece_walk_type const walk_at_end = get_walk_of_piece_on_square(sq_end);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceWalk(walk_rider);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEnumerator(Side,side_in_check);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceSquare(sq_end);
  TraceSquare(king_pos+dir);
  TraceWalk(walk_at_end);
  TraceEOL();

  if ((walk_at_end==walk_rider || walk_at_end==Queen)
      && TSTFLAG(being_solved.spec[sq_end],side_checking))
  {
    TraceValue("%u",nr_total_invisibles_left);TraceEOL();
    if (sq_end!=king_pos+dir
        && nr_total_invisibles_left>0)
      place_interceptor_on_line(kcurr,walk_at_end,recurse);
  }
  else
    (*recurse)(kcurr+1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_illegal_checks_diagonal(vec_index_type kcurr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParamListEnd();

  if (kcurr>vec_bishop_end)
    done_intercepting_illegal_checks();
  else
    intercept_line_if_check(kcurr,Bishop,&intercept_illegal_checks_diagonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_illegal_checks_orthogonal(vec_index_type kcurr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParamListEnd();

  if (kcurr>vec_rook_end)
    intercept_illegal_checks_diagonal(vec_bishop_start);
  else
    intercept_line_if_check(kcurr,Rook,&intercept_illegal_checks_orthogonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_illegal_checks(void)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (king_pos==initsquare)
    done_intercepting_illegal_checks();
  else if (!is_square_attacked_by_uninterceptable(side_in_check,king_pos))
    intercept_illegal_checks_orthogonal(vec_rook_start);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void start_iteration(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  intercept_illegal_checks();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_taboo(int delta)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;

  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  piece_walk_type const walk = move_effect_journal[movement].u.piece_movement.moving;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceWalk(walk);
  TraceEOL();

  taboo[sq_departure] += delta;

  switch (sq_capture)
  {
    case kingside_castling :
    {
      square s;
      for (s = sq_departure+dir_right; is_on_board(s); s += dir_right)
        taboo[s] += delta;
      break;
    }

    case queenside_castling:
    {
      square s;
      for (s = sq_departure+dir_left; is_on_board(s); s += dir_left)
        taboo[s] += delta;
      break;
    }

    case pawn_multistep:
    {
      square const sq_intermediate = (sq_departure+sq_arrival)/2;
      taboo[sq_intermediate] += delta;
      break;
    }

    case messigny_exchange:
    case retro_capture_departure:
    case no_capture:
      break;

    default:
      taboo[sq_capture] += delta;
      break;
  }

  if (is_rider(walk))
  {
    int const diff_move = sq_arrival-sq_departure;
    int const dir_move = CheckDir[walk][diff_move];

    square s;
    assert(dir_move!=0);
    for (s = sq_departure+dir_move; s!=sq_arrival; s += dir_move)
      taboo[s] += delta;
  }
  else if (is_pawn(walk))
    /* arrival square must not be blocked */
    taboo[sq_arrival] += delta;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacker(Side side_attacking,
                                        square s,
                                        piece_walk_type walk)
{
  Flags spec = BIT(side_attacking)|BIT(Chameleon);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(s);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  assert(nr_total_invisibles_left>0);

  ++taboo[s];

  --nr_total_invisibles_left;
  TraceValue("%u",nr_total_invisibles_left);TraceEOL();
  ++being_solved.number_of_pieces[side_attacking][walk];
  SetPieceId(spec,++next_invisible_piece_id);
  occupy_square(s,walk,spec);
  start_iteration();
  empty_square(s);
  --next_invisible_piece_id;
  --being_solved.number_of_pieces[side_attacking][walk];
  ++nr_total_invisibles_left;
  TraceValue("%u",nr_total_invisibles_left);TraceEOL();

  --taboo[s];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_rider(Side side_attacking,
                                               square sq_mating_piece,
                                               piece_walk_type walk_rider,
                                               vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square s;
    for (s = sq_mating_piece+vec[kcurr];
         !is_square_blocked(s) && !end_of_iteration;
         s += vec[kcurr])
      if (is_square_empty(s))
      {
        TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
        if (taboo[s]==0)
          place_mating_piece_attacker(side_attacking,s,walk_rider);
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_leaper(Side side_attacking,
                                                square sq_mating_piece,
                                                piece_walk_type walk_leaper,
                                                vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square const s = sq_mating_piece+vec[kcurr];
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[s]==0)
      place_mating_piece_attacker(side_attacking,s,walk_leaper);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_pawn(Side side_attacking,
                                              square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceFunctionParamListEnd();

  if (!end_of_iteration)
  {
    square s = sq_mating_piece+dir_up+dir_left;
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[s]==0)
      place_mating_piece_attacker(side_attacking,s,Pawn);
  }

  if (!end_of_iteration)
  {
    square s = sq_mating_piece+dir_up+dir_right;
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[s]==0)
      place_mating_piece_attacker(side_attacking,s,Pawn);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void attack_mating_piece(Side side_attacking,
                                square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(sq_mating_piece);
  TraceFunctionParamListEnd();

  /* We are defending against a verified mate. If we can't attack the mate,
   * we are dealing with a solution. */
  combined_result = previous_move_has_solved;

  place_mating_piece_attacking_rider(side_attacking,
                                     sq_mating_piece,
                                     Bishop,
                                     vec_bishop_start,vec_bishop_end);

  place_mating_piece_attacking_rider(side_attacking,
                                     sq_mating_piece,
                                     Rook,
                                     vec_rook_start,vec_rook_end);

  place_mating_piece_attacking_leaper(side_attacking,
                                      sq_mating_piece,
                                      Knight,
                                      vec_knight_start,vec_knight_end);

  place_mating_piece_attacking_pawn(side_attacking,sq_mating_piece);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_mate(void)
{
  numecoup const curr = CURRMOVE_OF_PLY(top_ply_of_regular_play);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",top_ply_of_regular_play);
  TraceSquare(move_gen_top->departure);
  TraceValue("%u",move_gen_top->departure);
  TraceSquare(move_gen_top->arrival);
  TraceValue("%u",move_gen_top->arrival);
  TraceValue("%u",move_by_invisible);
  TraceEOL();

  if (move_gen_top->departure==move_by_invisible
      && move_gen_top->arrival==move_by_invisible)
    combined_validation_result = mate_defendable_by_interceptors;
  else
  {
    combined_validation_result = mate_unvalidated;
    combined_result = previous_move_is_illegal;
    end_of_iteration = false;
    start_iteration();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void test_mate(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",combined_result);
  TraceValue("%u",combined_validation_result);
  TraceEOL();

  switch (combined_validation_result)
  {
    case mate_unvalidated:
      assert(combined_result==previous_move_is_illegal);
      break;

    case no_mate:
      assert(combined_result==previous_move_has_not_solved);
      break;

    case mate_attackable:
      end_of_iteration = false;
      combined_result = previous_move_is_illegal;
      attack_mating_piece(advers(trait[top_ply_of_regular_play]),sq_mating_piece_to_be_attacked);
      break;

    case mate_defendable_by_interceptors:
      end_of_iteration = false;
      combined_result = previous_move_is_illegal;
      start_iteration();
      break;

    case mate_with_2_uninterceptable_doublechecks:
      /* we only replay moves for TI revelation */
      end_of_iteration = false;
      combined_result = previous_move_is_illegal;
      start_iteration();
      assert(combined_result==previous_move_has_solved);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void rewind_effects(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (nbply!=ply_retro_move)
  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
    square const sq_departure = move_gen_top->departure;

    --taboo[sq_departure];

    undo_move_effects();
    --nbply;
  }

  ++nbply;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unrewind_effects(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  --nbply;

  while (nbply!=top_ply_of_regular_play)
  {
    ++nbply;
    redo_move_effects();

    {
      numecoup const curr = CURRMOVE_OF_PLY(nbply);
      move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
      square const sq_departure = move_gen_top->departure;

      ++taboo[sq_departure];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void make_revelations(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  top_ply_of_regular_play = nbply;
  setup_revelations();
  play_phase = play_rewinding;
  rewind_effects();
  play_phase = play_detecting_revelations;
  end_of_iteration = false;
  start_iteration();
  play_phase = play_unwinding;
  unrewind_effects();
  play_phase = play_regular;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_move_sequence_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply-ply_retro_move);TraceEOL();

  update_taboo(+1);

  /* necessary for detecting checks by pawns and leapers */
  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else
  {
    /* make sure that our length corresponds to the length of the tested move sequence
     * (which may vary if STFindShortest is used)
     */
    assert(slices[SLICE_NEXT2(si)].type==STHelpAdapter);
    slices[SLICE_NEXT2(si)].u.branch.length = slack_length+(nbply-ply_retro_move);

    TraceValue("%u",SLICE_NEXT2(si));
    TraceValue("%u",slices[SLICE_NEXT2(si)].u.branch.length);
    TraceEOL();

    top_ply_of_regular_play = nbply;
    tester_slice = si;

    play_phase = play_rewinding;
    rewind_effects();
    play_phase = play_validating_mate;
    validate_mate();
    play_phase = play_testing_mate;
    test_mate();
    play_phase = play_unwinding;
    unrewind_effects();
    play_phase = play_regular;

    solve_result = combined_result==immobility_on_next_move ? previous_move_has_not_solved : combined_result;
  }

  update_taboo(-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_reveal_after_mating_move(slice_index si)
{
  unsigned int const save_nr_total_invisibles_left = nr_total_invisibles_left;
  PieceIdType const save_next_invisible_piece_id = next_invisible_piece_id;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  make_revelations();

  if (!revelation_status_is_uninitialised)
    evaluate_revelations();

  pipe_solve_delegate(si);

  nr_total_invisibles_left = save_nr_total_invisibles_left;
  next_invisible_piece_id = save_next_invisible_piece_id;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_move_still_playable(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side = SLICE_STARTER(si);
    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].departure;
    square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].arrival;
    square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture;

    TraceValue("%u",ply_replayed);
    TraceSquare(sq_departure);
    TraceSquare(sq_arrival);
    TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture);
    TraceEOL();

    // TODO redo (and undo) the pre-capture effect?

    if (sq_departure==move_by_invisible
        && sq_arrival==move_by_invisible)
    {
      TraceText("we assume that un-fleshed-out random moves by total invisibles are always playable");
      result = true;
    }
    else
    {
      assert(TSTFLAG(being_solved.spec[sq_departure],side));

      if (sq_capture==queenside_castling)
      {
        SETCASTLINGFLAGMASK(side,ra_cancastle);
        generate_moves_for_piece(sq_departure);
        CLRCASTLINGFLAGMASK(side,ra_cancastle);
      }
      else if (sq_capture==kingside_castling)
      {
        SETCASTLINGFLAGMASK(side,rh_cancastle);
        generate_moves_for_piece(sq_departure);
        CLRCASTLINGFLAGMASK(side,rh_cancastle);
      }
      else if (!is_no_capture(sq_capture) && is_square_empty(sq_capture))
      {
        occupy_square(sq_capture,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
        generate_moves_for_piece(sq_departure);
        empty_square(sq_capture);
      }
      else
        generate_moves_for_piece(sq_departure);

      {
        numecoup start = MOVEBASE_OF_PLY(nbply);
        numecoup i;
        numecoup new_top = start;
        for (i = start+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
        {
          assert(move_generation_stack[i].departure==sq_departure);
          if (move_generation_stack[i].arrival==sq_arrival)
          {
            ++new_top;
            move_generation_stack[new_top] = move_generation_stack[i];
            break;
          }
        }

        SET_CURRMOVE(nbply,new_top);
      }

      result = CURRMOVE_OF_PLY(nbply)>MOVEBASE_OF_PLY(nbply);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void copy_move_effects(void)
{
  move_effect_journal_index_type const replayed_base = move_effect_journal_base[ply_replayed];
  move_effect_journal_index_type const replayed_top = move_effect_journal_base[ply_replayed+1];
  move_effect_journal_index_type replayed_curr;
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",ply_replayed);
  TraceValue("%u",move_effect_journal_base[ply_replayed]);
  TraceValue("%u",move_effect_journal_base[ply_replayed+1]);
  TraceValue("%u",nbply);
  TraceEOL();

  move_effect_journal_base[nbply+1] += (replayed_top-replayed_base);
  curr = move_effect_journal_base[nbply+1];
  replayed_curr = replayed_top;

  while (replayed_curr>replayed_base)
    move_effect_journal[--curr] = move_effect_journal[--replayed_curr];

  assert(curr==move_effect_journal_base[nbply]);

  TraceValue("%u",move_effect_journal_base[nbply]);
  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_move_repeater_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));

  if (is_move_still_playable(si))
  {
    copy_move_effects();
    redo_move_effects();
    ++ply_replayed;
    pipe_solve_delegate(si);
    --ply_replayed;
    undo_move_effects();
  }
  else
    solve_result = previous_move_is_illegal;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_uninterceptable_selfcheck_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO separate slice type for update taboo?
  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else if (nbply>ply_retro_move)
  {
    update_taboo(+1);

    make_revelations();

    if (revelation_status_is_uninitialised)
      pipe_solve_delegate(si);
    else
    {
      unsigned int nr_revealed_unplaced_invisibles = 0;
      {
        unsigned int i;
        for (i = 0; i!=nr_potential_revelations; ++i)
        {
          square const s = revelation_status[i].pos;
          TraceValue("%u",i);
          TraceSquare(s);
          TraceWalk(revelation_status[i].walk);
          TraceValue("%x",revelation_status[i].spec);
          TraceEOL();
          if (revelation_status[i].walk!=Empty && is_square_empty(s))
            ++nr_revealed_unplaced_invisibles;
        }
      }
      if (nr_revealed_unplaced_invisibles<=nr_total_invisibles_left)
      {
        PieceIdType const save_next_invisible_piece_id = next_invisible_piece_id;
        evaluate_revelations();
        pipe_solve_delegate(si);
        TraceValue("%u",nr_total_invisibles_left);TraceEOL();
        next_invisible_piece_id = save_next_invisible_piece_id;
      }
    }

    update_taboo(-1);
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static unsigned int find_nr_interceptors_needed(Side side_checking,
                                                square potential_flight,
                                                unsigned int nr_interceptors_available,
                                                vec_index_type start, vec_index_type end,
                                                piece_walk_type walk_rider)
{
  unsigned int result = 0;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(potential_flight);
  TraceValue("%u",nr_interceptors_available);
  TraceValue("%u",start);
  TraceValue("%u",end);
  TraceWalk(walk_rider);
  TraceFunctionParamListEnd();

  for (k = start; k<=end && result<=nr_interceptors_available; ++k)
  {
    square const end = find_end_of_line(potential_flight,vec[k]);
    piece_walk_type const walk = get_walk_of_piece_on_square(end);
    Flags const flags = being_solved.spec[end];
    if ((walk==Queen || walk==walk_rider) && TSTFLAG(flags,side_checking))
    {
      square s;
      for (s = potential_flight+vec[k]; s!=end; s += vec[k])
        if (taboo[s]==0)
        {
          ++result;
          break;
        }

      if (s==end)
        result = nr_interceptors_available+1;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean are_all_guards_interceptable(Side side_in_check, square potential_flight)
{
  Side const side_checking = advers(side_in_check);
  unsigned int nr_interceptors_needed;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(potential_flight);
  TraceFunctionParamListEnd();

  nr_interceptors_needed = find_nr_interceptors_needed(side_checking,
                                                       potential_flight,
                                                       nr_total_invisibles_left,
                                                       vec_rook_start,vec_rook_end,
                                                       Rook);

  if (nr_interceptors_needed<=nr_total_invisibles_left)
    nr_interceptors_needed += find_nr_interceptors_needed(side_checking,
                                                          potential_flight,
                                                          nr_total_invisibles_left-nr_interceptors_needed,
                                                          vec_bishop_start,vec_bishop_end,
                                                          Bishop);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",nr_interceptors_needed);
  TraceFunctionResultEnd();
  return nr_interceptors_needed<=nr_total_invisibles_left;
}

static boolean make_flight(Side side_in_check, square s)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  if ((is_square_empty(s)
      || TSTFLAG(being_solved.spec[s],advers(side_in_check)))
      && !is_square_uninterceptably_attacked(side_in_check,s)
      && are_all_guards_interceptable(side_in_check,s))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean make_a_flight(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_in_check = advers(trait[nbply]);
    square const king_pos = being_solved.king_square[side_in_check];
    int dir_vert;
    int dir_horiz;

    piece_walk_type const walk = get_walk_of_piece_on_square(king_pos);
    Flags const flags = being_solved.spec[king_pos];

    TraceWalk(walk);
    TraceValue("%x",flags);
    TraceEOL();

    empty_square(king_pos);
    ++taboo[king_pos];

    for (dir_vert = dir_down; dir_vert<=dir_up && !result; dir_vert += dir_up)
      for (dir_horiz = dir_left; dir_horiz<=dir_right; dir_horiz += dir_right)
      {
        square const flight = king_pos+dir_vert+dir_horiz;
        if (flight!=king_pos && make_flight(side_in_check,flight))
        {
          result = true;
          break;
        }
      }

    --taboo[king_pos];
    occupy_square(king_pos,walk,flags);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void attack_checks(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_delivering_check = trait[nbply];
    Side const side_in_check = advers(side_delivering_check);
    square const king_pos = being_solved.king_square[side_in_check];
    vec_index_type const k = is_square_uninterceptably_attacked(side_in_check,king_pos);
    if (k==0)
    {
      if (are_all_guards_interceptable(side_in_check,king_pos))
      {
        TraceText("interceptable check\n");
        /* the king square can be made a "flight" */
        mate_validation_result = no_mate;
        solve_result = previous_move_has_not_solved;
      }
      else
      {
        TraceText("mate can be refuted by interceptors or free TIs (if any)\n");
        mate_validation_result = mate_defendable_by_interceptors;
      }
    }
    else if (nr_total_invisibles_left>0)
    {
      square const sq_attacker = find_end_of_line(king_pos,vec[k]);
      TraceSquare(king_pos);TraceValue("%u",k);TraceValue("%d",vec[k]);TraceSquare(sq_attacker);TraceEOL();
      assert(TSTFLAG(being_solved.spec[sq_attacker],side_delivering_check));
      CLRFLAG(being_solved.spec[sq_attacker],side_delivering_check);
      if (is_square_uninterceptably_attacked(side_in_check,king_pos))
      {
        TraceText("mate can not be defended\n");
        mate_validation_result = mate_with_2_uninterceptable_doublechecks;
      }
      else
      {
        TraceText("mate can be refuted by free TIs\n");
        mate_validation_result = mate_attackable;
        sq_mating_piece_to_be_attacked = sq_attacker;
      }
      SETFLAG(being_solved.spec[sq_attacker],side_delivering_check);
    }
    else
    {
      TraceText("mate can be refuted by interceptors - no free TI avaliable\n");
      mate_validation_result = mate_defendable_by_interceptors;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_goal_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* make sure that we don't generate pawn captures total invisible */
  assert(play_phase!=play_regular);

  pipe_solve_delegate(si);

  if (play_phase==play_validating_mate)
  {
    if (solve_result==previous_move_has_not_solved)
      mate_validation_result = no_mate;
    else if (make_a_flight())
    {
      solve_result = previous_move_has_not_solved;
      mate_validation_result = no_mate;
    }
    else
      attack_checks();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_generate_moves_by_invisible(slice_index si)
{
  Side const side_moving = trait[nbply];
  Side const side_capturee = advers(side_moving);
  square const *s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  curr_generation->departure = capture_by_invisible;

  for (s = boardnum; *s; ++s)
  {
    if (!is_square_empty(*s))
    {
      TraceWalk(get_walk_of_piece_on_square(*s));
      TraceValue("%x",being_solved.spec[*s]);
      TraceEOL();
    }
    if (TSTFLAG(being_solved.spec[*s],side_capturee)
        && get_walk_of_piece_on_square(*s)!=Dummy
        && !TSTFLAG(being_solved.spec[*s],Chameleon)
        && !TSTFLAG(being_solved.spec[*s],Royal))
    {
      curr_generation->arrival = *s;
      push_move_regular_capture();
    }
  }

  curr_generation->departure = move_by_invisible;
  curr_generation->arrival = move_by_invisible;
  push_move_capture_extra(move_by_invisible);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_pawn_capture_right(slice_index si, int dir_vertical)
{
  square const s = curr_generation->departure+dir_vertical+dir_right;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_empty(s) && taboo[s]==0)
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
    pipe_move_generation_delegate(si);
    empty_square(s);
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_pawn_capture_left(slice_index si, int dir_vertical)
{
  square const s = curr_generation->departure+dir_vertical+dir_left;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_empty(s) && taboo[s]==0)
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
    generate_pawn_capture_right(si,dir_vertical);
    empty_square(s);
  }
  else
    generate_pawn_capture_right(si,dir_vertical);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepare_king_side_castling_generation(slice_index si)
{
  Side const side = trait[nbply];
  square const square_a = side==White ? square_a1 : square_a8;
  square const square_h = square_a+file_h;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (nr_total_invisibles_left>0
      && is_square_empty(square_h)
      && taboo[square_h]==0)
  {
    ++being_solved.number_of_pieces[side][Rook];
    occupy_square(square_h,Rook,BIT(side)|BIT(Chameleon));
    SETCASTLINGFLAGMASK(side,rh_cancastle);
    pipe_move_generation_delegate(si);
    CLRCASTLINGFLAGMASK(side,rh_cancastle);
    empty_square(square_h);
    --being_solved.number_of_pieces[side][Rook];
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepare_queen_side_castling_generation(slice_index si)
{
  Side const side = trait[nbply];
  square const square_a = side==White ? square_a1 : square_a8;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (nr_total_invisibles_left>0
      && is_square_empty(square_a)
      && taboo[square_a]==0)
  {
    ++being_solved.number_of_pieces[side][Rook];
    occupy_square(square_a,Rook,BIT(side)|BIT(Chameleon));
    SETCASTLINGFLAGMASK(side,ra_cancastle);
    prepare_king_side_castling_generation(si);
    CLRCASTLINGFLAGMASK(side,ra_cancastle);
    empty_square(square_a);
    --being_solved.number_of_pieces[side][Rook];
  }
  else
    prepare_king_side_castling_generation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void total_invisible_generate_special_moves(slice_index si)
{
  square const sq_departure = curr_generation->departure ;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (play_phase==play_regular)
  {
    switch (being_solved.board[sq_departure])
    {
      case Pawn:
        if (nr_total_invisibles_left>0)
          generate_pawn_capture_left(si,trait[nbply]==White ? dir_up : dir_down);
        break;

      case King:
        if (TSTCASTLINGFLAGMASK(trait[nbply],castlings)>=k_cancastle)
          prepare_queen_side_castling_generation(si);
        else
          pipe_move_generation_delegate(si);
        break;

      default:
        pipe_move_generation_delegate(si);
        break;
    }
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_special_moves_player_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt * const move_gen_top = move_generation_stack+curr;
    square const sq_departure = move_gen_top->departure;
    square const sq_capture = move_gen_top->capture;

    TraceSquare(sq_departure);
    TraceSquare(move_gen_top->arrival);
    TraceSquare(sq_capture);
    TraceValue("%u",nr_total_invisibles_left);
    TraceEOL();

    if (sq_departure==move_by_invisible)
      pipe_solve_delegate(si);
    else if (sq_departure>=capture_by_invisible)
    {
      // TODO:
      /* a) unplaced invisible
       * b) placed invisible that can reach sq_capture (with added knowledge)
       * the following test only applies to case a)
      if (nr_total_invisibles_left>0)*/
      {
        Side const side = trait[nbply];
        Flags spec = BIT(side)|BIT(Chameleon);
        SetPieceId(spec,++next_invisible_piece_id);
        move_effect_journal_do_piece_readdition(move_effect_reason_removal_of_invisible,
                                                sq_departure,Dummy,spec,side);

        /* No adjustment of nr_total_invisibles_left here!
         * The capture may be done by an existing invisible. We can only do the
         * adjustment when we flesh out this capture by inserting a new invisible.
         */
        pipe_solve_delegate(si);
        --next_invisible_piece_id;
      }
//      else
//      {
//        pop_move();
//        solve_result = previous_move_is_illegal;
//      }
    }
    else
      switch (sq_capture)
      {
        case pawn_multistep:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        case messigny_exchange:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        case kingside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;
          square const square_h = square_a+file_h;

          TraceText("kingside_castling\n");

          if (is_square_empty(square_h))
          {
            Flags spec = BIT(side)|BIT(Chameleon);
            assert(nr_total_invisibles_left>0);
            SetPieceId(spec,++next_invisible_piece_id);
            move_effect_journal_do_piece_readdition(move_effect_reason_castling_partner,
                                                    square_h,Rook,spec,side);
            --nr_total_invisibles_left;
            TraceValue("%u",nr_total_invisibles_left);TraceEOL();
            pipe_solve_delegate(si);
            ++nr_total_invisibles_left;
            TraceValue("%u",nr_total_invisibles_left);TraceEOL();
            --next_invisible_piece_id;
          }
          else
          {
            move_effect_journal_do_null_effect();
            pipe_solve_delegate(si);
          }
          break;
        }

        case queenside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;

          TraceText("queenside_castling\n");

          if (is_square_empty(square_a))
          {
            Flags spec = BIT(side)|BIT(Chameleon);
            assert(nr_total_invisibles_left>0);
            SetPieceId(spec,++next_invisible_piece_id);
            move_effect_journal_do_piece_readdition(move_effect_reason_castling_partner,
                                                    square_a,Rook,spec,side);
            --nr_total_invisibles_left;
            TraceValue("%u",nr_total_invisibles_left);TraceEOL();
            pipe_solve_delegate(si);
            ++nr_total_invisibles_left;
            TraceValue("%u",nr_total_invisibles_left);TraceEOL();
            --next_invisible_piece_id;
          }
          else
          {
            move_effect_journal_do_null_effect();
            pipe_solve_delegate(si);
          }
          break;
        }

        case no_capture:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        default:
          /* pawn captures total invisible? */
          if (is_square_empty(sq_capture))
          {
            Side const side_victim = advers(SLICE_STARTER(si));
            Flags spec = BIT(side_victim)|BIT(Chameleon);

            assert(nr_total_invisibles_left>0);
            SetPieceId(spec,++next_invisible_piece_id);
            move_effect_journal_do_piece_readdition(move_effect_reason_removal_of_invisible,
                                                    sq_capture,Dummy,spec,side_victim);

            /* No adjustment of nr_total_invisibles_left here! Another invisible may
             * have moved to sq_capture and serve as a victim.
             */
            pipe_solve_delegate(si);

            --next_invisible_piece_id;
          }
          else
          {
            move_effect_journal_do_null_effect();
            pipe_solve_delegate(si);
          }
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_reserve_king_movement(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* reserve a spot in the move effect journal for the case that a move by an invisible
   * turns out to move a side's king square
   */
  move_effect_journal_do_null_effect();
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_generator(slice_index si,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleMoveSequenceMoveRepeater);
    SLICE_STARTER(prototype) = SLICE_STARTER(si);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_the_pipe(slice_index si,
                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_castling_player(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  dealloc_slice(SLICE_NEXT2(si));
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_self_check_guard(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_index * const remembered = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  *remembered = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_goal_guard(slice_index si,
                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index remembered = no_slice;

    stip_structure_traversal st_nested;
    stip_structure_traversal_init_nested(&st_nested,st,&remembered);
    stip_structure_traversal_override_single(&st_nested,
                                             STSelfCheckGuard,
                                             &remember_self_check_guard);
    stip_traverse_structure(SLICE_NEXT2(si),&st_nested);

    if (remembered!=no_slice)
    {
      slice_index prototype = alloc_pipe(STTotalInvisibleGoalGuard);
      SLICE_STARTER(prototype) = SLICE_STARTER(remembered);
      goal_branch_insert_slices(SLICE_NEXT2(si),&prototype,1);
      pipe_remove(remembered);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_replay_branch(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;

    stip_structure_traversal_init_nested(&st_nested,st,0);
    // TODO prevent instrumentation in the first place?
    stip_structure_traversal_override_single(&st_nested,
                                             STFindShortest,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STFindAttack,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveEffectJournalUndoer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPostMoveIterationInitialiser,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STCastlingPlayer,
                                             &remove_castling_player);
    stip_structure_traversal_override_single(&st_nested,
                                             STMovePlayer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPawnPromoter,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveGenerator,
                                             &subsitute_generator);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachedTester,
                                             &subsitute_goal_guard);
    /* self check is impossible with the current optimisations for orthodox pieces ...
     */
//    stip_structure_traversal_override_single(&st_nested,
//                                             STSelfCheckGuard,
//                                             &remove_the_pipe);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void replace_self_check_guard(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* This iteration ends at STTotalInvisibleMoveSequenceTester. We can therefore
   * blindly tamper with all STSelfCheckGuard slices that we meet.
   */
  stip_traverse_structure_children_pipe(si,st);

  SLICE_TYPE(si) = STTotalInvisibleUninterceptableSelfCheckGuard;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STTotalInvisibleMoveSequenceTester,
                                             &instrument_replay_branch);
    stip_structure_traversal_override_single(&st,
                                             STSelfCheckGuard,
                                             &replace_self_check_guard);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  output_plaintext_check_indication_disabled = true;

  solving_instrument_move_generation(si,nr_sides,STTotalInvisibleSpecialMoveGenerator);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInvisiblesAllocator);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    square const *s;
    for (s = boardnum; *s; ++s)
      if (!is_square_empty(*s))
        ++taboo[*s];
  }

  pipe_solve_delegate(si);

  {
    square const *s;
    for (s = boardnum; *s; ++s)
      if (!is_square_empty(*s))
        --taboo[*s];
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_invisibles_allocator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  next_invisible_piece_id = being_solved.currPieceId;

  being_solved.currPieceId += total_invisible_number;

  pipe_solve_delegate(si);

  being_solved.currPieceId -= total_invisible_number;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean instrumenting;
    slice_index the_copy;
    stip_length_type length;
} insertion_state_type;

static void insert_copy(slice_index si,
                        stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->the_copy==no_slice)
    stip_traverse_structure_children(si,st);
  else
  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const substitute = alloc_pipe(STTotalInvisibleMoveSequenceTester);
    pipe_link(proxy,substitute);
    link_to_branch(substitute,state->the_copy);
    SLICE_NEXT2(substitute) = state->the_copy;
    state->the_copy = no_slice;
    dealloc_slices(SLICE_NEXT2(si));
    SLICE_NEXT2(si) = proxy;

    assert(state->length!=no_stip_length);
    if (state->length%2!=0)
      pipe_append(proxy,alloc_pipe(STMoveInverter));

    {
      slice_index const prototypes[] = {
          alloc_pipe(STTotalInvisibleRevealAfterFinalMove)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      help_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void copy_help_branch(slice_index si,
                             stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",state->instrumenting);
  TraceEOL();

  state->length = slices[si].u.branch.length;

  if (state->instrumenting)
    stip_traverse_structure_children(si,st);
  else
  {
    state->instrumenting = true;
    state->the_copy = stip_deep_copy(si);
    stip_traverse_structure_children(si,st);
    assert(state->the_copy==no_slice);

    {
      slice_index const prototypes[] = {
          alloc_pipe(STTotalInvisibleMovesByInvisibleGenerator),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleReserveKingMovement),
          alloc_pipe(STTotalInvisibleMovesByInvisibleGenerator),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleReserveKingMovement)
      };
      enum { nr_protypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert(si,prototypes,nr_protypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with support for Total Invisible pieces
 * @param si identifies the root slice of the stipulation
 */
void solving_instrument_total_invisible(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  // later:
  // - in original
  //   - insert revelation logic
  // - in copy
  //   - logic for iteration over all possibilities of invisibles
  //     - special case of invisible king
  //     - special case: position has no legal placement of all invisibles may have to be dealt with:
  //       - not enough empty squares :-)
  //   - substitute for STFindShortest

  // bail out at STAttackAdapter

  // input for nr_total_invisibles_left, initialize to 0

  // what about:
  // - structured stipulations?
  // - goals that don't involve immobility
  // ?

  // we shouldn't need to set the starter of
  // - STTotalInvisibleMoveSequenceMoveRepeater
  // - STTotalInvisibleGoalGuard

  // check indication should also be deactivated in tree output

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInstrumenter);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    stip_structure_traversal st;
    insertion_state_type state = { false, no_slice, no_stip_length };

    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override_single(&st,
                                             STHelpAdapter,
                                             &copy_help_branch);
    stip_structure_traversal_override_single(&st,
                                             STGoalReachedTester,
                                             &insert_copy);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  nr_total_invisibles_left = total_invisible_number;
  TraceValue("%u",nr_total_invisibles_left);TraceEOL();

  move_effect_journal_register_pre_capture_effect();

  move_effect_journal_set_effect_doers(move_effect_revelation_of_new_invisible,
                                       &undo_revelation_of_new_invisible,
                                       &redo_revelation_of_new_invisible);

  move_effect_journal_set_effect_doers(move_effect_revelation_of_castling_partner,
                                       &undo_revelation_of_castling_partner,
                                       &redo_revelation_of_castling_partner);

  move_effect_journal_set_effect_doers(move_effect_revelation_of_placed_invisible,
                                       &undo_revelation_of_placed_invisible,
                                       &redo_revelation_of_placed_invisible);

  TraceFunctionResultEnd();
  TraceFunctionExit(__func__);
}
