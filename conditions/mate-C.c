#include "conditions/mate-C.h"
#include "pieces/attributes/neutral/neutral.h"
#include "position/effects/side_change.h"
#include "position/effects/utils.h"
#include "solving/observation.h"
#include "solving/check.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "solving/has_solution_type.h"
#include "solving/conditional_pipe.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/move.h"
#include "stipulation/pipe.h"
#include "solving/pipe.h"

#include "debugging/trace.h"
#include "debugging/assert.h"

static square current_observer_pos[maxply+1];

/* Validate an observation or observer by making sure it's the checking piece
 * @param si identifies the validator slice
 */
boolean mate_C_enforce_observer(slice_index si)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observer = current_observer_pos[parent_ply[nbply]];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = ((sq_observer==initsquare || sq_observer==sq_departure)
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Deterine if the piece on a square is observed by the piece of another square
 * @param on_this position of the potential observee
 * @param by_that position of the potential observer
 * @return true iff the piece on by_that observes the piece on on_this
 */
static boolean observes(square sq_observer)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceFunctionParamListEnd();

  current_observer_pos[parent_ply[nbply]] = sq_observer;

  result = is_square_observed_general(trait[nbply],
                                      being_solved.king_square[advers(trait[nbply])],
                                      EVALUATE(observation));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_recoloring_candidate(square sq_observer)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceFunctionParamListEnd();

  result = (!is_square_empty(sq_observer)
            && !TSTFLAG(being_solved.spec[sq_observer],Royal)
            && TSTFLAG(being_solved.spec[sq_observer],trait[nbply])
            && !TSTFLAG(being_solved.spec[sq_observer],advers(trait[nbply]))
            && observes(sq_observer));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void change_checkers(slice_index si)
{
  move_effect_journal_index_type const before_recoloring = move_effect_journal_base[nbply+1];
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",move_effect_journal_base[nbply]);
  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceEOL();

  for (bnp = boardnum; *bnp; bnp++)
    if (is_recoloring_candidate(*bnp))
      move_effect_journal_do_side_change(move_effect_reason_masand,*bnp);

  if (is_in_check(trait[nbply]))
  {
    move_effect_journal_index_type const save_parent_top = move_effect_journal_base[nbply];

    TraceText("HAALLOO!!\n");

    move_effect_journal_base[nbply] = before_recoloring;
    undo_move_effects();
    move_effect_journal_base[nbply] = save_parent_top;
    move_effect_journal_base[nbply+1] = before_recoloring;

    TraceValue("%u",move_effect_journal_base[nbply]);
    TraceValue("%u",move_effect_journal_base[nbply+1]);
    TraceEOL();
  }

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
void mate_C_recolorer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (conditional_pipe_solve_delegate(temporary_hack_mate_tester[advers(trait[nbply])]))
  {
    case this_move_is_illegal:
      solve_result = this_move_is_illegal;
      break;

    case previous_move_has_not_solved:
      pipe_solve_delegate(si);
      break;

    default:
      change_checkers(si);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototypes[] = {
        alloc_pipe(STMate_CRecolorer)
    };

    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    move_insert_slices(si,st->context,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor mate_C_inserters[] =
{
  { STGoalMateReachedTester, &stip_structure_visitor_noop },
  { STMove,                  &instrument_move             }
};

enum
{
  nr_mate_C_inserters = sizeof mate_C_inserters / sizeof mate_C_inserters[0]
};

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_mate_C(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override(&st,
                                    mate_C_inserters,
                                    nr_mate_C_inserters);
  stip_traverse_structure(si,&st);

  stip_instrument_observation_validation(si,nr_sides,STMate_CEnforceObserver);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
