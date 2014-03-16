#include "conditions/circe/circe.h"
#include "pieces/walks/walks.h"
#include "pieces/walks/classification.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "conditions/conditions.h"

#include "debugging/assert.h"

circe_variant_type circe_variant;

circe_rebirth_context_elmt_type circe_rebirth_context_stack[maxply+1];
circe_rebirth_context_index circe_rebirth_context_stack_pointer = 0;

/* Reset a circe_variant object to the default values
 * @param variant address of the variant object to be reset
 */
void circe_reset_variant(circe_variant_type *variant)
{
  variant->is_promotion_possible = false;
  variant->rebirth_reason = move_effect_reason_rebirth_no_choice;
  variant->is_mirror = false;
  variant->is_diametral = false;
  variant->on_occupied_rebirth_square = circe_on_occupied_rebirth_square_default;
  variant->on_occupied_rebirth_square_default = circe_on_occupied_rebirth_square_relaxed;
  variant->reborn_walk_adapter = circe_reborn_walk_adapter_none;
  variant->is_turncoat = false;
  variant->relevant_piece = circe_relevant_piece_default;
  variant->relevant_capture = circe_relevant_capture_thismove;
  variant->determine_rebirth_square = circe_determine_rebirth_square_from_pas;
  variant->is_frischauf = false;
  variant->anticirce_type = anticirce_type_count;
  variant->is_rex_inclusive = false;
}

/* Override the reborn walk adapter of a Circe variant object
 * @param adapter the overrider
 * @return true if the adapter hasn't been overridden yet
 */
boolean circe_override_reborn_walk_adapter(circe_variant_type *variant,
                                           circe_reborn_walk_adapter_type adapter)
{
  boolean result;

  if (variant->reborn_walk_adapter==circe_reborn_walk_adapter_none)
  {
    variant->reborn_walk_adapter = adapter;
    result = true;
  }
  else
    result = false;

  return result;
}

/* Override the method for determining the rebirth square of a Circe variant object
 * @param adapter the overrider
 * @return true if it hasn't been overridden yet
 */
boolean circe_override_determine_rebirth_square(circe_variant_type *variant,
                                                circe_reborn_walk_adapter_type determine)
{
  boolean result;

  if (variant->determine_rebirth_square==circe_determine_rebirth_square_from_pas)
  {
    variant->determine_rebirth_square = determine;
    result = true;
  }
  else
    result = false;

  return result;
}

circe_behaviour_on_occupied_rebirth_square_type
circe_get_on_occupied_rebirth_square(circe_variant_type const *variant)
{
  if (variant->on_occupied_rebirth_square==circe_on_occupied_rebirth_square_default)
    return variant->on_occupied_rebirth_square_default;
  else
    return variant->on_occupied_rebirth_square;
}

/* Find the Circe rebirth effect in the current move
 * @return the index of the rebirth effect
 *         move_effect_journal_base[nbply+1] if there is none
 */
