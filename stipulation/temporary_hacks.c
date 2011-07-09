#include "stipulation/temporary_hacks.h"
#include "pybrafrk.h"
#include "pymovein.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/mate/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "trace.h"

slice_index temporary_hack_mate_tester[nr_sides];

slice_index temporary_hack_immobility_tester[nr_sides];

void temporary_hacks_swap_colors(void)
{
  slice_index tmp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  tmp = temporary_hack_mate_tester[White];
  temporary_hack_mate_tester[White] = temporary_hack_mate_tester[Black];
  temporary_hack_mate_tester[Black] = tmp;

  tmp = temporary_hack_immobility_tester[White];
  temporary_hack_immobility_tester[White] = temporary_hack_immobility_tester[Black];
  temporary_hack_immobility_tester[Black] = tmp;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

    slice_index const mate_tester_black = alloc_goal_mate_reached_tester_system();
    slice_index const mate_tester_white = alloc_goal_mate_reached_tester_system();

    Goal const mate_goal = { goal_mate, initsquare };

    temporary_hack_mate_tester[Black] = alloc_goal_reached_tester_slice(mate_goal,mate_tester_black);
    dealloc_slice(slices[temporary_hack_mate_tester[Black]].u.pipe.next);
    stip_impose_starter(temporary_hack_mate_tester[Black],Black);

    temporary_hack_mate_tester[White] = alloc_goal_reached_tester_slice(mate_goal,mate_tester_white);
    dealloc_slice(slices[temporary_hack_mate_tester[White]].u.pipe.next);
    stip_impose_starter(temporary_hack_mate_tester[White],White);

    temporary_hack_immobility_tester[Black] = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);
    stip_impose_starter(temporary_hack_immobility_tester[Black],Black);

    temporary_hack_immobility_tester[White] = alloc_goal_immobile_reached_tester_slice(goal_applies_to_starter);
    stip_impose_starter(temporary_hack_immobility_tester[White],White);

    pipe_append(root_slice,entry_point);
    pipe_append(proxy,temporary_hack_mate_tester[White]);
    if (slices[root_slice].starter==Black)
      pipe_append(proxy,alloc_move_inverter_slice());
    pipe_append(temporary_hack_mate_tester[White],temporary_hack_immobility_tester[White]);
    pipe_append(temporary_hack_immobility_tester[White],inverter);
    pipe_append(inverter,temporary_hack_mate_tester[Black]);
    pipe_append(temporary_hack_mate_tester[Black],temporary_hack_immobility_tester[Black]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
