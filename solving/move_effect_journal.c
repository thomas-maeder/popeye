#include "solving/move_effect_journal.h"
#include "pieces/pieces.h"
#include "solving/castling.h"
#include "stipulation/stipulation.h"
#include "conditions/bgl.h"
#include "conditions/duellists.h"
#include "conditions/imitator.h"
#include "conditions/actuated_revolving_centre.h"
#include "conditions/haunted_chess.h"
#include "conditions/sat.h"
#include "conditions/circe/parachute.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/attributes/neutral/half.h"
#include "position/pieceid.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

move_effect_journal_entry_type move_effect_journal[move_effect_journal_size];

move_effect_journal_index_type move_effect_journal_base[maxply+1];

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
void move_effect_journal_reset(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_index_offset_capture = 0;
  move_effect_journal_index_offset_movement = 1;
  move_effect_journal_index_offset_other_effects = 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

#if defined(DOTRACE)
unsigned long move_effect_journal_next_id;
#endif

static void push_movement_elmt(move_effect_reason_type reason,
                               square from,
                               square to)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_piece_movement;
  top_elmt->reason = reason;
  top_elmt->u.piece_movement.moving = get_walk_of_piece_on_square(from);
  top_elmt->u.piece_movement.movingspec = spec[from];
  top_elmt->u.piece_movement.from = from;
  top_elmt->u.piece_movement.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];
  TraceValue("%u\n",move_effect_journal_base[nbply+1]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_movement(square from, square to)
{
  if (to!=from)
  {
    occupy_square(to,get_walk_of_piece_on_square(from),spec[from]);
    empty_square(from);
  }
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

  TraceValue("%u\n",GetPieceId(spec[from]));

  push_movement_elmt(reason,from,to);
  do_movement(from,to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_movement(move_effect_journal_index_type curr)
{
  square const from = move_effect_journal[curr].u.piece_movement.from;
  square const to = move_effect_journal[curr].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  TraceSquare(from);TraceSquare(to);TraceWalk(e[to]);TraceEOL();

  if (to!=from)
  {
    occupy_square(from,get_walk_of_piece_on_square(to),spec[to]);
    empty_square(to);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_movement(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const from = curr_elmt->u.piece_movement.from;
  square const to = curr_elmt->u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif
  TraceSquare(from);
  TraceSquare(to);
  TraceEOL();

  if (to!=from)
  {
    occupy_square(to,get_walk_of_piece_on_square(from),spec[from]);
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
 */
void move_effect_journal_do_piece_readdition(move_effect_reason_type reason,
                                             square on,
                                             piece_walk_type added,
                                             Flags addedspec)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(added);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_piece_readdition;
  top_elmt->reason = reason;
  top_elmt->u.piece_addition.on = on;
  top_elmt->u.piece_addition.walk = added;
  top_elmt->u.piece_addition.flags = addedspec;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  assert(is_square_empty(on));
  if (TSTFLAG(addedspec,White))
    ++number_of_pieces[White][added];
  if (TSTFLAG(addedspec,Black))
    ++number_of_pieces[Black][added];
  occupy_square(on,added,addedspec);
  assert(GetPieceId(addedspec)!=NullPieceId);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_readdition(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const on = curr_elmt->u.piece_addition.on;
  piece_walk_type const added = curr_elmt->u.piece_addition.walk;
  Flags const addedspec = curr_elmt->u.piece_addition.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  if (TSTFLAG(addedspec,White))
    --number_of_pieces[White][added];
  if (TSTFLAG(addedspec,Black))
    --number_of_pieces[Black][added];

  empty_square(on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_readdition(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const on = curr_elmt->u.piece_addition.on;
  piece_walk_type const added = curr_elmt->u.piece_addition.walk;
  Flags const addedspec = curr_elmt->u.piece_addition.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  if (TSTFLAG(addedspec,White))
    ++number_of_pieces[White][added];
  if (TSTFLAG(addedspec,Black))
    ++number_of_pieces[Black][added];

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
 */
void move_effect_journal_do_piece_creation(move_effect_reason_type reason,
                                           square on,
                                           piece_walk_type created,
                                           Flags createdspec)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(created);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_piece_creation;
  top_elmt->reason = reason;
  top_elmt->u.piece_addition.on = on;
  top_elmt->u.piece_addition.walk = created;
  top_elmt->u.piece_addition.flags = createdspec;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  assert(is_square_empty(on));
  if (TSTFLAG(createdspec,White))
    ++number_of_pieces[White][created];
  if (TSTFLAG(createdspec,Black))
    ++number_of_pieces[Black][created];
  occupy_square(on,created,createdspec);
  SetPieceId(spec[on],currPieceId++);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_creation(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const on = curr_elmt->u.piece_addition.on;
  piece_walk_type const created = curr_elmt->u.piece_addition.walk;
  Flags const createdspec = curr_elmt->u.piece_addition.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  if (TSTFLAG(createdspec,White))
    --number_of_pieces[White][created];
  if (TSTFLAG(createdspec,Black))
    --number_of_pieces[Black][created];

  --currPieceId;
  assert(GetPieceId(spec[on])==currPieceId);

  empty_square(on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_creation(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const on = curr_elmt->u.piece_addition.on;
  piece_walk_type const created = curr_elmt->u.piece_addition.walk;
  Flags const createdspec = curr_elmt->u.piece_addition.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  if (TSTFLAG(createdspec,White))
    ++number_of_pieces[White][created];
  if (TSTFLAG(createdspec,Black))
    ++number_of_pieces[Black][created];

  assert(is_square_empty(on));
  occupy_square(on,created,createdspec);
  SetPieceId(spec[on],currPieceId++);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Fill the capture gap at the head of each move by no capture
 */
void move_effect_journal_do_no_piece_removal(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_no_piece_removal;
  top_elmt->reason = move_effect_no_reason;
  top_elmt->u.piece_removal.walk = Empty;
  CLEARFL(top_elmt->u.piece_removal.flags);
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];
  TraceValue("%u\n",move_effect_journal_base[nbply+1]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void push_removal_elmt(move_effect_reason_type reason, square from)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  TraceValue("%u",top);TraceText("removal");TraceValue("%u",nbply);TraceSquare(from);TraceWalk(e[from]);TraceEOL();

  top_elmt->type = move_effect_piece_removal;
  top_elmt->reason = reason;
  top_elmt->u.piece_removal.on = from;
  top_elmt->u.piece_removal.walk = get_walk_of_piece_on_square(from);
  top_elmt->u.piece_removal.flags = spec[from];
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_removal(square from)
{
  piece_walk_type const removed = get_walk_of_piece_on_square(from);
  Flags const removedspec = spec[from];

  assert(!is_square_empty(from));

  if (TSTFLAG(removedspec,White))
    --number_of_pieces[White][removed];
  if (TSTFLAG(removedspec,Black))
    --number_of_pieces[Black][removed];

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

  TraceValue("%u\n",GetPieceId(spec[from]));

  push_removal_elmt(reason,from);
  do_removal(from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_removal(move_effect_journal_index_type curr)
{
  square const from = move_effect_journal[curr].u.piece_removal.on;
  piece_walk_type const removed = move_effect_journal[curr].u.piece_removal.walk;
  Flags const removedspec = move_effect_journal[curr].u.piece_removal.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  occupy_square(from,removed,removedspec);

  if (TSTFLAG(removedspec,White))
    ++number_of_pieces[White][removed];
  if (TSTFLAG(removedspec,Black))
    ++number_of_pieces[Black][removed];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_removal(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const from = curr_elmt->u.piece_removal.on;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif
  TraceValue("%u",curr);TraceText("removal");TraceValue("%u",nbply);TraceSquare(from);TraceWalk(curr_elmt->u.piece_removal.walk);TraceEOL();

  do_removal(from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceWalk(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);
  assert(!is_square_blocked(on));
  assert(!is_square_empty(on));

  top_elmt->type = move_effect_piece_change;
  top_elmt->reason = reason;
  top_elmt->u.piece_change.on = on;
  top_elmt->u.piece_change.from = get_walk_of_piece_on_square(on);
  top_elmt->u.piece_change.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  if (TSTFLAG(spec[on],White))
    --number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    --number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  replace_walk(on,to);

  if (TSTFLAG(spec[on],White))
    ++number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    ++number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_change(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.piece_change.on;
  piece_walk_type const from = move_effect_journal[curr].u.piece_change.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  if (TSTFLAG(spec[on],White))
    --number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    --number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  replace_walk(on,from);

  if (TSTFLAG(spec[on],White))
    ++number_of_pieces[White][from];
  if (TSTFLAG(spec[on],Black))
    ++number_of_pieces[Black][from];

  TraceValue("%d",GetPieceId(spec[on]));
  TraceValue("%d",TSTFLAG(spec[on],White));
  TraceValue("%d",TSTFLAG(spec[on],Black));
  TraceValue("%d",is_piece_neutral(spec[on]));
  TraceValue("%d",TSTFLAG(spec[on],HalfNeutral));
  TraceValue("%d\n",e[on]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_change(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.piece_change.on;
  piece_walk_type const to = move_effect_journal[curr].u.piece_change.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  if (TSTFLAG(spec[on],White))
    --number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    --number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  replace_walk(on,to);

  if (TSTFLAG(spec[on],White))
    ++number_of_pieces[White][to];
  if (TSTFLAG(spec[on],Black))
    ++number_of_pieces[Black][to];

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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];
  piece_walk_type const pi_to = get_walk_of_piece_on_square(to);
  Flags const spec_pi_to = spec[to];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_piece_exchange;
  top_elmt->reason = reason;
  top_elmt->u.piece_exchange.from = from;
  top_elmt->u.piece_exchange.fromflags = spec[from];
  top_elmt->u.piece_exchange.to = to;
  top_elmt->u.piece_exchange.toflags = spec_pi_to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  occupy_square(to,get_walk_of_piece_on_square(from),spec[from]);
  occupy_square(from,pi_to,spec_pi_to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_exchange(move_effect_journal_index_type curr)
{
  square const from = move_effect_journal[curr].u.piece_exchange.from;
  square const to = move_effect_journal[curr].u.piece_exchange.to;
  piece_walk_type const pi_to = get_walk_of_piece_on_square(to);
  Flags const spec_pi_to = spec[to];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  occupy_square(to,get_walk_of_piece_on_square(from),spec[from]);
  occupy_square(from,pi_to,spec_pi_to);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_exchange(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const from = curr_elmt->u.piece_exchange.from;
  square const to = curr_elmt->u.piece_exchange.to;
  piece_walk_type const pi_to = get_walk_of_piece_on_square(to);
  Flags const spec_pi_to = spec[to];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif
  TraceSquare(from);
  TraceSquare(to);
  TraceEOL();

  occupy_square(to,get_walk_of_piece_on_square(from),spec[from]);
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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];
  Side const to = TSTFLAG(spec[on],White) ? Black : White;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceEnumerator(Side,to,"");
  TraceFunctionParamListEnd();

  assert(!is_piece_neutral(spec[on]));

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_side_change;
  top_elmt->reason = reason;
  top_elmt->u.side_change.on = on;
  top_elmt->u.side_change.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  --number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];
  piece_change_side(&spec[on]);
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]);
  ++number_of_pieces[to][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_side_change(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.side_change.on;
  Side const from = TSTFLAG(spec[on],White) ? Black : White;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  --number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];
  piece_change_side(&spec[on]);
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]);
  ++number_of_pieces[from][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_side_change(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.side_change.on;
  Side const to = TSTFLAG(spec[on],White) ? Black : White;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  --number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];
  piece_change_side(&spec[on]);
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]);
  ++number_of_pieces[to][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Complete blocking of a square
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 */
void move_effect_journal_do_square_block(move_effect_reason_type reason,
                                         square square)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(square);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_square_block;
  top_elmt->reason = reason;
  top_elmt->u.square_block.square = square;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  block_square(square);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_square_block(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.square_block.square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  empty_square(on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_square_block(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.square_block.square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  block_square(on);

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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceEnumerator(Side,side,"");
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_king_square_movement;
  top_elmt->reason = reason;
  top_elmt->u.king_square_movement.side = side;
  top_elmt->u.king_square_movement.from = king_square[side];
  top_elmt->u.king_square_movement.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  king_square[side] = to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_king_square_movement(move_effect_journal_index_type curr)
{
  Side const side = move_effect_journal[curr].u.king_square_movement.side;
  square const from = move_effect_journal[curr].u.king_square_movement.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  TraceEnumerator(Side,side,"");
  TraceSquare(from);
  TraceSquare(move_effect_journal[curr].u.king_square_movement.to);
  TraceEOL();

  assert(king_square[side]==move_effect_journal[curr].u.king_square_movement.to);

  king_square[side] = from;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_king_square_movement(move_effect_journal_index_type curr)
{
  Side const side = move_effect_journal[curr].u.king_square_movement.side;
  square const to = move_effect_journal[curr].u.king_square_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  TraceEnumerator(Side,side,"");
  TraceSquare(move_effect_journal[curr].u.king_square_movement.from);
  TraceSquare(to);
  TraceEOL();

  assert(king_square[side]==move_effect_journal[curr].u.king_square_movement.from);

  king_square[side] = to;

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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceSquare(on);
  TraceFunctionParamListEnd();

  assert(GetPieceId(spec[on])==GetPieceId(to));

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_flags_change;
  top_elmt->reason = reason;
  top_elmt->u.flags_change.on = on;
  top_elmt->u.flags_change.from = spec[on];
  top_elmt->u.flags_change.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  if (TSTFLAG(spec[on],White))
    --number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    --number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  spec[on] = to;

  if (TSTFLAG(spec[on],White))
    ++number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    ++number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_flags_change(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.flags_change.on;
  Flags const from = move_effect_journal[curr].u.flags_change.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  TraceSquare(on);TraceEOL();

  assert(spec[on]==move_effect_journal[curr].u.flags_change.to);
  assert(GetPieceId(spec[on])==GetPieceId(from));

  if (TSTFLAG(spec[on],White))
    --number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    --number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  spec[on] = from;

  if (TSTFLAG(spec[on],White))
    ++number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    ++number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_flags_change(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.flags_change.on;
  Flags const to = move_effect_journal[curr].u.flags_change.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  assert(spec[on]==move_effect_journal[curr].u.flags_change.from);
  assert(GetPieceId(spec[on])==GetPieceId(to));

  if (TSTFLAG(spec[on],White))
    --number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    --number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  spec[on] = to;

  if (TSTFLAG(spec[on],White))
    ++number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    ++number_of_pieces[Black][get_walk_of_piece_on_square(on)];

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
    t_spec[i] = spec[boardnum[i]];
  }

  for (i = 0; i<maxinum; i++)
    t_isquare[i] = isquare[i];

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
    isquare[i]= transformSquare(t_isquare[i], transformation);
}

/* Add transforming the board to the current move of the current ply
 * @param reason reason for moving the king square
 * @param transformation how to transform the board
 */
void move_effect_journal_do_board_transformation(move_effect_reason_type reason,
                                                 SquareTransformation transformation)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceValue("%u",transformation);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_board_transformation;
  top_elmt->reason = reason;
  top_elmt->u.board_transformation.transformation = transformation;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

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

static void undo_board_transformation(move_effect_journal_index_type curr)
{
  SquareTransformation const transformation = move_effect_journal[curr].u.board_transformation.transformation;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  transformBoard(inverse_transformation[transformation]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_board_transformation(move_effect_journal_index_type curr)
{
  SquareTransformation const transformation = move_effect_journal[curr].u.board_transformation.transformation;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

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

  TraceValue("%u\n",GetPieceId(spec[sq_capture]));

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
  move_effect_journal_index_type top = move_effect_journal_base[nbply+1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(top+1<move_effect_journal_size);

  move_effect_journal[top].type = move_effect_none;
  move_effect_journal[top].reason = move_effect_no_reason;

  ++move_effect_journal_base[nbply+1];

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
    TraceValue("%u\n",move_effect_journal[other].type);
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
        if (GetPieceId(move_effect_journal[other].u.piece_addition.flags)==followed_id)
        {
          assert(pos==initsquare);
          pos = move_effect_journal[other].u.piece_addition.on;
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

/* Redo the effects of the current move in ply nbply
 */
void redo_move_effects(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply]<=top);

  for (curr = move_effect_journal_base[nbply]; curr!=top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_none:
        /* nothing */
        break;

      case move_effect_piece_movement:
        redo_piece_movement(curr);
        break;

      case move_effect_piece_readdition:
        redo_piece_readdition(curr);
        break;

      case move_effect_piece_creation:
        redo_piece_creation(curr);
        break;

      case move_effect_no_piece_removal:
        break;

      case move_effect_piece_removal:
        redo_piece_removal(curr);
        break;

      case move_effect_piece_change:
        redo_piece_change(curr);
        break;

      case move_effect_piece_exchange:
        redo_piece_exchange(curr);
        break;

      case move_effect_side_change:
        redo_side_change(curr);
        break;

      case move_effect_king_square_movement:
        redo_king_square_movement(curr);
        break;

      case move_effect_flags_change:
        redo_flags_change(curr);
        break;

      case move_effect_board_transformation:
        redo_board_transformation(curr);
        break;

      case move_effect_centre_revolution:
        redo_centre_revolution(curr);
        break;

      case move_effect_imitator_addition:
        redo_imitator_addition(curr);
        break;

      case move_effect_imitator_movement:
        redo_imitator_movement(curr);
        break;

      case move_effect_remember_ghost:
        move_effect_journal_redo_remember_ghost(curr);
        break;

      case move_effect_forget_ghost:
        move_effect_journal_redo_forget_ghost(curr);
        break;

      case move_effect_half_neutral_deneutralisation:
        redo_half_neutral_deneutralisation(curr);
        break;

      case move_effect_half_neutral_neutralisation:
        redo_half_neutral_neutralisation(curr);
        break;

      case move_effect_square_block:
        redo_square_block(curr);
        break;

      case move_effect_bgl_adjustment:
        move_effect_journal_redo_bgl_adjustment(curr);
        break;

      case move_effect_strict_sat_adjustment:
        move_effect_journal_redo_strict_sat_adjustment(curr);
        break;

      case move_effect_disable_castling_right:
        move_effect_journal_redo_disabling_castling_right(curr);
        break;

      case move_effect_enable_castling_right:
        move_effect_journal_redo_enabling_castling_right(curr);
        break;

      case move_effect_remember_ep_capture_potential:
        move_effect_journal_redo_remember_ep(curr);
        break;

      case move_effect_remember_duellist:
        move_effect_journal_redo_remember_duellist(curr);
        break;

      case move_effect_remember_parachuted:
        move_effect_journal_redo_circe_parachute_remember(curr);
        break;

      case move_effect_remember_volcanic:
        move_effect_journal_redo_circe_volcanic_remember(curr);
        break;

      default:
        assert(0);
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo the effects of the current move in ply nbply
 */
void undo_move_effects(void)
{
  move_effect_journal_index_type const parent_top = move_effect_journal_base[nbply];
  move_effect_journal_index_type top;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]>=parent_top);

  for (top = move_effect_journal_base[nbply+1]; top!=parent_top; --top)
  {
    switch (move_effect_journal[top-1].type)
    {
      case move_effect_none:
        /* nothing */
        break;

      case move_effect_piece_movement:
        undo_piece_movement(top-1);
        break;

      case move_effect_piece_readdition:
        undo_piece_readdition(top-1);
        break;

      case move_effect_piece_creation:
        undo_piece_creation(top-1);
        break;

      case move_effect_no_piece_removal:
        break;

      case move_effect_piece_removal:
        undo_piece_removal(top-1);
        break;

      case move_effect_piece_change:
        undo_piece_change(top-1);
        break;

      case move_effect_piece_exchange:
        undo_piece_exchange(top-1);
        break;

      case move_effect_side_change:
        undo_side_change(top-1);
        break;

      case move_effect_king_square_movement:
        undo_king_square_movement(top-1);
        break;

      case move_effect_flags_change:
        undo_flags_change(top-1);
        break;

      case move_effect_board_transformation:
        undo_board_transformation(top-1);
        break;

      case move_effect_centre_revolution:
        undo_centre_revolution(top-1);
        break;

      case move_effect_imitator_addition:
        undo_imitator_addition(top-1);
        break;

      case move_effect_imitator_movement:
        undo_imitator_movement(top-1);
        break;

      case move_effect_remember_ghost:
        move_effect_journal_undo_remember_ghost(top-1);
        break;

      case move_effect_forget_ghost:
        move_effect_journal_undo_forget_ghost(top-1);
        break;

      case move_effect_half_neutral_deneutralisation:
        undo_half_neutral_deneutralisation(top-1);
        break;

      case move_effect_half_neutral_neutralisation:
        undo_half_neutral_neutralisation(top-1);
        break;

      case move_effect_square_block:
        undo_square_block(top-1);
        break;

      case move_effect_bgl_adjustment:
        move_effect_journal_undo_bgl_adjustment(top-1);
        break;

      case move_effect_strict_sat_adjustment:
        move_effect_journal_undo_strict_sat_adjustment(top-1);
        break;

      case move_effect_disable_castling_right:
        move_effect_journal_undo_disabling_castling_right(top-1);
        break;

      case move_effect_enable_castling_right:
        move_effect_journal_undo_enabling_castling_right(top-1);
        break;

      case move_effect_remember_ep_capture_potential:
        move_effect_journal_undo_remember_ep(top-1);
        break;

      case move_effect_remember_duellist:
        move_effect_journal_undo_remember_duellist(top-1);
        break;

      case move_effect_remember_parachuted:
        move_effect_journal_undo_circe_parachute_remember(top-1);
        break;

      case move_effect_remember_volcanic:
        move_effect_journal_undo_circe_volcanic_remember(top-1);
        break;

      default:
        assert(0);
        break;
    }
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

/* Update the king squares according to the effects since king_square_horizon
 * @note Updates king_square_horizon; solvers invoking this function should
 *       reset king_square_horizon to its previous value before returning
 */
void update_king_squares(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  square new_king_square[nr_sides] = { king_square[0], king_square[1] };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (; king_square_horizon<top; ++king_square_horizon)
    switch (move_effect_journal[king_square_horizon].type)
    {
      case move_effect_piece_removal:
      {
        Flags const spec_captured = move_effect_journal[king_square_horizon].u.piece_removal.flags;
        if (TSTFLAG(spec_captured,Royal))
        {
          if (TSTFLAG(spec_captured,White))
            new_king_square[White] = initsquare;
          if (TSTFLAG(spec_captured,Black))
            new_king_square[Black] = initsquare;
        }
        break;
      }

      case move_effect_piece_readdition:
      case move_effect_piece_creation:
      {
        Flags const addedspec = move_effect_journal[king_square_horizon].u.piece_addition.flags;
        TraceValue("%u",TSTFLAG(addedspec,Royal));
        TraceValue("%u",TSTFLAG(addedspec,White));
        TraceValue("%u\n",TSTFLAG(addedspec,Black));
        if (TSTFLAG(addedspec,Royal))
        {
          square const on = move_effect_journal[king_square_horizon].u.piece_addition.on;
          if (new_king_square[White]==initsquare && TSTFLAG(addedspec,White))
            new_king_square[White] = on;
          if (new_king_square[Black]==initsquare && TSTFLAG(addedspec,Black))
            new_king_square[Black] = on;
        }
        break;
      }

      case move_effect_piece_movement:
      {
        square const from = move_effect_journal[king_square_horizon].u.piece_movement.from;
        square const to = move_effect_journal[king_square_horizon].u.piece_movement.to;
        if (to!=from)
        {
          Flags const movingspec = move_effect_journal[king_square_horizon].u.piece_movement.movingspec;
          if (TSTFLAG(movingspec,Royal))
          {
            if (TSTFLAG(movingspec,White))
              new_king_square[White] = to;
            if (TSTFLAG(movingspec,Black))
              new_king_square[Black] = to;
          }
        }
        break;
      }

      case move_effect_piece_exchange:
      {
        square const from = move_effect_journal[king_square_horizon].u.piece_exchange.from;
        Flags const fromflags = move_effect_journal[king_square_horizon].u.piece_exchange.fromflags;
        square const to = move_effect_journal[king_square_horizon].u.piece_exchange.to;
        Flags const toflags = move_effect_journal[king_square_horizon].u.piece_exchange.toflags;

        if (TSTFLAG(fromflags,Royal))
        {
          if (TSTFLAG(fromflags,White))
            new_king_square[White] = to;
          if (TSTFLAG(fromflags,Black))
            new_king_square[Black] = to;
        }

        if (TSTFLAG(toflags,Royal))
        {
          if (TSTFLAG(toflags,White))
            new_king_square[White] = from;
          if (TSTFLAG(toflags,Black))
            new_king_square[Black] = from;
        }

        TraceSquare(new_king_square[White]);
        TraceSquare(new_king_square[Black]);
        TraceEOL();
        break;
      }

      case move_effect_flags_change:
      {
        square const on = move_effect_journal[king_square_horizon].u.flags_change.on;
        Flags const from = move_effect_journal[king_square_horizon].u.flags_change.from;
        Flags const to = move_effect_journal[king_square_horizon].u.flags_change.to;

        if (!TSTFLAG(from,Royal) && TSTFLAG(to,Royal))
        {
          if (TSTFLAG(to,White))
            new_king_square[White] = on;
          if (TSTFLAG(to,Black))
            new_king_square[Black] = on;
        }
        if (TSTFLAG(from,Royal) && !TSTFLAG(to,Royal))
        {
          if (TSTFLAG(to,White))
            new_king_square[White] = initsquare;
          if (TSTFLAG(to,Black))
            new_king_square[Black] = initsquare;
        }
        break;
      }

      case move_effect_board_transformation:
      {
        SquareTransformation const transformation = move_effect_journal[king_square_horizon].u.board_transformation.transformation;
        if (new_king_square[White]!=initsquare)
          new_king_square[White] = transformSquare(new_king_square[White],
                                                   transformation);
        if (new_king_square[Black]!=initsquare)
          new_king_square[Black] = transformSquare(new_king_square[Black],
                                                   transformation);
        break;
      }

      case move_effect_centre_revolution:
      {
        {
          square revolved = actuated_revolving_centre_revolve_square(new_king_square[White]);
          if (revolved!=initsquare)
            new_king_square[White] = revolved;
        }
        {
          square revolved = actuated_revolving_centre_revolve_square(new_king_square[Black]);
          if (revolved!=initsquare)
            new_king_square[Black] = revolved;
        }
        break;
      }

      case move_effect_half_neutral_neutralisation:
      {
        Side const from = move_effect_journal[king_square_horizon].u.half_neutral_phase_change.side;
        square const on = move_effect_journal[king_square_horizon].u.half_neutral_phase_change.on;
        if (new_king_square[from]==on)
          new_king_square[advers(from)] = on;
        break;
      }

      case move_effect_half_neutral_deneutralisation:
      {
        Side const to = move_effect_journal[king_square_horizon].u.half_neutral_phase_change.side;
        square const on = move_effect_journal[king_square_horizon].u.half_neutral_phase_change.on;
        if (new_king_square[advers(to)]==on)
          new_king_square[advers(to)] = initsquare;
        break;
      }

      default:
        break;
    }

  if (new_king_square[White]!=king_square[White])
    move_effect_journal_do_king_square_movement(move_effect_no_reason,
                                                White,
                                                new_king_square[White]);
  if (new_king_square[Black]!=king_square[Black])
    move_effect_journal_do_king_square_movement(move_effect_no_reason,
                                                Black,
                                                new_king_square[Black]);

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
void king_square_updater_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const save_horizon = king_square_horizon;
    update_king_squares();
    pipe_solve_delegate(si);
    king_square_horizon = save_horizon;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
