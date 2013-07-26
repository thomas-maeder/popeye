#include "conditions/imitator.h"
#include "pydata.h"
#include "pymsg.h"
#include "pieces/walks/pawns/promotion.h"
#include "pieces/walks/angle/angles.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/fork.h"
#include "stipulation/branch.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/temporary_hacks.h"
#include "solving/post_move_iteration.h"
#include "solving/move_effect_journal.h"
#include "solving/single_move_generator.h"
#include "solving/observation.h"
#include "conditions/castling_chess.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

boolean promotion_of_moving_into_imitator[maxply+1];
square im0;                    /* position of the 1st imitator */
imarr isquare;                 /* Imitatorstandfelder */
unsigned int number_of_imitators;       /* aktuelle Anzahl Imitatoren */

static post_move_iteration_id_type prev_post_move_iteration_id[maxply+1];

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
  while (i<=current_move[nbply]
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

  for (imi_idx = number_of_imitators; imi_idx>0; imi_idx--)
  {
    square const j2 = isquare[imi_idx-1]+diff;
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

  while (i<=current_move[nbply]
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

static numecoup remove_illegal_moves_by_same_rider(numecoup i, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[i].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (i<=current_move[nbply]
         && move_generation_stack[i].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[i].arrival;
    numvec const diff = sq_arrival-sq_departure;
    i = remove_illegal_moves_by_same_rider_on_line(i,diff,new_top);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",i);
  TraceFunctionResultEnd();
  return i;
}

static boolean is_imitator_line_clear(unsigned int i,
                                      numvec diff_first_necessarily_empty,
                                      numvec step,
                                      numvec diff_first_not_necessarily_empty)
{
  boolean result = true;
  square const sq_first_not_necessarily_empty = isquare[i]+diff_first_not_necessarily_empty;
  square sq_curr;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceValue("%d",diff_first_necessarily_empty);
  TraceValue("%d",step);
  TraceValue("%d",diff_first_not_necessarily_empty);
  TraceFunctionParamListEnd();

  for (sq_curr = isquare[i]+diff_first_necessarily_empty; sq_curr!=sq_first_not_necessarily_empty; sq_curr += step)
  {
    TraceSquare(sq_curr);TraceText("\n");
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

  TraceValue("%u\n",number_of_imitators);

  for (i = 0; i!=number_of_imitators; ++i)
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

  for (i = 0; i!=number_of_imitators; ++i)
  {
    square const sq_hurdle = isquare[i]+diff_hurdle;
    if (is_square_empty(sq_hurdle) || is_square_blocked(sq_hurdle))
      return false;
  }

  return true;
}

static numecoup remove_illegal_moves_by_same_equihopper(numecoup i, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[i].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (i<=current_move[nbply]
         && move_generation_stack[i].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[i].arrival;
    square const sq_hurdle = move_generation_stack[i].auxiliary.hopper.sq_hurdle;
    numvec const diff = vec[move_generation_stack[i].auxiliary.hopper.vec_index];
    numvec const diff_hurdle = sq_hurdle-sq_departure;
    numvec const diff_arrival = sq_arrival-sq_departure;

    if (have_all_imitators_hurdle(diff_hurdle)
        && are_all_imitator_lines_clear(diff,diff,diff_hurdle)
        && are_all_imitator_lines_clear(diff_hurdle+diff,diff,diff_arrival+diff)
        && are_all_imitator_arrivals_empty(sq_departure,sq_arrival))
    {
      ++*new_top;
      move_generation_stack[*new_top] = move_generation_stack[i];
    }

    ++i;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",i);
  TraceFunctionResultEnd();
  return i;
}

static numecoup remove_illegal_moves_by_same_nonstop_equihopper(numecoup i, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[i].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (i<=current_move[nbply]
         && move_generation_stack[i].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[i].arrival;
    square const sq_hurdle = move_generation_stack[i].auxiliary.hopper.sq_hurdle;
    numvec const diff_hurdle = sq_hurdle-sq_departure;

    if (have_all_imitators_hurdle(diff_hurdle)
        && are_all_imitator_arrivals_empty(sq_departure,sq_arrival))
    {
      ++*new_top;
      move_generation_stack[*new_top] = move_generation_stack[i];
    }

    ++i;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",i);
  TraceFunctionResultEnd();
  return i;
}

static numecoup remove_illegal_moves_by_same_hopper(numecoup i, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[i].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (i<=current_move[nbply]
         && move_generation_stack[i].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[i].arrival;
    square const sq_hurdle = move_generation_stack[i].auxiliary.hopper.sq_hurdle;
    numvec const diff = vec[move_generation_stack[i].auxiliary.hopper.vec_index];
    numvec const diff_hurdle = sq_hurdle-sq_departure;

    if (have_all_imitators_hurdle(diff_hurdle)
        && are_all_imitator_lines_clear(diff,diff,diff_hurdle)
        && are_all_imitator_arrivals_empty(sq_departure,sq_arrival))
    {
      ++*new_top;
      move_generation_stack[*new_top] = move_generation_stack[i];
    }

    ++i;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",i);
  TraceFunctionResultEnd();
  return i;
}

static numecoup remove_illegal_moves_by_same_mao(numecoup i, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[i].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (i<=current_move[nbply]
         && move_generation_stack[i].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[i].arrival;
    square const sq_hurdle = move_generation_stack[i].auxiliary.hopper.sq_hurdle;

    if (are_all_imitator_arrivals_empty(sq_departure,sq_hurdle) && are_all_imitator_arrivals_empty(sq_departure,sq_arrival))
    {
      ++*new_top;
      move_generation_stack[*new_top] = move_generation_stack[i];
    }

    ++i;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",i);
  TraceFunctionResultEnd();
  return i;
}

static boolean castlingimok(square sq_departure, square sq_arrival)
{
  boolean ret= false;
  PieNam const p = get_walk_of_piece_on_square(sq_departure);
  Flags const flags = spec[sq_departure];

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

static numecoup remove_illegal_moves_by_same_king(numecoup i, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[i].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (i<=current_move[nbply]
         && move_generation_stack[i].departure==sq_departure)
  {
    square const sq_arrival = move_generation_stack[i].arrival;
    square const sq_capture = move_generation_stack[i].capture;

    TraceSquare(sq_arrival);TraceText("\n");

    if (sq_capture==kingside_castling || sq_capture==queenside_castling)
    {
      if (castlingimok(sq_departure,sq_arrival))
      {
        TraceText("accepting castling\n");
        ++*new_top;
        move_generation_stack[*new_top] = move_generation_stack[i];
      }
    }
    else
    {
      if (are_all_imitator_arrivals_empty(sq_departure,sq_arrival))
      {
        TraceText("accepting regular move\n");
        ++*new_top;
        move_generation_stack[*new_top] = move_generation_stack[i];
      }
    }

    ++i;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",i);
  TraceFunctionResultEnd();
  return i;
}

static numecoup accept_all_moves_by_same_piece(numecoup i, numecoup *new_top)
{
  square const sq_departure = move_generation_stack[i].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceSquare(sq_departure);
  TraceFunctionParamListEnd();

  while (i<=current_move[nbply]
         && move_generation_stack[i].departure==sq_departure)
  {
    ++*new_top;
    move_generation_stack[*new_top] = move_generation_stack[i];
    ++i;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",i);
  TraceFunctionResultEnd();
  return i;
}

static numecoup remove_illegal_moves_by_same_piece(numecoup i, numecoup *new_top)
{
  numecoup result = i;
  square sq_departure = move_generation_stack[i].departure;

  TraceFunctionEntry(__func__);
  TraceValue("%u",i);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);TraceText("\n");

  switch (get_walk_of_piece_on_square(sq_departure))
  {
    case King:
      result = remove_illegal_moves_by_same_king(i,new_top);
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
      result = remove_illegal_moves_by_same_rider(i,new_top);
      break;

    case Grasshopper:
    case NightriderHopper:
    case CamelHopper:
    case ZebraHopper:
    case GnuHopper:
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
      result = remove_illegal_moves_by_same_hopper(i,new_top);
      break;

    case Mao:
    case Moa:
      result = remove_illegal_moves_by_same_mao(i,new_top);
      break;

    case EquiHopper:
    case GrassHopper2:
    case GrassHopper3:
    case Orix:
      result = remove_illegal_moves_by_same_equihopper(i,new_top);
      break;

    case NonStopEquihopper:
    case NonStopOrix:
      result = remove_illegal_moves_by_same_nonstop_equihopper(i,new_top);
      break;

    default:
      result = accept_all_moves_by_same_piece(i,new_top);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void remove_illegal_moves(void)
{
  numecoup i = current_move[nbply-1]+1;
  numecoup new_top = current_move[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (i<=current_move[nbply])
    i = remove_illegal_moves_by_same_piece(i,&new_top);

  current_move[nbply] = new_top;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean avoid_observing_if_imitator_blocked_rider(square sq_observer,
                                                         square sq_landing,
                                                         square sq_observee)
{
  boolean result;
  numvec const step = -vec[interceptable_observation_vector_index[observation_context]];
  PieNam const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = spec[sq_observer];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  empty_square(sq_observer);/* an imitator might be disturbed by the moving rider! */
  result = are_all_imitator_lines_clear(step,step,sq_landing-sq_observer+step);
  occupy_square(sq_observer,p,flags);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean avoid_observing_if_imitator_blocked_chinese_leaper(square sq_observer,
                                                                  square sq_landing,
                                                                  square sq_observee)
{
  boolean result;
  PieNam const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = spec[sq_observer];
  numvec const vec_pass_target = vec[interceptable_observation_vector_index[observation_context]];
  square const sq_pass = sq_observee+vec_pass_target;

  empty_square(sq_observer);
  result = (are_all_imitator_arrivals_empty(sq_observer,sq_pass)
            && are_all_imitator_arrivals_empty(sq_observer,sq_observee));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_rider_hopper(square sq_observer,
                                                                square sq_landing,
                                                                square sq_observee)
{
  boolean result;
  PieNam const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = spec[sq_observer];
  numvec const step = -vec[interceptable_observation_vector_index[observation_context]];
  square const sq_hurdle = sq_landing-step;
  numvec const diff_hurdle = sq_hurdle-sq_observer;

  empty_square(sq_observer);
  result = (have_all_imitators_hurdle(diff_hurdle)
            && are_all_imitator_arrivals_empty(sq_observer,sq_landing)
            && are_all_imitator_lines_clear(step,step,diff_hurdle));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_contragrasshopper(square sq_observer,
                                                                     square sq_landing,
                                                                     square sq_observee)
{
  boolean result;
  PieNam const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = spec[sq_observer];
  numvec const step = -vec[interceptable_observation_vector_index[observation_context]];

  empty_square(sq_observer);
  result = (have_all_imitators_hurdle(step)
            && are_all_imitator_lines_clear(step+step,step,sq_landing-sq_observer+step));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_grasshopper_n(square sq_observer,
                                                                 square sq_landing,
                                                                 square sq_observee,
                                                                 unsigned int dist_hurdle_target)
{
  boolean result;
  numvec const step_hurdle_target = -vec[interceptable_observation_vector_index[observation_context]];
  square const sq_hurdle = sq_landing - dist_hurdle_target*step_hurdle_target;

  PieNam const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = spec[sq_observer];

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

static boolean avoid_observing_if_imitator_blocked_nonstop_equihopper(square sq_observer,
                                                                      square sq_landing,
                                                                      square sq_observee)
{
  boolean result;
  PieNam const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = spec[sq_observer];
  numvec const diff_hurdle = (sq_landing-sq_observer)/2;

  empty_square(sq_observer);
  result = (have_all_imitators_hurdle(diff_hurdle)
            && are_all_imitator_arrivals_empty(sq_observer,sq_landing));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_orix(square sq_observer,
                                                        square sq_landing,
                                                        square sq_observee)
{
  boolean result;
  PieNam const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = spec[sq_observer];
  numvec const step = -vec[interceptable_observation_vector_index[observation_context]];
  numvec const diff_observer_landing = sq_landing-sq_observer;
  numvec const diff_observer_hurdle = diff_observer_landing/2;

  empty_square(sq_observer);
  result = (have_all_imitators_hurdle(diff_observer_hurdle)
            && are_all_imitator_lines_clear(step,step,diff_observer_hurdle)
            && are_all_imitator_lines_clear(diff_observer_hurdle+step,step,diff_observer_landing+step));
  occupy_square(sq_observer,p,flags);

  return result;
}

static boolean avoid_observing_if_imitator_blocked_angle_hopper(square sq_observer,
                                                                square sq_landing,
                                                                square sq_observee,
                                                                angle_t angle)
{
  boolean result;
  PieNam const p = get_walk_of_piece_on_square(sq_observer);
  Flags const flags = spec[sq_observer];
  vec_index_type const vec_index_departure_hurdle = 2*interceptable_observation_vector_index[observation_context];
  numvec const vec_departure_hurdle1 = -angle_vectors[angle][vec_index_departure_hurdle];
  numvec const vec_departure_hurdle2 = -angle_vectors[angle][vec_index_departure_hurdle-1];
  square const sq_hurdle = sq_landing+vec[interceptable_observation_vector_index[observation_context]];
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
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
boolean imitator_validate_observation(slice_index si,
                                      square sq_observer,
                                      square sq_landing,
                                      square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  TracePiece(e[sq_observer]);TraceText("\n");
  switch (e[sq_observer])
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
      result = are_all_imitator_arrivals_empty(sq_observer,sq_observee);
      break;

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
      result = avoid_observing_if_imitator_blocked_rider(sq_observer,
                                                         sq_landing,
                                                         sq_observee);
      break;

    case Mao:
    case Moa:
      result = avoid_observing_if_imitator_blocked_chinese_leaper(sq_observer,
                                                                  sq_landing,
                                                                  sq_observee);
      break;

    case NonStopEquihopper:
    case NonStopOrix:
      result = avoid_observing_if_imitator_blocked_nonstop_equihopper(sq_observer,
                                                                      sq_landing,
                                                                      sq_observee);
      break;

    case Orix:
      result = avoid_observing_if_imitator_blocked_orix(sq_observer,
                                                        sq_landing,
                                                        sq_observee);
      break;

    case EquiHopper:
      if (interceptable_observation_vector_index[observation_context]==0)
        result = avoid_observing_if_imitator_blocked_nonstop_equihopper(sq_observer,
                                                                        sq_landing,
                                                                        sq_observee);
      else
        result = avoid_observing_if_imitator_blocked_orix(sq_observer,
                                                          sq_landing,
                                                          sq_observee);
        break;

    case Grasshopper:
    case NightriderHopper:
    case CamelHopper:
    case ZebraHopper:
    case GnuHopper:
    case RookHopper:
    case BishopHopper:
    case KingHopper:
    case KnightHopper:
      result = avoid_observing_if_imitator_blocked_rider_hopper(sq_observer,
                                                                sq_landing,
                                                                sq_observee);
      break;

    case ContraGras:
      result = avoid_observing_if_imitator_blocked_contragrasshopper(sq_observer,
                                                                     sq_landing,
                                                                     sq_observee);
      break;

    case GrassHopper2:
      result = avoid_observing_if_imitator_blocked_grasshopper_n(sq_observer,
                                                                 sq_landing,
                                                                 sq_observee,
                                                                 2);
      break;

    case GrassHopper3:
      result = avoid_observing_if_imitator_blocked_grasshopper_n(sq_observer,
                                                                 sq_landing,
                                                                 sq_observee,
                                                                 3);
      break;

    case Elk:
    case RookMoose:
    case BishopMoose:
      result = avoid_observing_if_imitator_blocked_angle_hopper(sq_observer,
                                                                sq_landing,
                                                                sq_observee,
                                                                angle_45);
      break;

    case Eagle:
    case RookEagle:
    case BishopEagle:
      result = avoid_observing_if_imitator_blocked_angle_hopper(sq_observer,
                                                                sq_landing,
                                                                sq_observee,
                                                                angle_90);
      break;

    case Sparrow:
    case RookSparrow:
    case BishopSparrow:
      result = avoid_observing_if_imitator_blocked_angle_hopper(sq_observer,
                                                                sq_landing,
                                                                sq_observee,
                                                                angle_135);
      break;

    default:
      result = true;
      break;
  }

  if (result)
    result = validate_observation_recursive(slices[si].next1,
                                            sq_observer,
                                            sq_landing,
                                            sq_observee);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
stip_length_type imitator_remove_illegal_moves_solve(slice_index si,
                                                     stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  remove_illegal_moves();

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void move_imitators(int delta)
{
  unsigned int i;
  unsigned int number_of_imitators_to_be_moved = number_of_imitators;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  if (promotion_of_moving_into_imitator[nbply])
    --number_of_imitators_to_be_moved;

  TraceValue("%u",number_of_imitators_to_be_moved);
  for (i=0; i!=number_of_imitators_to_be_moved; ++i)
  {
    isquare[i] += delta;
    TraceSquare(isquare[i]);
  }
  TraceText("\n");

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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_imitator_movement;
  top_elmt->reason = reason;
  top_elmt->u.imitator_movement.delta = delta;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  move_imitators(delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void undo_imitator_movement(move_effect_journal_index_type curr)
{
  int const delta = move_effect_journal[curr].u.imitator_movement.delta;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  move_imitators(-delta);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void redo_imitator_movement(move_effect_journal_index_type curr)
{
  int const delta = move_effect_journal[curr].u.imitator_movement.delta;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

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
  move_effect_journal_entry_type * const top_elmt = &move_effect_journal[move_effect_journal_top[nbply]];

  TraceFunctionEntry(__func__);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  assert(move_effect_journal_top[nbply]+1<move_effect_journal_size);

  top_elmt->type = move_effect_imitator_addition;
  top_elmt->reason = reason;
  top_elmt->u.imitator_addition.to = to;
#if defined(DOTRACE)
  top_elmt->id = move_effect_journal_next_id++;
  TraceValue("%lu\n",top_elmt->id);
#endif

  ++move_effect_journal_top[nbply];

  if (number_of_imitators==maxinum)
    FtlMsg(ManyImitators);

  isquare[number_of_imitators] = to;
  ++number_of_imitators;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void undo_imitator_addition(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  assert(number_of_imitators>0);
  --number_of_imitators;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void redo_imitator_addition(move_effect_journal_index_type curr)
{
  square const to = move_effect_journal[curr].u.imitator_addition.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

#if defined(DOTRACE)
  TraceValue("%lu\n",move_effect_journal[curr].id);
#endif

  if (number_of_imitators==maxinum)
    FtlMsg(ManyImitators);

  isquare[number_of_imitators] = to;
  ++number_of_imitators;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static int imitator_diff(void)
{
  move_effect_journal_index_type const top = move_effect_journal_top[nbply];
  move_effect_journal_index_type curr;
  int result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (curr = move_effect_journal_top[nbply-1]; curr!=top; ++curr)
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
stip_length_type imitator_mover_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  int const diff = imitator_diff();

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_effect_journal_do_imitator_movement(move_effect_reason_movement_imitation,
                                           diff);
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
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type moving_pawn_to_imitator_promoter_solve(slice_index si,
                                                        stip_length_type n)
{
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (post_move_iteration_id[nbply]!=prev_post_move_iteration_id[nbply])
    promotion_of_moving_into_imitator[nbply] = is_square_occupied_by_promotable_pawn(sq_arrival)!=no_side;

  if (promotion_of_moving_into_imitator[nbply])
  {
    move_effect_journal_do_piece_removal(move_effect_reason_pawn_promotion,
                                         sq_arrival);
    move_effect_journal_do_imitator_addition(move_effect_reason_pawn_promotion,
                                             sq_arrival);

    result = solve(slices[si].next2,n);

    if (!post_move_iteration_locked[nbply])
    {
      promotion_of_moving_into_imitator[nbply] = false;
      lock_post_move_iterations();
    }
  }
  else
    result = solve(slices[si].next1,n);

  prev_post_move_iteration_id[nbply] = post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_landing(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STLandingAfterMovingPawnPromoter);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
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
    slice_index const prototype = alloc_fork_slice(STMovingPawnToImitatorPromoter,proxy);
    assert(*landing!=no_slice);
    link_to_branch(proxy,*landing);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  slice_index * const landing = st->param;
  slice_index const save_landing = *landing;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *landing = no_slice;
  insert_landing(si,st);
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
  stip_structure_traversal_override_single(&st,STMove,&instrument_move);
  stip_structure_traversal_override_single(&st,STLandingAfterMovingPawnPromoter,&remember_landing);
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
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void stip_insert_imitator(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  if (!CondFlag[noiprom])
    insert_promoters(si);

  stip_instrument_moves(si,STImitatorMover);

  stip_instrument_observation_testing(si,nr_sides,STTestingObservationImitator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
