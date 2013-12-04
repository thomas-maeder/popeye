#include "solving/check.h"
#include "conditions/extinction.h"
#include "conditions/sat.h"
#include "conditions/vogtlaender.h"
#include "conditions/circe/assassin.h"
#include "conditions/bicolores.h"
#include "conditions/antikings.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/temporary_hacks.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

#include <assert.h>

static boolean is_no_king_possible = false;
static boolean is_check_possible_if_no_king = false;

/* Tell the check detection machinery to forget everythign about no kings */
void check_reset_no_king_knowledge(void)
{
  is_no_king_possible = false;
  is_check_possible_if_no_king = false;
}

/* Tell the check detection machinery that a side may have no king */
void check_no_king_is_possible(void)
{
  is_no_king_possible = true;
}

/* Tell the check detection machinery that a side may be in check even if it
 * doesn't have a king*/
void check_even_if_no_king(void)
{
  is_check_possible_if_no_king = true;
}

/* Optimise the check machinery if possible
 * @param si identifies the root slice of the solving machinery
 */
void optimise_is_in_check(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (king_square[White]==initsquare || king_square[Black]==initsquare)
    is_no_king_possible = true;

  if (is_no_king_possible && !is_check_possible_if_no_king)
    solving_instrument_check_testing(si,STNoKingCheckTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean no_king_check_tester_is_in_check(slice_index si,
                                                Side side_in_check)
{
  if (king_square[side_in_check]==initsquare
      || king_square[side_in_check]==nullsquare)
    return false;
  else
    return is_in_check_recursive(slices[si].next1,side_in_check);
}

static boolean king_square_observation_tester_ply_initialiser_is_in_check(slice_index si,
                                                                          Side side_in_check)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_in_check,"");
  TraceFunctionParamListEnd();

  nextply(advers(side_in_check));
  push_observation_target(king_square[side_in_check]);
  result = is_in_check_recursive(slices[si].next1,side_in_check);
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

DEFINE_COUNTER(is_white_king_square_attacked)
DEFINE_COUNTER(is_black_king_square_attacked)

static boolean king_square_observation_tester_is_in_check(slice_index si,
                                                          Side side_king_attacked)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_king_attacked,"");
  TraceFunctionParamListEnd();

  if (side_king_attacked==White)
  {
    INCREMENT_COUNTER(is_white_king_square_attacked);
  }
  else
  {
    INCREMENT_COUNTER(is_black_king_square_attacked);
  }

  TraceSquare(king_square[side_king_attacked]);TraceEOL();
  assert(king_square[side_king_attacked]!=initsquare);

  result = is_square_observed(EVALUATE(check));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Continue determining whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean is_in_check_recursive(slice_index si, Side side_in_check)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_in_check,"");
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STNoCheckConceptCheckTester:
      result = false;
      break;

    case STVogtlaenderCheckTester:
      result = vogtlaender_check_tester_is_in_check(si,side_in_check);
      break;

    case STNoKingCheckTester:
      result = no_king_check_tester_is_in_check(si,side_in_check);
      break;

    case STSATCheckTester:
      result = sat_check_tester_is_in_check(si,side_in_check);
      break;

    case STSATxyCheckTester:
      result = satxy_check_tester_is_in_check(si,side_in_check);
      break;

    case STStrictSATCheckTester:
      result = strictsat_check_tester_is_in_check(si,side_in_check);
      break;

    case STAssassinCirceCheckTester:
      result = assassin_circe_check_tester_is_in_check(si,side_in_check);
      break;

    case STKingSquareObservationTesterPlyInitialiser:
      result = king_square_observation_tester_ply_initialiser_is_in_check(si,side_in_check);
      break;

    case STBicoloresCheckTester:
      result = bicolores_check_tester_is_in_check(si,side_in_check);
      break;

    case STAntikingsCheckTester:
      result = antikings_check_tester_is_in_check(si,side_in_check);
      break;

    case STKingSquareObservationTester:
      result = king_square_observation_tester_is_in_check(si,side_in_check);
      break;

    case STExtinctionAllSquareObservationTester:
      result = exctinction_all_square_observation_tester_is_in_check(si,side_in_check);
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a side is in check
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean is_in_check(Side side_in_check)
{
  return is_in_check_recursive(slices[temporary_hack_check_tester].next2,
                               side_in_check);
}

static slice_index const slice_rank_order[] =
{
    STTestingCheck,
    STNoCheckConceptCheckTester,
    STVogtlaenderCheckTester,
    STNoKingCheckTester,
    STSATCheckTester,
    STSATxyCheckTester,
    STStrictSATCheckTester,
    STAssassinCirceCheckTester,
    STKingSquareObservationTesterPlyInitialiser,
    STBicoloresCheckTester,
    STAntikingsCheckTester,
    STKingSquareObservationTester,
    STExtinctionAllSquareObservationTester,
    STTrue
};

enum
{
  nr_slice_rank_order_elmts = sizeof slice_rank_order / sizeof slice_rank_order[0]
};

static void insert_slice(slice_index testing, slice_type type)
{
  slice_index const prototype = alloc_pipe(type);
  stip_structure_traversal st;
  branch_slice_insertion_state_type state =
  {
    &prototype,1,
    slice_rank_order, nr_slice_rank_order_elmts,
    branch_slice_rank_order_nonrecursive,
    0,
    testing,
    0
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",testing);
  TraceEnumerator(slice_type,type,"");
  TraceFunctionParamListEnd();

  state.base_rank = get_slice_rank(slices[testing].type,&state);
  assert(state.base_rank!=no_slice_rank);
  init_slice_insertion_traversal(&st,&state,stip_traversal_context_intro);
  stip_traverse_structure_children_pipe(testing,&st);
  dealloc_slice(prototype);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_testing(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_type const * const type = st->param;
    insert_slice(si,*type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument check testing with a slice type
 * @param identifies where to start instrumentation
 * @param type type of slice with which to instrument moves
 */
void solving_instrument_check_testing(slice_index si, slice_type type)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&type);
  stip_structure_traversal_override_single(&st,
                                           STTestingCheck,
                                           &instrument_testing);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
