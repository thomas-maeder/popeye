#include "conditions/transmuting_kings/super.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "solving/observation.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "debugging/trace.h"
#include "pydata.h"

#include <assert.h>

#define MAX_OTHER_LEN 1000 /* needs to be at least the max of any value that can be returned in the len functions */

/* the mummer logic is (ab)used to priorise transmuting king moves */
int len_supertransmuting_kings(void)
{
  return MAX_OTHER_LEN * (move_generation_stack[current_move[nbply]-1].current_supertransmutation!=Empty ? 1 : 0);
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type supertransmuting_kings_transmuter_solve(slice_index si,
                                                          stip_length_type n)
{
  stip_length_type result;
  numecoup const coup_id = current_move[nbply]-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_generation_stack[coup_id].current_supertransmutation!=Empty)
  {
    move_effect_journal_index_type const base = move_effect_journal_top[nbply-1];
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

    move_effect_journal_do_piece_change(move_effect_reason_king_transmutation,
                                        pos,
                                        move_generation_stack[coup_id].current_supertransmutation);
  }

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean is_square_observed_by_opponent(PieNam p, square sq_departure)
{
  boolean result;

  siblingply(advers(trait[nbply]));
  current_move[nbply] = current_move[nbply-1]+1;
  move_generation_stack[current_move[nbply]-1].capture = sq_departure;
  result = (*checkfunctions[p])(p,&validate_observation);
  finply();

  return result;
}

static void remember_transmuter(numecoup base, PieNam p)
{
  numecoup curr;
  for (curr = base; curr<current_move[nbply]; ++curr)
    move_generation_stack[curr].current_supertransmutation = p;
}

static boolean generate_moves_of_supertransmuting_king(slice_index si, square sq_departure)
{
  boolean result = false;
  Side const side_moving = trait[nbply];
  Side const side_transmuting = advers(side_moving);

  PieNam const *ptrans;
  for (ptrans = transmpieces[side_moving]; *ptrans!=Empty; ++ptrans)
    if (number_of_pieces[side_transmuting][*ptrans]>0
        && is_square_observed_by_opponent(*ptrans,sq_departure))
    {
      numecoup const base = current_move[nbply];
      generate_moves_for_piece(slices[si].next1,sq_departure,*ptrans);
      remember_transmuter(base,*ptrans);
      result = true;
    }

  return result;
}

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param sq_departure departure square of generated moves
 * @param p walk to be used for generating
 */
void supertransmuting_kings_generate_moves_for_piece(slice_index si,
                                                     square sq_departure,
                                                     PieNam p)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  if (!(p==King && generate_moves_of_supertransmuting_king(si,sq_departure)))
  {
    numecoup const base = current_move[nbply]-1;
    generate_moves_for_piece(slices[si].next1,sq_departure,p);
    remember_transmuter(base,Empty);
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
  instrument_alternative_is_square_observed_king_testing(si,side,STTransmutingKingIsSquareObserved);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
