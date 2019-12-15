#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/random_move_by_invisible.h"
#include "pieces/attributes/total_invisible/capture_by_invisible.h"
#include "pieces/walks/pawns/en_passant.h"
#include "position/effects/piece_removal.h"
#include "position/effects/piece_movement.h"
#include "position/effects/walk_change.h"
#include "position/effects/king_square.h"
#include "solving/pipe.h"
#include "solving/castling.h"
#include "solving/has_solution_type.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

#include <string.h>

boolean revelation_status_is_uninitialised;
unsigned int nr_potential_revelations;
revelation_status_type revelation_status[nr_squares_on_board];
move_effect_journal_index_type top_before_revelations[maxply+1];
motivation_type motivation[MaxPieceId+1];

motivation_type const motivation_null = {
    { purpose_none },
    { purpose_none }
};

PieceIdType top_visible_piece_id;
PieceIdType top_invisible_piece_id;

void initialise_invisible_piece_ids(PieceIdType last_visible_piece_id)
{
  top_visible_piece_id = last_visible_piece_id;
  top_invisible_piece_id = top_visible_piece_id;
}

PieceIdType get_top_visible_piece_id(void)
{
  return top_visible_piece_id;
}

PieceIdType get_top_invisible_piece_id(void)
{
  return top_invisible_piece_id;
}

