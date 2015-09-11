#include "platform/timer.h"
#include "solving/pipe.h"

void timer_starter_solve(slice_index si)
{
  StartTimer();
  pipe_solve_delegate(si);
}
