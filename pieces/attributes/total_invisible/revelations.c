#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

#include <string.h>

boolean revelation_status_is_uninitialised;
unsigned int nr_potential_revelations;
revelation_status_type revelation_status[nr_squares_on_board];
decision_level_type curr_decision_level = 2;
decision_level_type max_decision_level = decision_level_latest;
move_effect_journal_index_type top_before_relevations[maxply+1];

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
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      taint_history_of_placed_piece(entry-&move_effect_journal[0]);
      assert(!is_square_empty(on));
      assert(!TSTFLAG(being_solved.spec[on],Chameleon));
      assert((being_solved.spec[on]&PieSpMask)==(flags_revealed&PieSpMask));
      SETFLAG(being_solved.spec[on],Chameleon);
      if (TSTFLAG(being_solved.spec[on],Royal))
      {
        Side const side = TSTFLAG(flags_revealed,White) ? White : Black;
        assert(being_solved.king_square[side]==on);
        assert(TSTFLAG(flags_revealed,Royal));
        being_solved.king_square[side] = initsquare;
        CLRFLAG(being_solved.spec[on],Royal);
      }
      break;

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

static void add_revelation_effect(square s, unsigned int idx)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  Flags spec = revelation_status[idx].spec;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceWalk(revelation_status[idx].walk);
  TraceFunctionParam("%x",revelation_status[idx].spec);
  TraceFunctionParamListEnd();

  assert(TSTFLAG(spec,Chameleon));
  CLRFLAG(spec,Chameleon);

  if (is_square_empty(s))
  {
    TraceValue("%u",nbply);
    TraceConsumption();TraceEOL();
    TraceText("revelation of a hitherto unplaced invisible (typically a king)\n");

    ++top_invisible_piece_id;
    TraceValue("%u",top_invisible_piece_id);TraceEOL();

    motivation[top_invisible_piece_id].first = revelation_status[idx].first;
    motivation[top_invisible_piece_id].first.acts_when = nbply;
    motivation[top_invisible_piece_id].last = revelation_status[idx].last;
    motivation[top_invisible_piece_id].last.acts_when = nbply;

    TraceValue("%u",motivation[top_invisible_piece_id].first.purpose);
    TraceValue("%u",motivation[top_invisible_piece_id].first.acts_when);
    TraceSquare(motivation[top_invisible_piece_id].first.on);
    TraceValue("%u",motivation[top_invisible_piece_id].last.purpose);
    TraceValue("%u",motivation[top_invisible_piece_id].last.acts_when);
    TraceSquare(motivation[top_invisible_piece_id].last.on);
    TraceEOL();

    SetPieceId(spec,top_invisible_piece_id);
    do_revelation_of_new_invisible(move_effect_reason_revelation_of_invisible,
                                   s,revelation_status[idx].walk,spec);
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
  TraceFunctionResultEnd();
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
      TraceSquare(s);
      TraceWalk(walk);
      TraceValue("%x",being_solved.spec[s]);
      TraceEOL();
      revelation_status[i].walk = walk;
      revelation_status[i].spec = being_solved.spec[s];
      revelation_status[i].first = motivation[id].first;
      revelation_status[i].first.on = initsquare;
      revelation_status[i].last = motivation[id].last;
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

      TraceValue("%u",i);
      TraceSquare(s);
      TraceValue("%x",being_solved.spec[s]);
      TraceValue("%u",id);
      TraceSquare(motivation[id].last.on);
      TraceValue("%u",motivation[id].last.acts_when);
      TraceValue("%x",revelation_status[i].spec);
      TraceValue("%u",revelation_status[i].last.acts_when);
      TraceEOL();

      assert(id!=NullPieceId);
      assert(is_on_board(first_on));

      revelation_status[i].spec = being_solved.spec[s];

      if (revelation_status[i].last.acts_when!=ply_nil)
      {
        if (motivation[id].last.on!=revelation_status[i].last.on)
          revelation_status[i].last.acts_when = ply_nil;
        else if (motivation[id].last.acts_when<revelation_status[i].last.acts_when)
          // TODO this looks a little dubious
          revelation_status[i].last.acts_when = motivation[id].last.acts_when;
      }

      ++i;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void evaluate_revelations(void)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_potential_revelations; ++i)
  {
    square const s = revelation_status[i].first_on;
    TraceSquare(s);TraceWalk(revelation_status[i].walk);TraceEOL();
    if (revelation_status[i].walk!=Empty)
    {
      add_revelation_effect(s,i);
      TraceSquare(revelation_status[i].first.on);TraceEOL();
      if (revelation_status[i].first.on!=initsquare)
      {
        PieceIdType const id = GetPieceId(being_solved.spec[s]);

        knowledge[size_knowledge].revealed_on = s;
        knowledge[size_knowledge].first_on = revelation_status[i].first.on;
        knowledge[size_knowledge].last = revelation_status[i].last;
        knowledge[size_knowledge].walk = revelation_status[i].walk;
        knowledge[size_knowledge].spec = revelation_status[i].spec;
        knowledge[size_knowledge].is_allocated = motivation[id].first.purpose==purpose_castling_partner;

        /* if we revealed a so far unplaced invisible piece, the id will have changed */
        SetPieceId(knowledge[size_knowledge].spec,id);

        TraceValue("%u",revelation_status[i].last.acts_when);
        TraceSquare(revelation_status[i].last.on);
        TraceValue("%u",revelation_status[i].last.purpose);
        TraceValue("%u",id);
        TraceValue("%u",motivation[id].first.acts_when);
        TraceValue("%u",motivation[id].first.purpose);
        TraceSquare(motivation[id].first.on);
        TraceValue("%u",motivation[id].last.acts_when);
        TraceValue("%u",motivation[id].last.purpose);
        TraceSquare(motivation[id].last.on);
        TraceWalk(knowledge[size_knowledge].walk);
        TraceSquare(knowledge[size_knowledge].first_on);
        TraceValue("%x",knowledge[size_knowledge].spec);
        TraceEOL();

        ++size_knowledge;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void do_revelation_bookkeeping(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  REPORT_DECISION_DECLARE(unsigned int const prev_nr_potential_revelations = nr_potential_revelations);
  TraceText("Updating revelation candidates\n");
  if (revelation_status_is_uninitialised)
  {
    initialise_revelations();
    REPORT_DECISION_OUTCOME("initialised revelation candidates."
                            " %u found",
                            nr_potential_revelations);
  }
  else
  {
    update_revelations();
    REPORT_DECISION_OUTCOME("updated revelation candidates."
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
      TraceValue("%u",motivation[id].levels.side);
      TraceValue("%u",motivation[id].levels.walk);
      TraceEOL();
      assert(motivation[id].levels.side!=decision_level_uninitialised);
      assert(motivation[id].levels.walk!=decision_level_uninitialised);
      if (motivation[id].levels.side>max_level)
        max_level = motivation[id].levels.side;
      if (motivation[id].levels.walk>max_level)
        max_level = motivation[id].levels.walk;
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
  TraceValue("%u",top_before_relevations[nbply]);
  TraceEOL();

  if (curr==top_before_relevations[nbply])
  {
    move_effect_journal_index_type const save_top = move_effect_journal_base[nbply+1];

    move_effect_journal_base[nbply+1] = top_before_relevations[nbply];
    undo_move_effects();
    move_effect_journal_base[nbply+1] = save_top;

    if (is_random_move_by_invisible(nbply))
      backward_fleshout_random_move_by_invisible();
    else
    {
      // TODO WHEN IS curr_decision_level<=max_decision_level?
      TraceValue("%u",curr_decision_level);
      TraceValue("%u",max_decision_level);
      TraceEOL();
      if (curr_decision_level<=max_decision_level)
      {
        max_decision_level = decision_level_latest;
        restart_from_scratch();
      }
    }

    move_effect_journal_base[nbply+1] = top_before_relevations[nbply];
    redo_move_effects();
    move_effect_journal_base[nbply+1] = save_top;
  }
  else
  {
    move_effect_journal_entry_type * const entry = &move_effect_journal[curr-1];
    switch (entry->type)
    {
      case move_effect_revelation_of_new_invisible:
      {
        unreveal_new(entry);
        undo_revelation_effects(curr-1);
        reveal_new(entry);
        break;
      }

      case move_effect_revelation_of_placed_invisible:
      {
        undo_revelation_of_placed_invisible(entry);
        undo_revelation_effects(curr-1);
        redo_revelation_of_placed_invisible(entry);
        break;
      }

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
