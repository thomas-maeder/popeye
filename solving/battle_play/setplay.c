#include "solving/battle_play/setplay.h"
#include "stipulation/slice_insertion.h"
#include "solving/machinery/slack_length.h"
#include "stipulation/battle_play/branch.h"
#include "solving/battle_play/try.h"
#include "debugging/trace.h"

#include "debugging/assert.h"

static void filter_output_mode(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_U(si).output_mode_selector.mode==output_mode_tree)
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_setplay_solvers_defense_adapter(slice_index si,
                                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level==structure_traversal_level_setplay
      && SLICE_U(si).branch.length>slack_length)
  {
    slice_index const prototypes[] =
    {
      alloc_refutations_allocator()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    slice_insertion_insert(si,prototypes,nr_prototypes);

    stip_traverse_structure_children_pipe(si,st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_refutations_solver(slice_index si,
                                      stip_structure_traversal *st)
{
  unsigned int const max_nr_refutations = UINT_MAX;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(st->context==stip_traversal_context_defense);

  {
    slice_index const prototypes[] =
    {
      alloc_refutations_avoider_slice(max_nr_refutations)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    defense_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  {
    slice_index const prototypes[] =
    {
      alloc_refutations_collector_slice(max_nr_refutations)
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    defense_branch_insert_slices_behind_proxy(SLICE_NEXT2(si),prototypes,nr_prototypes,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const setplay_solver_inserters[] =
{
  { STOutputModeSelector, &filter_output_mode                                    },
  { STSetplayFork,        &stip_traverse_structure_children_setplay_fork_setplay },
  { STAttackAdapter,      &stip_structure_visitor_noop                           },
  { STDefenseAdapter,     &insert_setplay_solvers_defense_adapter                },
  { STContinuationSolver, &insert_refutations_solver                             }
};

enum
{
  nr_setplay_solver_inserters = sizeof setplay_solver_inserters / sizeof setplay_solver_inserters[0]
};

/* Instrument the stipulation structure with slices solving set play
 * @param root_slice root slice of the stipulation
 */
void solving_insert_setplay_solvers(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    setplay_solver_inserters,
                                    nr_setplay_solver_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
