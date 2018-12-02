#include "conditions/bgl.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "solving/has_solution_type.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <stdlib.h>

long int BGL_values[nr_sides];
boolean BGL_global;

static long int BGL_move_diff_code[square_h8 - square_a1 + 1] =
{
 /* left/right   */        0,   100,   200,   300,  400,  500,  600,  700,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 1 left  up   */            707,  608,  510,  412,  316,   224,   141,
 /* 1 right up   */        100,   141,   224,  316,  412,  510,  608,  707,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 2 left  up   */            728,  632,  539,  447,  361,   283,   224,
 /* 2 right up   */        200,   224,   283,  361,  447,  539,  632,  728,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 3 left  up   */            762,  671,  583,  500,  424,  361,  316,
 /* 3 right up   */        300,  316,  361,  424,  500,  583,  671,  762,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 4 left  up   */            806,  721,  640,  566,  500,  447,  412,
 /* 4 right up   */       400,  412,  447,  500,  566,  640,  721,  806,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 5 left  up   */            860,  781,  707,  640,  583,  539,  510,
 /* 5 right up   */       500,  510,  539,  583,  640,  707,  781,  860,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 6 left  up   */            922,  849,  781,  721,  671,  632,  608,
 /* 6 right up   */       600,  608,  632,  671,  721,  781,  849,  922,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 7 left  up   */            990,  922,  860,  806,  762,  728,  707,
 /* 7 right up   */       700,  707,  728,  762,  806,  860,  922,  990
};

/* Adjust the BGL values
 * @param diff adjustment
 */
static void do_bgl_adjustment(Side side, long int diff)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_bgl_adjustment,move_effect_reason_moving_piece_movement);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%lu",diff);
  TraceFunctionParamListEnd();

  entry->u.bgl_adjustment.side = side;
  entry->u.bgl_adjustment.diff = diff;

  BGL_values[side] -= diff;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Undo a BGL adjustment
 * @param curr identifies the adjustment effect
 */
static void move_effect_journal_undo_bgl_adjustment(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.bgl_adjustment.side;
  long int const diff = entry->u.bgl_adjustment.diff;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%lu",diff);
  TraceEOL();

  BGL_values[side] += diff;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Redo a BGL adjustment
 * @param curr identifies the adjustment effect
 */
static void move_effect_journal_redo_bgl_adjustment(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.bgl_adjustment.side;
  long int const diff = entry->u.bgl_adjustment.diff;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%lu",diff);
  TraceEOL();

  BGL_values[side] -= diff;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static long int calculate_diff(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  square const sq_arrival = move_generation_stack[n].arrival;
  return BGL_move_diff_code[abs(sq_departure-sq_arrival)];
}

static boolean is_move_within_bounds(numecoup n)
{
  return calculate_diff(n)<=BGL_values[trait[nbply]];
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
void bgl_enforcer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(MOVEBASE_OF_PLY(nbply),&is_move_within_bounds);
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
void bgl_adjuster_solve(slice_index si)
{
  long int const diff = calculate_diff(CURRMOVE_OF_PLY(nbply));

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(BGL_values[trait[nbply]]>=diff); /* BGL enforcer takes care of that */

  if (BGL_global)
  {
    do_bgl_adjustment(White,diff);
    do_bgl_adjustment(Black,diff);
  }
  else
    do_bgl_adjustment(trait[nbply],diff);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Validate an observation according to BGL
 * @return true iff the observation is valid
 */
boolean bgl_validate_observation(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = (is_move_within_bounds(CURRMOVE_OF_PLY(nbply))
            && pipe_validate_observation_recursive_delegate(si));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (BGL_values[SLICE_STARTER(si)]!=BGL_infinity)
  {
    slice_index const prototype = alloc_pipe(STBGLEnforcer);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (BGL_values[SLICE_STARTER(si)]!=BGL_infinity)
  {
    slice_index const prototype = alloc_pipe(STBGLAdjuster);
    move_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise solving with BGL
 */
void bgl_initialise_solving(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_set_effect_doers(move_effect_bgl_adjustment,
                                       &move_effect_journal_undo_bgl_adjustment,
                                       &move_effect_journal_redo_bgl_adjustment);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STDoneGeneratingMoves,
                                             &insert_remover);
    stip_traverse_structure(si,&st);
  }

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,STMove,&instrument_move);
    stip_traverse_structure(si,&st);
  }

  if (BGL_values[White]!=BGL_infinity)
  {
    stip_instrument_observation_validation(si,White,STBGLEnforcer);
    stip_instrument_check_validation(si,White,STBGLEnforcer);
  }
  if (BGL_values[Black]!=BGL_infinity)
  {
    stip_instrument_observation_validation(si,Black,STBGLEnforcer);
    stip_instrument_check_validation(si,Black,STBGLEnforcer);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
