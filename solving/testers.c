#include "solving/testers.h"
#include "stipulation/fork.h"
#include "solving/has_solution_type.h"
#include "stipulation/if_then_else.h"
#include "stipulation/testing_pipe.h"
#include "stipulation/conditional_pipe.h"
#include "stipulation/branch.h"
#include "stipulation/binary.h"
#include "stipulation/boolean/true.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

/* Spin off slices for testing whethere there is a solution
 * @param si root slice of the stipulation
 */
void solving_spin_off_testers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);

  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_pipe,
                                                 &stip_spin_off_testers_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_branch,
                                                 &stip_spin_off_testers_pipe);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_fork,
                                                 &stip_spin_off_testers_fork);
  stip_structure_traversal_override_by_structure(&st,
                                                 slice_structure_leaf,
                                                 &stip_spin_off_testers_leaf);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_binary,
                                                &stip_spin_off_testers_binary);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_testing_pipe,
                                                &stip_spin_off_testers_testing_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_spin_off_testers_conditional_pipe);

  stip_structure_traversal_override_single(&st,STTemporaryHackFork,&stip_spin_off_testers_pipe_skip);

  stip_structure_traversal_override_single(&st,STPlaySuppressor,&stip_spin_off_testers_pipe_skip);
  stip_structure_traversal_override_single(&st,STIfThenElse,&stip_spin_off_testers_if_then_else);

  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
