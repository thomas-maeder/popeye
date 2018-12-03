#include "position/walk_change.h"
#include "position/position.h"
#include "debugging/assert.h"


static void do_walk_change(square on, piece_walk_type to)
{
  if (TSTFLAG(being_solved.spec[on],White))
    --being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(being_solved.spec[on],Black))
    --being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  replace_walk(on,to);

  if (TSTFLAG(being_solved.spec[on],White))
    ++being_solved.number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(being_solved.spec[on],Black))
    ++being_solved.number_of_pieces[Black][get_walk_of_piece_on_square(on)];
}

/* Add changing the walk of a piece to the current move of the current ply
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 * @param to new nature of piece
 */
void move_effect_journal_do_walk_change(move_effect_reason_type reason,
                                        square on,
                                        piece_walk_type to)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_walk_change,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(to);
  TraceFunctionParamListEnd();

  assert(!is_square_blocked(on));
  assert(!is_square_empty(on));

  entry->u.piece_change.on = on;
  entry->u.piece_change.from = get_walk_of_piece_on_square(on);
  entry->u.piece_change.to = to;

  do_walk_change(on,to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_walk_change(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_change.on;
  piece_walk_type const from = entry->u.piece_change.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do_walk_change(on,from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_walk_change(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_change.on;
  piece_walk_type const to = entry->u.piece_change.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do_walk_change(on,to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void position_walk_change_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_walk_change,
                                       &undo_walk_change,
                                       &redo_walk_change);
}