PieceIdType initialise_motivation(purpose_type purpose_first, square sq_first,
                                  purpose_type purpose_last, square sq_last)
{
  PieceIdType const result = ++top_invisible_piece_id;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",purpose_first);
  TraceSquare(sq_first);
  TraceFunctionParam("%u",purpose_last);
  TraceSquare(sq_last);
  TraceFunctionParamListEnd();

  assert(motivation[result].last.purpose==purpose_none);
  motivation[result].first.purpose = purpose_first;
  motivation[result].first.acts_when = nbply;
  motivation[result].first.on = sq_first;
  motivation[result].last.purpose = purpose_last;
  motivation[result].last.acts_when = nbply;
  motivation[result].last.on = sq_last;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

PieceIdType initialise_motivation_from_revelation(revelation_status_type const *revelation)
{
  PieceIdType const result = ++top_invisible_piece_id;

  TraceFunctionEntry(__func__);
  TraceAction(&revelation->first);
  TraceAction(&revelation->last);
  TraceFunctionParamListEnd();

  assert(motivation[result].last.purpose==purpose_none);
  motivation[result].first = revelation->first;
  motivation[result].last = revelation->last;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void initialise_motivation_of_inserted_capturer(PieceIdType id)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",id);
  TraceFunctionParamListEnd();

  decision_levels[id].side = curr_decision_level;
  decision_levels[id].walk = curr_decision_level;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void uninitialise_motivation(PieceIdType id_uninitialised)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",id_uninitialised);
  TraceFunctionParamListEnd();

  assert(top_invisible_piece_id==id_uninitialised);
  assert(top_invisible_piece_id>top_visible_piece_id);
  motivation[top_invisible_piece_id] = motivation_null;
  --top_invisible_piece_id;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

// TODO what is a good size for this?
knowledge_type knowledge[MaxPieceId];
knowledge_index_type size_knowledge;

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

void reveal_new(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;
  Side const side_revealed = TSTFLAG(spec,White) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",GetPieceId(entry->u.piece_addition.added.flags));
  TraceValue("%u",GetPieceId(being_solved.spec[on]));

  if (TSTFLAG(spec,White))
    ++being_solved.number_of_pieces[White][walk];
  if (TSTFLAG(spec,Black))
    ++being_solved.number_of_pieces[Black][walk];

  replace_walk(on,walk);

  ((move_effect_journal_entry_type *)entry)->u.piece_addition.added.flags = being_solved.spec[on];
  being_solved.spec[on] = spec;

  if (TSTFLAG(spec,Royal) && walk==King)
  {
    TraceSquare(being_solved.king_square[side_revealed]);
    being_solved.king_square[side_revealed] = on;
  }

  TraceValue("%x",being_solved.spec[on]);TraceEOL();
  assert(!TSTFLAG(being_solved.spec[on],Chameleon));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void unreveal_new(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;
  Side const side_revealed = TSTFLAG(spec,White) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(play_phase==play_validating_mate);
  assert(get_walk_of_piece_on_square(on)==walk);

  TraceText("substituting dummy for revealed piece\n");
  if (TSTFLAG(being_solved.spec[on],Royal) && walk==King)
    being_solved.king_square[side_revealed] = initsquare;

  ((move_effect_journal_entry_type *)entry)->u.piece_addition.added.flags = being_solved.spec[on];
  being_solved.spec[on] = spec;

  replace_walk(on,Dummy);

  if (TSTFLAG(spec,White))
    --being_solved.number_of_pieces[White][walk];
  if (TSTFLAG(spec,Black))
    --being_solved.number_of_pieces[Black][walk];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void undo_revelation_of_new_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(on);
  TraceWalk(walk);
  TraceWalk(get_walk_of_piece_on_square(on));
  TraceValue("%x",spec);
  TraceValue("%x",being_solved.spec[on]);
  TraceValue("%u",play_phase);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
    case play_rewinding:
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

    case play_validating_mate:
      assert(!is_square_empty(on));
      SETFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_initialising_replay:
    case play_replay_validating:
    case play_replay_testing:
      break;

    case play_detecting_revelations:
    case play_testing_mate:
    case play_attacking_mating_piece:
    case play_unwinding:
    case play_finalising_replay:
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void redo_revelation_of_new_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const walk = entry->u.piece_addition.added.walk;
  Flags const spec = entry->u.piece_addition.added.flags;
  Side const side_revealed = TSTFLAG(spec,White) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(on);
  TraceWalk(walk);
  TraceWalk(get_walk_of_piece_on_square(on));
  TraceValue("%x",spec);
  TraceValue("%x",being_solved.spec[on]);
  TraceValue("%u",play_phase);
  TraceEnumerator(Side,side_revealed);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
    case play_unwinding:
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
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      break;

    case play_validating_mate:
      assert(!is_square_empty(on));
      assert(TSTFLAG(being_solved.spec[on],side_revealed));
      CLRFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_finalising_replay:
    case play_replay_validating:
    case play_replay_testing:
      break;

    case play_detecting_revelations:
    case play_testing_mate:
    case play_attacking_mating_piece:
    case play_rewinding:
    case play_initialising_replay:
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

void undo_revelation_of_castling_partner(move_effect_journal_entry_type const *entry)
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
    case play_attacking_mating_piece:
    case play_initialising_replay:
    case play_replay_validating:
    case play_replay_testing:
    case play_finalising_replay:
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      SETFLAG(being_solved.spec[on],Chameleon);
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

void redo_revelation_of_castling_partner(move_effect_journal_entry_type const *entry)
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
      assert(TSTFLAG(being_solved.spec[on],Chameleon));
      CLRFLAG(being_solved.spec[on],Chameleon);
      break;

    case play_testing_mate:
    case play_attacking_mating_piece:
    case play_initialising_replay:
    case play_replay_validating:
    case play_replay_testing:
    case play_finalising_replay:
      assert(!is_square_empty(on));
      assert(TSTFLAG(being_solved.spec[on],Chameleon));
      CLRFLAG(being_solved.spec[on],Chameleon);
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
  square pos = move_effect_journal[idx].u.revelation_of_placed_piece.on;
  PieceIdType const id = GetPieceId(move_effect_journal[idx].u.revelation_of_placed_piece.flags_revealed);
  piece_walk_type const walk_to = move_effect_journal[idx].u.revelation_of_placed_piece.walk_revealed;

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
        }
        else
        {
          assert(pos!=move_effect_journal[idx].u.piece_movement.to);
        }
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
          }
          idx = 1;
        }
        else
        {
          assert(pos!=move_effect_journal[idx].u.piece_addition.added.on);
        }
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
  square pos = move_effect_journal[idx].u.revelation_of_placed_piece.on;
  PieceIdType const id = GetPieceId(move_effect_journal[idx].u.revelation_of_placed_piece.flags_revealed);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx);
  TraceFunctionParamListEnd();

  TraceSquare(pos);
  TraceWalk(move_effect_journal[idx].u.revelation_of_placed_piece.walk_revealed);
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
        }
        else
        {
          assert(pos!=move_effect_journal[idx].u.piece_movement.to);
        }
        break;

      case move_effect_piece_readdition:
        TraceValue("%u",GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags));
        TraceSquare(move_effect_journal[idx].u.piece_addition.added.on);
        TraceEOL();
        if (id==GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags))
        {
          assert(pos==move_effect_journal[idx].u.piece_addition.added.on);
          move_effect_journal[idx].u.piece_addition.added.walk = Dummy;
          idx = 1;
        }
        else
        {
          assert(pos!=move_effect_journal[idx].u.piece_addition.added.on);
        }
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
  assert(TSTFLAG(being_solved.spec[on],Chameleon));

  entry->u.revelation_of_placed_piece.on = on;
  entry->u.revelation_of_placed_piece.walk_original = get_walk_of_piece_on_square(on);
  entry->u.revelation_of_placed_piece.flags_original = being_solved.spec[on];
  entry->u.revelation_of_placed_piece.walk_revealed = walk;
  entry->u.revelation_of_placed_piece.flags_revealed = spec;

  if (TSTFLAG(spec,Royal) && walk==King)
  {
    Side const side = TSTFLAG(spec,White) ? White : Black;
    being_solved.king_square[side] = on;
  }

  replace_walk(on,walk);
  being_solved.spec[on] = spec;

  if (TSTFLAG(spec,White))
    ++being_solved.number_of_pieces[White][walk];
  if (TSTFLAG(spec,Black))
    ++being_solved.number_of_pieces[Black][walk];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void replace_moving_piece_ids_in_past_moves(PieceIdType from, PieceIdType to, ply up_to_ply)
{
  ply ply;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",from);
  TraceFunctionParam("%u",to);
  TraceFunctionParam("%u",up_to_ply);
  TraceFunctionParamListEnd();

  for (ply = ply_retro_move+1; ply<=up_to_ply; ++ply)
  {
    move_effect_journal_index_type const effects_base = move_effect_journal_base[ply];
    move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
    PieceIdType const id_moving = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);

    // TODO what about other effects?
    // castling partner movement
    // piece removal

    TraceValue("%u",ply);
    TraceValue("%u",id_moving);
    TraceEOL();

    if (id_moving==from)
      SetPieceId(move_effect_journal[movement].u.piece_movement.movingspec,to);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void adapt_id_of_existing_to_revealed(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.revelation_of_placed_piece.on;
  Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;
  PieceIdType const id_on_board = GetPieceId(being_solved.spec[on]);
  PieceIdType const id_revealed = GetPieceId(flags_revealed);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",id_on_board);
  TraceValue("%u",id_revealed);
  TraceEOL();

  assert(id_on_board!=id_revealed);
  assert(TSTFLAG(being_solved.spec[on],Chameleon));
  being_solved.spec[on] = flags_revealed;
  replace_moving_piece_ids_in_past_moves(id_on_board,id_revealed,nbply);
  assert(!TSTFLAG(being_solved.spec[on],Chameleon));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void unadapt_id_of_existing_to_revealed(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.revelation_of_placed_piece.on;
  piece_walk_type const walk_revealed = entry->u.revelation_of_placed_piece.walk_revealed;
  Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;
  Flags const flags_original = entry->u.revelation_of_placed_piece.flags_original;
  PieceIdType const id_original = GetPieceId(flags_original);
  PieceIdType const id_revealed = GetPieceId(flags_revealed);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",id_original);
  TraceValue("%u",id_revealed);
  TraceEOL();

  assert(id_original!=id_revealed);

  assert(get_walk_of_piece_on_square(on)==walk_revealed);
  assert(!TSTFLAG(being_solved.spec[on],Chameleon));
  assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
  replace_moving_piece_ids_in_past_moves(id_revealed,id_original,nbply);
  being_solved.spec[on] = flags_original;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void reveal_placed(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.revelation_of_placed_piece.on;
  Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;
  PieceIdType const id_on_board = GetPieceId(being_solved.spec[on]);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(TSTFLAG(being_solved.spec[on],Chameleon));
  being_solved.spec[on] = flags_revealed;
  SetPieceId(being_solved.spec[on],id_on_board);
  assert(!TSTFLAG(being_solved.spec[on],Chameleon));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void unreveal_placed(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.revelation_of_placed_piece.on;
  piece_walk_type const walk_revealed = entry->u.revelation_of_placed_piece.walk_revealed;
  Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(get_walk_of_piece_on_square(on)==walk_revealed);
  assert(!TSTFLAG(being_solved.spec[on],Chameleon));
  assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
  SETFLAG(being_solved.spec[on],Chameleon);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void undo_revelation_of_placed_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.revelation_of_placed_piece.on;
  piece_walk_type const walk_original = entry->u.revelation_of_placed_piece.walk_original;
  Flags const flags_original = entry->u.revelation_of_placed_piece.flags_original;
  piece_walk_type const walk_revealed = entry->u.revelation_of_placed_piece.walk_revealed;
  Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",play_phase);
  TraceSquare(on);
  TraceWalk(walk_original);
  TraceWalk(walk_revealed);
  TraceWalk(get_walk_of_piece_on_square(on));
  TraceValue("%x",flags_original);
  TraceValue("%x",flags_revealed);
  TraceValue("%x",being_solved.spec[on]);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
      assert(!is_square_empty(on));

      if (TSTFLAG(flags_revealed,Royal) && walk_revealed==King)
      {
        Side const side = TSTFLAG(flags_revealed,White) ? White : Black;
        being_solved.king_square[side] = initsquare;
      }

      replace_walk(on,walk_original);

      if (TSTFLAG(being_solved.spec[on],White))
        --being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
      if (TSTFLAG(being_solved.spec[on],Black))
        --being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
      SETFLAG(being_solved.spec[on],Chameleon);
      /* just in case */
      CLRFLAG(being_solved.spec[on],Royal);
      being_solved.spec[on] = flags_original;
      break;

    case play_rewinding:
    {
      Side const side_revealed = TSTFLAG(flags_revealed,White) ? White : Black;
      Side const side_original = TSTFLAG(flags_original,White) ? White : Black;
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      taint_history_of_placed_piece(entry-&move_effect_journal[0]);
      assert(!is_square_empty(on));
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
      SETFLAG(being_solved.spec[on],Chameleon);
      assert(TSTFLAG(being_solved.spec[on],side_revealed));
      if (side_revealed!=side_original)
      {
        /* a move by invisible must have captured the original piece,
         * thereby changing the colour of the piece on square on! */
        CLRFLAG(being_solved.spec[on],side_revealed);
        SETFLAG(being_solved.spec[on],side_original);
      }
      if (TSTFLAG(being_solved.spec[on],Royal))
      {
        Side const side = TSTFLAG(flags_revealed,White) ? White : Black;
        assert(being_solved.king_square[side]==on);
        assert(TSTFLAG(flags_revealed,Royal));
        being_solved.king_square[side] = initsquare;
        CLRFLAG(being_solved.spec[on],Royal);
      }
      break;
    }

    case play_detecting_revelations:
    case play_validating_mate:
    case play_testing_mate:
    case play_attacking_mating_piece:
    case play_initialising_replay:
    case play_replay_validating:
    case play_replay_testing:
    case play_finalising_replay:
      unreveal_placed(entry);
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

void redo_revelation_of_placed_invisible(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.revelation_of_placed_piece.on;
  piece_walk_type const walk_revealed = entry->u.revelation_of_placed_piece.walk_revealed;
  Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;
  Side const side_revealed = TSTFLAG(flags_revealed,White) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",play_phase);
  TraceSquare(on);
  TraceWalk(walk_revealed);
  TraceWalk(get_walk_of_piece_on_square(on));
  TraceValue("%x",flags_revealed);
  TraceValue("%x",being_solved.spec[on]);
  TraceEnumerator(Side,side_revealed);
  TraceEOL();

  switch (play_phase)
  {
    case play_regular:
    {
      PieceIdType const id_on_board = GetPieceId(being_solved.spec[on]);
      assert(get_walk_of_piece_on_square(on)==Dummy);

      if (TSTFLAG(flags_revealed,Royal) && walk_revealed==King)
      {
        Side const side = TSTFLAG(flags_revealed,White) ? White : Black;
        being_solved.king_square[side] = on;
      }

      replace_walk(on,walk_revealed);

      if (TSTFLAG(being_solved.spec[on],White))
        ++being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
      if (TSTFLAG(being_solved.spec[on],Black))
        ++being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

      being_solved.spec[on] = flags_revealed;
      SetPieceId(being_solved.spec[on],id_on_board);
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
      break;
    }

    case play_rewinding:
      assert(0);
      break;

    case play_detecting_revelations:
    case play_validating_mate:
    case play_testing_mate:
    case play_attacking_mating_piece:
    case play_initialising_replay:
    case play_replay_validating:
    case play_replay_testing:
    case play_finalising_replay:
      assert(!is_square_empty(on));
      assert(get_walk_of_piece_on_square(on)==walk_revealed
             && TSTFLAG(being_solved.spec[on],side_revealed));
      reveal_placed(entry);
      break;

    case play_unwinding:
    {
      PieceIdType const id_on_board = GetPieceId(being_solved.spec[on]);
      assert(!is_square_empty(on));
      assert(TSTFLAG(being_solved.spec[on],Chameleon));
      being_solved.spec[on] = flags_revealed;
      SetPieceId(being_solved.spec[on],id_on_board);
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      untaint_history_of_placed_piece(entry-&move_effect_journal[0]);
      assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
      if (TSTFLAG(flags_revealed,Royal))
      {
        assert(being_solved.king_square[side_revealed]==initsquare);
        being_solved.king_square[side_revealed] = on;
      }
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static PieceIdType add_revelation_effect(square s, unsigned int idx)
{
  PieceIdType result = NullPieceId;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  Flags spec = revelation_status[idx].spec;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParam("%u",idx);
  TraceFunctionParamListEnd();

  TraceWalk(revelation_status[idx].walk);
  TraceFunctionParam("%x",revelation_status[idx].spec);
  TraceEOL();

  assert(TSTFLAG(spec,Chameleon));
  CLRFLAG(spec,Chameleon);

  if (is_square_empty(s))
  {
    TraceValue("%u",nbply);
    TraceConsumption();TraceEOL();
    TraceText("revelation of a hitherto unplaced invisible (typically a king)\n");

    {
      result = initialise_motivation_from_revelation(&revelation_status[idx]);

      SetPieceId(spec,result);
      do_revelation_of_new_invisible(move_effect_reason_revelation_of_invisible,
                                     s,revelation_status[idx].walk,spec);
    }
  }
  else
  {
    if (move_effect_journal[base].type==move_effect_piece_readdition
        && move_effect_journal[base].reason==move_effect_reason_castling_partner
        && (GetPieceId(move_effect_journal[base].u.piece_addition.added.flags)
            ==GetPieceId(spec)))
    {
      TraceText("pseudo revelation of a castling partner\n");
      assert(TSTFLAG(being_solved.spec[s],Chameleon));
      CLRFLAG(being_solved.spec[s],Chameleon);
      assert(GetPieceId(spec)==GetPieceId(being_solved.spec[s]));
      do_revelation_of_castling_partner(move_effect_reason_revelation_of_invisible,
                                        s,revelation_status[idx].walk,spec);
    }
    else
    {
      TraceText("revelation of a placed invisible\n");
      SetPieceId(spec,GetPieceId(being_solved.spec[s]));
      do_revelation_of_placed_invisible(move_effect_reason_revelation_of_invisible,
                                        s,revelation_status[idx].walk,spec);
    }
  }

  assert(!TSTFLAG(being_solved.spec[s],Chameleon));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void setup_revelations(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  square sq_ep_capture = initsquare;

  square const *s;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].reason==move_effect_reason_ep_capture)
    sq_ep_capture = move_effect_journal[capture].u.piece_removal.on;

  nr_potential_revelations = 0;

  for (s = boardnum; *s; ++s)
    if (*s!=sq_ep_capture
        && (is_square_empty(*s) || TSTFLAG(being_solved.spec[*s],Chameleon)))
    {
      revelation_status[nr_potential_revelations].first_on = *s;
      ++nr_potential_revelations;
    }

  revelation_status_is_uninitialised = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void initialise_revelations(void)
{
  unsigned int i = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (i!=nr_potential_revelations)
  {
    square const s = revelation_status[i].first_on;
    piece_walk_type const walk = get_walk_of_piece_on_square(s);
    if (walk==Empty)
    {
      memmove(&revelation_status[i],&revelation_status[i+1],
              (nr_potential_revelations-i-1)*sizeof revelation_status[0]);
      --nr_potential_revelations;
    }
    else
    {
      PieceIdType const id = GetPieceId(being_solved.spec[s]);

      TraceValue("%u",i);
      TraceSquare(s);
      TraceWalk(get_walk_of_piece_on_square(s));
      TraceValue("%x",being_solved.spec[s]);
      TraceValue("%u",id);TraceEOL();
      TraceAction(&motivation[id].first);TraceEOL();
      TraceAction(&motivation[id].last);TraceEOL();

      revelation_status[i].walk = walk;
      revelation_status[i].spec = being_solved.spec[s];
      revelation_status[i].first = motivation[id].first;
      revelation_status[i].first.on = initsquare;
      revelation_status[i].last = motivation[id].last;
      TraceAction(&revelation_status[i].first);TraceEOL();
      TraceAction(&revelation_status[i].last);TraceEOL();

      ++i;
    }
  }

  revelation_status_is_uninitialised = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void update_revelations(void)
{
  unsigned int i = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (i!=nr_potential_revelations)
  {
    square const s = revelation_status[i].first_on;
    if (get_walk_of_piece_on_square(s)!=revelation_status[i].walk
        || (being_solved.spec[s]&PieSpMask)!=(revelation_status[i].spec&PieSpMask))
    {
      TraceValue("%u",i);
      TraceSquare(s);
      TraceWalk(get_walk_of_piece_on_square(s));
      TraceValue("%x",being_solved.spec[s]);
      TraceEOL();
      memmove(&revelation_status[i],&revelation_status[i+1],
              (nr_potential_revelations-i-1)*sizeof revelation_status[0]);
      --nr_potential_revelations;
    }
    else
    {
      PieceIdType const id = GetPieceId(being_solved.spec[s]);
      square const first_on = motivation[id].first.on;

      assert(id!=NullPieceId);
      assert(is_on_board(first_on));

      revelation_status[i].spec = being_solved.spec[s];

      if (revelation_status[i].last.acts_when!=ply_nil)
      {
        if (motivation[id].last.on!=revelation_status[i].last.on)
          revelation_status[i].last.acts_when = ply_nil;
        else
        {
          if (motivation[id].last.acts_when>revelation_status[i].last.acts_when)
            revelation_status[i].last = motivation[id].last;
          if (motivation[id].first.acts_when<revelation_status[i].first.acts_when)
          {
            // TODO leaving .first.on == initsquare
            revelation_status[i].first.acts_when = motivation[id].first.acts_when;
            revelation_status[i].first.purpose = motivation[id].first.purpose;
          }
        }
      }

      TraceValue("%u",i);
      TraceSquare(s);
      TraceWalk(get_walk_of_piece_on_square(s));
      TraceValue("%x",being_solved.spec[s]);
      TraceValue("%u",id);TraceEOL();
      TraceAction(&motivation[id].first);TraceEOL();
      TraceAction(&motivation[id].last);TraceEOL();
      TraceValue("%x",revelation_status[i].spec);TraceEOL();
      TraceAction(&revelation_status[i].first);TraceEOL();
      TraceAction(&revelation_status[i].last);TraceEOL();

      ++i;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void evaluate_revelations(slice_index si,
                          unsigned int nr_potential_revelations)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_potential_revelations);
  TraceFunctionParamListEnd();

  if (nr_potential_revelations==0)
    pipe_solve_delegate(si);
  else
  {
    unsigned int const i = nr_potential_revelations-1;
    square const s = revelation_status[i].first_on;

    TraceSquare(s);
    TraceWalk(revelation_status[i].walk);TraceEOL();
    TraceAction(&revelation_status[i].first);TraceEOL();
    TraceAction(&revelation_status[i].last);TraceEOL();

    if (revelation_status[i].walk!=Empty)
    {
      PieceIdType const id_new = add_revelation_effect(s,i);

      if (revelation_status[i].first.on!=initsquare)
      {
        knowledge[size_knowledge].revealed_on = s;
        knowledge[size_knowledge].first_on = revelation_status[i].first.on;
        knowledge[size_knowledge].last = revelation_status[i].last;
        knowledge[size_knowledge].walk = revelation_status[i].walk;
        knowledge[size_knowledge].spec = revelation_status[i].spec;
        knowledge[size_knowledge].is_allocated = motivation[id_new].first.purpose==purpose_castling_partner;

        /* if we revealed a so far unplaced invisible piece, the id will have changed */
        SetPieceId(knowledge[size_knowledge].spec,id_new);

        TraceValue("%u",id_new);TraceEOL();
        TraceAction(&motivation[id_new].first);TraceEOL();
        TraceAction(&motivation[id_new].last);TraceEOL();
        TraceWalk(knowledge[size_knowledge].walk);
        TraceSquare(knowledge[size_knowledge].first_on);
        TraceValue("%x",knowledge[size_knowledge].spec);
        TraceEOL();

        ++size_knowledge;
        evaluate_revelations(si,i);
        --size_knowledge;
      }
      else
        evaluate_revelations(si,i);

      if (id_new!=NullPieceId)
        uninitialise_motivation(id_new);
    }
    else
      evaluate_revelations(si,i);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void apply_royal_knowledge(knowledge_index_type idx_knowledge,
                                  void (*next_step)(void))
{
  square const s = knowledge[idx_knowledge].first_on;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx_knowledge);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[s],Royal))
  {
    Side const side = TSTFLAG(knowledge[idx_knowledge].spec,White) ? White : Black;
    TraceSquare(being_solved.king_square[side]);
    TraceWalk(get_walk_of_piece_on_square(being_solved.king_square[side]));
    TraceEOL();
    assert(being_solved.king_square[side]==initsquare);
    being_solved.king_square[side] = s;
    apply_knowledge(idx_knowledge+1,next_step);
    being_solved.king_square[side] = initsquare;
  }
  else
    apply_knowledge(idx_knowledge+1,next_step);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void apply_knowledge(knowledge_index_type idx_knowledge,
                     void (*next_step)(void))
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx_knowledge);
  TraceFunctionParamListEnd();

  TraceValue("%u",size_knowledge);TraceEOL();

  if (idx_knowledge==size_knowledge)
    (*next_step)();
  else
  {
    dynamic_consumption_type const save_consumption = current_consumption;
    square const sq_first_on = knowledge[idx_knowledge].first_on;
    Side const side = TSTFLAG(knowledge[idx_knowledge].spec,White) ? White : Black;

    TraceSquare(sq_first_on);
    TraceSquare(knowledge[idx_knowledge].revealed_on);
    TraceSquare(knowledge[idx_knowledge].first_on);
    TraceWalk(knowledge[idx_knowledge].walk);
    TraceValue("%x",knowledge[idx_knowledge].spec);
    TraceEnumerator(Side,side);
    TraceValue("%u",knowledge[idx_knowledge].last.acts_when);
    TraceValue("%u",knowledge[idx_knowledge].last.purpose);
    TraceSquare(knowledge[idx_knowledge].last.on);
    TraceEOL();

    if ((knowledge[idx_knowledge].is_allocated
         || allocate_placed(side))
        && !(is_taboo(sq_first_on,side) || will_be_taboo(sq_first_on,side)))
    {
      ++being_solved.number_of_pieces[side][knowledge[idx_knowledge].walk];
      occupy_square(knowledge[idx_knowledge].first_on,
                    knowledge[idx_knowledge].walk,
                    knowledge[idx_knowledge].spec);

      TraceValue("%u",knowledge[idx_knowledge].last.acts_when);TraceEOL();
      if (knowledge[idx_knowledge].last.acts_when!=ply_nil)
      {
        PieceIdType const id_on_board = GetPieceId(being_solved.spec[knowledge[idx_knowledge].last.on]);
        motivation_type const save_motivation = motivation[id_on_board];
        move_effect_journal_index_type const effects_base = move_effect_journal_base[knowledge[idx_knowledge].last.acts_when];
        move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

        TraceWalk(move_effect_journal[movement].u.piece_movement.moving);
        TraceValue("%x",move_effect_journal[movement].u.piece_movement.movingspec);
        TraceSquare(move_effect_journal[movement].u.piece_movement.from);
        TraceSquare(move_effect_journal[movement].u.piece_movement.to);
        TraceEOL();

        assert(move_effect_journal[movement].type==move_effect_piece_movement);

        if (knowledge[idx_knowledge].last.purpose==purpose_capturer)
        {
          /* applying this knowledge doesn't work if the revealed piece has
           * moved before this capturing move */
          PieceIdType const id_from_knowledge = GetPieceId(knowledge[idx_knowledge].spec);
          if (id_on_board!=id_from_knowledge)
            apply_knowledge(idx_knowledge+1,next_step);
          else
          {
            ply const save_nbply = nbply;
            move_effect_journal_index_type const precapture = effects_base;
            move_effect_journal_entry_type const save_movement = move_effect_journal[movement];

            motivation[id_on_board].first = knowledge[idx_knowledge].last;
            motivation[id_on_board].first.on = sq_first_on;
            motivation[id_on_board].first.acts_when = 0;
            motivation[id_on_board].last = knowledge[idx_knowledge].last;

            TraceValue("%u",motivation[id_on_board].last.purpose);TraceEOL();

            assert(move_effect_journal[precapture].type==move_effect_piece_readdition);
            assert(move_effect_journal[movement].u.piece_movement.to==knowledge[idx_knowledge].revealed_on);
            assert(move_effect_journal[movement].u.piece_movement.from==capture_by_invisible);

            TraceText("prevent searching for capturer - we know who did it\n");

            move_effect_journal[precapture].type = move_effect_none;
            move_effect_journal[movement].u.piece_movement.from = knowledge[idx_knowledge].last.on;
            move_effect_journal[movement].u.piece_movement.moving = get_walk_of_piece_on_square(knowledge[idx_knowledge].last.on);
            move_effect_journal[movement].u.piece_movement.movingspec = being_solved.spec[sq_first_on];

            nbply = knowledge[idx_knowledge].last.acts_when;
            remember_taboos_for_current_move();
            nbply = save_nbply;

            apply_royal_knowledge(idx_knowledge,next_step);

            nbply = knowledge[idx_knowledge].last.acts_when;
            forget_taboos_for_current_move();
            nbply = save_nbply;

            move_effect_journal[movement] = save_movement;
            move_effect_journal[precapture].type = move_effect_piece_readdition;
          }
        }
        else if (knowledge[idx_knowledge].last.purpose==purpose_castling_partner)
        {
          motivation[id_on_board].first = knowledge[idx_knowledge].last;
          motivation[id_on_board].first.on = sq_first_on;
          motivation[id_on_board].first.acts_when = 0;
          motivation[id_on_board].last = knowledge[idx_knowledge].last;

          TraceValue("%u",motivation[id_on_board].last.purpose);TraceEOL();

          assert(move_effect_journal[movement].u.piece_movement.moving==King);
          assert(is_on_board(move_effect_journal[movement].u.piece_movement.from));
          assert(is_on_board(move_effect_journal[movement].u.piece_movement.to));
          apply_royal_knowledge(idx_knowledge,next_step);
        }
        else if (knowledge[idx_knowledge].last.purpose==purpose_random_mover)
        {
          motivation[id_on_board].first = knowledge[idx_knowledge].last;
          motivation[id_on_board].first.on = sq_first_on;
          motivation[id_on_board].first.acts_when = 0;
          motivation[id_on_board].last = knowledge[idx_knowledge].last;

          TraceValue("%u",motivation[id_on_board].last.purpose);TraceEOL();

          // TODO can we restrict generation of random move to the revealed piece?
          assert(move_effect_journal[movement].u.piece_movement.moving==Empty);
          assert(move_effect_journal[movement].u.piece_movement.from==move_by_invisible);
          assert(move_effect_journal[movement].u.piece_movement.to==move_by_invisible);
          apply_royal_knowledge(idx_knowledge,next_step);
        }
        else if (knowledge[idx_knowledge].last.purpose==purpose_interceptor)
        {
          motivation[id_on_board].first = knowledge[idx_knowledge].last;
          motivation[id_on_board].first.on = sq_first_on;
          motivation[id_on_board].first.acts_when = 0;
          motivation[id_on_board].last = knowledge[idx_knowledge].last;

          TraceValue("%u",motivation[id_on_board].last.purpose);TraceEOL();

          // We no longer look for different insertion squares for pieces
          // revealed after having been inserted to move to intercept an
          // illegal check.
          // This seems to have been useless knowledge anyway...
//          assert(sq_first_on==knowledge[idx_knowledge].revealed_on);
          apply_royal_knowledge(idx_knowledge,next_step);
        }
        else
          assert(0);

        motivation[id_on_board] = save_motivation;
      }
      else
        apply_royal_knowledge(idx_knowledge,next_step);

      assert(sq_first_on==knowledge[idx_knowledge].first_on);
      empty_square(knowledge[idx_knowledge].first_on);
      --being_solved.number_of_pieces[side][knowledge[idx_knowledge].walk];
    }
    else
    {
      combined_result = previous_move_has_not_solved;
      TraceText("allocation for application of knowledge not possible\n");
      record_decision_outcome("%s","allocation for application of knowledge not possible");
      REPORT_DEADEND;
    }

    current_consumption = save_consumption;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void make_revelations(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  top_ply_of_regular_play = nbply;
  setup_revelations();
  play_phase = play_rewinding;
  rewind_effects();
  play_phase = play_detecting_revelations;
  max_decision_level = decision_level_latest;
  record_decision_context();

  static_consumption.king[White] = being_solved.king_square[White]==initsquare;
  static_consumption.king[Black] = being_solved.king_square[Black]==initsquare;

  apply_knowledge(0,&start_iteration);

  static_consumption.king[White] = false;
  static_consumption.king[Black] = false;

  play_phase = play_unwinding;
  unrewind_effects();
  play_phase = play_regular;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void do_revelation_bookkeeping(void)
{
  unsigned int const prev_nr_potential_revelations = nr_potential_revelations;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceText("Updating revelation candidates\n");
  if (revelation_status_is_uninitialised)
  {
    initialise_revelations();
    record_decision_outcome("initialised revelation candidates."
                            " %u found",
                            nr_potential_revelations);
  }
  else
  {
    update_revelations();
    record_decision_outcome("updated revelation candidates."
                            " %u of %u left",
                            nr_potential_revelations,
                            prev_nr_potential_revelations);
  }

  {
    decision_level_type max_level = decision_level_forever;
    unsigned int i;
    TraceValue("%u",nr_potential_revelations);TraceEOL();
    for (i = 0; i!=nr_potential_revelations; ++i)
    {
      PieceIdType const id = GetPieceId(revelation_status[i].spec);

      TraceValue("%u",i);
      TraceWalk(revelation_status[i].walk);
      TraceSquare(revelation_status[i].last.on);
      TraceValue("%x",revelation_status[i].spec);
      TraceValue("%u",id);
      TraceValue("%u",decision_levels[id].side);
      TraceValue("%u",decision_levels[id].walk);
      TraceValue("%u",decision_levels[id].to);
      TraceEOL();
      assert(decision_levels[id].side!=decision_level_uninitialised);
      assert(decision_levels[id].walk!=decision_level_uninitialised);
      if (decision_levels[id].side>max_level)
        max_level = decision_levels[id].side;
      if (decision_levels[id].walk>max_level)
        max_level = decision_levels[id].walk;
      if (decision_levels[id].to>max_level)
        max_level = decision_levels[id].to;
    }
    TraceValue("%u",max_level);TraceEOL();
    max_decision_level = max_level;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

}

void undo_revelation_effects(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceValue("%u",top_before_revelations[nbply]);
  TraceEOL();

  if (curr==move_effect_journal_base[nbply])
  {
    if (is_random_move_by_invisible(nbply))
      backward_fleshout_random_move_by_invisible();
    else
    {
      max_decision_level = decision_level_latest;
      restart_from_scratch();
    }
  }
  else
  {
    move_effect_journal_entry_type const * const entry = &move_effect_journal[curr-1];

    TraceValue("%u",entry->type);TraceEOL();
    switch (entry->type)
    {
      case move_effect_none:
      case move_effect_no_piece_removal:
        undo_revelation_effects(curr-1);
        break;

      case move_effect_piece_removal:
        undo_piece_removal(entry);
        undo_revelation_effects(curr-1);
        redo_piece_removal(entry);
        break;

      case move_effect_piece_movement:
        /* we may have added an interceptor on the square evacuated here, but failed to move
         * it to our departure square in a random move
         */
        if (is_square_empty(entry->u.piece_movement.from))
        {
          undo_piece_movement(entry);
          undo_revelation_effects(curr-1);
          redo_piece_movement(entry);
        }
        else
        {
          // TODO backtrack how far? placement of the blocker? or rather "non-removal" of it (i.e. the last random move of its side)?
        }
        break;

      case move_effect_walk_change:
        undo_walk_change(entry);
        undo_revelation_effects(curr-1);
        redo_walk_change(entry);
        break;

      case move_effect_king_square_movement:
        undo_king_square_movement(entry);
        undo_revelation_effects(curr-1);
        redo_king_square_movement(entry);
        break;

      case move_effect_disable_castling_right:
        move_effect_journal_undo_disabling_castling_right(entry);
        undo_revelation_effects(curr-1);
        move_effect_journal_redo_disabling_castling_right(entry);
        break;

      case move_effect_remember_ep_capture_potential:
        move_effect_journal_undo_remember_ep(entry);
        undo_revelation_effects(curr-1);
        move_effect_journal_redo_remember_ep(entry);
        break;

      case move_effect_revelation_of_new_invisible:
        unreveal_new(entry);
        undo_revelation_effects(curr-1);
        reveal_new(entry);
        break;

      case move_effect_revelation_of_placed_invisible:
        undo_revelation_of_placed_invisible(entry);
        undo_revelation_effects(curr-1);
        redo_revelation_of_placed_invisible(entry);
        break;

      case move_effect_revelation_of_castling_partner:
      {
        PieceIdType const id = GetPieceId(entry->u.piece_addition.added.flags);

        undo_revelation_of_castling_partner(entry);
        motivation[id].last.purpose = purpose_castling_partner;
        undo_revelation_effects(curr-1);
        motivation[id].last.purpose = purpose_none;
        redo_revelation_of_castling_partner(entry);
        break;
      }

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void test_and_execute_revelations(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

  if (curr==move_effect_journal_base[nbply+1])
  {
    ++nbply;
    TraceValue("%u",nbply);TraceEOL();
    start_iteration();
    --nbply;
  }
  else
  {
    move_effect_journal_entry_type * const entry = &move_effect_journal[curr];
    switch (entry->type)
    {
      case move_effect_revelation_of_new_invisible:
      {
        square const on = entry->u.piece_addition.added.on;
        piece_walk_type const walk = entry->u.piece_addition.added.walk;
        Flags const spec = entry->u.piece_addition.added.flags;
        Side const side_revealed = TSTFLAG(spec,White) ? White : Black;

        if (is_square_empty(on))
        {
          TraceText("revelation expected, but square is empty - aborting\n");
          record_decision_outcome("%s","revelation expected, but square is empty - aborting");
          REPORT_DEADEND;
        }
        else if (play_phase==play_validating_mate && get_walk_of_piece_on_square(on)==Dummy)
        {
          if (TSTFLAG(spec,Royal)
              && walk==King
              && being_solved.king_square[side_revealed]!=initsquare)
          {
            // TODO can we avoid this situation?
            TraceText("revelation of king - but king has already been placed - aborting\n");
            record_decision_outcome("%s","revelation of king - but king has already been placed - aborting");
            REPORT_DEADEND;
          }
          else if (TSTFLAG(being_solved.spec[on],side_revealed))
          {
            square const on = entry->u.piece_addition.added.on;
            Flags const spec_on_board = being_solved.spec[on];
            PieceIdType const id_on_board = GetPieceId(spec_on_board);
            purpose_type const purpose_on_board = motivation[id_on_board].last.purpose;

            Flags const spec_added = entry->u.piece_addition.added.flags;
            PieceIdType const id_added = GetPieceId(spec_added);
            purpose_type const purpose_added = motivation[id_added].last.purpose;

            reveal_new(entry);
            motivation[id_on_board].last.purpose = purpose_none;
            motivation[id_added].last.purpose = purpose_none;
            test_and_execute_revelations(curr+1);
            motivation[id_added].last.purpose = purpose_added;
            motivation[id_on_board].last.purpose = purpose_on_board;
            unreveal_new(entry);
          }
          else
          {
            TraceText("revealed piece belongs to different side than actual piece\n");
            record_decision_outcome("%s","revealed piece belongs to different side than actual piece");
            REPORT_DEADEND;
          }
        }
        else if (get_walk_of_piece_on_square(on)==walk
                 && TSTFLAG(being_solved.spec[on],side_revealed))
        {
          PieceIdType const id_on_board = GetPieceId(being_solved.spec[on]);
          purpose_type const purpose_on_board = motivation[id_on_board].last.purpose;

          PieceIdType const id_revealed = GetPieceId(spec);
          purpose_type const purpose_revealed = motivation[id_revealed].last.purpose;

          TraceText("treat revelation of new invisible as revelation of placed invisible\n");

          assert(id_on_board!=id_revealed);

          entry->type = move_effect_revelation_of_placed_invisible;
          entry->u.revelation_of_placed_piece.on = on;
          entry->u.revelation_of_placed_piece.walk_original = get_walk_of_piece_on_square(on);
          entry->u.revelation_of_placed_piece.flags_original = being_solved.spec[on];
          entry->u.revelation_of_placed_piece.walk_revealed = walk;
          entry->u.revelation_of_placed_piece.flags_revealed = spec;

          adapt_id_of_existing_to_revealed(entry);

          motivation[id_on_board].last.purpose = purpose_none;
          motivation[id_revealed].last.purpose = purpose_none;

          test_and_execute_revelations(curr+1);

          motivation[id_revealed].last.purpose = purpose_revealed;
          motivation[id_on_board].last.purpose = purpose_on_board;

          unadapt_id_of_existing_to_revealed(entry);

          entry->type = move_effect_revelation_of_new_invisible;
          entry->u.piece_addition.added.on = on;
          entry->u.piece_addition.added.walk = walk;
          entry->u.piece_addition.added.flags = spec;
        }
        else
        {
          TraceText("revelation expected - but walk of present piece is different - aborting\n");
          record_decision_outcome("%s","revelation expected - but walk of present piece is different - aborting");
          REPORT_DEADEND;
        }
        break;
      }

      case move_effect_revelation_of_placed_invisible:
      {
        square const on = entry->u.revelation_of_placed_piece.on;
        piece_walk_type const walk_revealed = entry->u.revelation_of_placed_piece.walk_revealed;
        Flags const flags_revealed = entry->u.revelation_of_placed_piece.flags_revealed;
        Side const side_revealed = TSTFLAG(flags_revealed,White) ? White : Black;

        TraceEnumerator(Side,side_revealed);
        TraceWalk(walk_revealed);
        TraceSquare(on);
        TraceEOL();

        if (is_square_empty(on))
        {
          TraceText("the revealed piece isn't here (any more?)\n");
          record_decision_outcome("%s","the revealed piece isn't here (any more?)");
          REPORT_DEADEND;
        }
        else if (get_walk_of_piece_on_square(on)==walk_revealed
                 && TSTFLAG(being_solved.spec[on],side_revealed))
        {
          PieceIdType const id_revealed = GetPieceId(flags_revealed);
          purpose_type const purpose_revealed = motivation[id_revealed].last.purpose;

          PieceIdType const id_original = GetPieceId(entry->u.revelation_of_placed_piece.flags_original);
          purpose_type const purpose_original = motivation[id_original].last.purpose;

          reveal_placed(entry);

          motivation[id_revealed].last.purpose = purpose_none;

          /* the following distinction isn't strictly necessary, but it clarifies nicely
           * that the two ids may be, but aren't necessarily equal */
          if (id_revealed==id_original)
            test_and_execute_revelations(curr+1);
          else
          {
            motivation[id_original].last.purpose = purpose_none;
            test_and_execute_revelations(curr+1);
            motivation[id_original].last.purpose = purpose_original;
          }

          motivation[id_revealed].last.purpose = purpose_revealed;

          unreveal_placed(entry);
        }
        else
        {
          TraceText("the revelation has been violated - terminating redoing effects with this ply\n");
          record_decision_outcome("%s","the revelation has been violated - terminating redoing effects with this ply");
          REPORT_DEADEND;
        }
        break;
      }

      case move_effect_revelation_of_castling_partner:
      {
        PieceIdType const id = GetPieceId(entry->u.piece_addition.added.flags);

        redo_revelation_of_castling_partner(entry);
        motivation[id].last.purpose = purpose_none;
        test_and_execute_revelations(curr+1);
        motivation[id].last.purpose = purpose_castling_partner;
        undo_revelation_of_castling_partner(entry);
        break;
      }

      default:
        assert(0);
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
void total_invisible_reveal_after_mating_move(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  remember_taboos_for_current_move();
  make_revelations();
  forget_taboos_for_current_move();

  if (revelation_status_is_uninitialised)
    pipe_solve_delegate(si);
  else
    evaluate_revelations(si,nr_potential_revelations);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
