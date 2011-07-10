#include "stipulation/temporary_hacks.h"
#include "pybrafrk.h"
#include "pymovein.h"
#include "pypipe.h"
#include "stipulation/branch.h"
#include "stipulation/proxy.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/mate/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/help_play/branch.h"
#include "solving/legal_move_counter.h"
#include "trace.h"

slice_index temporary_hack_mate_tester[nr_sides];
slice_index temporary_hack_immobility_tester[nr_sides];
slice_index temporary_hack_exclusive_mating_move_counter[nr_sides];

static void swap_colors(slice_index (*testers)[nr_sides])
{
  slice_index const tmp = (*testers)[White];
  (*testers)[White] = (*testers)[Black];
  (*testers)[Black] = tmp;
}

void temporary_hacks_swap_colors(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  swap_colors(&temporary_hack_mate_tester);
  swap_colors(&temporary_hack_immobility_tester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index make_mate_tester_fork(Side side)
{
  Goal const mate_goal = { goal_mate, initsquare };
  slice_index const mate_tester = alloc_goal_mate_reached_tester_system();
  slice_index const result = alloc_goal_reached_tester_slice(mate_goal,mate_tester);
  dealloc_slice(slices[result].u.pipe.next);
  stip_impose_starter(result,side);
  return result;
}

static slice_index make_mating_move_counter_fork(Side side)
{
  slice_index result;
  Goal const dummy_goal = { no_goal, initsquare };
  slice_index const proxy_branch = alloc_proxy_slice();
  slice_index const proxy_to_goal = alloc_proxy_slice();
  Goal const goal = { goal_mate, initsquare };
  slice_index const tester_system = alloc_goal_mate_reached_tester_system();
  slice_index const tester_slice = alloc_goal_reached_tester_slice(goal,tester_system);
  slice_index const help = alloc_help_branch(slack_length_help+1,
                                             slack_length_help+1);
  slice_index const counter_proto = alloc_legal_move_counter_slice();
  slice_index const unsuspender_proto = alloc_pipe(STExclusiveChessUnsuspender);
  leaf_branch_insert_slices(tester_slice,&counter_proto,1);
  link_to_branch(proxy_to_goal,tester_slice);
  link_to_branch(proxy_branch,help);
  help_branch_set_end_goal(help,proxy_to_goal,1);
  help_branch_insert_slices(help,&unsuspender_proto,1);
  result = alloc_branch_fork(STExclusiveChessMatingMoveCounter,proxy_branch);
  stip_impose_starter(result,side);
  return result;
}

static slice_index make_immobility_tester_fork(Side side)
{
  slice_index const result = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);
  stip_impose_starter(result,side);
  return result;
}

void insert_temporary_hacks(slice_index root_slice)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const entry_point = alloc_branch_fork(STTemporaryHackFork,proxy);

    slice_index const inverter = alloc_move_inverter_slice();

    pipe_link(proxy,alloc_true_slice());

    temporary_hack_mate_tester[Black] = make_mate_tester_fork(Black);
    temporary_hack_mate_tester[White] = make_mate_tester_fork(White);

    temporary_hack_immobility_tester[Black] = make_immobility_tester_fork(Black);
    temporary_hack_immobility_tester[White] = make_immobility_tester_fork(White);

    temporary_hack_exclusive_mating_move_counter[Black] = make_mating_move_counter_fork(Black);
    temporary_hack_exclusive_mating_move_counter[White] = make_mating_move_counter_fork(White);

    pipe_append(root_slice,entry_point);

    pipe_append(proxy,temporary_hack_mate_tester[White]);
    pipe_append(temporary_hack_mate_tester[White],
                temporary_hack_immobility_tester[White]);
    pipe_append(temporary_hack_immobility_tester[White],
                temporary_hack_exclusive_mating_move_counter[White]);
    pipe_append(temporary_hack_exclusive_mating_move_counter[White],inverter);

    pipe_append(inverter,temporary_hack_mate_tester[Black]);
    pipe_append(temporary_hack_mate_tester[Black],
                temporary_hack_immobility_tester[Black]);
    pipe_append(temporary_hack_immobility_tester[Black],
                temporary_hack_exclusive_mating_move_counter[Black]);

    if (slices[root_slice].starter==Black)
      pipe_append(proxy,alloc_move_inverter_slice());
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
