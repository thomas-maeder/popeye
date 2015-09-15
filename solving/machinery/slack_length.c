#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "stipulation/structure_traversal.h"
#include "debugging/trace.h"

stip_length_type slack_length = 0;


static void adjust_branch(slice_index si, stip_structure_traversal *st)
{
  int const * const diff = st->param;

  stip_traverse_structure_children(si,st);

  SLICE_U(si).branch.length += *diff;
  SLICE_U(si).branch.min_length += *diff;
}

void adjust_slack_length(slice_index si)
{
  int diff = previous_move_has_solved-slack_length;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&diff);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &adjust_branch);
  stip_traverse_structure(si,&st);

  TraceStipulation(si);

  slack_length = previous_move_has_solved;

  pipe_solve_delegate(si);

  slack_length = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
