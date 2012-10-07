#include "solving/move_effect_journal.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "conditions/imitator.h"
#include "conditions/actuated_revolving_centre.h"
#include "conditions/haunted_chess.h"
#include "pieces/attributes/neutral/half.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

move_effect_journal_entry_type move_effect_journal[move_effect_journal_size];

move_effect_journal_index_type move_effect_journal_top[maxply+1];

#if defined(DOTRACE)
unsigned long move_effect_journal_next_id;
#endif

/* Add moving a piece to the current move of the current ply
 * @param reason reason for moving the piece
 * @param from current position of the piece
 * @param to where to move the piece
 * @return index of piece piece_movement effect
 */
move_effect_journal_index_type
move_effect_journal_do_piece_movement(move_effect_reason_type reason,
                                      square from,
                                      square to)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];
  move_effect_journal_index_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_piece_movement;
  top_elmt->reason = reason;
  top_elmt->u.piece_movement.moving = e[from];
  top_elmt->u.piece_movement.movingspec = spec[from];
  top_elmt->u.piece_movement.from = from;
  top_elmt->u.piece_movement.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  result = move_effect_journal_top[nbply]++;

  if (to!=from)
  {
    e[to] = e[from];
    spec[to] = spec[from];

    e[from] = vide;
    CLEARFL(spec[from]);

    if (TSTFLAG(spec[to],Royal))
    {
      if (TSTFLAG(spec[to],White))
        move_effect_journal_do_king_square_movement(reason,White,to);
      if (TSTFLAG(spec[to],Black))
        move_effect_journal_do_king_square_movement(reason,Black,to);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
    e[from] = e[to];
    spec[from] = spec[to];

    e[to] = vide;
    CLEARFL(spec[to]);
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
    e[to] = e[from];
    spec[to] = spec[from];

    e[from] = vide;
    CLEARFL(spec[from]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add adding a piece to the current move of the current ply
 * @param reason reason for adding the piece
 * @param on where to insert the piece
 * @param added nature of added piece
 * @param addedspec specs of added piece
 */
void move_effect_journal_do_piece_addition(move_effect_reason_type reason,
                                           square on,
                                           piece added,
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

  top_elmt->type = move_effect_piece_addition;
  top_elmt->reason = reason;
  top_elmt->u.piece_addition.on = on;
  top_elmt->u.piece_addition.added = added;
  top_elmt->u.piece_addition.addedspec = addedspec;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  assert(e[on]==vide);
  ++nbpiece[added];
  e[on] = added;
  spec[on] = addedspec;

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

static void undo_piece_addition(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.piece_addition.on;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  --nbpiece[e[on]];
  e[on] = vide;
  CLEARFL(spec[on]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_addition(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const on = curr_elmt->u.piece_addition.on;
  piece const added = curr_elmt->u.piece_addition.added;
  piece const addedspec = curr_elmt->u.piece_addition.addedspec;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  ++nbpiece[added];

  assert(e[on]==vide);
  e[on] = added;
  spec[on] = addedspec;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Link the piece_removal and piece_movement just inserted as a capture
 * @param piece_removal index of the piece_removal effect
 * @param piece_removal index of the piece_movement effect
 */
void move_effect_journal_link_capture_to_movement(move_effect_journal_index_type removal,
                                                  move_effect_journal_index_type movement)
{
  move_effect_journal_entry_type * const removal_elmt = &move_effect_journal[removal];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",removal);
  TraceFunctionParam("%u",movement);
  TraceFunctionParamListEnd();

  assert(removal_elmt->type==move_effect_piece_removal);
  assert(removal_elmt->reason==move_effect_reason_regular_capture);

  assert(move_effect_journal[movement].type==move_effect_piece_movement);
  assert(move_effect_journal[movement].reason==move_effect_reason_moving_piece_movement);

  removal_elmt->u.piece_removal.capturing_movement = movement;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add removing a piece to the current move of the current ply
 * @param reason reason for removing the piece
 * @param from current position of the piece
 * @return index of piece piece_removal effect
 */
move_effect_journal_index_type
move_effect_journal_do_piece_removal(move_effect_reason_type reason,
                                     square from)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];
  move_effect_journal_index_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  TraceValue("%u",top);TraceText("removal");TraceValue("%u",nbply);TraceSquare(from);TracePiece(e[from]);TraceText("\n");

  top_elmt->type = move_effect_piece_removal;
  top_elmt->reason = reason;
  top_elmt->u.piece_removal.from = from;
  top_elmt->u.piece_removal.removed = e[from];
  top_elmt->u.piece_removal.removedspec = spec[from];
  top_elmt->u.piece_removal.capturing_movement = move_effect_journal_index_null;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  result = move_effect_journal_top[nbply]++;

  assert(e[from]!=vide);
  --nbpiece[e[from]];
  e[from] = vide;

  if (TSTFLAG(spec[from],Royal))
  {
    if (TSTFLAG(spec[from],White))
      move_effect_journal_do_king_square_movement(reason,White,initsquare);
    if (TSTFLAG(spec[from],Black))
      move_effect_journal_do_king_square_movement(reason,Black,initsquare);
  }

  CLEARFL(spec[from]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void undo_piece_removal(move_effect_journal_index_type curr)
{
  square const from = move_effect_journal[curr].u.piece_removal.from;
  piece const removed = move_effect_journal[curr].u.piece_removal.removed;
  Flags const removedspec = move_effect_journal[curr].u.piece_removal.removedspec;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  e[from] = removed;
  spec[from] = removedspec;
  ++nbpiece[removed];

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

  --nbpiece[e[from]];

  e[from] = vide;
  CLEARFL(spec[from]);

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
                                         piece to)
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
  top_elmt->u.piece_change.from = e[on];
  top_elmt->u.piece_change.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  --nbpiece[e[on]];
  e[on] = to;
  ++nbpiece[to];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_piece_change(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.piece_change.on;
  piece const from = move_effect_journal[curr].u.piece_change.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  TraceValue("%d",e[on]);
  TraceValue("%d\n",from);

  --nbpiece[e[on]];
  e[on] = from;
  ++nbpiece[from];

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
  piece const to = move_effect_journal[curr].u.piece_change.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  --nbpiece[e[on]];
  e[on] = to;
  ++nbpiece[to];

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
  piece const pi_to = e[to];
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

  e[to] = e[from];
  spec[to] = spec[from];

  e[from] = pi_to;
  spec[from] = spec_pi_to;

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
  piece const pi_to = e[to];
  Flags const spec_pi_to = spec[to];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  e[to] = e[from];
  spec[to] = spec[from];

  e[from] = pi_to;
  spec[from] = spec_pi_to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_piece_exchange(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  square const from = curr_elmt->u.piece_exchange.from;
  square const to = curr_elmt->u.piece_exchange.to;
  piece const pi_to = e[to];
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

  e[to] = e[from];
  spec[to] = spec[from];

  e[from] = pi_to;
  spec[from] = spec_pi_to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add changing the side of a piece to the current move of the current ply
 * @param reason reason for changing the piece's nature
 * @param on position of the piece to be changed
 * @param to to side
 */
void move_effect_journal_do_side_change(move_effect_reason_type reason,
                                        square on,
                                        Side to)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(on);
  TraceEnumerator(Side,to,"");
  TraceFunctionParamListEnd();

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

  CLRFLAGMASK(spec[on],BIT(Black)|BIT(White)|BIT(Neutral));
  SETFLAG(spec[on],to);

  --nbpiece[e[on]];
  e[on] = to==White ? abs(e[on]) : -abs(e[on]);
  ++nbpiece[e[on]];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_side_change(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.side_change.on;
  Side const to = move_effect_journal[curr].u.side_change.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  CLRFLAGMASK(spec[on],BIT(Black)|BIT(White)|BIT(Neutral));
  SETFLAG(spec[on],advers(to));

  --nbpiece[e[on]];
  e[on] = to==White ? -abs(e[on]) : abs(e[on]);
  ++nbpiece[e[on]];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_side_change(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.side_change.on;
  Side const to = move_effect_journal[curr].u.side_change.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  CLRFLAGMASK(spec[on],BIT(Black)|BIT(White)|BIT(Neutral));
  SETFLAG(spec[on],to);

  --nbpiece[e[on]];
  e[on] = to==White ? abs(e[on]) : -abs(e[on]);
  ++nbpiece[e[on]];

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

  spec[on] = to;

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
  spec[on] = from;

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
  spec[on] = to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void transformBoard(SquareTransformation transformation)
{
  piece t_e[nr_squares_on_board];
  Flags t_spec[nr_squares_on_board];
  square sq1, sq2;
  imarr t_isquare;
  int i;

  /* save the position to be mirrored/rotated */
  for (i = 0; i<nr_squares_on_board; i++)
  {
    t_e[i] = e[boardnum[i]];
    t_spec[i] = spec[boardnum[i]];
  }

  for (i = 0; i<maxinum; i++)
    t_isquare[i] = isquare[i];

  /* now rotate/mirror */
  /* pieces */
  for (i = 0; i<nr_squares_on_board; i++)
  {
    sq1 = boardnum[i];
    sq2 = transformSquare(sq1,transformation);

    e[sq2] = t_e[i];
    spec[sq2] = t_spec[i];
  }

  /* imitators */
  for (i= 0; i<maxinum; i++)
  {
    sq1 = t_isquare[i];
    sq2 = transformSquare(sq1, transformation);
    isquare[i]= sq2;
  }
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
      case move_effect_piece_movement:
        redo_piece_movement(curr);
        break;

      case move_effect_piece_addition:
        redo_piece_addition(curr);
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

      case move_effect_neutral_recoloring_do:
        neutral_initialiser_recolor_replaying();
        break;

      case move_effect_neutral_recoloring_undo:
        /* nothing */
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
      case move_effect_piece_movement:
        undo_piece_movement(top-1);
        break;

      case move_effect_piece_addition:
        undo_piece_addition(top-1);
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

      case move_effect_neutral_recoloring_do:
        /* nothing */
        break;

      case move_effect_neutral_recoloring_undo:
        neutral_initialiser_recolor_retracting();
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
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
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
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
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