move_effect_journal_index_type circe_find_current_rebirth(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (result = top-1; result>=base+move_effect_journal_index_offset_other_effects; --result)
    if ((move_effect_journal[result].type==move_effect_piece_readdition
         && (move_effect_journal[result].reason==move_effect_reason_rebirth_no_choice
             || move_effect_journal[result].reason==move_effect_reason_rebirth_choice))
        || move_effect_journal[result].type==move_effect_remember_volcanic)
      break;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the Circe machinery from the capture in a particular ply
 * @param ply identifies the ply
 */
void circe_initialise_from_capture_in_ply(ply ply)
{
  move_effect_journal_index_type const base = move_effect_journal_base[ply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParamListEnd();

  /* circe capture fork makes sure of that */
  assert(move_effect_journal[capture].type==move_effect_piece_removal);

  context->reborn_walk = move_effect_journal[capture].u.piece_removal.removed;
  context->reborn_spec = move_effect_journal[capture].u.piece_removal.removedspec;
  context->relevant_square = move_effect_journal[capture].u.piece_removal.from;

  context->relevant_walk = context->reborn_walk;
  context->relevant_spec = context->reborn_spec;

  context->relevant_side = trait[ply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_initialise_from_current_capture_solve(slice_index si,
                                                             stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  circe_initialise_from_capture_in_ply(nbply);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_determine_rebirth_square_solve(slice_index si,
                                                      stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  context->rebirth_square = rennormal(context->relevant_walk,
                                      context->relevant_spec,
                                      context->relevant_square,
                                      context->relevant_side);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_test_rebirth_square_empty_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(context->reborn_walk!=Empty);

  if (is_square_empty(context->rebirth_square))
    result = solve(slices[si].next1,n);
  else
    result = solve(slices[si].next2,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type circe_place_reborn_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  circe_rebirth_context_elmt_type const * const context = &circe_rebirth_context_stack[circe_rebirth_context_stack_pointer];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(context->reborn_walk!=Empty);
  assert(is_square_empty(context->rebirth_square));

  move_effect_journal_do_piece_readdition(circe_variant.rebirth_reason,
                                          context->rebirth_square,
                                          context->reborn_walk,
                                          context->reborn_spec);
  ++circe_rebirth_context_stack_pointer;
  result = solve(slices[si].next1,n);
  --circe_rebirth_context_stack_pointer;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

square renfile_polymorphic(PieNam p_captured, Flags p_captured_spec,
                           square sq_capture, square sq_departure, square sq_arrival,
                           Side capturer)
{
  return renfile(p_captured,sq_capture,capturer);
}
square renfile(PieNam p_captured, square sq_capture, Side capturer)
{
  int col = sq_capture % onerow;
  square result;

  TraceFunctionEntry(__func__);
  TracePiece(p_captured);
  TraceSquare(sq_capture);
  TraceEnumerator(Side,capturer,"");
  TraceFunctionParamListEnd();

  if (capturer==Black)
  {
    if (is_pawn(p_captured))
      result = col + (nr_of_slack_rows_below_board+1)*onerow;
    else
      result = col + nr_of_slack_rows_below_board*onerow;
  }
  else
  {
    if (is_pawn(p_captured))
      result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
    else
      result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-1)*onerow;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

square renspiegelfile_polymorphic(PieNam p_captured, Flags p_captured_spec,
                      square sq_capture,
                      square sq_departure, square sq_arrival,
                      Side capturer)
{
  return renfile(p_captured,sq_capture,advers(capturer));
}

square renpwc(PieNam p_captured, Flags p_captured_spec,
              square sq_capture, square sq_departure, square sq_arrival,
              Side capturer)
{
  return sq_departure;
} /* renpwc */

square renequipollents_polymorphic(PieNam p_captured, Flags p_captured_spec,
                       square sq_capture,
                       square sq_departure, square sq_arrival,
                       Side capturer)
{
  /* we have to solve the enpassant capture / locust capture problem in the future. */
#if defined(WINCHLOE)
  return sq_capture + sq_arrival - sq_departure;
#endif
  return sq_capture + sq_capture - sq_departure;
}
//
//square renantiequipollents_polymorphic(PieNam p_captured, Flags p_captured_spec,
//                            square sq_capture,
//                            square sq_departure, square sq_arrival,
//                            Side capturer)
//{
//  /* we have to solve the enpassant capture / locust capture problem in the future. */
//#if defined(WINCHLOE)
//  return sq_arrival + sq_arrival - sq_departure;
//#endif
//  return sq_capture + sq_capture - sq_departure;
//}

square rensymmetrie_polymorphic(PieNam p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  return (square_h8+square_a1) - sq_capture;
}

square renantipoden_polymorphic(PieNam p_captured, Flags p_captured_spec,
                    square sq_capture,
                    square sq_departure, square sq_arrival,
                    Side capturer)
{
  unsigned int const row = sq_capture/onerow - nr_of_slack_rows_below_board;
  unsigned int const file = sq_capture%onerow - nr_of_slack_files_left_of_board;

  square result = sq_capture;

  if (row<nr_rows_on_board/2)
    result += nr_rows_on_board/2*dir_up;
  else
    result += nr_rows_on_board/2*dir_down;

  if (file<nr_files_on_board/2)
    result += nr_files_on_board/2*dir_right;
  else
    result += nr_files_on_board/2*dir_left;

  return result;
}

square rendiagramm_polymorphic(PieNam p_captured, Flags p_captured_spec,
                   square sq_capture, square sq_departure, square sq_arrival,
                   Side capturer)
{
  return GetPositionInDiagram(p_captured_spec);
}

square rennormal_polymorphic(PieNam pnam_captured, Flags p_captured_spec,
                 square sq_capture, square sq_departure, square sq_arrival,
                 Side capturer)
{
  return rennormal(pnam_captured,p_captured_spec,sq_capture,capturer);
}

square rennormal(PieNam pnam_captured, Flags p_captured_spec,
                  square sq_capture,
                  Side capturer)
{
  square  Result;
  unsigned int col = sq_capture % onerow;
  unsigned int const ran = sq_capture / onerow;

  TraceFunctionEntry(__func__);
  TracePiece(pnam_captured);
  TraceSquare(sq_capture);
  TraceEnumerator(Side,capturer,"");
  TraceFunctionParamListEnd();

  if (CondFlag[circemirrorvertical])
  {
    col = onerow-1 - col;
    if (pnam_captured==Queen)
      pnam_captured = King;
    else if (pnam_captured==King)
      pnam_captured = Queen;
  }

  {
    Side const cou = (ran&1) != (col&1) ? White : Black;

    if (capturer == Black)
    {
      if (is_pawn(pnam_captured))
        Result = col + (nr_of_slack_rows_below_board+1)*onerow;
      else if (circe_variant.is_frischauf && TSTFLAG(p_captured_spec,FrischAuf))
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                       : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
      else if (pnam_captured==standard_walks[Knight])
        Result = cou == White ? square_b1 : square_g1;
      else if (pnam_captured==standard_walks[Rook])
        Result = cou == White ? square_h1 : square_a1;
      else if (pnam_captured==standard_walks[Queen])
        Result = square_d1;
      else if (pnam_captured==standard_walks[Bishop])
        Result = cou == White ? square_f1 : square_c1;
      else if (pnam_captured==standard_walks[King])
        Result = square_e1;
      else
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+nr_rows_on_board-2
                       : nr_of_slack_rows_below_board+nr_rows_on_board-1)));
    }
    else
    {
      if (is_pawn(pnam_captured))
        Result = col + (nr_of_slack_rows_below_board+nr_rows_on_board-2)*onerow;
      else if (circe_variant.is_frischauf && TSTFLAG(p_captured_spec,FrischAuf))
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+1
                       : nr_of_slack_rows_below_board)));
      else if (pnam_captured==standard_walks[King])
        Result = square_e8;
      else if (pnam_captured==standard_walks[Knight])
        Result = cou == White ? square_g8 : square_b8;
      else if (pnam_captured==standard_walks[Rook])
        Result = cou == White ? square_a8 : square_h8;
      else if (pnam_captured==standard_walks[Queen])
        Result = square_d8;
      else if (pnam_captured==standard_walks[Bishop])
        Result = cou == White ? square_c8 : square_f8;
      else
        Result = (col
                  + (onerow
                     *(CondFlag[glasgow]
                       ? nr_of_slack_rows_below_board+1
                       : nr_of_slack_rows_below_board)));
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(Result);
  TraceFunctionResultEnd();
  return(Result);
}

square renspiegel_polymorphic(PieNam p_captured, Flags p_captured_spec,
                              square sq_capture,
                              square sq_departure, square sq_arrival,
                              Side capturer)
{
  return rennormal(p_captured,p_captured_spec,sq_capture,advers(capturer));
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STCirceConsideringRebirth),
        alloc_pipe(STCirceDeterminingRebornPiece),
        alloc_pipe(STCircePlacingReborn),
        alloc_pipe(STCircePlaceReborn)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    branch_insert_slices_contextual(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void circe_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STCageCirceNonCapturingMoveFinder,
                                           &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_type const * const type = st->param;
    slice_index const prototype = alloc_pipe(*type);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the Circe solving machinery with some slice
 * @param si identifies root slice of stipulation
 * @param type slice type of which to add instances
 */
void circe_instrument_solving(slice_index si, slice_type type)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&type);
  stip_structure_traversal_override_single(&st,STCirceDeterminingRebornPiece,&instrument);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
