#include "conditions/football.h"
#include "stipulation/pipe.h"
#include "pydata.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

PieNam current_football_substitution[maxply+1];

static PieNam const *bench[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

/* Initialise the substitutes' bench for the current twin
 */
void init_football_substitutes(void)
{
  PieNam first_candidate;
  PieNam substitute_index = Empty;
  PieNam p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

 if (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction])
    first_candidate = King;
  else if ((CondFlag[singlebox] && SingleBoxType!=singlebox_type1) || CondFlag[football])
    first_candidate = Pawn;
  else
    first_candidate = Queen;

  if (!football_are_substitutes_limited)
    for (p = first_candidate; p<PieceCount; ++p)
      if (may_exist[p])
        is_football_substitute[p] = ((p!=King
                                      || CondFlag[losingchess]
                                      || CondFlag[dynasty]
                                      || CondFlag[extinction])
                                     && p!=Dummy
                                     && (p==Pawn || !is_pawn(p)));

  for (p = first_candidate; p<PieceCount; ++p)
    if (is_football_substitute[p])
    {
      next_football_substitute[substitute_index] = p;
      substitute_index = p;
    }
    else
      next_football_substitute[p] = Empty;

  next_football_substitute[substitute_index] = Empty;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static PieNam const *get_bench(void)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  if (sq_arrival!=king_square[Black] && sq_arrival!=king_square[White]
      && (sq_arrival%onerow==left_file || sq_arrival%onerow==right_file))
  {
    PieNam const p = get_walk_of_piece_on_square(sq_arrival);
    PieNam tmp = next_football_substitute[Empty];

    /* ensure moving piece is on list to allow null (= non-) substitutions */
    if (tmp!=p)
    {
      /* remove old head-of-list if not part of standard set */
      if (!is_football_substitute[tmp])
        next_football_substitute[Empty] = next_football_substitute[tmp];

      /* add moving piece to head-of-list if not already part of standard set */
      if (!is_football_substitute[p])
      {
        next_football_substitute[p] = next_football_substitute[vide];
        next_football_substitute[Empty] = p;
      }
    }

    return next_football_substitute;
  }
  else
    return NULL;
}

static void init_substitution(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  bench[nbply] = get_bench();
  current_football_substitution[nbply] = bench[nbply]==0 ? Empty : bench[nbply][vide];
  TracePiece(current_football_substitution[nbply]);TraceText("\n");

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean advance_substitution(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  current_football_substitution[nbply] = bench[nbply][current_football_substitution[nbply]];
  TracePiece(current_football_substitution[nbply]);TraceText("\n");
  result = current_football_substitution[nbply]!=Empty;

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
stip_length_type football_chess_substitutor_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
    init_substitution();

  if (current_football_substitution[nbply]==Empty)
    result = solve(slices[si].next1,n);
  else
  {
    square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

    if (get_walk_of_piece_on_square(sq_arrival)!=current_football_substitution[nbply])
      move_effect_journal_do_piece_change(move_effect_reason_football_chess_substitution,
                                          sq_arrival,
                                          current_football_substitution[nbply]);

    result = solve(slices[si].next1,n);

    if (!post_move_iteration_locked[nbply])
    {
      if (advance_substitution())
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with promotee markers
 */
void stip_insert_football_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STFootballChessSubsitutor);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
