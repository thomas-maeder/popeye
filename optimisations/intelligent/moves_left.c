#include "optimisations/intelligent/moves_left.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "solving/machinery/slack_length.h"
#include "solving/moves_traversal.h"
#include "solving/pipe.h"
#include "solving/castling.h"
#include "position/position.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

unsigned int MovesLeft[nr_sides];

/* Allocate a STIntelligentMovesLeftInitialiser slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_moves_left_initialiser(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STIntelligentMovesLeftInitialiser);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void moves_left_move(slice_index si, stip_moves_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++MovesLeft[SLICE_STARTER(si)];

  TraceValue("%u",MovesLeft[White]);
  TraceValue("%u",MovesLeft[Black]);
  TraceEOL();

  stip_traverse_moves_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Calculate the number of moves of each side, starting at the root
 * slice.
 * @param si identifies starting slice
 * @param n length of the solution(s) we are looking for (without slack)
 * @param full_length full length of the initial branch (without slack)
 */
static void init_moves_left(slice_index si,
                            stip_length_type n,
                            stip_length_type full_length)
{
  stip_moves_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",full_length);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  MovesLeft[Black] = 0;
  MovesLeft[White] = 0;

  stip_moves_traversal_init(&st,&n);
  st.context = stip_traversal_context_help;
  stip_moves_traversal_set_remaining(&st,n,full_length);
  stip_moves_traversal_override_single(&st,STForEachAttack,&moves_left_move);
  stip_moves_traversal_override_single(&st,STAnd,&stip_traverse_moves_binary_operand1);
  stip_traverse_moves(si,&st);

  TraceValue("%u",MovesLeft[White]);
  TraceValue("%u",MovesLeft[Black]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void delegate(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_moves_left(si,solve_nr_remaining-slack_length,solve_nr_remaining-slack_length);
  pipe_this_move_doesnt_solve_if(si,MovesLeft[White]+MovesLeft[Black]==0);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean is_mated[nr_sides];
    goal_type goal;
} mated_side_state_type;

static void remember_goaled_side(slice_index si, stip_structure_traversal *st)
{
  mated_side_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->is_mated[SLICE_STARTER(si)] = true;
  state->goal = SLICE_U(si).goal_handler.goal.type;
  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static Side goaled_side(slice_index si)
{
  Side result;
  stip_structure_traversal st;
  mated_side_state_type state = { { false, false }, no_goal };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,
                                           STAnd,
                                           &stip_traverse_structure_binary_operand1);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTester,
                                           &remember_goaled_side);
  stip_traverse_structure(si,&st);

  assert(state.is_mated[White] || state.is_mated[Black]);
  assert(!(state.is_mated[White] && state.is_mated[Black]));

  if (state.goal==goal_mate || state.goal==goal_stale)
    result = state.is_mated[White] ? White : Black;
  else
    result = no_side;

  TraceFunctionExit(__func__);
  TraceEnumerator(Side,result);
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
void intelligent_moves_left_initialiser_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* the mate and stalemate machineries rely on Black being (stale)mated */
  if (goaled_side(si)==White)
  {
    solving_impose_starter(si,advers(SLICE_STARTER(si)));
    swap_sides();
    reflect_position();
    delegate(si);
    reflect_position();
    swap_sides();
    solving_impose_starter(si,advers(SLICE_STARTER(si)));
  }
  else
    delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
