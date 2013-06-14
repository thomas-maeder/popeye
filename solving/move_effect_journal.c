#include "solving/move_effect_journal.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "conditions/imitator.h"
#include "conditions/actuated_revolving_centre.h"
#include "conditions/haunted_chess.h"
#include "pieces/attributes/neutral/half.h"
#include "position/pieceid.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

move_effect_journal_entry_type move_effect_journal[move_effect_journal_size];

move_effect_journal_index_type move_effect_journal_top[maxply+1] = { 3, 4 };

move_effect_journal_index_type move_effect_journal_index_offset_capture = 0;
move_effect_journal_index_type move_effect_journal_index_offset_movement = 1;
move_effect_journal_index_type move_effect_journal_index_offset_other_effects = 2;

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

  /* this acrobatics is needed in Circe Parrain */
  move_effect_journal[0].type = move_effect_none;
  move_effect_journal[1] = move_effect_journal[2];
  move_effect_journal[2].type = move_effect_none;

  move_effect_journal_top[0] = 2;
  move_effect_journal_top[1] = 4;

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

  move_effect_journal[2].type = move_effect_no_piece_removal;
  move_effect_journal[2].u.piece_removal.removed = Empty;
  CLEARFL(move_effect_journal[2].u.piece_removal.removedspec);
  move_effect_journal_top[0] = 3;
  move_effect_journal_top[1] = 4;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Store a retro capture, e.g. for Circe Parrain key moves
 * @param from square where the retro capture took place
 * @param removed piece removed by the capture
 * @param removedspec flags of that piece
 */
void move_effect_journal_store_retro_capture(square from,
                                             piece removed,
                                             Flags removedspec)
{
  move_effect_journal[3].type = move_effect_piece_removal;
  move_effect_journal[3].reason = move_effect_reason_regular_capture;
  move_effect_journal[3].u.piece_removal.from = from;
  move_effect_journal[3].u.piece_removal.removed = abs(removed);
  move_effect_journal[3].u.piece_removal.removedspec = removedspec;
}

/* Reset the stored retro capture
 */
void move_effect_journal_reset_retro_capture(void)
{
  move_effect_journal[3].type = move_effect_no_piece_removal;
}

#if defined(DOTRACE)
unsigned long move_effect_journal_next_id;
#endif

static void push_movement_elmt(move_effect_reason_type reason,
                               square from,
                               square to)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

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

  ++move_effect_journal_top[nbply];

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

