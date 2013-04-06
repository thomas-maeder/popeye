#include "conditions/republican.h"
#include "stipulation/stipulation.h"
#include "pydata.h"
#include "pylang.h"
#include "stipulation/has_solution_type.h"
#include "solving/solve.h"
#include "pymsg.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "optimisations/goals/mate/neutralretractable.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/temporary_hacks.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <string.h>

static pilecase king_placement;

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

static boolean is_mate_square_dirty[maxply+1];

enum
{
  to_be_initialised = initsquare,
  king_not_placed = square_h8+1,
  no_place_for_king_left
};

static Goal republican_goal = { no_goal, initsquare };

static boolean is_mate_square(Side other_side, piece king_type)
{
  boolean result = false;

  if (e[king_square[other_side]]==vide)
  {
    TraceFunctionEntry(__func__);
    TraceEnumerator(Side,other_side,"");
    TraceFunctionParamListEnd();

    TraceSquare(king_square[other_side]);TraceText("\n");

    e[king_square[other_side]] = king_type;
    spec[king_square[other_side]] = BIT(Royal)|BIT(other_side);

    if (solve(slices[temporary_hack_mate_tester[other_side]].next2,slack_length)==slack_length)
      result = true;

    CLEARFL(spec[king_square[other_side]]);
    e[king_square[other_side]] = vide;

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
  piece const to_be_placed = other_side==White ? roib : roin;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  assert(republican_goal.type==goal_mate);

  king_square[other_side] =  king_placement[nbply]+1;
  ++nbpiece[to_be_placed];
  while (king_square[other_side]<=square_h8)
    if (is_mate_square(other_side,to_be_placed))
      break;
    else
      ++king_square[other_side];

  --nbpiece[to_be_placed];
  king_placement[nbply] = king_square[other_side];
  king_square[other_side] = initsquare;

  TraceSquare(king_placement[nbply]);TraceText("\n");

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
    VerifieMsg(IncompatibleRoyalSettings);
    result = false;
  }
  else if (CondFlag[masand]) /* TODO what else should we prohibit here? */
  {
    VerifieMsg(NoRepublicanWithConditionsDependingOnCheck);
    result = false;
  }
  else if (supergenre)
  {
    VerifieMsg(SuperCirceAndOthers);
    result = false;
  }
  else
  {
    Goal const goal = find_unique_goal(si);
    if (goal.type==no_goal)
    {
      VerifieMsg(StipNotSupported);
      result = false;
    }
    else
    {
      republican_goal = goal;
      OptFlag[sansrn] = true;
      OptFlag[sansrb] = true;
      optim_neutralretractable = false;
      disable_orthodox_mating_move_optimisation(nr_sides);
      supergenre = true;
      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* Write the Republican Chess diagram caption
 * @param CondLine string containing the condition caption
 * @param lineLength size of array CondLine
 */
void republican_write_diagram_caption(char CondLine[], size_t lineLength)
{
  strncat(CondLine, "    ", lineLength);
  if (RepublicanType==republican_type1)
    strncat(CondLine, VariantTypeString[UserLanguage][Type1], lineLength);
  if (RepublicanType==republican_type2)
    strncat(CondLine, VariantTypeString[UserLanguage][Type2], lineLength);
}

/* Place the opposite king as part of playing a move
 * @param moving side at the move
 */
static void place_king(Side moving)
{
  Side const other_side = advers(moving);
  piece const king_type = other_side==White ? roib : roin;
  Flags const king_flags = all_royals_flags|BIT(Royal)|BIT(other_side);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,moving,"");
  TraceFunctionParamListEnd();

  move_effect_journal_do_piece_addition(move_effect_reason_republican_king_insertion,
                                        king_placement[nbply],
                                        king_type,
                                        king_flags);

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
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_republican_king_placers(slice_index si)
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
  if (RepublicanType==republican_type1)
    stip_structure_traversal_override_single(&st,
                                             STGoalReachedTester,
                                             &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  if (RepublicanType==republican_type1)
    insert_type1_dead_end(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void determine_king_placement(Side trait_ply)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,trait_ply,"");
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
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
stip_length_type republican_king_placer_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (king_square[advers(slices[si].starter)]==vide)
  {
    determine_king_placement(slices[si].starter);

    if (king_placement[nbply]==king_not_placed)
    {
      result = solve(slices[si].next1,n);
      king_placement[nbply] = to_be_initialised;
    }
    else
    {
      place_king(slices[si].starter);
      result = solve(slices[si].next1,n);

      if (!post_move_iteration_locked[nbply])
      {
        is_mate_square_dirty[nbply] = true;
        lock_post_move_iterations();
      }
    }

    prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];
  }
  else
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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type republican_type1_dead_end_solve(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (king_placement[nbply]==king_not_placed)
    result = solve(slices[si].next1,n);
  else
    /* defender has inserted the attacker's king - no use to go on */
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
