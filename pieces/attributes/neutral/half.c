#include "pieces/attributes/neutral/half.h"
#include "pieces/pieces.h"
#include "pieces/attributes/neutral/neutral.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

/* Deneutralise a half-neutral piece
 * @param on position of the piece to be changed
 * @param to new side of half-neutral piece
 */
static void do_deneutralisation(square on, Side to)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceSquare(on);
  TraceEnumerator(Side,to,"");
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_half_neutral_deneutralisation;
  top_elmt->reason = move_effect_reason_half_neutral_deneutralisation;
  top_elmt->u.half_neutral_phase_change.on = on;
  top_elmt->u.half_neutral_phase_change.side = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  assert(TSTFLAG(being_solved.spec[on],White));
  assert(TSTFLAG(being_solved.spec[on],Black));
  assert(is_piece_neutral(being_solved.spec[on]));

  --being_solved.number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];
  occupy_square(on,get_walk_of_piece_on_square(on),being_solved.spec[on]&~BIT(advers(to)));

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

  occupy_square(on,get_walk_of_piece_on_square(on),being_solved.spec[on]|BIT(advers(to)));
  ++being_solved.number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];

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

  --being_solved.number_of_pieces[advers(to)][get_walk_of_piece_on_square(on)];
  occupy_square(on,get_walk_of_piece_on_square(on),being_solved.spec[on]&~BIT(advers(to)));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Neutralise a half-neutral piece
 * @param on position of the piece to be changed
 */
static void do_neutralisation(square on)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];
  Side const from = TSTFLAG(being_solved.spec[on],White) ? White : Black;

  TraceFunctionEntry(__func__);
  TraceSquare(on);
  TraceEnumerator(Side,from,"");
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_half_neutral_neutralisation;
  top_elmt->reason = move_effect_reason_half_neutral_neutralisation;
  top_elmt->u.half_neutral_phase_change.on = on;
  top_elmt->u.half_neutral_phase_change.side = from;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  occupy_square(on,get_walk_of_piece_on_square(on),being_solved.spec[on]|BIT(advers(from)));
  ++being_solved.number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];

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

  assert(TSTFLAG(being_solved.spec[on],White));
  assert(TSTFLAG(being_solved.spec[on],Black));
  assert(is_piece_neutral(being_solved.spec[on]));

  --being_solved.number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];
  occupy_square(on,get_walk_of_piece_on_square(on),being_solved.spec[on]&~BIT(advers(from)));

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

  occupy_square(on,get_walk_of_piece_on_square(on),being_solved.spec[on]|BIT(advers(from)));
  ++being_solved.number_of_pieces[advers(from)][get_walk_of_piece_on_square(on)];

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
void half_neutral_recolorer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    Flags const movingspec = move_effect_journal[movement].u.piece_movement.movingspec;
    PieceIdType const moving_id = GetPieceId(movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);

    if (TSTFLAG(movingspec,HalfNeutral))
    {
      if (is_piece_neutral(being_solved.spec[pos]))
        do_deneutralisation(pos,slices[si].starter);
      else
        do_neutralisation(pos);
    }
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void solving_insert_half_neutral_recolorers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_instrument_moves(si,STPiecesHalfNeutralRecolorer);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
