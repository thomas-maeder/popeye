#include "conditions/transmuting_kings/super.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/observation.h"
#include "solving/move_generator.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/check.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/slice_insertion.h"
#include "solving/pipe.h"
#include "pieces/pieces.h"
#include "position/position.h"
#include "position/effects/walk_change.h"
#include "position/effects/flags_change.h"
#include "position/effects/utils.h"

#include "debugging/trace.h"
#include "debugging/assert.h"


static piece_walk_type supertransmutation[toppile+1];

static boolean exists_transmutation[maxply+1];

#define MAX_OTHER_LEN 1000 /* needs to be at least the max of any value that can be returned in the len functions */

/* the mummer logic is (ab)used to priorise transmuting king moves */
mummer_length_type len_supertransmuting_kings(void)
{
  return MAX_OTHER_LEN * (supertransmutation[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id]!=Empty ? 1 : 0);
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
void supertransmuting_kings_transmuter_solve(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (supertransmutation[move_generation_stack[curr].id]!=Empty)
  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    Flags flags = being_solved.spec[pos];

    CLRFLAG(flags,Royal);
    move_effect_journal_do_flags_change(move_effect_reason_king_transmutation,
                                        pos,flags);

    move_effect_journal_do_walk_change(move_effect_reason_king_transmutation,
                                        pos,
                                        supertransmutation[move_generation_stack[curr].id]);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean move_has_transmutation(numecoup n)
{
  return supertransmutation[move_generation_stack[n].id]!=Empty;
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
void supertransmuting_kings_move_generation_filter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (exists_transmutation[nbply])
  {
    move_generator_filter_moves(MOVEBASE_OF_PLY(nbply),&move_has_transmutation);
    exists_transmutation[nbply] = false;
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_moves_of_supertransmuting_king(slice_index si)
{
  piece_walk_type const *ptrans;
  for (ptrans = transmuting_kings_potential_transmutations; *ptrans!=Empty; ++ptrans)
    if (transmuting_kings_is_king_transmuting_as(*ptrans))
    {
      numecoup curr_id = current_move_id[nbply];
      pipe_move_generation_different_walk_delegate(si,*ptrans);
      for (; curr_id<current_move_id[nbply]; ++curr_id)
        supertransmutation[curr_id] = *ptrans;
      exists_transmutation[nbply] = true;
    }
}

static void generate_moves_for_piece_delegate(slice_index si)
{
  if (!exists_transmutation[nbply])
  {
    numecoup curr_id = current_move_id[nbply];
    pipe_move_generation_delegate(si);
    for (; curr_id<current_move_id[nbply]; ++curr_id)
      supertransmutation[curr_id] = Empty;
  }
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void supertransmuting_kings_generate_moves_for_piece(slice_index si)
{
  Flags const mask = BIT(trait[nbply])|BIT(Royal);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (TSTFULLFLAGMASK(being_solved.spec[curr_generation->departure],mask))
    generate_moves_of_supertransmuting_king(si);

  generate_moves_for_piece_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  Side const * const side = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (SLICE_STARTER(si)==*side)
  {
    slice_index const prototype = alloc_pipe(STSuperTransmutingKingTransmuter);
    move_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_filter(slice_index si,
                          stip_structure_traversal *st,
                          void *param)
{
  Side const * const side = param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (SLICE_STARTER(si)==*side)
  {
    slice_index const prototype = alloc_pipe(STSuperTransmutingKingMoveGenerationFilter);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 * @param si identifies root slice of solving machinery
 * @param side for whom
 */
void supertransmuting_kings_initialise_solving(slice_index si, Side side)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  solving_impose_starter(si,SLICE_STARTER(si));

  stip_structure_traversal_init(&st,&side);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

  solving_instrument_move_generation2(si,&insert_filter,&side);
  solving_instrument_moves_for_piece_generation(si,side,STSuperTransmutingKingsMovesForPieceGenerator);

  transmuting_kings_initialise_observing(si,side);

  check_no_king_is_possible();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
