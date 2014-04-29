#include "conditions/duellists.h"
#include "solving/move_generator.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

square duellists[nr_sides];

/* Determine the length of a move for the Duellists condition; the higher the
 * value the more likely the move is going to be played.
 * @return a value expressing the precedence of this move
 */
int duellists_measure_length(void)
{
  return move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure==duellists[trait[nbply]];
}

/* Remember a duellist
 * @param diff adjustment
 */
static void remember_duellist(Side side, square to)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];

  TraceFunctionEntry(__func__);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_remember_duellist;
  top_elmt->reason = move_effect_reason_moving_piece_movement;
  top_elmt->u.duellist.side = side;
  top_elmt->u.duellist.from = duellists[side];
  top_elmt->u.duellist.to = to;
 #if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
 #endif

  ++move_effect_journal_base[nbply+1];

  duellists[side] = to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo remembering a duellist
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_undo_remember_duellist(move_effect_journal_index_type curr)
{
  Side const side = move_effect_journal[curr].u.duellist.side;
  square const from = move_effect_journal[curr].u.duellist.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  duellists[side] = from;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo remembering a duellist
 * @param curr identifies the adjustment effect
 */
void move_effect_journal_redo_remember_duellist(move_effect_journal_index_type curr)
{
  Side const side = move_effect_journal[curr].u.duellist.side;
  square const to = move_effect_journal[curr].u.duellist.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  duellists[side] = to;

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
void duellists_remember_duellist_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    remember_duellist(slices[si].starter,pos);
    pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_duellists(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STDuellistsRememberDuellist);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
