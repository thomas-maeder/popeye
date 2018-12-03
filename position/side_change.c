#include "position/side_change.h"
#include "position/position.h"
#include "debugging/assert.h"

/* Add changing the side of a piece to the current move of the current ply
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 */
void move_effect_journal_do_side_change(move_effect_reason_type reason, square on)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_side_change,reason);
  Side const to = TSTFLAG(being_solved.spec[on],White) ? Black : White;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceEnumerator(Side,to);
  TraceFunctionParamListEnd();

  assert(!TSTFLAG(being_solved.spec[on],to));

  entry->u.side_change.on = on;
  entry->u.side_change.to = to;

  --being_solved.number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];
  piece_change_side(&being_solved.spec[on]);
  occupy_square(on,get_walk_of_piece_on_square(on),being_solved.spec[on]);
  ++being_solved.number_of_pieces[to][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_side_change(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.side_change.on;
  Side const from = TSTFLAG(being_solved.spec[on],White) ? Black : White;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  --being_solved.number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];
  piece_change_side(&being_solved.spec[on]);
  occupy_square(on,get_walk_of_piece_on_square(on),being_solved.spec[on]);
  ++being_solved.number_of_pieces[from][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_side_change(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.side_change.on;
  Side const to = TSTFLAG(being_solved.spec[on],White) ? Black : White;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  --being_solved.number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];
  piece_change_side(&being_solved.spec[on]);
  occupy_square(on,get_walk_of_piece_on_square(on),being_solved.spec[on]);
  ++being_solved.number_of_pieces[to][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void position_side_change_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_side_change,
                                       &undo_side_change,
                                       &redo_side_change);
}
