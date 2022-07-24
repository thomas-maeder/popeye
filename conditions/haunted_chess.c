#include "conditions/haunted_chess.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "position/effects/piece_readdition.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/move.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

void move_effect_journal_do_forget_ghost(underworld_index_type const summoned)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_forget_ghost,move_effect_reason_summon_ghost);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",summoned);
  TraceFunctionParamListEnd();

  entry->u.handle_ghost.pos = summoned;
  entry->u.handle_ghost.ghost = underworld[summoned];

  underworld_lose_space(summoned);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_effect_journal_undo_forget_ghost(move_effect_journal_entry_type const *entry)
{
  underworld_index_type const ghost_pos = entry->u.handle_ghost.pos;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  underworld_make_space(ghost_pos);

  underworld[ghost_pos] = entry->u.handle_ghost.ghost;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_effect_journal_redo_forget_ghost(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  underworld_lose_space(entry->u.handle_ghost.pos);

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
void haunted_chess_ghost_summoner_solve(slice_index si)
{
  underworld_index_type idx_ghost = nr_ghosts;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  while (idx_ghost>0)
  {
    --idx_ghost;
    if (is_square_empty(underworld[idx_ghost].on))
    {
      move_effect_journal_do_piece_readdition(move_effect_reason_summon_ghost,
                                              underworld[idx_ghost].on,
                                              underworld[idx_ghost].walk,
                                              underworld[idx_ghost].flags,
                                              no_side);

      move_effect_journal_do_forget_ghost(idx_ghost);
    }
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_do_remember_ghost(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
  piece_walk_type const removed = move_effect_journal[capture].u.piece_removal.walk;
  Flags const removedspec = move_effect_journal[capture].u.piece_removal.flags;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(top+1<move_effect_journal_size);
  assert(move_effect_journal[capture].type==move_effect_piece_removal);

  top_elmt->type = move_effect_remember_ghost;
  top_elmt->reason = move_effect_reason_regular_capture;
  top_elmt->u.handle_ghost.pos = nr_ghosts;
  top_elmt->u.handle_ghost.ghost.walk = removed;
  top_elmt->u.handle_ghost.ghost.flags = removedspec;
  top_elmt->u.handle_ghost.ghost.on = sq_capture;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu",top_elmt->id);
  TraceEOL();
#endif

  ++move_effect_journal_base[nbply+1];

  underworld[nr_ghosts] = top_elmt->u.handle_ghost.ghost;
  ++nr_ghosts;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_effect_journal_undo_remember_ghost(move_effect_journal_entry_type const *entry)
{
  underworld_index_type const ghost_pos = entry->u.handle_ghost.pos;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  underworld_lose_space(ghost_pos);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_effect_journal_redo_remember_ghost(move_effect_journal_entry_type const *entry)
{
  underworld_index_type const ghost_pos = entry->u.handle_ghost.pos;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  underworld_make_space(ghost_pos);

  underworld[ghost_pos] = entry->u.handle_ghost.ghost;

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
void haunted_chess_ghost_rememberer_solve(slice_index si)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type!=move_effect_no_piece_removal)
  {
    square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
    underworld_index_type const preempted_idx = underworld_find_last(sq_capture);

    if (preempted_idx!=ghost_not_found)
      move_effect_journal_do_forget_ghost(preempted_idx);

    move_effect_journal_do_remember_ghost();
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Make sure the effects of remembering and forgetting ghosts are properly
 * undone and redone
 */
void haunted_chess_initialise_move_doers(void)
{
  move_effect_journal_set_effect_doers(move_effect_forget_ghost,
                                       &move_effect_journal_undo_forget_ghost,
                                       &move_effect_journal_redo_forget_ghost);

  move_effect_journal_set_effect_doers(move_effect_remember_ghost,
                                       &move_effect_journal_undo_remember_ghost,
                                       &move_effect_journal_redo_remember_ghost);
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_haunted_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  haunted_chess_initialise_move_doers();

  stip_instrument_moves(si,STHauntedChessGhostRememberer);
  stip_instrument_moves(si,STHauntedChessGhostSummoner);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
