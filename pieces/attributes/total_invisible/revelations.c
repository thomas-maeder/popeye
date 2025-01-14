#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "solving/pipe.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

boolean revelation_status_is_uninitialised;
unsigned int nr_potential_revelations;
revelation_status_type revelation_status[nr_squares_on_board];
move_effect_journal_index_type top_before_revelations[maxply+1];
motivation_type motivation[MaxPieceId+1];

motivation_type const motivation_null = {
    { purpose_none, ply_nil, initsquare },
    { purpose_none, ply_nil, initsquare }
};

static PieceIdType top_visible_piece_id;
static PieceIdType top_invisible_piece_id;

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

PieceIdType initialise_motivation(ply ply,
                                  purpose_type purpose_first, square sq_first,
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
  motivation[result].first.acts_when = ply;
  motivation[result].first.on = sq_first;
  motivation[result].last.purpose = purpose_last;
  motivation[result].last.acts_when = ply;
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

void uninitialise_motivation(PieceIdType id_uninitialised)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",id_uninitialised);
  TraceFunctionParamListEnd();

  TraceValue("%u",top_invisible_piece_id);
  TraceEOL();

  assert(top_invisible_piece_id==id_uninitialised);
  assert(top_invisible_piece_id>top_visible_piece_id);
  motivation[top_invisible_piece_id] = motivation_null;
  --top_invisible_piece_id;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

void reveal_new(move_effect_journal_entry_type *entry)
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

  entry->u.piece_addition.added.flags = being_solved.spec[on];
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

void unreveal_new(move_effect_journal_entry_type *entry)
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

  entry->u.piece_addition.added.flags = being_solved.spec[on];
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
    case play_finalising_replay:
    case play_replay_validating:
    case play_replay_testing:
      break;

    case play_detecting_revelations:
    case play_testing_mate:
    case play_attacking_mating_piece:
    case play_unwinding:
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

    case play_initialising_replay:
    case play_finalising_replay:
    case play_replay_validating:
    case play_replay_testing:
      break;

    case play_detecting_revelations:
    case play_testing_mate:
    case play_attacking_mating_piece:
    case play_rewinding:
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
          if (pos==capture_by_invisible || pos==move_by_invisible)
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
      taint_history_of_placed_piece((unsigned int)(entry-&move_effect_journal[0]));
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
      untaint_history_of_placed_piece((unsigned int)(entry-&move_effect_journal[0]));
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

static void evaluate_revelations_recursive(slice_index si,
                                           unsigned int nr_potential_revelations);

/* Create a piece revelation effect in the move effects journal
 * @param s place of revelation
 * @param ids identifies the revelation in revelation_status
 */
static void add_revelation_effect(square s,
                                  slice_index si,
                                  unsigned int i)
{
  revelation_status_type const * const status = &revelation_status[i];
  Flags spec = status->spec;

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  TraceWalk(status->walk);
  TraceFunctionParam("%x",status->spec);
  TraceEOL();

  assert(TSTFLAG(spec,Chameleon));
  CLRFLAG(spec,Chameleon);

  if (is_square_empty(s))
  {
    TraceValue("%u",nbply);TraceEOL();
    TraceConsumption();TraceEOL();
    record_decision_outcome("revelation of a hitherto unplaced invisible side:%u walk:%u on:%u",
                            TSTFLAG(spec,White) ? White : Black, status->walk, s);

    {
      PieceIdType const id_revealed = initialise_motivation_from_revelation(status);

      SetPieceId(spec,id_revealed);

      do_revelation_of_new_invisible(move_effect_reason_revelation_of_invisible,
                                     s,status->walk,spec);

      assert(!TSTFLAG(being_solved.spec[s],Chameleon));

      evaluate_revelations_recursive(si,i);

      uninitialise_motivation(id_revealed);
    }
  }
  else
  {
    record_decision_outcome("revelation of a placed invisible side:%u walk:%u on:%u",
                            TSTFLAG(spec,White) ? White : Black, status->walk, s);
    SetPieceId(spec,GetPieceId(being_solved.spec[s]));

    do_revelation_of_placed_invisible(move_effect_reason_revelation_of_invisible,
                                      s,status->walk,spec);

    assert(!TSTFLAG(being_solved.spec[s],Chameleon));

    evaluate_revelations_recursive(si,i);
  }

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

      record_decision_outcome("revelation candidate - side:%u walk:%u on:%u",
                              TSTFLAG(being_solved.spec[s],White) ? White : Black,
                              walk,
                              s);

      assert(motivation[id].first.acts_when<=motivation[id].last.acts_when);
      if (motivation[id].first.acts_when<motivation[id].last.acts_when
          || motivation[id].first.purpose==purpose_capturer
          || motivation[id].first.purpose==purpose_interceptor)
      {
        /* we don't know where this invisible started its career */
        revelation_status[i].first.on = initsquare;
      }
      else if (motivation[id].first.purpose==purpose_castling_partner)
      {
        /* let's apply knowledge */
      }
      else
      {
        /* are there other cases? */
        assert(0);
      }
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

      record_decision_outcome("ruling out revelation candidate - side:%u walk:%u on:%u",
                              TSTFLAG(revelation_status[i].spec,White) ? White : Black,
                              revelation_status[i].walk,
                              revelation_status[i].first_on);

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

static void evaluate_revelations_recursive(slice_index si,
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

    if (revelation_status[i].walk==Empty)
      evaluate_revelations_recursive(si,i);
    else
      add_revelation_effect(s,si,i);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void evaluate_revelations(slice_index si,
                          unsigned int nr_potential_revelations)
{
  dynamic_consumption_type const save_consumption = current_consumption;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];

  evaluate_revelations_recursive(si,nr_potential_revelations);

  current_consumption = save_consumption;

  while (move_effect_journal_base[nbply+1]>top)
    move_effect_journal_pop_effect();
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
  initialise_decision_context();

  static_consumption.king[White] = being_solved.king_square[White]==initsquare;
  static_consumption.king[Black] = being_solved.king_square[Black]==initsquare;

  --nbply;
  forward_prevent_illegal_checks();
  ++nbply;

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

  backtrack_from_revelation_update();

  TraceValue("%u",nr_potential_revelations);TraceEOL();

  {
    unsigned int i;
    for (i = 0; i!=nr_potential_revelations; ++i)
    {
      PieceIdType const id = GetPieceId(revelation_status[i].spec);

      TraceValue("%u",i);
      TraceWalk(revelation_status[i].walk);
      TraceSquare(revelation_status[i].last.on);
      TraceValue("%x",revelation_status[i].spec);
      TraceValue("%u",id);
      TraceEOL();

      backtrack_no_further_than(decision_levels[id].side);
      backtrack_no_further_than(decision_levels[id].walk);
      backtrack_no_further_than(decision_levels[id].to);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_test_and_execute_revelations_recursive(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

  if (curr==move_effect_journal_base[nbply+1])
    forward_conclude_move_just_played();
  else
  {
    move_effect_journal_entry_type * const entry = &move_effect_journal[curr];
    TraceValue("%u",entry->type);TraceEOL();
    switch (entry->type)
    {
      case move_effect_revelation_of_new_invisible:
      {
        square const revealed_on = entry->u.piece_addition.added.on;
        piece_walk_type const walk_revealed = entry->u.piece_addition.added.walk;
        Flags const spec_revealed = entry->u.piece_addition.added.flags;
        Side const side_revealed = TSTFLAG(spec_revealed,White) ? White : Black;
        piece_walk_type const walk_on_board = get_walk_of_piece_on_square(revealed_on);
        Flags const spec_on_board = being_solved.spec[revealed_on];

        TraceSquare(revealed_on);TraceEOL();

        if (is_square_empty(revealed_on))
        {
          /* This can happen because we don't flesh out all invisibles while
           * validating a mate. We make use from the knowledge gained when
           * detecting revelations to prune paths that we know won't lead
           * anywhere.
           */
          assert(play_phase==play_validating_mate);
          TraceText("revelation expected, but square is empty - aborting\n");
          record_decision_outcome("%s","revelation expected, but square is empty - aborting");
          REPORT_DEADEND;
        }
        else if (play_phase==play_validating_mate && walk_on_board==Dummy)
        {
          TraceWalk(walk_revealed);
          TraceValue("%0x",spec_revealed);
          TraceEnumerator(Side,side_revealed);
          TraceSquare(revealed_on);
          TraceEOL();
          TraceWalk(walk_on_board);
          TraceValue("%0x",spec_on_board);
          TraceValue("%u",TSTFLAG(spec_on_board,side_revealed));
          TraceEOL();

          if (TSTFLAG(spec_on_board,side_revealed))
          {
            PieceIdType const id_on_board = GetPieceId(spec_on_board);
            purpose_type const purpose_on_board = motivation[id_on_board].last.purpose;

            PieceIdType const id_revealed = GetPieceId(spec_revealed);
            purpose_type const purpose_revealed = motivation[id_revealed].last.purpose;

            reveal_new(entry);
            motivation[id_on_board].last.purpose = purpose_none;
            motivation[id_revealed].last.purpose = purpose_none;
            forward_test_and_execute_revelations_recursive(curr+1);
            motivation[id_revealed].last.purpose = purpose_revealed;
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
        else if (walk_on_board==walk_revealed
                 && TSTFLAG(spec_on_board,side_revealed))
        {
          PieceIdType const id_on_board = GetPieceId(being_solved.spec[revealed_on]);
          purpose_type const purpose_on_board = motivation[id_on_board].last.purpose;

          PieceIdType const id_revealed = GetPieceId(spec_revealed);
          purpose_type const purpose_revealed = motivation[id_revealed].last.purpose;

          TraceText("treat revelation of new invisible as revelation of placed invisible\n");

          assert(id_on_board!=id_revealed);

          entry->type = move_effect_revelation_of_placed_invisible;
          entry->u.revelation_of_placed_piece.on = revealed_on;
          entry->u.revelation_of_placed_piece.walk_original = walk_on_board;
          entry->u.revelation_of_placed_piece.flags_original = spec_on_board;
          entry->u.revelation_of_placed_piece.walk_revealed = walk_revealed;
          entry->u.revelation_of_placed_piece.flags_revealed = spec_revealed;

          adapt_id_of_existing_to_revealed(entry);

          motivation[id_on_board].last.purpose = purpose_none;
          motivation[id_revealed].last.purpose = purpose_none;

          forward_test_and_execute_revelations_recursive(curr+1);

          motivation[id_revealed].last.purpose = purpose_revealed;
          motivation[id_on_board].last.purpose = purpose_on_board;

          unadapt_id_of_existing_to_revealed(entry);

          entry->type = move_effect_revelation_of_new_invisible;
          entry->u.piece_addition.added.on = revealed_on;
          entry->u.piece_addition.added.walk = walk_revealed;
          entry->u.piece_addition.added.flags = spec_revealed;
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

          PieceIdType const id_original = GetPieceId(entry->u.revelation_of_placed_piece.flags_original);
          purpose_type const purpose_original = motivation[id_original].last.purpose;

          square const on = entry->u.revelation_of_placed_piece.on;
          PieceIdType const id_on_board = GetPieceId(being_solved.spec[on]);

          if (id_revealed==id_on_board)
          {
            reveal_placed(entry);

            assert(id_revealed==id_on_board);

            /* the following distinction isn't strictly necessary, but it clarifies nicely
             * that the two ids may be, but aren't necessarily equal */
            if (id_revealed==id_original)
              forward_test_and_execute_revelations_recursive(curr+1);
            else
            {
              motivation[id_original].last.purpose = purpose_none;
              forward_test_and_execute_revelations_recursive(curr+1);
              motivation[id_original].last.purpose = purpose_original;
            }

            unreveal_placed(entry);
          }
          else
          {
            TraceText("it is unclear what happend here\n");
          }
        }
        else
        {
          TraceText("the revelation has been violated - terminating redoing effects with this ply\n");
          record_decision_outcome("%s","the revelation has been violated - terminating redoing effects with this ply");
          REPORT_DEADEND;
        }
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

void forward_test_and_execute_revelations(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  forward_test_and_execute_revelations_recursive(top_before_revelations[nbply]);

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
