#include "position/effects/piece_readdition.h"
#include "position/position.h"
#include "debugging/assert.h"

/* Readd an already existing piece to the current move of the current ply
 * @param reason reason for adding the piece
 * @param on where to insert the piece
 * @param added nature of added piece
 * @param addedspec specs of added piece
 * @param for_side for which side is the (potientally neutral) piece re-added
 */
void move_effect_journal_do_piece_readdition(move_effect_reason_type reason,
                                             square on,
                                             piece_walk_type added,
                                             Flags addedspec,
                                             Side for_side)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_piece_readdition,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(added);
  TraceEnumerator(Side,for_side);
  TraceFunctionParamListEnd();

  assert(for_side==no_side || (addedspec&BIT(for_side))!=0);

  entry->u.piece_addition.added.on = on;
  entry->u.piece_addition.added.walk = added;
  entry->u.piece_addition.added.flags = addedspec;
  entry->u.piece_addition.for_side = for_side;

  assert(is_square_empty(on));
  if (TSTFLAG(addedspec,White))
    ++being_solved.number_of_pieces[White][added];
  if (TSTFLAG(addedspec,Black))
    ++being_solved.number_of_pieces[Black][added];
  occupy_square(on,added,addedspec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_readdition(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const added = entry->u.piece_addition.added.walk;
  Flags const addedspec = entry->u.piece_addition.added.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceWalk(added);
  TraceSquare(on);
  TraceEOL();

  if (on==initsquare)
    TraceText("disabled\n");
  else
  {
    if (TSTFLAG(addedspec,White))
      --being_solved.number_of_pieces[White][added];
    if (TSTFLAG(addedspec,Black))
      --being_solved.number_of_pieces[Black][added];

    empty_square(on);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_readdition(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_addition.added.on;
  piece_walk_type const added = entry->u.piece_addition.added.walk;
  Flags const addedspec = entry->u.piece_addition.added.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceWalk(added);
  TraceSquare(on);
  TraceEOL();

  if (on==initsquare)
    TraceText("disabled\n");
  else
  {
    if (TSTFLAG(addedspec,White))
      ++being_solved.number_of_pieces[White][added];
    if (TSTFLAG(addedspec,Black))
      ++being_solved.number_of_pieces[Black][added];

    assert(is_square_empty(on));
    occupy_square(on,added,addedspec);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void position_piece_readdition_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_piece_readdition,
                                       &undo_piece_readdition,
                                       &redo_piece_readdition);
}
