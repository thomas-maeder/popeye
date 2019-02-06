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
#include "output/plaintext/pieces.h"
#include "optimisations/orthodox_square_observation.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

#include <stdlib.h>
#include <string.h>

unsigned int total_invisible_number;

static unsigned int nr_bound_invisible = 0;
static square bound_around;

static unsigned int nr_placed_victims = 0;

static unsigned int nr_placed_interceptors = 0;

static ply ply_replayed;

static stip_length_type combined_result;

static square square_order_unbound[65];

static unsigned long nr_tries_with_pieces;
static unsigned long nr_tries_with_dummies;

static struct
{
    Side side;
    piece_walk_type walk;
    square pos;
} piece_choice[nr_squares_on_board];

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
//  else if (++nr_tries_with_pieces > (1<<(total_invisible_number*5)))
//    solve_result = previous_move_has_not_solved;
  else
  {
    ++nr_tries_with_pieces;
    pipe_solve_delegate(si);
  }

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

          piece_choice[idx].pos = s;

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
      place_invisible_breadth_first(si,square_order_unbound+top-base-1,base,base,top);
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
  piece_walk_type walk;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",idx);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",top);
  TraceFunctionParamListEnd();

  for (walk = Pawn;
       walk<=Bishop && combined_result!=previous_move_has_not_solved;
       ++walk)
  {
    Side const side = piece_choice[idx].side;
    SquareFlags PromSq = side==White ? WhPromSq : BlPromSq;
    SquareFlags BaseSq = side==White ? WhBaseSq : BlBaseSq;
    square const s = piece_choice[idx].pos;
    piece_choice[idx].walk = walk;

    TraceWalk(walk);TraceEOL();

    if (!(is_pawn(walk)
        && (TSTFLAG(sq_spec[s],PromSq) || TSTFLAG(sq_spec[s],BaseSq))))
    {
      ++being_solved.number_of_pieces[side][walk];
      occupy_square(s,walk,BIT(side));

      if (idx+1==top)
      {
        if (top==total_invisible_number)
          play_with_placed_invisibles(si);
        else
        {
          unsigned int i;
          for (i = top+1;
               i<=total_invisible_number && combined_result!=previous_move_has_not_solved;
               ++i)
          colour_invisible_breadth_first(si,top,top,i);
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
//          if (++nr_tries_with_dummies > (1<<(total_invisible_number*5)))
//            solve_result = previous_move_has_not_solved;
//          else
          ++nr_tries_with_dummies;
          {
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
      }
      else
        place_invisible_depth_first(si,pos+1,idx+1,base,top);

      empty_square(*pos);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static int square_compare_around_specific_square(void const *v1, void const *v2)
{
  square const *s1 = v1;
  square const *s2 = v2;

  return move_diff_code[abs(bound_around-*s1)]-move_diff_code[abs(bound_around-*s2)];
}

static void distribute_invisibles(slice_index si, unsigned int base)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  /* make sure that our length corresponds to the length of the tested move sequence
   * (which may vary if STFindShortest is used)
   */
  assert(slices[SLICE_NEXT2(si)].type==STHelpAdapter);
  slices[SLICE_NEXT2(si)].u.branch.length = slack_length+(nbply-ply_retro_move);

  play_phase = replaying_moves;

  if (base==total_invisible_number)
    play_with_placed_invisibles(si);
  else if (nr_bound_invisible==0)
  {
    unsigned int i;
    for (i = base+1;
         i<=total_invisible_number && combined_result!=previous_move_has_not_solved;
         ++i)
      colour_invisible_breadth_first(si,base,base,i);
  }
  else if (base+nr_bound_invisible<=total_invisible_number)
  {
    square square_order_bound[65];
    memmove(square_order_bound, boardnum, sizeof boardnum);
    qsort(square_order_bound, 64, sizeof square_order_bound[0], &square_compare_around_specific_square);

    {
      unsigned int i;
      for (i = nr_bound_invisible;
           base+i<=total_invisible_number && combined_result!=previous_move_has_not_solved;
           ++i)
        place_invisible_depth_first(si,square_order_bound,base,base,base+i);
    }
  }

  play_phase = regular_play;

  if (nr_tries_with_dummies>10000
      || nr_tries_with_pieces>100000)
  {
    printf("\n");
    move_generator_write_history();
    printf(" vic:%u",base);
    printf(" bou:%u",nr_bound_invisible);
    printf(" aro:");WriteSquare(&output_plaintext_engine,stdout,bound_around);
    printf(" dum:%lu",nr_tries_with_dummies);
    printf(" pie:%lu ",nr_tries_with_pieces);
  }
  nr_tries_with_dummies = 0;
  nr_tries_with_pieces = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_checks_in_diagram(slice_index si, unsigned int base)
{
  unsigned int count_white_checks = count_interceptable_orthodox_checks(White,being_solved.king_square[Black]);
  unsigned int count_black_checks = count_interceptable_orthodox_checks(Black,being_solved.king_square[White]);
  unsigned int const count_max = count_white_checks>count_black_checks ? count_white_checks : count_black_checks;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  TraceValue("%u",count_white_checks);TraceValue("%u",count_black_checks);TraceValue("%u",count_max);TraceEOL();

  if (base+count_max>total_invisible_number)
    solve_result = previous_move_is_illegal;
  else if (count_max>nr_bound_invisible)
  {
    unsigned int const save_bound = nr_bound_invisible;
    nr_bound_invisible = count_max;
    bound_around = being_solved.king_square[count_white_checks>count_black_checks ? Black : White];
    distribute_invisibles(si,base);
    nr_bound_invisible = save_bound;
  }
  else
    distribute_invisibles(si,base);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptors(slice_index si, unsigned int base, unsigned int idx, unsigned int top)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",idx);
  TraceFunctionParam("%u",top);
  TraceFunctionParamListEnd();

  if (idx==top)
    deal_with_checks_in_diagram(si,top);
  else
  {
    square const place = piece_choice[idx].pos;
    TraceSquare(place);TraceEOL();
    if (is_square_empty(place))
    {
      occupy_square(place,Dummy,BIT(White)|BIT(Black));
      place_interceptors(si,base,idx+1,top);
      empty_square(place);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_check_to_be_intercepted(ply current_ply, slice_index si, unsigned int base);

static void unwrap_move_effects(ply current_ply, slice_index si, unsigned int base)
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
    place_interceptors(si,base,base,base+nr_placed_interceptors);
    solve_result = combined_result==immobility_on_next_move ? previous_move_has_not_solved : combined_result;
  }
  else
  {
    nbply = parent_ply[nbply];
    deal_with_check_to_be_intercepted(current_ply,si,base);
  }

  nbply = save_nbply;

  redo_move_effects();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square rider_to_distance_2(slice_index si)
{
  square result = nullsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
    square const sq_departure = move_gen_top->departure;
    square const sq_arrival = move_gen_top->arrival;
    piece_walk_type const walk_moving = being_solved.board[sq_arrival];

    TraceSquare(sq_departure);
    TraceSquare(sq_arrival);
    TraceWalk(walk_moving);
    TraceEOL();

    if (is_rider(walk_moving))
    {
      Side const side_in_check = advers(trait[nbply]);
      square const king_pos = being_solved.king_square[side_in_check];
      int const diff_departure = king_pos-sq_departure;
      int const diff_arrival = king_pos-sq_arrival;
      int const dir_departure = CheckDir[walk_moving][diff_departure];
      int const dir_arrival = CheckDir[walk_moving][diff_arrival];
      square const sq_intermediate = sq_arrival+dir_arrival;
      TraceEnumerator(Side,side_in_check);
      TraceSquare(king_pos);
      TraceValue("%d",diff_departure);
      TraceValue("%d",diff_arrival);
      TraceValue("%d",dir_departure);
      TraceValue("%d",dir_arrival);
      TraceEOL();

      if (dir_departure==dir_arrival && 2*dir_arrival==diff_arrival
          && is_square_empty(sq_intermediate))
        result = sq_intermediate;
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static square king_to_distance_2(slice_index si)
{
  square result = nullsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
    square const sq_arrival = move_gen_top->arrival;
    TraceSquare(sq_arrival);
    TraceEOL();

    if (TSTFLAG(being_solved.spec[sq_arrival],Royal))
    {
      Side const side_in_check = trait[nbply];
      Side const side_checker = advers(side_in_check);
      square const king_pos = being_solved.king_square[side_in_check];
      TraceEnumerator(Side,side_in_check);
      TraceEnumerator(Side,side_checker);
      TraceSquare(king_pos);
      TraceEOL();
      {
        unsigned int i;
        for (i = vec_rook_start; i<=vec_rook_end; ++i)
        {
          square const sq_intercepted = king_pos+vec[i];
          if (is_square_empty(sq_intercepted))
          {
            square const pos_checker = sq_intercepted+vec[i];
            piece_walk_type const walk_checker = being_solved.board[pos_checker];
            if (TSTFLAG(being_solved.spec[pos_checker],side_checker)
                && (walk_checker==Queen || walk_checker==Rook))
              result = sq_intercepted;
          }
        }
      }
      if (result==nullsquare)
      {
        unsigned int i;
        for (i = vec_bishop_start; i<=vec_bishop_end; ++i)
        {
          square const sq_intercepted = king_pos+vec[i];
          if (is_square_empty(sq_intercepted))
          {
            square const pos_checker = sq_intercepted+vec[i];
            piece_walk_type const walk_checker = being_solved.board[pos_checker];
            if (TSTFLAG(being_solved.spec[pos_checker],side_checker)
                && (walk_checker==Queen || walk_checker==Bishop))
              result = sq_intercepted;
          }
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static square discovered_check_from_distance_2(slice_index si)
{
  square result = nullsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
    square const sq_departure = move_gen_top->departure;
    Side const side_checker = trait[nbply];
    Side const side_in_check = advers(side_checker);
    square const king_pos = being_solved.king_square[side_in_check];
    int const diff_departure = king_pos-sq_departure;
    square const sq_checker = sq_departure-diff_departure;
    piece_walk_type const walk_checker = being_solved.board[sq_checker];
    TraceSquare(sq_departure);
    TraceEnumerator(Side,side_checker);
    TraceEnumerator(Side,side_in_check);
    TraceSquare(king_pos);
    TraceValue("%d",diff_departure);
    TraceSquare(sq_checker);
    TraceWalk(walk_checker);

    if (TSTFLAG(being_solved.spec[sq_checker],side_checker)
        && is_rider(walk_checker))
    {
      int const diff_checker = king_pos-sq_checker;
      int const dir_departure = CheckDir[walk_checker][diff_departure];
      int const dir_checker = CheckDir[walk_checker][diff_checker];
      TraceValue("%d",diff_checker);
      TraceValue("%d",dir_departure);
      TraceValue("%d",dir_checker);
      TraceEOL();

      if (dir_departure==diff_departure && dir_departure==dir_checker)
        result = sq_departure;
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static square find_interception_placement(slice_index si)
{
  square result = nullsquare;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = rider_to_distance_2(si);

  if (result==nullsquare)
    result = king_to_distance_2(si);

  if (result==nullsquare)
    result = discovered_check_from_distance_2(si);

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static void deal_with_check_to_be_intercepted(ply current_ply, slice_index si, unsigned int base)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",current_ply);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  {
    square const s = find_interception_placement(si);
    if (s==nullsquare)
      unwrap_move_effects(current_ply,si,base);
    else if (base+nr_placed_interceptors>=total_invisible_number)
      solve_result = previous_move_is_illegal;
    else
    {
      piece_choice[base+nr_placed_interceptors].pos = s;
      ++nr_placed_interceptors;
      unwrap_move_effects(current_ply,si,base);
      --nr_placed_interceptors;
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
    deal_with_check_to_be_intercepted(nbply,si,nr_placed_victims);

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
  Side const side_moving = SLICE_STARTER(si);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_uninterceptably_observed_ortho(side_moving,
                                               being_solved.king_square[advers(side_moving)]))
    solve_result = previous_move_is_illegal;
  else
  {
    unsigned int count_white_checks = count_interceptable_orthodox_checks(White,being_solved.king_square[Black]);
    unsigned int count_black_checks = count_interceptable_orthodox_checks(Black,being_solved.king_square[White]);
    unsigned int const count_max = count_white_checks>count_black_checks ? count_white_checks : count_black_checks;
    if (count_max>total_invisible_number)
       solve_result = previous_move_is_illegal;
    else if (count_max>nr_bound_invisible)
    {
      unsigned int const save_bound = nr_bound_invisible;
      nr_bound_invisible = count_max;
      bound_around = being_solved.king_square[count_white_checks>count_black_checks ? Black : White];
      pipe_solve_delegate(si);
      nr_bound_invisible = save_bound;
    }
    else
      pipe_solve_delegate(si);
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

  if (is_in_check(advers(SLICE_STARTER(si))))
    solve_result = previous_move_is_illegal;
  else
  {
    /* make sure that we don't generate pawn captures total invisible */
    assert(play_phase==replaying_moves);
    pipe_solve_delegate(si);
  }

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

  if (play_phase==regular_play)
  {
    square const sq_departure = curr_generation->departure ;
    Side const side = trait[nbply];
    if (TSTFLAG(being_solved.spec[sq_departure],side) && being_solved.board[sq_departure]==Pawn)
    {
      int const dir_vertical = trait[nbply]==White ? dir_up : dir_down;

      curr_generation->arrival = curr_generation->departure+dir_vertical+dir_left;
      if (is_square_empty(curr_generation->arrival))
        push_move_capture_extra(curr_generation->arrival);

      curr_generation->arrival = curr_generation->departure+dir_vertical+dir_right;
      if (is_square_empty(curr_generation->arrival))
        push_move_capture_extra(curr_generation->arrival);
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
    square const sq_capture = move_gen_top->capture;
    Side const side_victim = advers(SLICE_STARTER(si));

    if (!is_no_capture(sq_capture) && is_square_empty(sq_capture))
    {
      /* sneak the creation of a dummy piece into the previous ply - very dirty...,
       * but this prevents the dummy from being un-created when we undo this move's
       * effects
       */
      move_effect_journal_do_piece_creation(move_effect_reason_removal_of_invisible,
                                            sq_capture,
                                            Dummy,
                                            BIT(side_victim),
                                            side_victim);
      ++move_effect_journal_base[nbply];

      piece_choice[nr_placed_victims].pos = sq_capture;

      ++nr_placed_victims;
      pipe_solve_delegate(si);
      --nr_placed_victims;

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

  if (st->context==stip_traversal_context_intro)
    pipe_remove(si);
  else
    SLICE_TYPE(si) = STTotalInvisibleUninterceptableSelfCheckGuard;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static int square_compare_around_both_kings(void const *v1, void const *v2)
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

  memmove(square_order_unbound, boardnum, sizeof boardnum);
  qsort(square_order_unbound, 64, sizeof square_order_unbound[0], &square_compare_around_both_kings);

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
      slice_index const prototypes[] = {
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer)
      };
      enum { nr_protypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert(si,prototypes,nr_protypes);
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
  //   - insert revelation logic
  // - in copy
  //   - logic for iteration over all possibilities of invisibles
  //     - special case of invisible king
  //     - special case: position has no legal placement of all invisibles may have to be dealt with:
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
