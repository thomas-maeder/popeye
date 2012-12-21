#include "conditions/ohneschach/legality_tester.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/temporary_hacks.h"
#include "conditions/ohneschach/immobility_tester.h"
#include "debugging/trace.h"

#include <assert.h>

/* Determine whether a side is immobile in Ohneschach
 * @return true iff side is immobile
 */
boolean immobile(Side side)
{
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  /* ohneschach_check_guard_solve() may invoke itself recursively. Protect
   * ourselves from infinite recursion. */
  if (nbply>250)
    FtlMsg(ChecklessUndecidable);

  result = solve(slices[temporary_hack_immobility_tester[side]].next2,length_unspecified)==has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the move just played has led to a legal position according
 * to condition Ohneschach
 * @param just_moved identifies the side that has just moved
 * @return true iff the position reached is legal according to Ohneschach
 */
static boolean is_position_legal(Side just_moved)
{
  boolean result = true;
  Side const ad = advers(just_moved);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,just_moved,"");
  TraceFunctionParamListEnd();

  if (echecc(just_moved))
    result = false;
  else if (echecc(ad) && !immobile(ad))
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_complex(slice_index si, stip_structure_traversal *st)
{
  boolean const * const inside_immobility_tester = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  switch (st->context)
  {
    case stip_traversal_context_attack:
    case stip_traversal_context_defense:
      break;

    case stip_traversal_context_help:
      if (*inside_immobility_tester)
      {
        slice_index const prototype = alloc_pipe(STOhneschachLegalityTester);
        branch_insert_slices_contextual(si,st->context,&prototype,1);
      }
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_simple(slice_index si, stip_structure_traversal *st)
{
  boolean const * const inside_immobility_tester = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  switch (st->context)
  {
    case stip_traversal_context_attack:
    {
      slice_index const prototype = alloc_pipe(STOhneschachCheckGuardDefense);
      branch_insert_slices_contextual(si,st->context,&prototype,1);
      break;
    }

    case stip_traversal_context_defense:
    {
      slice_index const prototype = alloc_pipe(STOhneschachCheckGuard);
      branch_insert_slices_contextual(si,st->context,&prototype,1);
      break;
    }

    case stip_traversal_context_help:
      if (!*inside_immobility_tester)
      {
        slice_index const prototype = alloc_pipe(STOhneschachCheckGuard);
        branch_insert_slices_contextual(si,st->context,&prototype,1);
      }
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_immobility_tester(slice_index si, stip_structure_traversal *st)
{
  boolean * const inside_immobility_tester = st->param;
  boolean const save_inside_immobility_tester = *inside_immobility_tester;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *inside_immobility_tester = true;
  stip_traverse_structure_children(si,st);
  *inside_immobility_tester = save_inside_immobility_tester;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_ohneschach_legality_testers(slice_index si)
{
  stip_structure_traversal st;
  boolean inside_immobility_tester = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&inside_immobility_tester);
  stip_structure_traversal_override_single(&st,STMove,&instrument_complex);
  stip_structure_traversal_override_single(&st,STImmobilityTester,&remember_immobility_tester);
  stip_structure_traversal_override_single(&st,STNotEndOfBranchGoal,&instrument_simple);
  stip_structure_traversal_override_single(&st,STOhneschachSuspender,&stip_structure_visitor_noop);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_legality_tester_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_position_legal(trait[nbply]))
    result = solve(next,n);
  else
    result = slack_length-2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
