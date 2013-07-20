#include "pieces/attributes/neutral/half.h"
#include "pydata.h"
#include "pieces/attributes/neutral/neutral.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

/* Deneutralise a half-neutral piece
 * @param on position of the piece to be changed
 * @param to new side of half-neutral piece
 */
static void do_deneutralisation(square on, Side to)
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
  assert(is_piece_neutral(spec[on]));

  --number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]&~BIT(advers(to)));

  if (king_square[advers(to)]==on)
    move_effect_journal_do_king_square_movement(move_effect_reason_half_neutral_king_movement,
                                                advers(to),
                                                initsquare);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo the deneutralisation a half-neutral piece
 * @param curr identifies the deneutralisation effect
 */
void undo_half_neutral_deneutralisation(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.half_neutral_phase_change.on;
  Side const to = move_effect_journal[curr].u.half_neutral_phase_change.side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]|BIT(advers(to)));
  ++number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo the deneutralisation a half-neutral piece
 * @param curr identifies the deneutralisation effect
 */
void redo_half_neutral_deneutralisation(move_effect_journal_index_type curr)
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
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]&~BIT(advers(to)));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Neutralise a half-neutral piece
 * @param on position of the piece to be changed
 */
static void do_neutralisation(square on)
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

  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]|BIT(advers(from)));
  ++number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];

  if (king_square[from]==on)
    move_effect_journal_do_king_square_movement(move_effect_reason_half_neutral_king_movement,
                                                advers(from),
                                                on);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo the neutralisation a half-neutral piece
 * @param curr identifies the neutralisation effect
 */
void undo_half_neutral_neutralisation(move_effect_journal_index_type curr)
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
  assert(is_piece_neutral(spec[on]));

  --number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];
  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]&~BIT(advers(from)));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo the neutralisation a half-neutral piece
 * @param curr identifies the neutralisation effect
 */
void redo_half_neutral_neutralisation(move_effect_journal_index_type curr)
{
  square const on = move_effect_journal[curr].u.half_neutral_phase_change.on;
  Side const from = move_effect_journal[curr].u.half_neutral_phase_change.side;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  occupy_square(on,get_walk_of_piece_on_square(on),spec[on]|BIT(advers(from)));
  ++number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];

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
stip_length_type half_neutral_recolorer_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_top[nbply-1];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
    PieceIdType const moving_id = GetPieceId(movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);

    if (TSTFLAG(movingspec,HalfNeutral))
    {
      if (is_piece_neutral(spec[pos]))
        do_deneutralisation(pos,slices[si].starter);
      else
        do_neutralisation(pos);
    }
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_half_neutral_recolorers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_instrument_moves(si,STPiecesHalfNeutralRecolorer);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
