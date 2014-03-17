#include "conditions/circe/parachute.h"
#include "conditions/circe/circe.h"
#include "conditions/haunted_chess.h"
#include "solving/observation.h"
#include "solving/check.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"

#include "debugging/trace.h"
#include "debugging/assert.h"

#include <string.h>

static void move_effect_journal_do_circe_parachute_remember(move_effect_reason_type reason,
                                                            square sq_rebirth)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceSquare(sq_rebirth);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_remember_parachuted;
  top_elmt->reason = reason;
  top_elmt->u.handle_ghost.ghost.on = sq_rebirth;
  top_elmt->u.handle_ghost.ghost.walk = get_walk_of_piece_on_square(sq_rebirth);
  top_elmt->u.handle_ghost.ghost.flags = spec[sq_rebirth];
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  underworld_make_space(0);

  underworld[0] = top_elmt->u.handle_ghost.ghost;

  ++move_effect_journal_base[nbply+1];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_undo_circe_parachute_remember(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  underworld_lose_space(0);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_redo_circe_parachute_remember(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[curr];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  underworld_make_space(0);

  underworld[0] = top_elmt->u.handle_ghost.ghost;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_effect_journal_do_circe_volcanic_remember(move_effect_reason_type reason)
{
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_remember_volcanic;
  top_elmt->reason = reason;
  top_elmt->u.handle_ghost.ghost.on = context->rebirth_square;
  top_elmt->u.handle_ghost.ghost.walk = context->reborn_walk;
  top_elmt->u.handle_ghost.ghost.flags = context->reborn_spec;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  TraceSquare(top_elmt->u.handle_ghost.ghost.on);
  TracePiece(top_elmt->u.handle_ghost.ghost.walk);
  TraceValue("%u\n",GetPieceId(top_elmt->u.handle_ghost.ghost.flags));

  underworld_make_space(nr_ghosts);

  underworld[nr_ghosts-1] = top_elmt->u.handle_ghost.ghost;

  ++move_effect_journal_base[nbply+1];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_undo_circe_volcanic_remember(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  underworld_lose_space(nr_ghosts-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_redo_circe_volcanic_remember(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[curr];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  underworld_make_space(nr_ghosts);

  underworld[nr_ghosts-1] = top_elmt->u.handle_ghost.ghost;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_parachute_remember_solve(slice_index si,
                                                stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  square const sq_rebirth = context->rebirth_square;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_do_circe_parachute_remember(move_effect_reason_assassin_circe_rebirth,
                                                  sq_rebirth);
  move_effect_journal_do_piece_removal(move_effect_reason_assassin_circe_rebirth,
                                       sq_rebirth);
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_volcanic_remember_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_do_circe_volcanic_remember(move_effect_reason_volcanic_remember);

  ++circe_rebirth_context_stack_pointer;
  result = solve(slices[si].next1,n);
  --circe_rebirth_context_stack_pointer;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_parachute_uncoverer_solve(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  unsigned int i = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  while (i<nr_ghosts)
  {
    square const from = underworld[i].on;

    if (is_square_empty(from))
    {
      move_effect_journal_do_piece_readdition(move_effect_reason_volcanic_uncover,
                                              from,underworld[i].walk,underworld[i].flags);
      move_effect_journal_do_forget_ghost(i);
    }
    else
      ++i;
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
