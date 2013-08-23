#include "conditions/haunted_chess.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include <assert.h>
#include <string.h>

ghosts_type ghosts;
ghost_index_type nr_ghosts;

static ghost_index_type find_ghost(square pos)
{
  ghost_index_type current = nr_ghosts;
  ghost_index_type result = ghost_not_found;

  TraceFunctionEntry(__func__);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  while (current>0)
  {
    --current;
    TraceSquare(ghosts[current].on);
    TraceValue("%u\n",current);
    if (ghosts[current].on==pos)
    {
      result = current;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

static void make_space(ghost_index_type ghost_pos)
{
  memmove(ghosts+ghost_pos+1, ghosts+ghost_pos,
          (nr_ghosts-ghost_pos) * sizeof ghosts[0]);
  ++nr_ghosts;
}

static void lose_space(ghost_index_type ghost_pos)
{
  --nr_ghosts;
  memmove(ghosts+ghost_pos, ghosts+ghost_pos+1,
          (nr_ghosts-ghost_pos) * sizeof ghosts[0]);
}

static void move_effect_journal_do_forget_ghost(ghost_index_type const summoned)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",summoned);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_forget_ghost;
  top_elmt->reason = move_effect_reason_summon_ghost;
  top_elmt->u.handle_ghost.ghost_pos = summoned;
  top_elmt->u.handle_ghost.ghost = ghosts[summoned].ghost;
  top_elmt->u.handle_ghost.flags = ghosts[summoned].flags;
  top_elmt->u.handle_ghost.on = ghosts[summoned].on;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  lose_space(summoned);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_undo_forget_ghost(move_effect_journal_index_type curr)
{
  ghost_index_type const ghost_pos = move_effect_journal[curr].u.handle_ghost.ghost_pos;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  make_space(ghost_pos);

  ghosts[ghost_pos].ghost = move_effect_journal[curr].u.handle_ghost.ghost;
  ghosts[ghost_pos].flags = move_effect_journal[curr].u.handle_ghost.flags;
  ghosts[ghost_pos].on = move_effect_journal[curr].u.handle_ghost.on;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_redo_forget_ghost(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  lose_space(move_effect_journal[curr].u.handle_ghost.ghost_pos);

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
stip_length_type haunted_chess_ghost_summoner_solve(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  ghost_index_type const ghost_pos = find_ghost(sq_departure);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (ghost_pos!=ghost_not_found)
  {
    move_effect_journal_do_piece_readdition(move_effect_reason_summon_ghost,
                                            sq_departure,
                                            ghosts[ghost_pos].ghost,
                                            ghosts[ghost_pos].flags);

    move_effect_journal_do_forget_ghost(ghost_pos);
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void move_effect_journal_do_remember_ghost(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[top];
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.from;
  PieNam const removed = move_effect_journal[capture].u.piece_removal.removed;
  Flags const removedspec = move_effect_journal[capture].u.piece_removal.removedspec;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(top+1<move_effect_journal_size);
  assert(move_effect_journal[capture].type==move_effect_piece_removal);

  top_elmt->type = move_effect_remember_ghost;
  top_elmt->reason = move_effect_reason_regular_capture;
  top_elmt->u.handle_ghost.ghost_pos = nr_ghosts;
  top_elmt->u.handle_ghost.ghost = removed;
  top_elmt->u.handle_ghost.flags = removedspec;
  top_elmt->u.handle_ghost.on = sq_capture;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  ghosts[nr_ghosts].on = sq_capture;
  ghosts[nr_ghosts].ghost = removed;
  ghosts[nr_ghosts].flags = removedspec;
  ++nr_ghosts;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_undo_remember_ghost(move_effect_journal_index_type curr)
{
  ghost_index_type const ghost_pos = move_effect_journal[curr].u.handle_ghost.ghost_pos;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  lose_space(ghost_pos);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_redo_remember_ghost(move_effect_journal_index_type curr)
{
  ghost_index_type const ghost_pos = move_effect_journal[curr].u.handle_ghost.ghost_pos;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  make_space(ghost_pos);

  ghosts[ghost_pos].on = move_effect_journal[curr].u.handle_ghost.on;
  ghosts[ghost_pos].ghost = move_effect_journal[curr].u.handle_ghost.ghost;
  ghosts[ghost_pos].flags = move_effect_journal[curr].u.handle_ghost.flags;

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
stip_length_type haunted_chess_ghost_rememberer_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_effect_journal[capture].type==move_effect_no_piece_removal)
    result = solve(slices[si].next1,n);
  else
  {
    square const sq_capture = move_generation_stack[current_move[nbply]-1].capture;
    ghost_index_type const preempted_idx = find_ghost(sq_capture);

    if (preempted_idx!=ghost_not_found)
      move_effect_journal_do_forget_ghost(preempted_idx);

    move_effect_journal_do_remember_ghost();

    result = solve(slices[si].next1,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_haunted_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STHauntedChessGhostRememberer);
  stip_instrument_moves(si,STHauntedChessGhostSummoner);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
