#include "position/piece_movement.h"
#include "position/position.h"
#include "debugging/assert.h"


static void push_movement_elmt(move_effect_reason_type reason,
                               square from,
                               square to)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_piece_movement,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  entry->u.piece_movement.moving = get_walk_of_piece_on_square(from);
  entry->u.piece_movement.movingspec = being_solved.spec[from];
  entry->u.piece_movement.from = from;
  entry->u.piece_movement.to = to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_movement(square from, square to)
{
  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  if (to!=from)
  {
    occupy_square(to,get_walk_of_piece_on_square(from),being_solved.spec[from]);
    empty_square(from);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add moving a piece to the current move of the current ply
 * @param reason reason for moving the piece
 * @param from current position of the piece
 * @param to where to move the piece
 */
void move_effect_journal_do_piece_movement(move_effect_reason_type reason,
                                           square from,
                                           square to)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  TraceValue("%u",GetPieceId(being_solved.spec[from]));
  TraceEOL();

  push_movement_elmt(reason,from,to);
  do_movement(from,to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_movement(move_effect_journal_entry_type const *entry)
{
  square const from = entry->u.piece_movement.from;
  square const to = entry->u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(from);TraceSquare(to);TraceWalk(being_solved.board[to]);TraceEOL();

  if (to!=from)
  {
    occupy_square(from,get_walk_of_piece_on_square(to),being_solved.spec[to]);
    empty_square(to);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_movement(move_effect_journal_entry_type const *entry)
{
  square const from = entry->u.piece_movement.from;
  square const to = entry->u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(from);
  TraceSquare(to);
  TraceEOL();

  if (to!=from)
  {
    occupy_square(to,get_walk_of_piece_on_square(from),being_solved.spec[from]);
    empty_square(from);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void position_piece_movement_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_piece_movement,
                                       &undo_piece_movement,
                                       &redo_piece_movement);
}
