#include "position/effects/piece_creation.h"
#include "position/position.h"
#include "debugging/assert.h"


/* Add a newly created piece to the current move of the current ply
 * @param reason reason for creating the piece
 * @param on where to insert the piece
 * @param created nature of created piece
 * @param createdspec specs of created piece
 * @param for which side is the (potentially neutral) piece created
 */
void move_effect_journal_do_piece_creation(move_effect_reason_type reason,
                                           square on,
                                           piece_walk_type created,
                                           Flags createdspec,
                                           Side for_side)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_piece_creation,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(created);
  TraceEnumerator(Side,for_side);
  TraceFunctionParamListEnd();

  SetPieceId(createdspec,++being_solved.currPieceId);

  entry->u.piece_addition.added.on = on;
  entry->u.piece_addition.added.walk = created;
  entry->u.piece_addition.added.flags = createdspec;
  entry->u.piece_addition.for_side = for_side;

  TraceValue("%u",being_solved.currPieceId);
  TraceValue("%u",GetPieceId(being_solved.spec[on]));
  TraceEOL();

  assert(is_square_empty(on));
  if (TSTFLAG(createdspec,White))
    ++being_solved.number_of_pieces[White][created];
  if (TSTFLAG(createdspec,Black))
    ++being_solved.number_of_pieces[Black][created];
  occupy_square(on,created,createdspec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_creation(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const created = entry->u.piece_addition.added.walk;
  Flags const createdspec = entry->u.piece_addition.added.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(on);
  TraceWalk(being_solved.board[on]);
  TraceWalk(created);
  TraceValue("%x",being_solved.spec[on]);
  TraceValue("%x",createdspec);
  TraceValue("%u",GetPieceId(being_solved.spec[on]));
  TraceValue("%u",being_solved.currPieceId);
  TraceEOL();

  assert(being_solved.board[on]==created);
  assert((being_solved.spec[on]&PieSpMask)==(createdspec&PieSpMask));
  assert(GetPieceId(being_solved.spec[on])==being_solved.currPieceId);
  --being_solved.currPieceId;

  TraceValue("%u",being_solved.currPieceId);
  TraceEOL();

  if (TSTFLAG(createdspec,White))
    --being_solved.number_of_pieces[White][created];
  if (TSTFLAG(createdspec,Black))
    --being_solved.number_of_pieces[Black][created];

  empty_square(on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_creation(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const created = entry->u.piece_addition.added.walk;
  Flags const createdspec = entry->u.piece_addition.added.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(on);
  TraceWalk(created);
  TraceValue("%x",createdspec);
  TraceEOL();

  if (TSTFLAG(createdspec,White))
    ++being_solved.number_of_pieces[White][created];
  if (TSTFLAG(createdspec,Black))
    ++being_solved.number_of_pieces[Black][created];

  assert(is_square_empty(on));
  occupy_square(on,created,createdspec);
  SetPieceId(being_solved.spec[on],++being_solved.currPieceId);

  TraceValue("%u",being_solved.currPieceId);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Follow the captured or a moved piece through the "other" effects of a move
 * @param followed_id id of the piece to be followed
 * @param idx index of a piece_creation effect
 * @param pos position of the piece when effect idx is played
 * @return the position of the piece with effect idx applied
 *         initsquare if the piece is not on the board after effect idx
 */
square position_piece_creation_follow_piece(PieceIdType followed_id,
                                           move_effect_journal_index_type idx,
                                           square pos)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",followed_id);
  TraceFunctionParam("%u",idx);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  if (GetPieceId(move_effect_journal[idx].u.piece_addition.added.flags)==followed_id)
  {
    assert(pos==initsquare);
    pos = move_effect_journal[idx].u.piece_addition.added.on;
  }

  TraceFunctionExit(__func__);
  TraceSquare(pos);
  TraceFunctionResultEnd();

  return pos;
}

/* Initalise the module */
void position_piece_creation_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_piece_creation,
                                       &undo_piece_creation,
                                       &redo_piece_creation);
}
