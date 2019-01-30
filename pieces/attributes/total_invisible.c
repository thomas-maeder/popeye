#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "position/position.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "solving/check.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/machinery/slack_length.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/move_effect_journal.h"
#include "output/plaintext/plaintext.h"
#include "optimisations/orthodox_square_observation.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

#include <stdlib.h>
#include <string.h>

unsigned int total_invisible_number = 3;

static ply ply_replayed;

stip_length_type result;

static square square_order[65];

typedef enum
{
  invisibles_placement_breadth_first,
  invisibles_placement_depth_first
} placement_strategy_type;

static placement_strategy_type invisibles_placement_strategy = invisibles_placement_breadth_first;

static struct
{
    Side side;
    piece_walk_type walk;
    square pos;
} piece_choice[3];

static void play_with_placed_invisibles(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TracePosition(being_solved.board,being_solved.spec);

  if (is_in_check(advers(SLICE_STARTER(si))))
    solve_result = previous_move_is_illegal;
  else
    pipe_solve_delegate(si);

  if (solve_result==previous_move_is_illegal)
  {
    if (result==previous_move_is_illegal)
      result = immobility_on_next_move;
  }
  else if (solve_result==immobility_on_next_move)
  {
    if (result==previous_move_is_illegal)
      result = immobility_on_next_move;
  }
  else if (solve_result>solve_nr_remaining)
    result = previous_move_has_not_solved;
  else
    result = previous_move_has_solved;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_invisible_breadth_first(slice_index si, unsigned int nr_remaining, square *pos_start)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_remaining);
  TraceSquare(*pos_start);
  TraceFunctionParamListEnd();

  --nr_remaining;

  {
    Side const side = piece_choice[nr_remaining].side;
    piece_walk_type const walk = piece_choice[nr_remaining].walk;
    SquareFlags PromSq = side==White ? WhPromSq : BlPromSq;
    SquareFlags BaseSq = side==White ? WhBaseSq : BlBaseSq;
    square *pos;

    for (pos = pos_start; *pos && result!=previous_move_has_not_solved; ++pos)
      if (is_square_empty(*pos))
        if (!(is_pawn(walk)
            && (TSTFLAG(sq_spec[*pos],PromSq) || TSTFLAG(sq_spec[*pos],BaseSq))))
        {
          square const s = *pos;
          ++being_solved.number_of_pieces[side][walk];
          occupy_square(s,walk,BIT(side));

          if (nr_remaining==0)
            play_with_placed_invisibles(si);
          else
          {
            *pos = 0;
            place_invisible_breadth_first(si,nr_remaining,pos_start-1);
            *pos = s;
          }

          empty_square(s);
          --being_solved.number_of_pieces[side][walk];
        }
  }

  ++nr_remaining;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_invisible_breadth_first(slice_index si, unsigned int nr_remaining)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_remaining);
  TraceFunctionParamListEnd();

  --nr_remaining;

  for (piece_choice[nr_remaining].walk = Pawn;
       piece_choice[nr_remaining].walk<=Bishop && result!=previous_move_has_not_solved;
       ++piece_choice[nr_remaining].walk)
    if (nr_remaining==0)
      place_invisible_breadth_first(si,total_invisible_number,square_order+total_invisible_number);
    else
      walk_invisible_breadth_first(si,nr_remaining);

  ++nr_remaining;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void colour_invisible_breadth_first(slice_index si, unsigned int nr_remaining)
{
  Side adversary;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_remaining);
  TraceFunctionParamListEnd();

  --nr_remaining;

  // TODO make this readable
  for (adversary = White; adversary!=nr_sides && result!=previous_move_has_not_solved; ++adversary)
  {
    piece_choice[nr_remaining].side = advers(adversary);

    if (nr_remaining==0)
      walk_invisible_breadth_first(si,total_invisible_number);
    else
      colour_invisible_breadth_first(si,nr_remaining);
  }

  ++nr_remaining;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


static void colour_invisble_depth_first(slice_index si, unsigned int nr_remaining);