static void do_king_square_movement(move_effect_reason_type reason,
                                    square from,
                                    square to)
{
  if (to!=from)
  {
    if (TSTFLAG(spec[to],Royal))
    {
      if (TSTFLAG(spec[to],White))
        move_effect_journal_do_king_square_movement(reason,White,to);
      if (TSTFLAG(spec[to],Black))
        move_effect_journal_do_king_square_movement(reason,Black,to);
    }
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

  push_movement_elmt(reason,from,to);
  do_movement(from,to);
  do_king_square_movement(reason,from,to);

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

  TraceSquare(from);TraceSquare(to);TracePiece(e[to]);TraceText("\n");

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
  TraceText("\n");

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
                                             PieNam added,
                                             Flags addedspec)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TracePiece(added);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_piece_readdition;
  top_elmt->reason = reason;
  top_elmt->u.piece_addition.on = on;
  top_elmt->u.piece_addition.added = added;
  top_elmt->u.piece_addition.addedspec = addedspec;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  assert(is_square_empty(on));
  if (TSTFLAG(addedspec,White))
    ++number_of_pieces[White][added];
  if (TSTFLAG(addedspec,Black))
    ++number_of_pieces[Black][added];
  occupy_square(on,added,addedspec);
  assert(GetPieceId(addedspec)!=NullPieceId);

  TraceValue("%u",TSTFLAG(addedspec,Royal));
  TraceValue("%u",TSTFLAG(addedspec,White));
  TraceValue("%u\n",TSTFLAG(addedspec,Black));
  if (TSTFLAG(addedspec,Royal))
  {
    if (king_square[White]==initsquare && TSTFLAG(addedspec,White))
      move_effect_journal_do_king_square_movement(reason,White,on);
    if (king_square[Black]==initsquare && TSTFLAG(addedspec,Black))
      move_effect_journal_do_king_square_movement(reason,Black,on);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_readdition(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const on = curr_elmt->u.piece_addition.on;
  PieNam const added = curr_elmt->u.piece_addition.added;
  Flags const addedspec = curr_elmt->u.piece_addition.addedspec;

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
  PieNam const added = curr_elmt->u.piece_addition.added;
  Flags const addedspec = curr_elmt->u.piece_addition.addedspec;

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
                                           PieNam created,
                                           Flags createdspec)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TracePiece(created);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_piece_creation;
  top_elmt->reason = reason;
  top_elmt->u.piece_addition.on = on;
  top_elmt->u.piece_addition.added = created;
  top_elmt->u.piece_addition.addedspec = createdspec;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  assert(is_square_empty(on));
  if (TSTFLAG(createdspec,White))
    ++number_of_pieces[White][created];
  if (TSTFLAG(createdspec,Black))
    ++number_of_pieces[Black][created];
  occupy_square(on,created,createdspec);
  SetPieceId(spec[on],currPieceId++);

  TraceValue("%u",TSTFLAG(createdspec,Royal));
  TraceValue("%u",TSTFLAG(createdspec,White));
  TraceValue("%u\n",TSTFLAG(createdspec,Black));
  if (TSTFLAG(createdspec,Royal))
  {
    if (king_square[White]==initsquare && TSTFLAG(createdspec,White))
      move_effect_journal_do_king_square_movement(reason,White,on);
    if (king_square[Black]==initsquare && TSTFLAG(createdspec,Black))
      move_effect_journal_do_king_square_movement(reason,Black,on);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_creation(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const on = curr_elmt->u.piece_addition.on;
  PieNam const created = curr_elmt->u.piece_addition.added;
  piece const createdspec = curr_elmt->u.piece_addition.addedspec;

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
  PieNam const created = curr_elmt->u.piece_addition.added;
  piece const createdspec = curr_elmt->u.piece_addition.addedspec;

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
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_no_piece_removal;
  top_elmt->reason = move_effect_no_reason;
  top_elmt->u.piece_removal.removed = Empty;
  CLEARFL(top_elmt->u.piece_removal.removedspec);
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void push_removal_elmt(move_effect_reason_type reason, square from)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  TraceValue("%u",top);TraceText("removal");TraceValue("%u",nbply);TraceSquare(from);TracePiece(e[from]);TraceText("\n");

  top_elmt->type = move_effect_piece_removal;
  top_elmt->reason = reason;
  top_elmt->u.piece_removal.from = from;
  top_elmt->u.piece_removal.removed = get_walk_of_piece_on_square(from);
  top_elmt->u.piece_removal.removedspec = spec[from];
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void do_removal(square from)
{
  PieNam const removed = get_walk_of_piece_on_square(from);
  Flags const removedspec = spec[from];

  assert(!is_square_empty(from));

  if (TSTFLAG(removedspec,White))
    --number_of_pieces[White][removed];
  if (TSTFLAG(removedspec,Black))
    --number_of_pieces[Black][removed];

  empty_square(from);
}

static void do_king_square_removal(move_effect_reason_type reason,
                                   square from,
                                   Flags spec_captured)
{
  if (TSTFLAG(spec_captured,Royal))
  {
    if (TSTFLAG(spec_captured,White))
      move_effect_journal_do_king_square_movement(reason,White,initsquare);
    if (TSTFLAG(spec_captured,Black))
      move_effect_journal_do_king_square_movement(reason,Black,initsquare);
  }
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
  Flags const spec_captured = spec[from];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  push_removal_elmt(reason,from);
  do_removal(from);
  do_king_square_removal(reason,from,spec_captured);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_removal(move_effect_journal_index_type curr)
{
  square const from = move_effect_journal[curr].u.piece_removal.from;
  PieNam const removed = move_effect_journal[curr].u.piece_removal.removed;
  Flags const removedspec = move_effect_journal[curr].u.piece_removal.removedspec;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  occupy_square(from,removed,removedspec);

  if (TSTFLAG(removedspec,White))
    ++number_of_pieces[White][abs(removed)];
  if (TSTFLAG(removedspec,Black))
    ++number_of_pieces[Black][abs(removed)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_removal(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const from = curr_elmt->u.piece_removal.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif
  TraceValue("%u",curr);TraceText("removal");TraceValue("%u",nbply);TraceSquare(from);TracePiece(curr_elmt->u.piece_removal.removed);TraceText("\n");

  do_removal(from);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add changing the nature of a piece to the current move of the current ply
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 * @param to new nature of piece
 */
void move_effect_journal_do_piece_change(move_effect_reason_type reason,
                                         square on,
                                         PieNam to)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TracePiece(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_piece_change;
  top_elmt->reason = reason;
  top_elmt->u.piece_change.on = on;
  top_elmt->u.piece_change.from = get_walk_of_piece_on_square(on);
  top_elmt->u.piece_change.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  if (TSTFLAG(spec[on],White))
    --number_of_pieces[White][get_walk_of_piece_on_square(on)];
  if (TSTFLAG(spec[on],Black))
    --number_of_pieces[Black][get_walk_of_piece_on_square(on)];

  replace_piece(on,to);

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
  PieNam const from = move_effect_journal[curr].u.piece_change.from;

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

  replace_piece(on,from);

  if (TSTFLAG(spec[on],White))
    ++number_of_pieces[White][abs(from)];
  if (TSTFLAG(spec[on],Black))
    ++number_of_pieces[Black][abs(from)];

  TraceValue("%d",GetPieceId(spec[on]));
  TraceValue("%d",TSTFLAG(spec[on],White));
  TraceValue("%d",TSTFLAG(spec[on],Black));
  TraceValue("%d",TSTFLAG(spec[on],Neutral));
  TraceValue("%d",TSTFLAG(spec[on],HalfNeutral));
  TraceValue("%d\n",e[on]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_change(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.piece_change.on;
  PieNam const to = move_effect_journal[curr].u.piece_change.to;

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

  replace_piece(on,to);

  if (TSTFLAG(spec[on],White))
    ++number_of_pieces[White][abs(to)];
  if (TSTFLAG(spec[on],Black))
    ++number_of_pieces[Black][abs(to)];

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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];
  PieNam const pi_to = get_walk_of_piece_on_square(to);
  Flags const spec_pi_to = spec[to];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_piece_exchange;
  top_elmt->reason = reason;
  top_elmt->u.piece_exchange.from = from;
  top_elmt->u.piece_exchange.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  occupy_square(to,get_walk_of_piece_on_square(from),spec[from]);
  occupy_square(from,pi_to,spec_pi_to);

  if (TSTFLAG(spec[from],Royal))
  {
    if (TSTFLAG(spec[from],White))
      move_effect_journal_do_king_square_movement(reason,White,from);
    if (TSTFLAG(spec[from],Black))
      move_effect_journal_do_king_square_movement(reason,Black,from);
  }

  if (TSTFLAG(spec[to],Royal))
  {
    if (TSTFLAG(spec[to],White))
      move_effect_journal_do_king_square_movement(reason,White,to);
    if (TSTFLAG(spec[to],Black))
      move_effect_journal_do_king_square_movement(reason,Black,to);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_exchange(move_effect_journal_index_type curr)
{
  square const from = move_effect_journal[curr].u.piece_exchange.from;
  square const to = move_effect_journal[curr].u.piece_exchange.to;
  PieNam const pi_to = get_walk_of_piece_on_square(to);
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
  PieNam const pi_to = get_walk_of_piece_on_square(to);
  Flags const spec_pi_to = spec[to];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif
  TraceSquare(from);
  TraceSquare(to);
  TraceText("\n");

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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];
  Side const to = TSTFLAG(spec[on],White) ? Black : White;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceEnumerator(Side,to,"");
  TraceFunctionParamListEnd();

  assert(!TSTFLAG(spec[on],Neutral));

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_side_change;
  top_elmt->reason = reason;
  top_elmt->u.side_change.on = on;
  top_elmt->u.side_change.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  --number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]^(BIT(White)|BIT(Black)));
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
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]^(BIT(White)|BIT(Black)));
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
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]^(BIT(White)|BIT(Black)));
  ++number_of_pieces[to][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Deneutralise a half-neutral piece
 * @param on position of the piece to be changed
 * @param to new side of half-neutral piece
 */
void move_effect_journal_do_half_neutral_deneutralisation(square on, Side to)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceSquare(on);
  TraceEnumerator(Side,to,"");
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_half_neutral_deneutralisation;
  top_elmt->reason = move_effect_reason_half_neutral_deneutralisation;
  top_elmt->u.half_neutral_phase_change.on = on;
  top_elmt->u.half_neutral_phase_change.side = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  assert(TSTFLAG(spec[on],White));
  assert(TSTFLAG(spec[on],Black));
  assert(TSTFLAG(spec[on],Neutral));

  --number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]&~(BIT(Neutral)|BIT(advers(to))));

  if (king_square[advers(to)]==on)
    move_effect_journal_do_king_square_movement(move_effect_reason_half_neutral_king_movement,
                                                advers(to),
                                                initsquare);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_half_neutral_deneutralisation(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.half_neutral_phase_change.on;
  Side const to = move_effect_journal[curr].u.half_neutral_phase_change.side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]|BIT(Neutral)|BIT(advers(to)));
  ++number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_half_neutral_deneutralisation(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.half_neutral_phase_change.on;
  Side const to = move_effect_journal[curr].u.half_neutral_phase_change.side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  --number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]&~(BIT(Neutral)|BIT(advers(to))));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Neutralise a half-neutral piece
 * @param on position of the piece to be changed
 */
void move_effect_journal_do_half_neutral_neutralisation(square on)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];
  Side const from = TSTFLAG(spec[on],White) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceSquare(on);
  TraceEnumerator(Side,from,"");
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_half_neutral_neutralisation;
  top_elmt->reason = move_effect_reason_half_neutral_neutralisation;
  top_elmt->u.half_neutral_phase_change.on = on;
  top_elmt->u.half_neutral_phase_change.side = from;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]|BIT(Neutral)|BIT(advers(from)));
  ++number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];

  if (king_square[from]==on)
    move_effect_journal_do_king_square_movement(move_effect_reason_half_neutral_king_movement,
                                                advers(from),
                                                on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_half_neutral_neutralisation(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.half_neutral_phase_change.on;
  Side const from = move_effect_journal[curr].u.half_neutral_phase_change.side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  assert(TSTFLAG(spec[on],White));
  assert(TSTFLAG(spec[on],Black));
  assert(TSTFLAG(spec[on],Neutral));

  --number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]&~(BIT(Neutral)|BIT(advers(from))));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_half_neutral_neutralisation(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.half_neutral_phase_change.on;
  Side const from = move_effect_journal[curr].u.half_neutral_phase_change.side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]|BIT(Neutral)|BIT(advers(from)));
  ++number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];

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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(square);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_square_block;
  top_elmt->reason = reason;
  top_elmt->u.square_block.square = square;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceEnumerator(Side,side,"");
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_king_square_movement;
  top_elmt->reason = reason;
  top_elmt->u.king_square_movement.side = side;
  top_elmt->u.king_square_movement.from = king_square[side];
  top_elmt->u.king_square_movement.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

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
  TraceText("\n");

  assert(king_square[side]==move_effect_journal[curr].u.king_square_movement.to);

  king_square[side] = from;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_king_square_movement(move_effect_journal_index_type curr)
{
  Side const side = move_effect_journal[curr].u.king_square_removal.side;
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
  TraceText("\n");

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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceSquare(on);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_flags_change;
  top_elmt->reason = reason;
  top_elmt->u.flags_change.on = on;
  top_elmt->u.flags_change.from = spec[on];
  top_elmt->u.flags_change.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  if (!TSTFLAG(spec[on],Royal) && TSTFLAG(to,Royal))
  {
    if (TSTFLAG(to,White))
      move_effect_journal_do_king_square_movement(reason,White,on);
    if (TSTFLAG(to,Black))
      move_effect_journal_do_king_square_movement(reason,Black,on);
  }
  if (TSTFLAG(spec[on],Royal) && !TSTFLAG(to,Royal))
  {
    if (TSTFLAG(to,White))
      move_effect_journal_do_king_square_movement(reason,White,initsquare);
    if (TSTFLAG(to,Black))
      move_effect_journal_do_king_square_movement(reason,Black,initsquare);
  }

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

  TraceSquare(on);TraceText("\n");

  assert(spec[on]==move_effect_journal[curr].u.flags_change.to);

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
  PieNam t_e[nr_squares_on_board];
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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceValue("%u",transformation);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_board_transformation;
  top_elmt->reason = reason;
  top_elmt->u.board_transformation.transformation = transformation;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  transformBoard(transformation);

  if (king_square[White]!=initsquare)
  {
    square const transformed = transformSquare(king_square[White],transformation);
    move_effect_journal_do_king_square_movement(reason,White,transformed);
  }
  if (king_square[Black]!=initsquare)
  {
    square const transformed = transformSquare(king_square[Black],transformation);
    move_effect_journal_do_king_square_movement(reason,Black,transformed);
  }

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
  Flags const spec_captured = spec[sq_capture];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParam("%u",removal_reason);
  TraceFunctionParamListEnd();

  /* Be careful when attempting to reordering. This sequence makes sure that
   * the effects are in the order removal - movement - king_square
   */
  push_removal_elmt(removal_reason,sq_capture);
  do_removal(sq_capture);

  push_movement_elmt(move_effect_reason_moving_piece_movement,sq_departure,sq_arrival);
  do_movement(sq_departure,sq_arrival);

  do_king_square_removal(removal_reason,sq_capture,spec_captured);
  do_king_square_movement(move_effect_reason_moving_piece_movement,sq_departure,sq_arrival);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add a null effect to the current move of the current ply
 */
void move_effect_journal_do_null_effect(void)
{
  move_effect_journal_index_type top = move_effect_journal_top[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(top+1<move_effect_journal_size);

  move_effect_journal[top].type = move_effect_none;
  move_effect_journal[top].reason = move_effect_no_reason;

  ++move_effect_journal_top[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add the effects of a null move to the current move of the current ply
 */
void move_effect_journal_do_null_move(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_do_null_effect(); /* no piece removal */
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
  move_effect_journal_index_type const base = move_effect_journal_top[ply-1];
  move_effect_journal_index_type const top = move_effect_journal_top[ply];
  move_effect_journal_index_type other;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParam("%x",followed_id);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  for (other = base+move_effect_journal_index_offset_other_effects;
       other<top;
       ++other)
    switch (move_effect_journal[other].type)
    {
      case move_effect_piece_removal:
        if (move_effect_journal[other].u.piece_removal.from==pos)
        {
          assert(GetPieceId(move_effect_journal[other].u.piece_removal.removedspec)==followed_id);
          pos = initsquare;
        }
        break;

      case move_effect_piece_readdition:
      case move_effect_piece_creation:
        if (GetPieceId(move_effect_journal[other].u.piece_addition.addedspec)==followed_id)
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
        /* nothing */
        break;

      default:
        assert(0);
        break;
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
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply-1]<=top);

  for (curr = move_effect_journal_top[nbply-1]; curr!=top; ++curr)
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
  move_effect_journal_index_type const parent_top = move_effect_journal_top[nbply-1];
  move_effect_journal_index_type top;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]>=parent_top);

  for (top = move_effect_journal_top[nbply]; top!=parent_top; --top)
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

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type move_effect_journal_undoer_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_top[nbply] = move_effect_journal_top[nbply-1];
  result = solve(slices[si].next1,n);
  undo_move_effects();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type move_effect_journal_redoer_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  redo_move_effects();
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
