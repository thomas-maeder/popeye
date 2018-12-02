#include "position/piece_creation.h"
#include "position/position.h"
#include "debugging/assert.h"


/* Add an newly created piece to the current move of the current ply
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

  entry->u.piece_addition.added.on = on;
  entry->u.piece_addition.added.walk = created;
  entry->u.piece_addition.added.flags = createdspec;
  entry->u.piece_addition.for_side = for_side;

  assert(is_square_empty(on));
  if (TSTFLAG(createdspec,White))
    ++being_solved.number_of_pieces[White][created];
  if (TSTFLAG(createdspec,Black))
    ++being_solved.number_of_pieces[Black][created];
  occupy_square(on,created,createdspec);
  SetPieceId(being_solved.spec[on],++being_solved.currPieceId);

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

  if (TSTFLAG(createdspec,White))
    --being_solved.number_of_pieces[White][created];
  if (TSTFLAG(createdspec,Black))
    --being_solved.number_of_pieces[Black][created];

  assert(GetPieceId(being_solved.spec[on])==being_solved.currPieceId);
  --being_solved.currPieceId;

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

  if (TSTFLAG(createdspec,White))
    ++being_solved.number_of_pieces[White][created];
  if (TSTFLAG(createdspec,Black))
    ++being_solved.number_of_pieces[Black][created];

  assert(is_square_empty(on));
  occupy_square(on,created,createdspec);
  SetPieceId(being_solved.spec[on],++being_solved.currPieceId);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void position_piece_creation_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_piece_creation,
                                       &undo_piece_creation,
                                       &redo_piece_creation);
}
