#include "conditions/transmuting_kings/super.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/observation.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/check.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "position/position.h"

#include "debugging/assert.h"


static piece_walk_type supertransmutation[toppile+1];

#define MAX_OTHER_LEN 1000 /* needs to be at least the max of any value that can be returned in the len functions */

/* the mummer logic is (ab)used to priorise transmuting king moves */
int len_supertransmuting_kings(void)
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
    Flags flags = spec[pos];

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

static boolean generate_moves_of_supertransmuting_king(slice_index si)
{
  boolean result = false;
  piece_walk_type const save_current_walk = move_generation_current_walk;

  piece_walk_type const *ptrans;
  for (ptrans = transmuting_kings_potential_transmutations; *ptrans!=Empty; ++ptrans)
    if (transmuting_kings_is_king_transmuting_as(*ptrans))
    {
      numecoup curr_id = current_move_id[nbply];
      move_generation_current_walk = *ptrans;
      generate_moves_for_piece(slices[si].next1);
      for (; curr_id<current_move_id[nbply]; ++curr_id)
        supertransmutation[curr_id] = *ptrans;
      result = true;
    }

  move_generation_current_walk = save_current_walk;

  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void supertransmuting_kings_generate_moves_for_piece(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (!(move_generation_current_walk==King
        && generate_moves_of_supertransmuting_king(si)))
  {
    numecoup curr_id = current_move_id[nbply];
    generate_moves_for_piece(slices[si].next1);
    for (; curr_id<current_move_id[nbply]; ++curr_id)
      supertransmutation[curr_id] = Empty;
  }

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

  if (slices[si].starter==*side)
  {
    slice_index const prototype = alloc_pipe(STSuperTransmutingKingTransmuter);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  stip_impose_starter(si,slices[si].starter);

  stip_structure_traversal_init(&st,&side);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

  solving_instrument_move_generation(si,side,STSuperTransmutingKingsMovesForPieceGenerator);

  transmuting_kings_initialise_observing(si,side);

  check_no_king_is_possible();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
