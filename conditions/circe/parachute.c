#include "conditions/circe/parachute.h"
#include "conditions/circe/circe.h"
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

enum { covered_capacity = 32 };

static move_effect_journal_index_type covered_pieces[covered_capacity];

static unsigned int nr_covered_pieces = 0;

static void insert_at(unsigned int i,
                      move_effect_journal_index_type idx_removal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",i);
  TraceFunctionParam("%u",idx_removal);
  TraceFunctionParamListEnd();

  assert(i<=nr_covered_pieces);
  assert(nr_covered_pieces<covered_capacity);

  memmove(&covered_pieces[i+1],&covered_pieces[i],
          (nr_covered_pieces-i) * sizeof covered_pieces[0]);
  ++nr_covered_pieces;

  covered_pieces[i] = idx_removal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static move_effect_journal_index_type remove_at(unsigned int i)
{
  move_effect_journal_index_type const result = covered_pieces[i];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",i);
  TraceFunctionParamListEnd();

  assert(i<nr_covered_pieces);

  --nr_covered_pieces;
  memmove(&covered_pieces[i],&covered_pieces[i+1],
          (nr_covered_pieces-i) * sizeof covered_pieces[0]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
  top_elmt->u.piece_removal.from = sq_rebirth;
  top_elmt->u.piece_removal.removed = get_walk_of_piece_on_square(sq_rebirth);
  top_elmt->u.piece_removal.removedspec = spec[sq_rebirth];
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  insert_at(0,move_effect_journal_base[nbply+1]);

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

  remove_at(0);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_redo_circe_parachute_remember(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  insert_at(0,curr);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_effect_journal_do_circe_volcanic_remember(move_effect_reason_type reason)
{
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];
  square const sq_rebirth = context->rebirth_square;
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_remember_volcanic;
  top_elmt->reason = reason;
  top_elmt->u.piece_removal.from = sq_rebirth;
  top_elmt->u.piece_removal.removed = context->reborn_walk;
  top_elmt->u.piece_removal.removedspec = context->reborn_spec;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  TraceSquare(top_elmt->u.piece_removal.from);
  TracePiece(top_elmt->u.piece_removal.removed);
  TraceValue("%u\n",GetPieceId(top_elmt->u.piece_removal.removedspec));

  insert_at(nr_covered_pieces,move_effect_journal_base[nbply+1]);

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

  remove_at(nr_covered_pieces-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_redo_circe_volcanic_remember(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  insert_at(nr_covered_pieces,curr);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_effect_journal_do_circe_parachute_uncover(move_effect_reason_type reason,
                                                           unsigned int i)
{
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_base[nbply+1]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParam("%u",i);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_base[nbply+1]+1<move_effect_journal_size);

  top_elmt->type = move_effect_uncover_parachuted;
  top_elmt->reason = reason;
  top_elmt->u.uncovered.idx_uncovered = i;
  top_elmt->u.uncovered.idx_remember = remove_at(i);
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_base[nbply+1];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_undo_circe_parachute_uncover(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  unsigned int const idx_uncovered = curr_elmt->u.uncovered.idx_uncovered;
  move_effect_journal_index_type const idx_remember = curr_elmt->u.uncovered.idx_remember;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  insert_at(idx_uncovered,idx_remember);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void move_effect_journal_redo_circe_parachute_uncover(move_effect_journal_index_type curr)
{
  move_effect_journal_entry_type * const curr_elmt = &move_effect_journal[curr];
  unsigned int const idx_uncovered = curr_elmt->u.uncovered.idx_uncovered;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  remove_at(idx_uncovered);

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

  move_effect_journal_do_circe_volcanic_remember(move_effect_reason_assassin_circe_rebirth);
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
stip_length_type circe_parachute_uncoverer_solve(slice_index si,
                                                 stip_length_type n)
{
  stip_length_type result;
  unsigned int i = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nr_covered_pieces);

  while (i<nr_covered_pieces)
  {
    move_effect_journal_index_type const idx_remember = covered_pieces[i];
    move_effect_journal_entry_type const * const entry = &move_effect_journal[idx_remember];
    square const from = entry->u.piece_removal.from;

    assert(move_effect_journal[idx_remember].type==move_effect_remember_parachuted
           || move_effect_journal[idx_remember].type==move_effect_remember_volcanic);

    if (is_square_empty(from))
    {
      move_effect_journal_do_piece_readdition(move_effect_reason_rebirth_no_choice,
                                              from,
                                              entry->u.piece_removal.removed,
                                              entry->u.piece_removal.removedspec);
      move_effect_journal_do_circe_parachute_uncover(move_effect_reason_rebirth_no_choice,i);
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
