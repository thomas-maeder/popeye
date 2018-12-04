#include "conditions/imitator.h"
#include "conditions/conditions.h"
#include "conditions/castling_chess.h"
#include "output/plaintext/message.h"
#include "pieces/pieces.h"
#include "pieces/walks/pawns/promotion.h"
#include "pieces/walks/pawns/promotee_sequence.h"
#include "pieces/walks/hoppers.h"
#include "pieces/walks/angle/angles.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/post_move_iteration.h"
#include "position/effects/piece_removal.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

#include <stdlib.h>

square im0;                    /* position of the 1st imitator */

enum
{
  stack_size = max_nr_promotions_per_ply*maxply+1
};

static unsigned int stack_pointer;

static boolean promotion_into_imitator_happening[stack_size];

static numecoup skip_over_remainder_of_line(numecoup i,
                                            square sq_departure,
                                            square sq_next_arrival,
                                            numvec diff)
{
  do
  {
    ++i;
    sq_next_arrival += diff;
  }
  while (i<=CURRMOVE_OF_PLY(nbply)
         && move_generation_stack[i].departure==sq_departure
         && move_generation_stack[i].arrival==sq_next_arrival);

  return i;
}

static boolean are_all_imitator_arrivals_empty(square sq_departure, square sq_arrival)
{
  unsigned int imi_idx;
  int const diff = sq_arrival-sq_departure;
  boolean result = true;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  for (imi_idx = being_solved.number_of_imitators; imi_idx>0; imi_idx--)
  {
    square const j2 = being_solved.isquare[imi_idx-1]+diff;
    if (j2!=sq_departure && !is_square_empty(j2))
    {
      result = false;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static numecoup remove_illegal_moves_by_same_rider_on_line(numecoup i,
                                                           numvec diff,
                                                           numecoup *new_top)
{
  square const sq_departure = move_generation_stack[i].departure;
  square sq_next_arrival = sq_departure+diff;

  while (i<=CURRMOVE_OF_PLY(nbply)
         && move_generation_stack[i].departure==sq_departure
         && move_generation_stack[i].arrival==sq_next_arrival)
    if (are_all_imitator_arrivals_empty(sq_departure,sq_next_arrival))
    {
      ++*new_top;
      move_generation_stack[*new_top] = move_generation_stack[i];
      ++i;
      sq_next_arrival += diff;
    }
    else
    {
      i = skip_over_remainder_of_line(i,sq_departure,sq_next_arrival,diff);
      break;
    }

  return i;
}

static numecoup remove_illegal_moves_by_same_rider(numecoup curr, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[curr].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",curr);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (curr<=CURRMOVE_OF_PLY(nbply)
         && move_generation_stack[curr].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[curr].arrival;
    numvec const diff = sq_arrival-sq_departure;
    curr = remove_illegal_moves_by_same_rider_on_line(curr,diff,new_top);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",curr);
  TraceFunctionResultEnd();
  return curr;
}

static boolean is_imitator_line_clear(unsigned int i,
                                      numvec diff_first_necessarily_empty,
                                      numvec step,
                                      numvec diff_first_not_necessarily_empty)
{
  boolean result = true;
  square const sq_first_not_necessarily_empty = being_solved.isquare[i]+diff_first_not_necessarily_empty;
  square sq_curr;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceValue("%d",diff_first_necessarily_empty);
  TraceValue("%d",step);
  TraceValue("%d",diff_first_not_necessarily_empty);
  TraceFunctionParamListEnd();

  for (sq_curr = being_solved.isquare[i]+diff_first_necessarily_empty; sq_curr!=sq_first_not_necessarily_empty; sq_curr += step)
  {
    TraceSquare(sq_curr);
    TraceEOL();
    if (!is_square_empty(sq_curr))
    {
      result = false;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean are_all_imitator_lines_clear(numvec diff_first_necessarily_empty,
                                            numvec step,
                                            numvec diff_first_not_necessarily_empty)
{
  boolean result = true;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceValue("%d",diff_first_necessarily_empty);
  TraceValue("%d",step);
  TraceValue("%d",diff_first_not_necessarily_empty);
  TraceFunctionParamListEnd();

  TraceValue("%u",being_solved.number_of_imitators);
  TraceEOL();

  for (i = 0; i!=being_solved.number_of_imitators; ++i)
    if (!is_imitator_line_clear(i,diff_first_necessarily_empty,step,diff_first_not_necessarily_empty))
    {
      result = false;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean have_all_imitators_hurdle(numvec diff_hurdle)
{
  unsigned int i;

  for (i = 0; i!=being_solved.number_of_imitators; ++i)
  {
    square const sq_hurdle = being_solved.isquare[i]+diff_hurdle;
    if (is_square_empty(sq_hurdle) || is_square_blocked(sq_hurdle))
      return false;
  }

  return true;
}

static numecoup remove_illegal_moves_by_same_equihopper(numecoup curr, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[curr].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",curr);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (curr<=CURRMOVE_OF_PLY(nbply)
         && move_generation_stack[curr].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[curr].arrival;
    square const sq_hurdle = hoppper_moves_auxiliary[move_generation_stack[curr].id].sq_hurdle;
    numvec const diff = vec[hoppper_moves_auxiliary[move_generation_stack[curr].id].idx_dir];
    numvec const diff_hurdle = sq_hurdle-sq_departure;
    numvec const diff_arrival = sq_arrival-sq_departure;

    if (have_all_imitators_hurdle(diff_hurdle)
        && are_all_imitator_lines_clear(diff,diff,diff_hurdle)
        && are_all_imitator_lines_clear(diff_hurdle+diff,diff,diff_arrival+diff)
        && are_all_imitator_arrivals_empty(sq_departure,sq_arrival))
    {
      ++*new_top;
      move_generation_stack[*new_top] = move_generation_stack[curr];
    }

    ++curr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",curr);
  TraceFunctionResultEnd();
  return curr;
}

static numecoup remove_illegal_moves_by_same_nonstop_equihopper(numecoup curr, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[curr].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",curr);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (curr<=CURRMOVE_OF_PLY(nbply)
         && move_generation_stack[curr].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[curr].arrival;
    square const sq_hurdle = hoppper_moves_auxiliary[move_generation_stack[curr].id].sq_hurdle;
    numvec const diff_hurdle = sq_hurdle-sq_departure;

    if (have_all_imitators_hurdle(diff_hurdle)
        && are_all_imitator_arrivals_empty(sq_departure,sq_arrival))
    {
      ++*new_top;
      move_generation_stack[*new_top] = move_generation_stack[curr];
    }

    ++curr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",curr);
  TraceFunctionResultEnd();
  return curr;
}

static numecoup remove_illegal_moves_by_same_hopper(numecoup curr, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[curr].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",curr);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (curr<=CURRMOVE_OF_PLY(nbply)
         && move_generation_stack[curr].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[curr].arrival;
    square const sq_hurdle = hoppper_moves_auxiliary[move_generation_stack[curr].id].sq_hurdle;
    numvec const diff = vec[hoppper_moves_auxiliary[move_generation_stack[curr].id].idx_dir];
    numvec const diff_hurdle = sq_hurdle-sq_departure;

    if (have_all_imitators_hurdle(diff_hurdle)
        && are_all_imitator_lines_clear(diff,diff,diff_hurdle)
        && are_all_imitator_arrivals_empty(sq_departure,sq_arrival))
    {
      ++*new_top;
      move_generation_stack[*new_top] = move_generation_stack[curr];
    }

    ++curr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",curr);
  TraceFunctionResultEnd();
  return curr;
}

static numecoup remove_illegal_moves_by_same_mao(numecoup curr, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[curr].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",curr);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (curr<=CURRMOVE_OF_PLY(nbply)
         && move_generation_stack[curr].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[curr].arrival;
    square const sq_hurdle = hoppper_moves_auxiliary[move_generation_stack[curr].id].sq_hurdle;

    if (are_all_imitator_arrivals_empty(sq_departure,sq_hurdle)
        && are_all_imitator_arrivals_empty(sq_departure,sq_arrival))
    {
      ++*new_top;
      move_generation_stack[*new_top] = move_generation_stack[curr];
    }

    ++curr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",curr);
  TraceFunctionResultEnd();
  return curr;
}

static boolean castlingimok(square sq_departure, square sq_arrival)
{
  boolean ret= false;
  piece_walk_type const p = get_walk_of_piece_on_square(sq_departure);
  Flags const flags = being_solved.spec[sq_departure];

  /* I think this should work - clear the K, and move the Is, but don't clear the rook. */
  /* If the Is crash into the R, the move would be illegal as the K moves first.        */
  /* The only other test here is for long castling when the Is have to be clear to move */
  /* one step right (put K back first)as well as two steps left.                        */
  /* But there won't be an I one sq to the left of a1 (a8) so no need to clear the R    */

  switch (sq_arrival-sq_departure)
  {
    case 2*dir_right:  /* 00 - can short-circuit here (only follow K, if ok rest will be ok) */
      empty_square(sq_departure);
      ret = (are_all_imitator_arrivals_empty(sq_departure, sq_departure+dir_right)
             && are_all_imitator_arrivals_empty(sq_departure, sq_departure+2*dir_right));
      occupy_square(sq_departure,p,flags);
      break;

    case 2*dir_left:  /* 000 - follow K, (and move K as well), then follow R */
      empty_square(sq_departure);
      ret = (are_all_imitator_arrivals_empty(sq_departure, sq_departure+dir_left)
             && are_all_imitator_arrivals_empty(sq_departure, sq_departure+2*dir_left));
      occupy_square(sq_departure+2*dir_left,p,flags);
      ret = (ret
             && are_all_imitator_arrivals_empty(sq_departure, sq_departure+dir_left)
             && are_all_imitator_arrivals_empty (sq_departure, sq_departure)
             && are_all_imitator_arrivals_empty(sq_departure, sq_departure+dir_right));
      empty_square(sq_departure+2*dir_left);
      occupy_square(sq_departure,p,flags);
      break;
  }
  return ret;
}

static numecoup remove_illegal_moves_by_same_king(numecoup curr, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[curr].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",curr);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (curr<=CURRMOVE_OF_PLY(nbply)
         && move_generation_stack[curr].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[curr].arrival;
    square const sq_capture = move_generation_stack[curr].capture;

    TraceSquare(sq_arrival);
    TraceEOL();

    if (sq_capture==kingside_castling || sq_capture==queenside_castling)
    {
      if (castlingimok(sq_departure,sq_arrival))
      {
        TraceText("accepting castling\n");
        ++*new_top;
        move_generation_stack[*new_top] = move_generation_stack[curr];
      }
    }
    else
    {
      if (are_all_imitator_arrivals_empty(sq_departure,sq_arrival))
      {
        TraceText("accepting regular move\n");
        ++*new_top;
        move_generation_stack[*new_top] = move_generation_stack[curr];
      }
    }

    ++curr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",curr);
  TraceFunctionResultEnd();
  return curr;
}

static numecoup accept_all_moves_by_same_piece(numecoup curr, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[curr].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",curr);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (curr<=CURRMOVE_OF_PLY(nbply)
         && move_generation_stack[curr].departure==sq_departure)
  {
    ++*new_top;
    move_generation_stack[*new_top] = move_generation_stack[curr];
    ++curr;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",curr);
  TraceFunctionResultEnd();
  return curr;
}

static numecoup remove_illegal_moves_by_same_piece(numecoup curr, numecoup *new_top)
{
  numecoup result = curr;
  square sq_departure = move_generation_stack[curr].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",curr);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);
  TraceEOL();

  switch (get_walk_of_piece_on_square(sq_departure))
  {
    case King:
      result = remove_illegal_moves_by_same_king(curr,new_top);
      break;

    case ErlKing:
    case Knight:
    case Wesir:
    case Dabbaba:
    case Fers:
    case Alfil:
    case Bucephale:
    case Giraffe:
    case Camel:
    case Zebra:
    case Okapi:
    case Bison:
    case Gnu:
    case Antilope:
    case Squirrel:
    case RootFiftyLeaper:
    case Leap15:
    case Leap16:
    case Leap24:
    case Leap25:
    case Leap35:
    case Leap36:
    case Leap37:

    case Queen:
    case Rook:
    case Bishop:
    case NightRider:
    case Elephant:
    case Waran:
    case Camelrider:
    case Zebrarider:
    case Gnurider:
    case Amazone:
    case Empress:
    case Princess:
    case RookHunter:
    case BishopHunter:
    case WesirRider:
    case FersRider:

    case Pawn:
    case BerolinaPawn:
    case ReversePawn:
      result = remove_illegal_moves_by_same_rider(curr,new_top);
      break;

    case Grasshopper:
    case NightriderHopper:
    case CamelRiderHopper:
    case ZebraRiderHopper:
    case GnuRiderHopper:
    case RookHopper:
    case BishopHopper:
    case KingHopper:
    case KnightHopper:
    case Elk:
    case RookMoose:
    case BishopMoose:
    case Eagle:
    case RookEagle:
    case BishopEagle:
    case Sparrow:
    case RookSparrow:
    case BishopSparrow:
      result = remove_illegal_moves_by_same_hopper(curr,new_top);
      break;

    case Mao:
    case Moa:
      result = remove_illegal_moves_by_same_mao(curr,new_top);
      break;

    case EquiHopper:
    case GrassHopper2:
    case GrassHopper3:
    case Orix:
      result = remove_illegal_moves_by_same_equihopper(curr,new_top);
      break;

    case NonStopEquihopper:
    case NonStopOrix:
      result = remove_illegal_moves_by_same_nonstop_equihopper(curr,new_top);
      break;

    default:
      result = accept_all_moves_by_same_piece(curr,new_top);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void remove_illegal_moves(void)
{
  numecoup curr = MOVEBASE_OF_PLY(nbply)+1;
  numecoup new_top = MOVEBASE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (curr<=CURRMOVE_OF_PLY(nbply))
    curr = remove_illegal_moves_by_same_piece(curr,&new_top);

  SET_CURRMOVE(nbply,new_top);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean avoid_observing_if_imitator_blocked_rider(void)
{
  boolean result;
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_landing = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  numvec const step = -vec[interceptable_observation[observation_context].vector_index1];
  piece_walk_type const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = being_solved.spec[sq_observer];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceFunctionParamListEnd();

  empty_square(sq_observer);/* an imitator might be disturbed by the moving rider! */
  result = are_all_imitator_lines_clear(step,step,sq_landing-sq_observer+step);
  occupy_square(sq_observer,p,flags);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean avoid_observing_if_imitator_blocked_chinese_leaper(void)
{
  boolean result;
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_landing = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  piece_walk_type const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = being_solved.spec[sq_observer];
  numvec const vec_pass_target = vec[interceptable_observation[observation_context].vector_index1];
  square const sq_pass = sq_landing+vec_pass_target;

  empty_square(sq_observer);
  result = (are_all_imitator_arrivals_empty(sq_observer,sq_pass)
            && are_all_imitator_arrivals_empty(sq_observer,sq_landing));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_rider_hopper(void)
{
  boolean result;
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_landing = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  piece_walk_type const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = being_solved.spec[sq_observer];
  numvec const step = -vec[interceptable_observation[observation_context].vector_index1];
  square const sq_hurdle = sq_landing-step;
  numvec const diff_hurdle = sq_hurdle-sq_observer;

  empty_square(sq_observer);
  result = (have_all_imitators_hurdle(diff_hurdle)
            && are_all_imitator_arrivals_empty(sq_observer,sq_landing)
            && are_all_imitator_lines_clear(step,step,diff_hurdle));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_contragrasshopper(void)
{
  boolean result;
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_landing = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  piece_walk_type const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = being_solved.spec[sq_observer];
  numvec const step = -vec[interceptable_observation[observation_context].vector_index1];

  empty_square(sq_observer);
  result = (have_all_imitators_hurdle(step)
            && are_all_imitator_lines_clear(step+step,step,sq_landing-sq_observer+step));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_grasshopper_n(unsigned int dist_hurdle_target)
{
  boolean result;
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_landing = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  numvec const step_hurdle_target = -vec[interceptable_observation[observation_context].vector_index1];
  square const sq_hurdle = sq_landing - dist_hurdle_target*step_hurdle_target;

  piece_walk_type const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = being_solved.spec[sq_observer];

  empty_square(sq_observer);/* an imitator might be disturbed by the moving rider! */
  result = (have_all_imitators_hurdle(sq_hurdle-sq_observer)
            && are_all_imitator_lines_clear(step_hurdle_target,
                                            step_hurdle_target,
                                            sq_hurdle-sq_observer)
            && are_all_imitator_lines_clear(sq_hurdle-sq_observer+step_hurdle_target,
                                            step_hurdle_target,
                                            sq_landing-sq_observer+step_hurdle_target));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_nonstop_equihopper(void)
{
  boolean result;
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_landing = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  piece_walk_type const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = being_solved.spec[sq_observer];
  numvec const diff_hurdle = (sq_landing-sq_observer)/2;

  empty_square(sq_observer);
  result = (have_all_imitators_hurdle(diff_hurdle)
            && are_all_imitator_arrivals_empty(sq_observer,sq_landing));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_orix(void)
{
  boolean result;
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_landing = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  piece_walk_type const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = being_solved.spec[sq_observer];
  numvec const step = -vec[interceptable_observation[observation_context].vector_index1];
  numvec const diff_observer_landing = sq_landing-sq_observer;
  numvec const diff_observer_hurdle = diff_observer_landing/2;

  empty_square(sq_observer);
  result = (have_all_imitators_hurdle(diff_observer_hurdle)
            && are_all_imitator_lines_clear(step,step,diff_observer_hurdle)
            && are_all_imitator_lines_clear(diff_observer_hurdle+step,step,diff_observer_landing+step));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_angle_hopper(angle_t angle)
{
  boolean result;
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_landing = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  piece_walk_type const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = being_solved.spec[sq_observer];
  vec_index_type const vec_index_departure_hurdle = 2*interceptable_observation[observation_context].vector_index1;
  numvec const vec_departure_hurdle1 = -angle_vectors[angle][vec_index_departure_hurdle];
  numvec const vec_departure_hurdle2 = -angle_vectors[angle][vec_index_departure_hurdle-1];
  square const sq_hurdle = sq_landing+vec[interceptable_observation[observation_context].vector_index1];
  numvec const diff_observer_hurdle = sq_hurdle-sq_observer;
  int const nr_steps1 = abs(diff_observer_hurdle/vec_departure_hurdle1);
  int const nr_steps2 = abs(diff_observer_hurdle/vec_departure_hurdle2);
  numvec step;

  if (nr_steps1==0)
    step = vec_departure_hurdle2;
  else if (nr_steps2==0)
    step = vec_departure_hurdle1;
  else
    step = nr_steps1<nr_steps2 ? vec_departure_hurdle1 : vec_departure_hurdle2;

  empty_square(sq_observer);
  result = (have_all_imitators_hurdle(diff_observer_hurdle)
            && are_all_imitator_lines_clear(step,step,diff_observer_hurdle)
            && are_all_imitator_arrivals_empty(sq_observer,sq_landing));
  occupy_square(sq_observer,p,flags);

  return result;
}

/* Validate an observation according to Imitators
 * @return true iff the observation is valid
 */
boolean imitator_validate_observation(slice_index si)
{
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceWalk(being_solved.board[sq_observer]);
  TraceEOL();
  switch (being_solved.board[sq_observer])
  {
    case King:
    case ErlKing:
    case Knight:
    case Wesir:
    case Dabbaba:
    case Fers:
    case Alfil:
    case Bucephale:
    case Giraffe:
    case Camel:
    case Zebra:
    case Okapi:
    case Bison:
    case Gnu:
    case Antilope:
    case Squirrel:
    case RootFiftyLeaper:
    case Leap15:
    case Leap16:
    case Leap24:
    case Leap25:
    case Leap35:
    case Leap36:
    case Leap37:

    case Pawn:
    case BerolinaPawn:
    case ReversePawn:
    {
      square const sq_landing = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
      result = are_all_imitator_arrivals_empty(sq_observer,sq_landing);
      break;
    }

    case Queen:
    case Rook:
    case Bishop:
    case NightRider:
    case Elephant:
    case Waran:
    case Camelrider:
    case Zebrarider:
    case Gnurider:
    case Amazone:
    case Empress:
    case Princess:
    case RookHunter:
    case BishopHunter:
    case WesirRider:
    case FersRider:
      result = avoid_observing_if_imitator_blocked_rider();
      break;

    case Mao:
    case Moa:
      result = avoid_observing_if_imitator_blocked_chinese_leaper();
      break;

    case NonStopEquihopper:
    case NonStopOrix:
      result = avoid_observing_if_imitator_blocked_nonstop_equihopper();
      break;

    case Orix:
      result = avoid_observing_if_imitator_blocked_orix();
      break;

    case EquiHopper:
      if (interceptable_observation[observation_context].vector_index1==0)
        result = avoid_observing_if_imitator_blocked_nonstop_equihopper();
      else
        result = avoid_observing_if_imitator_blocked_orix();
      break;

    case Grasshopper:
    case NightriderHopper:
    case CamelRiderHopper:
    case ZebraRiderHopper:
    case GnuRiderHopper:
    case RookHopper:
    case BishopHopper:
    case KingHopper:
    case KnightHopper:
      result = avoid_observing_if_imitator_blocked_rider_hopper();
      break;

    case ContraGras:
      result = avoid_observing_if_imitator_blocked_contragrasshopper();
      break;

    case GrassHopper2:
      result = avoid_observing_if_imitator_blocked_grasshopper_n(2);
      break;

    case GrassHopper3:
      result = avoid_observing_if_imitator_blocked_grasshopper_n(3);
      break;

    case Elk:
    case RookMoose:
    case BishopMoose:
      result = avoid_observing_if_imitator_blocked_angle_hopper(angle_45);
      break;

    case Eagle:
    case RookEagle:
    case BishopEagle:
      result = avoid_observing_if_imitator_blocked_angle_hopper(angle_90);
      break;

    case Sparrow:
    case RookSparrow:
    case BishopSparrow:
      result = avoid_observing_if_imitator_blocked_angle_hopper(angle_135);
      break;

    default:
      result = true;
      break;
  }

  if (result)
    result = pipe_validate_observation_recursive_delegate(si);

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
void imitator_remove_illegal_moves_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  remove_illegal_moves();

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_imitator_pos_occupied(void)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i=0; i!=being_solved.number_of_imitators; ++i)
    if (!is_square_empty(being_solved.isquare[i]))
    {
      TraceSquare(being_solved.isquare[i]);
      TraceEOL();
      result = true;
      break;
    }

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
void imitator_detect_illegal_moves_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  pipe_this_move_illegal_if(si,is_imitator_pos_occupied());

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_imitators(int delta)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  TraceValue("%u",being_solved.number_of_imitators);
  for (i=0; i!=being_solved.number_of_imitators; ++i)
  {
    being_solved.isquare[i] += delta;
    TraceSquare(being_solved.isquare[i]);
  }
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add the movement of all imitators
 * @param reason reason for moving the imitators
 * @param delta how to move the imitators (to-from)
 */
static void move_effect_journal_do_imitator_movement(move_effect_reason_type reason,
                                                     int delta)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_imitator_movement,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  entry->u.imitator_movement.delta = delta;
  entry->u.imitator_movement.nr_moved = being_solved.number_of_imitators;

  move_imitators(delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_imitator_movement(move_effect_journal_entry_type const *entry)
{
  int const delta = entry->u.imitator_movement.delta;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_imitators(-delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_imitator_movement(move_effect_journal_entry_type const *entry)
{
  int const delta = entry->u.imitator_movement.delta;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_imitators(delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add the addition of an imitator to the current move of the current ply
 * @param reason reason for adding the imitator
 * @param to where to add the imitator
 */
static void move_effect_journal_do_imitator_addition(move_effect_reason_type reason,
                                                     square to)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_imitator_addition,reason);

  TraceFunctionEntry(__func__);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  entry->u.imitator_addition.to = to;

  if (being_solved.number_of_imitators==maxinum)
    output_plaintext_fatal_message(ManyImitators);

  being_solved.isquare[being_solved.number_of_imitators] = to;
  ++being_solved.number_of_imitators;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_imitator_addition(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(being_solved.number_of_imitators>0);
  --being_solved.number_of_imitators;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_imitator_addition(move_effect_journal_entry_type const *entry)
{
  square const to = entry->u.imitator_addition.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (being_solved.number_of_imitators==maxinum)
    output_plaintext_fatal_message(ManyImitators);

  being_solved.isquare[being_solved.number_of_imitators] = to;
  ++being_solved.number_of_imitators;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static int imitator_diff(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;
  int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (curr = move_effect_journal_base[nbply]; curr!=top; ++curr)
    if (move_effect_journal[curr].type==move_effect_piece_movement)
      switch (move_effect_journal[curr].reason)
      {
        case move_effect_reason_moving_piece_movement:
        case move_effect_reason_castling_king_movement:
        case move_effect_reason_castling_partner_movement:
          result += move_effect_journal[curr].u.piece_movement.to-move_effect_journal[curr].u.piece_movement.from;
          break;

        default:
          break;
      }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
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
void imitator_mover_solve(slice_index si)
{
  int const diff = imitator_diff();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_effect_journal_do_imitator_movement(move_effect_reason_movement_imitation,
                                           diff);
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
void imitator_pawn_promoter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    square sq_arrival;
    Side as_side;

    find_potential_promotion_square(promotion_horizon[nbply],&sq_arrival,&as_side);

    assert(stack_pointer<stack_size);

    if (!post_move_am_i_iterating())
      promotion_into_imitator_happening[stack_pointer] = is_square_occupied_by_promotable_pawn(sq_arrival,as_side);

    if (promotion_into_imitator_happening[stack_pointer])
    {
      move_effect_journal_index_type const save_horizon = promotion_horizon[nbply];

      promotion_horizon[nbply] = move_effect_journal_base[nbply+1];

      move_effect_journal_do_piece_removal(move_effect_reason_pawn_promotion,
                                           sq_arrival);
      move_effect_journal_do_imitator_addition(move_effect_reason_pawn_promotion,
                                               sq_arrival);

      ++stack_pointer;
      post_move_iteration_solve_fork(si);
      --stack_pointer;

      promotion_horizon[nbply] = save_horizon;

      if (!post_move_iteration_is_locked())
        promotion_into_imitator_happening[stack_pointer] = false;
    }
    else
    {
      ++stack_pointer;
      post_move_iteration_solve_delegate(si);
      --stack_pointer;

      if (!post_move_iteration_is_locked())
        post_move_iteration_end();
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_promoter(slice_index si, stip_structure_traversal *st)
{
  slice_index const * const landing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const prototype = alloc_fork_slice(STPawnToImitatorPromoter,proxy);
    assert(*landing!=no_slice);
    link_to_branch(proxy,*landing);
    promotion_insert_slices(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_promotion(slice_index si, stip_structure_traversal *st)
{
  slice_index * const landing = st->param;
  slice_index const save_landing = *landing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *landing = no_slice;
  stip_traverse_structure_children_pipe(si,st);
  insert_promoter(si,st);
  *landing = save_landing;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_landing(slice_index si, stip_structure_traversal *st)
{
  slice_index * const landing = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  *landing = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_promoters(slice_index si)
{
  stip_structure_traversal st;
  slice_index landing = no_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&landing);
  stip_structure_traversal_override_single(&st,STBeforePawnPromotion,&instrument_promotion);
  stip_structure_traversal_override_single(&st,STLandingAfterPawnPromotion,&remember_landing);
  stip_traverse_structure(si,&st);

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
    slice_index const prototype = alloc_pipe(STImitatorRemoveIllegalMoves);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_imitator(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_set_effect_doers(move_effect_imitator_addition,
                                       &undo_imitator_addition,
                                       &redo_imitator_addition);
  move_effect_journal_set_effect_doers(move_effect_imitator_movement,
                                       &undo_imitator_movement,
                                       &redo_imitator_movement);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STDoneGeneratingMoves,
                                             &insert_remover);
    stip_traverse_structure(si,&st);
  }

  if (!CondFlag[noiprom])
    insert_promoters(si);

  stip_instrument_moves(si,STImitatorMover);
  stip_instrument_moves(si,STImitatorDetectIllegalMoves);

  stip_instrument_observation_geometry_validation(si,
                                                  nr_sides,
                                                  STImitatorRemoveIllegalMoves);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
