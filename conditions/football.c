#include "conditions/football.h"
#include "conditions/singlebox/type1.h"
#include "conditions/conditions.h"
#include "pieces/pieces.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/pawns/promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

piece_walk_type current_football_substitution[maxply+1];
boolean is_football_substitute[nr_piece_walks];
piece_walk_type next_football_substitute[nr_piece_walks];
boolean football_are_substitutes_limited;

static piece_walk_type const *bench[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

/* Initialise the substitutes' bench for the current twin
 */
void init_football_substitutes(void)
{
  piece_walk_type first_candidate;
  piece_walk_type substitute_index = Empty;
  piece_walk_type p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

 if (CondFlag[losingchess] || CondFlag[dynasty] || CondFlag[extinction])
    first_candidate = King;
  else if ((CondFlag[singlebox] && SingleBoxType!=ConditionType1) || CondFlag[football])
    first_candidate = Pawn;
  else
    first_candidate = Queen;

  if (!football_are_substitutes_limited)
    for (p = first_candidate; p<nr_piece_walks; ++p)
      if (piece_walk_may_exist[p])
        is_football_substitute[p] = ((p!=King
                                      || CondFlag[losingchess]
                                      || CondFlag[dynasty]
                                      || CondFlag[extinction])
                                     && p!=Dummy
                                     && (p==Pawn || !is_pawn(p)));

  for (p = first_candidate; p<nr_piece_walks; ++p)
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

static piece_walk_type const *get_bench(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
  PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
  square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                            moving_id,
                                                                            sq_arrival);

  if (!TSTFLAG(being_solved.spec[pos],Royal)
      && (pos%onerow==left_file || pos%onerow==right_file))
  {
    piece_walk_type const p = get_walk_of_piece_on_square(pos);
    piece_walk_type tmp = next_football_substitute[Empty];

    /* ensure moving piece is on list to allow null (= non-) substitutions */
    if (tmp!=p)
    {
      /* remove old head-of-list if not part of standard set */
      if (!is_football_substitute[tmp])
        next_football_substitute[Empty] = next_football_substitute[tmp];

      /* add moving piece to head-of-list if not already part of standard set */
      if (!is_football_substitute[p])
      {
        next_football_substitute[p] = next_football_substitute[Empty];
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
  current_football_substitution[nbply] = bench[nbply]==0 ? Empty : bench[nbply][Empty];
  TraceWalk(current_football_substitution[nbply]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean advance_substitution(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  current_football_substitution[nbply] = bench[nbply][current_football_substitution[nbply]];
  TraceWalk(current_football_substitution[nbply]);
  TraceEOL();
  result = current_football_substitution[nbply]!=Empty;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
void football_chess_substitutor_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
    init_substitution();

  if (current_football_substitution[nbply]==Empty)
    pipe_solve_delegate(si);
  else
  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);

    if (get_walk_of_piece_on_square(pos)!=current_football_substitution[nbply])
      move_effect_journal_do_walk_change(move_effect_reason_football_chess_substitution,
                                          pos,
                                          current_football_substitution[nbply]);

    pipe_solve_delegate(si);

    if (!post_move_iteration_locked[nbply])
    {
      if (advance_substitution())
        lock_post_move_iterations();
    }
  }

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with promotee markers
 */
void solving_insert_football_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STFootballChessSubsitutor);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
