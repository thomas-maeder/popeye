#include "position/piece_exchange.h"
#include "position/position.h"
#include "debugging/assert.h"

/* Add exchanging two pieces to the current move of the current ply
 * @param reason reason for exchanging the two pieces
 * @param from position of primary piece
 * @param to position of partner piece
 */
void move_effect_journal_do_piece_exchange(move_effect_reason_type reason,
                                           square from,
                                           square to)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_piece_exchange,reason);
  piece_walk_type const pi_to = get_walk_of_piece_on_square(to);
  Flags const spec_pi_to = being_solved.spec[to];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  entry->u.piece_exchange.from = from;
  entry->u.piece_exchange.fromflags = being_solved.spec[from];
  entry->u.piece_exchange.to = to;
  entry->u.piece_exchange.toflags = spec_pi_to;

  occupy_square(to,get_walk_of_piece_on_square(from),being_solved.spec[from]);
  occupy_square(from,pi_to,spec_pi_to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_exchange(move_effect_journal_entry_type const *entry)
{
  square const from = entry->u.piece_exchange.from;
  square const to = entry->u.piece_exchange.to;
  piece_walk_type const pi_to = get_walk_of_piece_on_square(to);
  Flags const spec_pi_to = being_solved.spec[to];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  occupy_square(to,get_walk_of_piece_on_square(from),being_solved.spec[from]);
  occupy_square(from,pi_to,spec_pi_to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_exchange(move_effect_journal_entry_type const *entry)
{
  square const from = entry->u.piece_exchange.from;
  square const to = entry->u.piece_exchange.to;
  piece_walk_type const pi_to = get_walk_of_piece_on_square(to);
  Flags const spec_pi_to = being_solved.spec[to];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(from);
  TraceSquare(to);
  TraceEOL();

  occupy_square(to,get_walk_of_piece_on_square(from),being_solved.spec[from]);
  occupy_square(from,pi_to,spec_pi_to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void position_piece_exchange_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_piece_exchange,
                                       &undo_piece_exchange,
                                       &redo_piece_exchange);
}