static void walk_invisible_depth_first(slice_index si, unsigned int nr_remaining)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_remaining);
  TraceFunctionParamListEnd();

  for (piece_choice[nr_remaining].walk = Pawn;
       piece_choice[nr_remaining].walk<=Bishop && result!=previous_move_has_not_solved;
       ++piece_choice[nr_remaining].walk)
  {
    Side const side = piece_choice[nr_remaining].side;
    piece_walk_type const walk = piece_choice[nr_remaining].walk;
    SquareFlags PromSq = side==White ? WhPromSq : BlPromSq;
    SquareFlags BaseSq = side==White ? WhBaseSq : BlBaseSq;
    square const s = piece_choice[nr_remaining].pos;

    TraceWalk(walk);TraceEOL();

    if (!(is_pawn(walk)
        && (TSTFLAG(sq_spec[s],PromSq) || TSTFLAG(sq_spec[s],BaseSq))))
    {
      ++being_solved.number_of_pieces[side][walk];
      occupy_square(s,walk,BIT(side));

      if (nr_remaining==0)
        play_with_placed_invisibles(si);
      else
        colour_invisble_depth_first(si,nr_remaining);

      empty_square(s);
      --being_solved.number_of_pieces[side][walk];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void colour_invisble_depth_first(slice_index si, unsigned int nr_remaining)
{
  Side adversary;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_remaining);
  TraceFunctionParamListEnd();

  --nr_remaining;

  // TODO make this readable
  for (adversary = White; adversary!=nr_sides && result!=previous_move_has_not_solved; ++adversary)
  {
    piece_choice[nr_remaining].side = advers(adversary);
    TraceEnumerator(Side,piece_choice[nr_remaining].side);TraceEOL();
    walk_invisible_depth_first(si,nr_remaining);
  }

  ++nr_remaining;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void try_square_combination(slice_index si)
{
  boolean success;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    for (unsigned int i = 0; i!=total_invisible_number; ++i)
    {
      ++being_solved.number_of_pieces[Black][Dummy];
      ++being_solved.number_of_pieces[White][Dummy];
      occupy_square(piece_choice[i].pos,Dummy,BIT(White)|BIT(Black));
    }

    {
      stip_length_type const save_solve_result = solve_result;
      play_with_placed_invisibles(si);
      success = solve_result>immobility_on_next_move;
      solve_result = save_solve_result;
    }

    for (unsigned int i = 0; i!=total_invisible_number; ++i)
    {
      empty_square(piece_choice[i].pos);
      --being_solved.number_of_pieces[Black][Dummy];
      --being_solved.number_of_pieces[White][Dummy];
    }
  }

  if (success)
    colour_invisble_depth_first(si,total_invisible_number);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_invisible_depth_first(slice_index si, unsigned int nr_remaining)
{
  square *pos;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_remaining);
  TraceFunctionParamListEnd();

  --nr_remaining;

  for (pos = square_order+nr_remaining; *pos && result!=previous_move_has_not_solved; ++pos)
    if (is_square_empty(*pos))
    {
      piece_choice[nr_remaining].pos = *pos;
      *pos = 0;

      TraceSquare(piece_choice[nr_remaining].pos);TraceEOL();

      if (nr_remaining==0)
        try_square_combination(si);
      else
        place_invisible_depth_first(si,nr_remaining);

      *pos = piece_choice[nr_remaining].pos;
    }

  ++nr_remaining;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void distribute_invisibles(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = previous_move_is_illegal;
  if (invisibles_placement_strategy==invisibles_placement_breadth_first)
    colour_invisible_breadth_first(si,total_invisible_number);
  else
    place_invisible_depth_first(si,total_invisible_number);
  solve_result = result==immobility_on_next_move ? previous_move_has_not_solved : result;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unwrap_move_effects(ply current_ply, slice_index si)
{
  ply const save_nbply = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",current_ply);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  undo_move_effects();

  if (parent_ply[nbply]==ply_retro_move)
  {
    ply_replayed = nbply;
    nbply = current_ply;
    distribute_invisibles(si);
  }
  else
  {
    nbply = parent_ply[nbply];
    unwrap_move_effects(current_ply,si);
  }

  nbply = save_nbply;

  redo_move_effects();

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
void total_invisible_move_sequence_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply-ply_retro_move);TraceEOL();

  if (is_square_uninterceptably_observed_ortho(Black,
                                               being_solved.king_square[White]))
    solve_result = previous_move_is_illegal;
  else if (count_interceptable_orthodox_checks(White,being_solved.king_square[Black])>total_invisible_number
           || count_interceptable_orthodox_checks(Black,being_solved.king_square[White])>total_invisible_number)
    solve_result = previous_move_is_illegal;
  else
  {
    /* make sure that our length corresponds to the length of the tested move sequence
     * (which may vary if STFindShortest is used)
     */
    assert(slices[SLICE_NEXT2(si)].type==STHelpAdapter);
    slices[SLICE_NEXT2(si)].u.branch.length = slack_length+(nbply-ply_retro_move);

    unwrap_move_effects(nbply,si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_move_still_playable(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].departure;
    square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].arrival;

    TraceSquare(sq_departure);
    TraceSquare(sq_arrival);
    TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture);
    TraceEOL();

    assert(TSTFLAG(being_solved.spec[sq_departure],SLICE_STARTER(si)));
    // TODO optimize with intelligent mode?
    generate_moves_for_piece(sq_departure);

    {
      numecoup start = MOVEBASE_OF_PLY(nbply);
      numecoup i;
      numecoup new_top = start;
      for (i = start+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
      {
        assert(move_generation_stack[i].departure==sq_departure);
        if (move_generation_stack[i].arrival==sq_arrival)
        {
          ++new_top;
          move_generation_stack[new_top] = move_generation_stack[i];
          break;
        }
      }

      SET_CURRMOVE(nbply,new_top);
    }

    result = CURRMOVE_OF_PLY(nbply)>MOVEBASE_OF_PLY(nbply);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void copy_move_effects(void)
{
  move_effect_journal_index_type replayed_curr = move_effect_journal_base[ply_replayed];
  move_effect_journal_index_type const replayed_top = move_effect_journal_base[ply_replayed+1];
  move_effect_journal_index_type curr = move_effect_journal_base[nbply+1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",ply_replayed);
  TraceValue("%u",move_effect_journal_base[ply_replayed]);
  TraceValue("%u",move_effect_journal_base[ply_replayed+1]);
  TraceValue("%u",nbply);
  TraceEOL();

  // TODO memcpy()?
  while (replayed_curr!=replayed_top)
  {
    if (move_effect_journal[replayed_curr].type==move_effect_no_piece_removal
        && move_effect_journal[replayed_curr+1].type==move_effect_piece_movement
        && move_effect_journal[replayed_curr+1].reason==move_effect_reason_moving_piece_movement
        && !is_square_empty(move_effect_journal[replayed_curr+1].u.piece_movement.to))
    {
      square const from = move_effect_journal[replayed_curr+1].u.piece_movement.to;

      move_effect_journal[curr].type = move_effect_piece_removal;
      move_effect_journal[curr].reason = move_effect_reason_regular_capture;
      move_effect_journal[curr].u.piece_removal.on = from;
      move_effect_journal[curr].u.piece_removal.flags = being_solved.spec[from];
      move_effect_journal[curr].u.piece_removal.walk =  being_solved.board[from];
    }
    else
      move_effect_journal[curr] = move_effect_journal[replayed_curr];
    ++replayed_curr;
    ++curr;
  }

  move_effect_journal_base[nbply+1] = curr;

  TraceValue("%u",move_effect_journal_base[nbply]);
  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceEOL();

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
void total_invisible_move_repeater_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));

  if (is_move_still_playable(si))
  {
    copy_move_effects();
    redo_move_effects();
    ++ply_replayed;
    pipe_solve_delegate(si);
    --ply_replayed;
    undo_move_effects();
  }
  else
    solve_result = previous_move_is_illegal;

  finply();

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
void total_invisible_uninterceptable_selfcheck_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_uninterceptably_observed_ortho(White,
                                               being_solved.king_square[Black])
      || is_square_uninterceptably_observed_ortho(Black,
                                                  being_solved.king_square[White]))
    solve_result = previous_move_is_illegal;
  else
  {
    unsigned int count_white_checks = count_interceptable_orthodox_checks(White,being_solved.king_square[Black]);
    unsigned int count_black_checks = count_interceptable_orthodox_checks(Black,being_solved.king_square[White]);
    if (count_white_checks>total_invisible_number
        || count_black_checks>total_invisible_number)
       solve_result = previous_move_is_illegal;
     else if (count_white_checks>0 || count_black_checks>0)
     {
       placement_strategy_type save_strategy = invisibles_placement_strategy;
       invisibles_placement_strategy = invisibles_placement_depth_first;
       pipe_solve_delegate(si);
       invisibles_placement_strategy = save_strategy;
     }
     else
       pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_generator(slice_index si,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleMoveSequenceMoveRepeater);
    SLICE_STARTER(prototype) = SLICE_STARTER(si);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_the_pipe(slice_index si,
                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_replay_branch(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;

    stip_structure_traversal_init_nested(&st_nested,st,0);
    stip_structure_traversal_override_by_structure(&st_nested,
                                                   slice_structure_fork,
                                                   &stip_traverse_structure_children_pipe);
    // TODO prevent instrumentation in the first place?
    stip_structure_traversal_override_single(&st_nested,
                                             STFindShortest,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STFindAttack,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveEffectJournalUndoer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPostMoveIterationInitialiser,
                                             &remove_the_pipe);
    // TODO like this, this would cause a slice leak (STCastlingPlayer is a fork type!)
//    stip_structure_traversal_override_single(&st_nested,
//                                             STCastlingPlayer,
//                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMovePlayer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPawnPromoter,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,STMoveGenerator,&subsitute_generator);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_self_check_guard(slice_index si,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* This iteration ends at STTotalInvisibleMoveSequenceTester. We can therefore
   * blindly tamper with all STSelfCheckGuard slices that we meet.
   */
  stip_traverse_structure_children_pipe(si,st);

  SLICE_TYPE(si) = STTotalInvisibleUninterceptableSelfCheckGuard;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static int square_compare(void const *v1, void const *v2)
{
  square const *s1 = v1;
  square const *s2 = v2;
  square const kpos = being_solved.king_square[Black];

  int const dx1 = (kpos-*s1)%onerow;
  int const dy1 = (kpos-*s1)/onerow;

  int const dx2 = (kpos-*s2)%onerow;
  int const dy2 = (kpos-*s2)/onerow;

  return (dx1*dx1+dy1*dy1)-(dx2*dx2+dy2*dy2);
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
void total_invisible_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STTotalInvisibleMoveSequenceTester,
                                             &instrument_replay_branch);
    stip_structure_traversal_override_single(&st,
                                             STSelfCheckGuard,
                                             &remove_self_check_guard);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  output_plaintext_check_indication_disabled = true;

  memmove(square_order, boardnum, sizeof boardnum);
  qsort(square_order, 64, sizeof square_order[0], &square_compare);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean instrumenting;
    slice_index the_copy;
    stip_length_type length;
} insertion_state_type;

static void insert_copy(slice_index si,
                        stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->the_copy==no_slice)
    stip_traverse_structure_children(si,st);
  else
  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const substitute = alloc_pipe(STTotalInvisibleMoveSequenceTester);
    pipe_link(proxy,substitute);
    link_to_branch(substitute,state->the_copy);
    SLICE_NEXT2(substitute) = state->the_copy;
    state->the_copy = no_slice;
    dealloc_slices(SLICE_NEXT2(si));
    SLICE_NEXT2(si) = proxy;

    assert(state->length!=no_stip_length);
    if (state->length%2!=0)
      pipe_append(proxy,alloc_pipe(STMoveInverter));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void copy_help_branch(slice_index si,
                             stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",state->instrumenting);
  TraceEOL();

  state->length = slices[si].u.branch.length;

  if (state->instrumenting)
    stip_traverse_structure_children(si,st);
  else
  {
    state->instrumenting = true;
    state->the_copy = stip_deep_copy(si);
    stip_traverse_structure_children(si,st);
    assert(state->the_copy==no_slice);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with support for Total Invisible pieces
 * @param si identifies the root slice of the stipulation
 */
void solving_instrument_total_invisible(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  // later:
  // - in original
  //   - generate pawn captures to empty squares (if an invisible piece is left)
  //   - insert revelation logic
  // - in copy
  //   - logic for iteration over all possibilities of invisibles
  //     - special case of invisible king
  //     - special case: position has no legal placement of all invisibles may have to be dealt with:
  //       - self-check in each attempt
  //       - not enough empty squares :-)
  //   - substitute for STFindShortest

  // bail out at STAttackAdapter

  // input for total_invisible_number, initialize to 0

  // what about:
  // - structured stipulations?
  // - goals that don't involve immobility
  // ?

  // we shouldn't need to set the starter of STTotalInvisibleMoveSequenceMoveRepeater

  // check indication should also be deactivated in tree output

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInstrumenter);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    stip_structure_traversal st;
    insertion_state_type state = { false, no_slice, no_stip_length };

    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override_single(&st,
                                             STHelpAdapter,
                                             &copy_help_branch);
    stip_structure_traversal_override_single(&st,
                                             STGoalReachedTester,
                                             &insert_copy);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
