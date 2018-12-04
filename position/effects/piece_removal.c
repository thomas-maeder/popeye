#include "position/effects/piece_removal.h"
#include "position/position.h"
#include "debugging/assert.h"

static void push_removal_elmt(move_effect_reason_type reason, square from)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_piece_removal,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);TraceSquare(from);TraceWalk(being_solved.board[from]);TraceEOL();

  entry->u.piece_removal.on = from;
  entry->u.piece_removal.walk = get_walk_of_piece_on_square(from);
  entry->u.piece_removal.flags = being_solved.spec[from];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_removal(square from)
{
  piece_walk_type const removed = get_walk_of_piece_on_square(from);
  Flags const removedspec = being_solved.spec[from];

  assert(!is_square_empty(from));

  if (TSTFLAG(removedspec,White))
    --being_solved.number_of_pieces[White][removed];
  if (TSTFLAG(removedspec,Black))
    --being_solved.number_of_pieces[Black][removed];

  empty_square(from);
}

/* Add removing a piece to the current move of the current ply
 * @param reason reason for removing the piece
 * @param from current position of the piece
 * @note use move_effect_journal_do_capture_move(), not
 * move_effect_journal_do_piece_removal() for regular captures
 */
void move_effect_journal_do_piece_removal(move_effect_reason_type reason,
                                          square from)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  TraceValue("%u",GetPieceId(being_solved.spec[from]));
  TraceEOL();

  push_removal_elmt(reason,from);
  do_removal(from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_removal(move_effect_journal_entry_type const *entry)
{
  square const from = entry->u.piece_removal.on;
  piece_walk_type const removed = entry->u.piece_removal.walk;
  Flags const removedspec = entry->u.piece_removal.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  occupy_square(from,removed,removedspec);

  if (TSTFLAG(removedspec,White))
    ++being_solved.number_of_pieces[White][removed];
  if (TSTFLAG(removedspec,Black))
    ++being_solved.number_of_pieces[Black][removed];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_removal(move_effect_journal_entry_type const *entry)
{
  square const from = entry->u.piece_removal.on;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);TraceSquare(from);TraceWalk(entry->u.piece_removal.walk);TraceEOL();

  do_removal(from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Fill the capture gap at the head of each move by no capture
 */
void move_effect_journal_do_no_piece_removal(void)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_no_piece_removal,
                                                                                    move_effect_no_reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  entry->u.piece_removal.walk = Empty;
  CLEARFL(entry->u.piece_removal.flags);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void position_piece_removal_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_piece_removal,
                                       &undo_piece_removal,
                                       &redo_piece_removal);
}
