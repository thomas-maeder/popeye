#include "position/flags_change.h"
#include "position/position.h"
#include "debugging/assert.h"

/* Add changing the flags of a piece to the current move of the current ply
 * @param reason reason for moving the king square
 * @param on position of pieces whose flags to piece_change
 * @param to changed flags
 */
void move_effect_journal_do_flags_change(move_effect_reason_type reason,
                                         square on,
                                         Flags to)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_flags_change,reason);

  TraceFunctionEntry(__func__);
  TraceSquare(on);
  TraceFunctionParamListEnd();

  assert(GetPieceId(being_solved.spec[on])==GetPieceId(to));

  entry->u.flags_change.on = on;
  entry->u.flags_change.from = being_solved.spec[on];
  entry->u.flags_change.to = to;

  if (TSTFLAG(being_solved.spec[on],White))
    --being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(being_solved.spec[on],Black))
    --being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  being_solved.spec[on] = to;

  if (TSTFLAG(being_solved.spec[on],White))
    ++being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(being_solved.spec[on],Black))
    ++being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_flags_change(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.flags_change.on;
  Flags const from = entry->u.flags_change.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(on);
  TraceEOL();

  assert(being_solved.spec[on]==entry->u.flags_change.to);
  assert(GetPieceId(being_solved.spec[on])==GetPieceId(from));

  if (TSTFLAG(being_solved.spec[on],White))
    --being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(being_solved.spec[on],Black))
    --being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  being_solved.spec[on] = from;

  if (TSTFLAG(being_solved.spec[on],White))
    ++being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(being_solved.spec[on],Black))
    ++being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_flags_change(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.flags_change.on;
  Flags const to = entry->u.flags_change.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(being_solved.spec[on]==entry->u.flags_change.from);
  assert(GetPieceId(being_solved.spec[on])==GetPieceId(to));

  if (TSTFLAG(being_solved.spec[on],White))
    --being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(being_solved.spec[on],Black))
    --being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  being_solved.spec[on] = to;

  if (TSTFLAG(being_solved.spec[on],White))
    ++being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(being_solved.spec[on],Black))
    ++being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void position_flags_change_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_flags_change,
                                       &undo_flags_change,
                                       &redo_flags_change);
}
