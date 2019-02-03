#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "position/position.h"
#include "position/move_diff_code.h"
#include "position/effects/piece_creation.h"
#include "position/effects/null_move.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/goals/slice_insertion.h"
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

static unsigned int bound_invisible_number = 0;

static unsigned int pawn_victims_number = 0;

static ply ply_replayed;

static stip_length_type combined_result;

static square square_order[65];

static struct
{
    Side side;
    piece_walk_type walk;
    square pos;
} piece_choice[3];

static enum
{
  regular_play,
  replaying_moves
} play_phase = regular_play;

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

  if (solve_result>combined_result)
    combined_result = solve_result;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_square_used(square s, unsigned int base)
{
  unsigned int i;
  for (i = 0; i!=base; ++i)
    if (s==piece_choice[i].pos)
      return true;

  return false;
}

static void place_invisible_breadth_first(slice_index si,
                                          square *pos_start,
                                          unsigned int idx,
                                          unsigned int base,
                                          unsigned int top)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",idx);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",top);
  TraceFunctionParamListEnd();

  {
    Side const side = piece_choice[idx].side;
    piece_walk_type const walk = piece_choice[idx].walk;
    SquareFlags PromSq = side==White ? WhPromSq : BlPromSq;
    SquareFlags BaseSq = side==White ? WhBaseSq : BlBaseSq;
    square *pos;

    for (pos = pos_start;
         *pos && combined_result!=previous_move_has_not_solved;
         ++pos)
      if (is_square_empty(*pos) && !is_square_used(*pos,base))
        if (!(is_pawn(walk)
            && (TSTFLAG(sq_spec[*pos],PromSq) || TSTFLAG(sq_spec[*pos],BaseSq))))
        {
          square const s = *pos;
          ++being_solved.number_of_pieces[side][walk];
          occupy_square(s,walk,BIT(side));

          if (idx+1==top)
            play_with_placed_invisibles(si);
          else
          {
            *pos = 0;
            place_invisible_breadth_first(si,pos_start-1,idx+1,base,top);
            *pos = s;
          }

          empty_square(s);
          --being_solved.number_of_pieces[side][walk];
        }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_invisible_breadth_first(slice_index si,
                                         unsigned int idx,
                                         unsigned int base,
                                         unsigned int top)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",idx);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",top);
  TraceFunctionParamListEnd();

  for (piece_choice[idx].walk = Pawn;
       piece_choice[idx].walk<=Bishop && combined_result!=previous_move_has_not_solved;
       ++piece_choice[idx].walk)
    if (idx+1==top)
      place_invisible_breadth_first(si,square_order+top-base-1,base,base,top);
    else
      walk_invisible_breadth_first(si,idx+1,base,top);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void colour_invisible_breadth_first(slice_index si,
                                           unsigned int idx,
                                           unsigned int base,
                                           unsigned int top)
{
  Side adversary;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",idx);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",top);
  TraceFunctionParamListEnd();

  // TODO make this readable
  for (adversary = White; adversary!=nr_sides && combined_result!=previous_move_has_not_solved; ++adversary)
  {
    piece_choice[idx].side = advers(adversary);

    if (idx+1==top)
      walk_invisible_breadth_first(si,base,base,top);
    else
      colour_invisible_breadth_first(si,idx+1,base,top);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_invisible_depth_first(slice_index si,
                                       unsigned int idx,
                                       unsigned int base,
                                       unsigned int top)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",idx);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",top);
  TraceFunctionParamListEnd();

  for (piece_choice[idx].walk = Pawn;
       piece_choice[idx].walk<=Bishop && combined_result!=previous_move_has_not_solved;
       ++piece_choice[idx].walk)
  {
    Side const side = piece_choice[idx].side;
    piece_walk_type const walk = piece_choice[idx].walk;
    SquareFlags PromSq = side==White ? WhPromSq : BlPromSq;
    SquareFlags BaseSq = side==White ? WhBaseSq : BlBaseSq;
    square const s = piece_choice[idx].pos;

    TraceWalk(walk);TraceEOL();

    if (!(is_pawn(walk)
        && (TSTFLAG(sq_spec[s],PromSq) || TSTFLAG(sq_spec[s],BaseSq))))
    {
      ++being_solved.number_of_pieces[side][walk];
      occupy_square(s,walk,BIT(side));

      if (idx+1==top)
      {
        if ((top-base)==total_invisible_number)
          play_with_placed_invisibles(si);
        else
        {
          unsigned int base_breadth_first = top;
          unsigned int top_breadth_first = base_breadth_first+total_invisible_number-(top-base);
          colour_invisible_breadth_first(si,base_breadth_first,base_breadth_first,top_breadth_first);
        }
      }
      else
        walk_invisible_depth_first(si,idx+1,base,top);

      --being_solved.number_of_pieces[side][walk];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void colour_invisble_depth_first(slice_index si,
                                        unsigned int idx,
                                        unsigned int base,
                                        unsigned int top)
{
  Side adversary;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",idx);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",top);
  TraceFunctionParamListEnd();

  // TODO make this readable
  for (adversary = White; adversary!=nr_sides && combined_result!=previous_move_has_not_solved; ++adversary)
  {
    piece_choice[idx].side = advers(adversary);
    TraceEnumerator(Side,piece_choice[idx].side);TraceEOL();
    if (idx+1==top)
      walk_invisible_depth_first(si,base,base,top);
    else
      colour_invisble_depth_first(si,idx+1,base,top);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_invisible_depth_first(slice_index si,
                                        square const *pos_start,
                                        unsigned int idx,
                                        unsigned int base,
                                        unsigned int top)
{
  square const *pos;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",idx);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",top);
  TraceFunctionParamListEnd();

  for (pos = pos_start; *pos && combined_result!=previous_move_has_not_solved; ++pos)
    if (is_square_empty(*pos) && !is_square_used(*pos,base))
    {
      piece_choice[idx].pos = *pos;
      occupy_square(*pos,Dummy,BIT(White)|BIT(Black));

      TraceSquare(piece_choice[idx].pos);TraceEOL();

      if (idx+1==top)
      {
        if (!is_in_check(advers(SLICE_STARTER(si))))
        {
          boolean success;
          stip_length_type const save_solve_result = solve_result;
          pipe_solve_delegate(si);
          success = solve_result>immobility_on_next_move;
          solve_result = save_solve_result;
          if (success)
          {
            TracePosition(being_solved.board,being_solved.spec);
            {
              unsigned int i;
              for (i = base; i!=top; ++i)
                empty_square(piece_choice[i].pos);
            }

            colour_invisble_depth_first(si,base,base,top);

            {
              unsigned int i;
              for (i = base; i!=top; ++i)
                occupy_square(piece_choice[i].pos,Dummy,BIT(White)|BIT(Black));;
            }
          }
        }
      }
      else
        place_invisible_depth_first(si,pos+1,idx+1,base,top);

      empty_square(*pos);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void distribute_invisibles(slice_index si, unsigned int base)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  if (bound_invisible_number==0)
  {
    unsigned int top = base+total_invisible_number;
    colour_invisible_breadth_first(si,base,base,top);
  }
  else if (base+bound_invisible_number<=total_invisible_number)
  {
    unsigned int i;
    for (i = bound_invisible_number;
         i<=total_invisible_number && combined_result!=previous_move_has_not_solved;
         ++i)
    {
      unsigned int top = base+i;
      place_invisible_depth_first(si,square_order,base,base,top);
    }
  }

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
    combined_result = previous_move_is_illegal;
    distribute_invisibles(si,pawn_victims_number);
    solve_result = combined_result==immobility_on_next_move ? previous_move_has_not_solved : combined_result;
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
  else
  {
    unsigned int count_white_checks = count_interceptable_orthodox_checks(White,being_solved.king_square[Black]);
    unsigned int count_black_checks = count_interceptable_orthodox_checks(Black,being_solved.king_square[White]);
    unsigned int const count_max = count_white_checks>count_black_checks ? count_white_checks : count_black_checks;

    /* make sure that our length corresponds to the length of the tested move sequence
     * (which may vary if STFindShortest is used)
     */
    assert(slices[SLICE_NEXT2(si)].type==STHelpAdapter);
    slices[SLICE_NEXT2(si)].u.branch.length = slack_length+(nbply-ply_retro_move);

    if (count_max>total_invisible_number)
      solve_result = previous_move_is_illegal;
    else
    {
      unsigned int const save_bound = bound_invisible_number;
      if (count_max>bound_invisible_number)
        bound_invisible_number = count_max;

      play_phase = replaying_moves;

      if (pawn_victims_number+bound_invisible_number>total_invisible_number)
        solve_result = previous_move_is_illegal;
      else
        unwrap_move_effects(nbply,si);

      play_phase = regular_play;

      bound_invisible_number = save_bound;
    }
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

  while (replayed_curr!=replayed_top)
  {
    /* accidental capture of a placed total invisible? */
    if (move_effect_journal[replayed_curr+move_effect_journal_index_offset_capture].type==move_effect_no_piece_removal
        && move_effect_journal[replayed_curr+move_effect_journal_index_offset_movement].type==move_effect_piece_movement
        && move_effect_journal[replayed_curr+move_effect_journal_index_offset_movement].reason==move_effect_reason_moving_piece_movement
        && !is_square_empty(move_effect_journal[replayed_curr+move_effect_journal_index_offset_movement].u.piece_movement.to))
    {
      square const from = move_effect_journal[replayed_curr+move_effect_journal_index_offset_movement].u.piece_movement.to;

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
    unsigned int const count_max = count_white_checks>count_black_checks ? count_white_checks : count_black_checks;
    if (count_max>total_invisible_number)
       solve_result = previous_move_is_illegal;
    else
    {
      unsigned int const save_bound = bound_invisible_number;
      if (count_max>bound_invisible_number)
        bound_invisible_number = count_max;

      pipe_solve_delegate(si);

      bound_invisible_number = save_bound;
    }
  }

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
void total_invisible_goal_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO remove self check guard
  if (is_in_check(advers(SLICE_STARTER(si))))
    solve_result = previous_move_is_illegal;
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void total_invisible_pawn_generate_pawn_captures(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  {
    square const sq_departure = curr_generation->departure ;
    Side const side = trait[nbply];
    if (TSTFLAG(being_solved.spec[sq_departure],side) && being_solved.board[sq_departure]==Pawn)
    {
      int const dir_vertical = trait[nbply]==White ? dir_up : dir_down;

      curr_generation->arrival = curr_generation->departure+dir_vertical+dir_left;
      if (is_square_empty(curr_generation->arrival))
        push_special_move(capture_of_invisible);

      curr_generation->arrival = curr_generation->departure+dir_vertical+dir_right;
      if (is_square_empty(curr_generation->arrival))
        push_special_move(capture_of_invisible);
    }
  }

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
void total_invisible_special_moves_player_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt * const move_gen_top = move_generation_stack+curr;
    square const sq_arrival = move_gen_top->arrival;
    square const sq_capture = move_gen_top->capture;
    Side const side_victim = advers(SLICE_STARTER(si));

    TraceValue("%u",sq_capture);
    TraceValue("%u",capture_of_invisible);
    TraceEOL();

    if (sq_capture==capture_of_invisible)
    {
      /* inject the creation of a dummy piece into the previous ply - very dirty... */
      --nbply;
      move_effect_journal_do_piece_creation(move_effect_reason_removal_of_invisible,
                                            sq_arrival,
                                            Dummy,
                                            BIT(side_victim),
                                            side_victim);
      ++nbply;
      ++move_effect_journal_base[nbply+1];

      // TODO this doesn't work once we add Locusts and the like
      move_gen_top->capture = sq_arrival;

      piece_choice[pawn_victims_number].pos = sq_arrival;

      ++pawn_victims_number;
      pipe_solve_delegate(si);
      --pawn_victims_number;

      --move_effect_journal_base[nbply];
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

static void remember_self_check_guard(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_index * const remembered = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  *remembered = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_goal_guard(slice_index si,
                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index remembered = no_slice;

    stip_structure_traversal st_nested;
    stip_structure_traversal_init_nested(&st_nested,st,&remembered);
    stip_structure_traversal_override_single(&st_nested,
                                             STSelfCheckGuard,
                                             &remember_self_check_guard);
    stip_traverse_structure(SLICE_NEXT2(si),&st_nested);

    if (remembered!=no_slice)
    {
      slice_index prototype = alloc_pipe(STTotalInvisibleGoalGuard);
      SLICE_STARTER(prototype) = SLICE_STARTER(remembered);
      goal_branch_insert_slices(SLICE_NEXT2(si),&prototype,1);
      pipe_remove(remembered);
    }
  }

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
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveGenerator,
                                             &subsitute_generator);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachedTester,
                                             &subsitute_goal_guard);
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
  int result;
  square const *s1 = v1;
  square const *s2 = v2;
  square const kpos = being_solved.king_square[Black];

  result = move_diff_code[abs(kpos-*s1)]-move_diff_code[abs(kpos-*s2)];

  if (being_solved.king_square[White]!=initsquare)
  {
    square const kpos = being_solved.king_square[White];
    result += move_diff_code[abs(kpos-*s1)]-move_diff_code[abs(kpos-*s2)];
  }

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

  solving_instrument_move_generation(si,nr_sides,STTotalInvisiblePawnCaptureGenerator);

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

    {
      slice_index const prototype = alloc_pipe(STTotalInvisibleSpecialMovesPlayer);
      slice_insertion_insert(si,&prototype,1);
    }
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

  // we shouldn't need to set the starter of
  // - STTotalInvisibleMoveSequenceMoveRepeater
  // - STTotalInvisibleGoalGuard

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
