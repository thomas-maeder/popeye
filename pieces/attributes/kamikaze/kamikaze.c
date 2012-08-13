#include "pieces/attributes/kamikaze/kamikaze.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/boolean/or.h"
#include "stipulation/boolean/true.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "conditions/anticirce/target_square_filter.h"
#include "conditions/anticirce/exchange_special.h"
#include "conditions/anticirce/exchange_filter.h"
#include "debugging/trace.h"

#include <assert.h>

static void instrument_goal_target_filter(slice_index si,
                                          stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    /* reusing the filter created for Anticirce */
    square const target = slices[si].u.goal_handler.goal.target;
    pipe_append(si,alloc_anticirce_target_square_filter_slice(target));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_goal_exchange_filter(slice_index si,
                                            stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const proxy_special = alloc_proxy_slice();
    /* reusing the special exchange detection created for Anticirce */
    slice_index const special = alloc_anticirce_exchange_special_slice();
    slice_index const leaf_special = alloc_true_slice();

    slice_index const proxy_filter = alloc_proxy_slice();
    /* reusing the special exchange filter created for Anticirce */
    slice_index const filter = alloc_anticirce_exchange_filter_slice();

    pipe_link(slices[si].prev,
              alloc_or_slice(proxy_filter,proxy_special));

    pipe_link(proxy_special,special);
    pipe_link(special,leaf_special);

    pipe_link(proxy_filter,filter);
    pipe_link(filter,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor goal_filter_inserters[] =
{
  { STGoalTargetReachedTester,   &instrument_goal_target_filter   },
  { STGoalExchangeReachedTester, &instrument_goal_exchange_filter }
};

enum
{
  nr_goal_filter_inserters = (sizeof goal_filter_inserters
                              / sizeof goal_filter_inserters[0])
};

static void insert_goal_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    goal_filter_inserters,
                                    nr_goal_filter_inserters);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_capturing_piece(void)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  if (TSTFLAG(spec[sq_arrival],Kamikaze) && pprise[nbply]!=vide)
  {
    --nbpiece[e[sq_arrival]];
    e[sq_arrival] = vide;
    spec[sq_arrival] = 0;
    jouearr[nbply] = vide;
  }
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type kamikaze_capturing_piece_remover_attack(slice_index si,
                                                         stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  remove_capturing_piece();
  result = attack(slices[si].next1,n);

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
stip_length_type kamikaze_capturing_piece_remover_defend(slice_index si,
                                                         stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  remove_capturing_piece();
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation with goal filter slices
 * @param si root of branch to be instrumented
 */
void stip_insert_kamikaze(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  insert_goal_filters(si);

  if (!anycirce)
    stip_instrument_moves(si,STKamikazeCapturingPieceRemover);

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
