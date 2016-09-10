#include "conditions/backhome.h"
#include "conditions/mummer.h"
#include "position/pieceid.h"
#include "position/position.h"
#include "solving/has_solution_type.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/legal_move_counter.h"
#include "solving/post_move_iteration.h"
#include "solving/pipe.h"
#include "solving/conditional_pipe.h"
#include "solving/check.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static square pieceid2pos[MaxPieceId+1];

static boolean goes_back_home(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  square const sq_arrival = move_generation_stack[n].arrival;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = sq_arrival==pieceid2pos[GetPieceId(being_solved.spec[sq_departure])];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void back_home_moves_only_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,!goes_back_home(CURRMOVE_OF_PLY(nbply)));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean exists_legal_move_back_home(void)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* stop counting once we have one legal move that goes back home */
  legal_move_count_init(0);

  /* the first found legal move back home refutes */
  result = (conditional_pipe_solve_delegate(temporary_hack_back_home_finder[trait[nbply]])
            == previous_move_has_not_solved);

  legal_move_count_fini();

  post_move_iteration_init_ply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean exists[maxply+1];

static boolean goes_back_home_or_neednt(numecoup n)
{
  return !exists[nbply] || goes_back_home(n);
}

/* Determine whether there are moves Back Home for observation validation
 * @return true iff the observation is valid
 */
boolean backhome_existance_tester_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  exists[nbply] = exists_legal_move_back_home();
  result = pipe_validate_observation_recursive_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Validate an observation according to Back Home
 * @return true iff the observation is valid
 */
boolean back_home_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (goes_back_home_or_neednt(CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void backhome_existance_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  exists[nbply] = exists_legal_move_back_home();
  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void backhome_remove_illegal_moves_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(MOVEBASE_OF_PLY(nbply),&goes_back_home_or_neednt);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STBackhomeExistanceTester),
        alloc_pipe(STBackhomeRemoveIllegalMoves)
    };
    slice_insertion_insert_contextually(si,st->context,prototypes,2);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving in Back-Home
 * @param si identifies root slice of stipulation
 */
void backhome_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    PieceIdType id;
    for (id = 0; id<=MaxPieceId; ++id)
      pieceid2pos[id] = initsquare;
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; ++bnp)
      if (!is_square_empty(*bnp) && !is_square_blocked(*bnp))
        pieceid2pos[GetPieceId(being_solved.spec[*bnp])] = *bnp;
  }

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STBackHomeFinderFork,
                                             stip_traverse_structure_children_pipe);
    stip_structure_traversal_override_single(&st,
                                             STDoneGeneratingMoves,
                                             &insert_remover);
    stip_traverse_structure(si,&st);
  }

  stip_instrument_observation_validation(si,nr_sides,STBackhomeExistanceTester);
  stip_instrument_observation_validation(si,nr_sides,STBackhomeRemoveIllegalMoves);

  stip_instrument_check_validation(si,nr_sides,STBackhomeExistanceTester);
  stip_instrument_check_validation(si,nr_sides,STBackhomeRemoveIllegalMoves);

  solving_instrument_check_testing(si,STKingCapturedObservationGuard);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
