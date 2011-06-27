#include "stipulation/temporary_hacks.h"
#include "pybrafrk.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/mate/reached_tester.h"
#include "trace.h"

slice_index temporary_hack_mate_tester[nr_sides];

void insert_temporary_hacks(slice_index root_slice)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root_slice);
  TraceFunctionParamListEnd();

  {
    Goal const mate_goal = { goal_mate, initsquare };
    slice_index const mate_tester_black = alloc_goal_mate_reached_tester_system();
    slice_index const black_mate_fork = alloc_goal_reached_tester_slice(mate_goal,mate_tester_black);
    slice_index const inverter = alloc_move_inverter_slice();
    slice_index const mate_tester_white = alloc_goal_mate_reached_tester_system();
    slice_index const white_mate_fork = alloc_goal_reached_tester_slice(mate_goal,mate_tester_white);
    slice_index const proxy = alloc_proxy_slice();
    slice_index const entry_point = alloc_branch_fork(STTemporaryHackFork,proxy);
    dealloc_slice(slices[black_mate_fork].u.pipe.next);
    dealloc_slice(slices[white_mate_fork].u.pipe.next);
    pipe_append(root_slice,entry_point);
    pipe_append(proxy,white_mate_fork);
    if (slices[root_slice].starter==Black)
      pipe_append(proxy,alloc_move_inverter_slice());
    pipe_append(white_mate_fork,inverter);
    pipe_append(inverter,black_mate_fork);
    temporary_hack_mate_tester[White] = white_mate_fork;
    stip_impose_starter(temporary_hack_mate_tester[White],White);
    temporary_hack_mate_tester[Black] = black_mate_fork;
    stip_impose_starter(temporary_hack_mate_tester[Black],Black);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
