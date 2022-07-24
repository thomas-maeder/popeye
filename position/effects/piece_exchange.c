#include "position/effects/piece_exchange.h"
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

/* Follow the captured or a moved piece through the "other" effects of a move
 * @param followed_id id of the piece to be followed
 * @param idx index of a piece_exchange effect
 * @param pos position of the piece when effect idx is played
 * @return the position of the piece with effect idx applied
 *         initsquare if the piece is not on the board after effect idx
 */
square position_piece_exchange_follow_piece(PieceIdType followed_id,
                                            move_effect_journal_index_type idx,
                                            square pos)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",followed_id);
  TraceFunctionParam("%u",idx);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  if (move_effect_journal[idx].u.piece_exchange.from==pos)
    pos = move_effect_journal[idx].u.piece_exchange.to;
  else if (move_effect_journal[idx].u.piece_exchange.to==pos)
    pos = move_effect_journal[idx].u.piece_exchange.from;

  TraceFunctionExit(__func__);
  TraceSquare(pos);
  TraceFunctionResultEnd();

  return pos;
}

/* Initalise the module */
void position_piece_exchange_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_piece_exchange,
                                       &undo_piece_exchange,
                                       &redo_piece_exchange);
}
