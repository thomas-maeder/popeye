#include "conditions/republican.h"
#include "stipulation/stipulation.h"
#include "pieces/pieces.h"
#include "options/options.h"
#include "conditions/conditions.h"
#include "input/plaintext/language.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/pipe.h"
#include "solving/conditional_pipe.h"
#include "solving/temporary_hacks.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "output/plaintext/message.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "optimisations/detect_retraction.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "output/plaintext/language_dependant.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <string.h>

ConditionNumberedVariantType RepublicanType;

static square king_placement[maxply+1];

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

static boolean is_mate_square_dirty[maxply+1];

enum
{
  to_be_initialised = initsquare,
  king_not_placed = square_h8+1,
  no_place_for_king_left
};

static Goal republican_goal = { no_goal, initsquare };

static boolean is_mate_square(Side other_side)
{
  boolean result = false;

  if (is_square_empty(being_solved.king_square[other_side]))
  {
    TraceFunctionEntry(__func__);
    TraceEnumerator(Side,other_side);
    TraceFunctionParamListEnd();

    TraceSquare(being_solved.king_square[other_side]);
    TraceEOL();

    occupy_square(being_solved.king_square[other_side],King,BIT(Royal)|BIT(other_side));

    if (conditional_pipe_solve_delegate(temporary_hack_mate_tester[other_side])
        ==previous_move_has_solved)
      result = true;

    empty_square(being_solved.king_square[other_side]);

    TraceFunctionExit(__func__);
    TraceFunctionResult("%u",result);
    TraceFunctionResultEnd();
  }

  return result;
}

/* Find a square for the opposite king
 * @param side side looking for a square for the opposite king
 */
static void advance_mate_square(Side side)
{
  Side const other_side = advers(side);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  assert(republican_goal.type==goal_mate);

  being_solved.king_square[other_side] = king_placement[nbply]+1;
  ++being_solved.number_of_pieces[other_side][King];
  while (being_solved.king_square[other_side]<=square_h8)
    if (is_mate_square(other_side))
      break;
    else
      ++being_solved.king_square[other_side];

  --being_solved.number_of_pieces[other_side][King];
  king_placement[nbply] = being_solved.king_square[other_side];
  being_solved.king_square[other_side] = initsquare;

  TraceSquare(king_placement[nbply]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Perform the necessary verification steps for solving a Republican
 * Chess problem
 * @param si identifies root slice of stipulation
 * @return true iff verification passed
 */
boolean republican_verifie_position(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (CondFlag[dynasty] || CondFlag[losingchess] || CondFlag[extinction])
  {
    output_plaintext_verifie_message(IncompatibleRoyalSettings);
    result = false;
  }
  else if (CondFlag[masand]) /* TODO what else should we prohibit here? */
  {
    output_plaintext_verifie_message(NoRepublicanWithConditionsDependingOnCheck);
    result = false;
  }
  else
  {
    Goal const goal = find_unique_goal(si);
    if (goal.type==no_goal)
    {
      output_plaintext_verifie_message(StipNotSupported);
      result = false;
    }
    else
    {
      republican_goal = goal;
      OptFlag[sansrn] = true;
      OptFlag[sansrb] = true;
      disable_orthodox_mating_move_optimisation(nr_sides);
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* Place the opposite king as part of playing a move
 * @param moving side at the move
 */
static void place_king(Side moving)
{
  Side const other_side = advers(moving);
  Flags king_flags = all_royals_flags|BIT(Royal)|BIT(other_side);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,moving);
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_creation(move_effect_reason_republican_king_insertion,
                                        king_placement[nbply],
                                        King,
                                        king_flags,
                                        trait[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_defense(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (st->context==stip_traversal_context_defense)
  {
    slice_index const prototype = alloc_pipe(STRepublicanType1DeadEnd);
    defense_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_type1_dead_end(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMove,&instrument_defense);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STRepublicanKingPlacer);
    move_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_republican_king_placers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STTemporaryHackFork,
                                           &stip_traverse_structure_children_pipe);
  if (RepublicanType==ConditionType1)
    stip_structure_traversal_override_single(&st,
                                             STGoalReachedTester,
                                             &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  if (RepublicanType==ConditionType1)
    insert_type1_dead_end(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void determine_king_placement(Side trait_ply)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply);
  TraceFunctionParamListEnd();

  if (!post_move_am_i_iterating(&prev_post_move_iteration_id[nbply]))
  {
    king_placement[nbply] = square_a1-1;
    is_mate_square_dirty[nbply] = true;
  }

  if (is_mate_square_dirty[nbply])
  {
    advance_mate_square(trait_ply);
    is_mate_square_dirty[nbply] = false;
  }

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
void republican_king_placer_solve(slice_index si)
{
  move_effect_journal_index_type const save_horizon = king_square_horizon;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  update_king_squares();

  if (being_solved.king_square[advers(SLICE_STARTER(si))]==initsquare)
  {
    determine_king_placement(SLICE_STARTER(si));

    if (king_placement[nbply]==king_not_placed)
    {
      pipe_solve_delegate(si);
      king_placement[nbply] = to_be_initialised;
    }
    else
    {
      place_king(SLICE_STARTER(si));
      pipe_solve_delegate(si);

      if (!post_move_iteration_is_locked(&prev_post_move_iteration_id[nbply]))
      {
        is_mate_square_dirty[nbply] = true;
        post_move_iteration_lock(&prev_post_move_iteration_id[nbply]);
      }
    }
  }
  else
    pipe_solve_delegate(si);

  king_square_horizon = save_horizon;

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
void republican_type1_dead_end_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* defender has inserted the attacker's king - no use to go on */
  pipe_this_move_doesnt_solve_if(si,king_placement[nbply]!=king_not_placed);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
