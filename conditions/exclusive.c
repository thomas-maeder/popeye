#include "conditions/exclusive.h"
#include "stipulation/stipulation.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "pymsg.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/branch.h"
#include "solving/legal_move_counter.h"
#include "debugging/trace.h"

#include <assert.h>

static Goal exclusive_goal;

static boolean is_reaching_goal_allowed[maxply+1];

/* Perform the necessary verification steps for solving an Exclusive
 * Chess problem
 * @param si identifies root slice of stipulation
 * @return true iff verification passed
 */
boolean exclusive_verifie_position(slice_index si)
{
  boolean result;
  Goal goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* we have to know which goal has to be reached in a dual-free
   * way */
  goal = find_unique_goal(si);
  if (goal.type==no_goal)
  {
    VerifieMsg(StipNotSupported);
    result = false;
  }
  else
  {
    exclusive_goal = goal;
    optim_neutralretractable = false;
    add_ortho_mating_moves_generation_obstacle();
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the current position is illegal because of
 * Exclusive Chess rules
 * @return true iff the position is legal
 */
static boolean pos_legal(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(exclusive_goal.type==goal_mate);

  if (nbply>maxply-1)
    FtlMsg(ChecklessUndecidable);

  result = (is_reaching_goal_allowed[nbply]
            || attack(slices[temporary_hack_mate_tester[advers(trait[nbply])]].next2,length_unspecified)!=has_solution);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Do preparations before generating moves for a side in an Exclusive
 * Chess problem
 * @param side side for which to generate moves
 */
void exclusive_init_genmove(Side side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  assert(exclusive_goal.type==goal_mate);

  CondFlag[exclusive] = false;
  remove_ortho_mating_moves_generation_obstacle();

  /* avoid concurrent counts */
  assert(legal_move_counter_count[nbply]==0);

  /* stop counting once we have found >1 mating moves */
  legal_move_counter_interesting[nbply] = 1;

  attack(slices[temporary_hack_exclusive_mating_move_counter[side]].next2,length_unspecified);

  is_reaching_goal_allowed[nbply] = legal_move_counter_count[nbply]<2;
  TraceValue("%u",legal_move_counter_count[nbply+1]);
  TraceValue("%u\n",is_reaching_goal_allowed[nbply]);

  /* clean up after ourselves */
  legal_move_counter_count[nbply] = 0;

  add_ortho_mating_moves_generation_obstacle();
  CondFlag[exclusive] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type exclusive_chess_unsuspender_attack(slice_index si,
                                                    stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  add_ortho_mating_moves_generation_obstacle();
  CondFlag[exclusive] = true;
  is_reaching_goal_allowed[nbply] = true;
  result = attack(slices[si].next1,n);
  is_reaching_goal_allowed[nbply] = false;
  CondFlag[exclusive] = false;
  remove_ortho_mating_moves_generation_obstacle();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void remove_guard(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const guard = branch_find_slice(STSelfCheckGuard,
                                                slices[si].next2,
                                                stip_traversal_context_intro);
    assert(guard!=no_slice);
    pipe_remove(guard);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* When counting mating moves, it is not necessary to detect self-check in moves
 * that don't deliver mate; remove the slices that would detect these
 * self-checks
 * @param si identifies slice where to start
 */
void optimise_away_unnecessary_selfcheckguards(slice_index si)
{
  stip_structure_traversal st;
  boolean in_constraint = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&in_constraint);
  stip_structure_traversal_override_single(&st,
                                           STExclusiveChessMatingMoveCounter,
                                           &remove_guard);
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
    slice_index const prototype = alloc_pipe(STExclusiveChessLegalityTester);
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_exclusive_chess_legality_testers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type exclusive_chess_legality_tester_attack(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (!CondFlag[exclusive] || pos_legal())
    result = attack(next,n);
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type exclusive_chess_legality_tester_defend(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (!CondFlag[exclusive] || pos_legal())
    result = defend(next,n);
  else
    result = slack_length-1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
