#include "solving/move_effect_journal.h"
#include "pieces/pieces.h"
#include "pieces/walks/pawns/en_passant.h"
#include "position/pieceid.h"
#include "solving/castling.h"
#include "solving/pipe.h"
#include "solving/machinery/twin.h"
#include "stipulation/stipulation.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/modifier.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

move_effect_journal_entry_type move_effect_journal[move_effect_journal_size];

/* starting at 1 simplifies pointer arithmetic in undo_move_effects */
move_effect_journal_index_type move_effect_journal_base[maxply+1] = { 1, 1 };

move_effect_journal_index_type move_effect_journal_index_offset_capture = 0;
move_effect_journal_index_type move_effect_journal_index_offset_movement = 1;
move_effect_journal_index_type move_effect_journal_index_offset_other_effects = 2;

move_effect_journal_index_type king_square_horizon;

/* Reserve space for an effect in each move before the capture (e.g. for
 * Singlebox Type 3 promotions). Conditions that do this have to make sure
 * that every move has such an effect, possibly by adding a null effect to
 * fill the reserved gap.
 */
void move_effect_journal_register_pre_capture_effect(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++move_effect_journal_index_offset_capture;
  ++move_effect_journal_index_offset_movement;
  ++move_effect_journal_index_offset_other_effects;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Reset the move effects journal from pre-capture effect reservations
 */
void move_effect_journal_reset(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_index_offset_capture = 0;
  move_effect_journal_index_offset_movement = 1;
  move_effect_journal_index_offset_other_effects = 2;

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

#if defined(DOTRACE)
unsigned long move_effect_journal_next_id;
#endif

/* Allocate an entry
 * @param type type of the effect
 * @param reason reason of the effect
 * @return address of allocated entry
 * @note terminates the program if the entries are exhausted
 */
move_effect_journal_entry_type *move_effect_journal_allocate_entry(move_effect_type type,
                                                                   move_effect_reason_type reason)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type * const result = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",type);
  TraceFunctionParam("%u",reason);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  result->type = type;
  result->reason = reason;

#if defined(DOTRACE)
  result->id = move_effect_journal_next_id++;
  TraceValue("%lu",result->id);
  TraceEOL();
#endif

  ++move_effect_journal_base[nbply+1];
  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
  return result;
}

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
  assert(GetPieceId(addedspec)!=NullPieceId);

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

  if (TSTFLAG(addedspec,White))
    --being_solved.number_of_pieces[White][added];
  if (TSTFLAG(addedspec,Black))
    --being_solved.number_of_pieces[Black][added];

  empty_square(on);

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

  if (TSTFLAG(addedspec,White))
    ++being_solved.number_of_pieces[White][added];
  if (TSTFLAG(addedspec,Black))
    ++being_solved.number_of_pieces[Black][added];

  assert(is_square_empty(on));
  occupy_square(on,added,addedspec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

/* Fill the capture gap at the head of each move by no capture
 */
void move_effect_journal_do_no_piece_removal(void)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_no_piece_removal,move_effect_no_reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  entry->u.piece_removal.walk = Empty;
  CLEARFL(entry->u.piece_removal.flags);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_piece_change,reason);

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

static void undo_piece_change(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_change.on;
  piece_walk_type const from = entry->u.piece_change.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do_walk_change(on,from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_change(move_effect_journal_entry_type const *entry)
{
  square const on = entry->u.piece_change.on;
  piece_walk_type const to = entry->u.piece_change.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do_walk_change(on,to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

/* Add king square piece_movement to the current move of the current ply
 * @param reason reason for moving the king square
 * @param side whose king square to move
 * @param to where to move the king square
 */
void move_effect_journal_do_king_square_movement(move_effect_reason_type reason,
                                                 Side side,
                                                 square to)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_king_square_movement,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceEnumerator(Side,side);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  entry->u.king_square_movement.side = side;
  entry->u.king_square_movement.from = being_solved.king_square[side];
  entry->u.king_square_movement.to = to;

  being_solved.king_square[side] = to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_king_square_movement(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.king_square_movement.side;
  square const from = entry->u.king_square_movement.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side);
  TraceSquare(from);
  TraceSquare(entry->u.king_square_movement.to);
  TraceEOL();

  assert(being_solved.king_square[side]==entry->u.king_square_movement.to);

  being_solved.king_square[side] = from;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_king_square_movement(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.king_square_movement.side;
  square const to = entry->u.king_square_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side);
  TraceSquare(entry->u.king_square_movement.from);
  TraceSquare(to);
  TraceEOL();

  assert(being_solved.king_square[side]==entry->u.king_square_movement.from);

  being_solved.king_square[side] = to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

static void transformBoard(SquareTransformation transformation)
{
  piece_walk_type t_e[nr_squares_on_board];
  Flags t_spec[nr_squares_on_board];
  imarr t_isquare;
  int i;

  /* save the position to be mirrored/rotated */
  for (i = 0; i<nr_squares_on_board; i++)
  {
    t_e[i] = get_walk_of_piece_on_square(boardnum[i]);
    t_spec[i] = being_solved.spec[boardnum[i]];
  }

  for (i = 0; i<maxinum; i++)
    t_isquare[i] = being_solved.isquare[i];

  /* now rotate/mirror */
  /* pieces */
  for (i = 0; i<nr_squares_on_board; i++)
  {
    square const sq = transformSquare(boardnum[i],transformation);

    switch (t_e[i])
    {
      case Empty:
        empty_square(sq);
        break;

      case Invalid:
        block_square(sq);
        break;

      default:
        occupy_square(sq,t_e[i],t_spec[i]);
        break;
    }
  }

  /* imitators */
  for (i= 0; i<maxinum; i++)
    being_solved.isquare[i]= transformSquare(t_isquare[i], transformation);
}

/* Add transforming the board to the current move of the current ply
 * @param reason reason for moving the king square
 * @param transformation how to transform the board
 */
void move_effect_journal_do_board_transformation(move_effect_reason_type reason,
                                                 SquareTransformation transformation)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_board_transformation,reason);

  TraceFunctionEntry(__func__);
  TraceValue("%u",transformation);
  TraceFunctionParamListEnd();

  entry->u.board_transformation.transformation = transformation;

  transformBoard(transformation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static SquareTransformation const inverse_transformation[] =
{
    rot270,
    rot180,
    rot90,
    mirra1h1,
    mirra1a8,
    mirra1h8,
    mirra8h1
};

static void undo_board_transformation(move_effect_journal_entry_type const *entry)
{
  SquareTransformation const transformation = entry->u.board_transformation.transformation;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  transformBoard(inverse_transformation[transformation]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_board_transformation(move_effect_journal_entry_type const *entry)
{
  SquareTransformation const transformation = entry->u.board_transformation.transformation;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  transformBoard(transformation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add the effects of a capture move to the current move of the current ply
 * @param sq_departure departure square
 * @param sq_arrival arrival square
 * @param sq_capture position of the captured piece
 * @param removal_reason reason for the capture (ep or regular?)
 */
void move_effect_journal_do_capture_move(square sq_departure,
                                         square sq_arrival,
                                         square sq_capture,
                                         move_effect_reason_type removal_reason)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParam("%u",removal_reason);
  TraceFunctionParamListEnd();

  TraceValue("%u",GetPieceId(being_solved.spec[sq_capture]));
  TraceEOL();

  push_removal_elmt(removal_reason,sq_capture);
  do_removal(sq_capture);

  push_movement_elmt(move_effect_reason_moving_piece_movement,sq_departure,sq_arrival);
  do_movement(sq_departure,sq_arrival);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add a null effect to the current move of the current ply
 */
void move_effect_journal_do_null_effect(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_allocate_entry(move_effect_none,move_effect_no_reason);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add the effects of a null move to the current move of the current ply
 */
void move_effect_journal_do_null_move(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_do_no_piece_removal();
  move_effect_journal_do_null_effect(); /* and no piece movement */

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

#include "conditions/actuated_revolving_centre.h"

/* Follow the captured or a moved piece through the "other" effects of a move
 * @param ply ply in which the move was played
 * @param followed_id id of the piece to be followed
 * @param pos position of the piece after the inital capture removal and piece movement have taken place
 * @return the position of the piece with the "other" effect applied
 *         initsquare if the piece is not on the board after the "other" effects
 */
square move_effect_journal_follow_piece_through_other_effects(ply ply,
                                                              PieceIdType followed_id,
                                                              square pos)
{
  move_effect_journal_index_type const base = move_effect_journal_base[ply];
  move_effect_journal_index_type const top = move_effect_journal_base[ply+1];
  move_effect_journal_index_type other;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParam("%x",followed_id);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  for (other = base+move_effect_journal_index_offset_other_effects;
       other<top;
       ++other)
  {
    TraceValue("%u",move_effect_journal[other].type);
    TraceEOL();
    switch (move_effect_journal[other].type)
    {
      case move_effect_piece_removal:
        if (move_effect_journal[other].u.piece_removal.on==pos)
        {
          assert(GetPieceId(move_effect_journal[other].u.piece_removal.flags)==followed_id);
          pos = initsquare;
        }
        break;

      case move_effect_piece_readdition:
      case move_effect_piece_creation:
        if (GetPieceId(move_effect_journal[other].u.piece_addition.added.flags)==followed_id)
        {
          assert(pos==initsquare);
          pos = move_effect_journal[other].u.piece_addition.added.on;
        }
        break;

      case move_effect_piece_movement:
        if (move_effect_journal[other].u.piece_movement.from==pos)
        {
          assert(GetPieceId(move_effect_journal[other].u.piece_movement.movingspec)==followed_id);
          pos = move_effect_journal[other].u.piece_movement.to;
        }
        break;

      case move_effect_piece_exchange:
        if (move_effect_journal[other].u.piece_exchange.from==pos)
          pos = move_effect_journal[other].u.piece_exchange.to;
        else if (move_effect_journal[other].u.piece_exchange.to==pos)
          pos = move_effect_journal[other].u.piece_exchange.from;
        break;

      case move_effect_board_transformation:
        pos = transformSquare(pos,move_effect_journal[other].u.board_transformation.transformation);
        break;

      case move_effect_centre_revolution:
        pos = actuated_revolving_centre_revolve_square(pos);
        break;

      case move_effect_none:
      case move_effect_no_piece_removal:
      case move_effect_piece_change:
      case move_effect_side_change:
      case move_effect_king_square_movement:
      case move_effect_flags_change:
      case move_effect_imitator_addition:
      case move_effect_imitator_movement:
      case move_effect_remember_ghost:
      case move_effect_forget_ghost:
      case move_effect_half_neutral_deneutralisation:
      case move_effect_half_neutral_neutralisation:
      case move_effect_square_block:
      case move_effect_bgl_adjustment:
      case move_effect_strict_sat_adjustment:
      case move_effect_disable_castling_right:
      case move_effect_enable_castling_right:
      case move_effect_remember_ep_capture_potential:
      case move_effect_remember_duellist:
      case move_effect_remember_parachuted:
      case move_effect_remember_volcanic:
      case move_effect_swap_volcanic:
        /* nothing */
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(pos);
  TraceFunctionResultEnd();
  return pos;
}

static struct
{
    move_effect_doer undoer;
    move_effect_doer redoer;
} move_effect_doers[nr_move_effect_types];

static void move_effect_none_do(move_effect_journal_entry_type const *entry)
{
}

void move_effect_journal_init_move_effect_doers(void)
{
  move_effect_type t;

  for (t = 0; t!=nr_move_effect_types; ++t)
  {
    move_effect_doers[t].undoer = &move_effect_none_do;
    move_effect_doers[t].redoer = &move_effect_none_do;
  }

  move_effect_doers[move_effect_board_transformation].redoer = &redo_board_transformation;
  move_effect_doers[move_effect_board_transformation].undoer = &undo_board_transformation;
  move_effect_doers[move_effect_disable_castling_right].redoer = &move_effect_journal_redo_disabling_castling_right;
  move_effect_doers[move_effect_disable_castling_right].undoer = &move_effect_journal_undo_disabling_castling_right;
  move_effect_doers[move_effect_enable_castling_right].redoer = &move_effect_journal_redo_enabling_castling_right;
  move_effect_doers[move_effect_enable_castling_right].undoer = &move_effect_journal_undo_enabling_castling_right;
  move_effect_doers[move_effect_flags_change].redoer = &redo_flags_change;
  move_effect_doers[move_effect_flags_change].undoer = &undo_flags_change;
  move_effect_doers[move_effect_king_square_movement].redoer = &redo_king_square_movement;
  move_effect_doers[move_effect_king_square_movement].undoer = &undo_king_square_movement;
  move_effect_doers[move_effect_piece_change].redoer = &redo_piece_change;
  move_effect_doers[move_effect_piece_change].undoer = &undo_piece_change;
  move_effect_doers[move_effect_piece_creation].redoer = &redo_piece_creation;
  move_effect_doers[move_effect_piece_creation].undoer = &undo_piece_creation;
  move_effect_doers[move_effect_piece_exchange].redoer = &redo_piece_exchange;
  move_effect_doers[move_effect_piece_exchange].undoer = &undo_piece_exchange;
  move_effect_doers[move_effect_piece_movement].redoer = &redo_piece_movement;
  move_effect_doers[move_effect_piece_movement].undoer = &undo_piece_movement;
  move_effect_doers[move_effect_piece_readdition].redoer = &redo_piece_readdition;
  move_effect_doers[move_effect_piece_readdition].undoer = &undo_piece_readdition;
  move_effect_doers[move_effect_piece_removal].redoer = &redo_piece_removal;
  move_effect_doers[move_effect_piece_removal].undoer = &undo_piece_removal;
  move_effect_doers[move_effect_remember_ep_capture_potential].redoer = &move_effect_journal_redo_remember_ep;
  move_effect_doers[move_effect_remember_ep_capture_potential].undoer = &move_effect_journal_undo_remember_ep;
  move_effect_doers[move_effect_side_change].redoer = &redo_side_change;
  move_effect_doers[move_effect_side_change].undoer = &undo_side_change;
}

void move_effect_journal_set_effect_doers(move_effect_type type,
                                          move_effect_doer undoer,
                                          move_effect_doer redoer)
{
  move_effect_doers[type].undoer = undoer;
  move_effect_doers[type].redoer = redoer;
}

/* Redo the effects of the current move in ply nbply
 */
void redo_move_effects(void)
{
  move_effect_journal_index_type const parent_top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type const *top_entry = &move_effect_journal[top];
  move_effect_journal_entry_type const *entry;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(parent_top<=top);

  for (entry = &move_effect_journal[parent_top]; entry!=top_entry; ++entry)
  {
#if defined(DOTRACE)
    TraceValue("%u",entry->type);
    TraceEOL();
    TraceValue("%lu",entry->id);
    TraceEOL();
#endif

    assert(move_effect_doers[entry->type].redoer!=0);
    (*move_effect_doers[entry->type].redoer)(entry);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo the effects of the current move in ply nbply
 */
void undo_move_effects(void)
{
  move_effect_journal_index_type const parent_top = move_effect_journal_base[nbply];
  move_effect_journal_entry_type const *parent_top_entry = &move_effect_journal[parent_top-1];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type const *entry;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(parent_top>0);
  assert(top>=parent_top);

  for (entry = &move_effect_journal[top-1]; entry!=parent_top_entry; --entry)
  {
#if defined(DOTRACE)
    TraceValue("%u",entry->type);
    TraceEOL();
    TraceValue("%lu",entry->id);
    TraceEOL();
#endif


    assert(move_effect_doers[entry->type].undoer!=0);
    (*move_effect_doers[entry->type].undoer)(entry);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void move_effect_journal_undoer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_effect_journal_base[nbply+1] = move_effect_journal_base[nbply];
  pipe_solve_delegate(si);

  undo_move_effects();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine the departure square of a moveplayed
 * Assumes that the move has a single moving piece (i.e. is not a castling).
 * @param ply identifies the ply where the move is being or was played
 * @return the departure square; initsquare if the last move didn't have a movement
 */
square move_effect_journal_get_departure_square(ply ply)
{
  move_effect_journal_index_type const base = move_effect_journal_base[ply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  PieceIdType const id_moving = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
  move_effect_journal_index_type curr;
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParamListEnd();

  /* this works even if there are early piece movements such as in MarsCirce */
  for (curr = base; curr<movement; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_movement
        && id_moving==GetPieceId(move_effect_journal[curr].u.piece_movement.movingspec))
      break;

  if (move_effect_journal[curr].type==move_effect_piece_movement)
    result = move_effect_journal[curr].u.piece_movement.from;

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}
