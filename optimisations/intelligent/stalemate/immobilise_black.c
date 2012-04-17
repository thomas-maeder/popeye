#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "pydata.h"
#include "stipulation/battle_play/attack_play.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/stalemate/white_block.h"
#include "optimisations/intelligent/stalemate/black_block.h"
#include "optimisations/intelligent/stalemate/pin_black_piece.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/temporary_hacks.h"
#include "debugging/trace.h"

#include <assert.h>

typedef enum
{
  no_requirement,
  block_of_pawn_required,
  block_of_officer_required,
  pin_required,
  immobilisation_impossible
} immobilisation_requirement_type;

typedef struct
{
    square target_square;
    unsigned int nr_flight_directions;
    square closest_flights[8];
    immobilisation_requirement_type requirement;
} trouble_maker_type;

typedef struct
{
    trouble_maker_type worst;
    trouble_maker_type current;
} immobilisation_state_type;


static immobilisation_state_type const null_state;
static trouble_maker_type const null_trouble_maker;
static immobilisation_state_type * current_state;

static
boolean can_we_block_all_necessary_squares(trouble_maker_type const *state)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",state->requirement);
  TraceFunctionParam("%u",state->nr_flight_directions);
  TraceFunctionParamListEnd();

  switch (state->requirement)
  {
    case no_requirement:
      result = true;
      break;

    case block_of_pawn_required:
      result = intelligent_get_nr_reservable_masses(no_side)>=state->nr_flight_directions;
      break;

    case block_of_officer_required:
      result = intelligent_get_nr_reservable_masses(Black)>=state->nr_flight_directions;
      break;

    case pin_required:
      result = false;
      break;

    case immobilisation_impossible:
    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void next_trouble_maker(void)
{
  if (current_state->worst.requirement
      <current_state->current.requirement
      || (current_state->worst.requirement
          ==current_state->current.requirement
          && current_state->worst.nr_flight_directions
             <current_state->current.nr_flight_directions))
    current_state->worst = current_state->current;
}

static void block_squares(trouble_maker_type const *trouble_maker)
{
  if (trouble_maker->requirement==block_of_pawn_required)
  {
    assert(trouble_maker->nr_flight_directions==1);
    intelligent_stalemate_white_block(trouble_maker->closest_flights[0]);

    if (*where_to_start_placing_black_pieces<=trouble_maker->closest_flights[0])
      intelligent_stalemate_black_block(trouble_maker->closest_flights,
                                        trouble_maker->nr_flight_directions);
  }
  else
    intelligent_stalemate_black_block(trouble_maker->closest_flights,
                                      trouble_maker->nr_flight_directions);
}

/* @return true iff >=1 black pieces needed to be immobilised
 */
boolean intelligent_stalemate_immobilise_black(void)
{
  boolean result = false;
  immobilisation_state_type immobilisation_state = null_state;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  current_state = &immobilisation_state;
  attack(slices[current_start_slice].u.fork.fork,length_unspecified);
  next_trouble_maker();
  current_state = 0;

  if (immobilisation_state.worst.requirement>no_requirement)
  {
    assert(immobilisation_state.worst.target_square!=initsquare);

    TraceSquare(immobilisation_state.worst.target_square);
    TraceValue("%u",immobilisation_state.worst.requirement);
    TraceValue("%u\n",immobilisation_state.worst.nr_flight_directions);

    if (immobilisation_state.worst.requirement<immobilisation_impossible)
    {
      intelligent_stalemate_pin_black_piece(immobilisation_state.worst.target_square);

      if (immobilisation_state.worst.requirement<pin_required
          && can_we_block_all_necessary_squares(&immobilisation_state.worst))
        block_squares(&immobilisation_state.worst);
    }

    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void update_leaper_requirement(immobilisation_requirement_type if_unblockable)
{
  boolean const is_block_possible = (pprise[nbply]==vide
                                     && nr_reasons_for_staying_empty[move_generation_stack[nbcou].arrival]==0
                                     && *where_to_start_placing_black_pieces<=move_generation_stack[nbcou].arrival);
  immobilisation_requirement_type const new_req = is_block_possible ? block_of_officer_required : if_unblockable;
  if (current_state->current.requirement<new_req)
    current_state->current.requirement = new_req;
  assert(current_state->current.nr_flight_directions<8);
  current_state->current.closest_flights[current_state->current.nr_flight_directions] = move_generation_stack[nbcou].arrival;
  ++current_state->current.nr_flight_directions;
}

static void update_rider_requirement(immobilisation_requirement_type if_unblockable)
{
  int const diff = (move_generation_stack[nbcou].arrival
                    -move_generation_stack[nbcou].departure);
  int const dir = CheckDir[Queen][diff];
  if (diff==dir)
  {
    square const closest_flight = move_generation_stack[nbcou].departure+dir;
    boolean const is_block_possible = (pprise[nbply]==vide
                                       && nr_reasons_for_staying_empty[closest_flight]==0
                                       && *where_to_start_placing_black_pieces<=closest_flight);
    immobilisation_requirement_type const new_req = is_block_possible ? block_of_officer_required : if_unblockable;
    if (current_state->current.requirement<new_req)
      current_state->current.requirement = new_req;
    assert(current_state->current.nr_flight_directions<8);
    current_state->current.closest_flights[current_state->current.nr_flight_directions] = closest_flight;
    ++current_state->current.nr_flight_directions;
  }
}

static void update_pawn_requirement(void)
{
  int const diff = (move_generation_stack[nbcou].arrival
                    -move_generation_stack[nbcou].departure);
  switch (diff)
  {
    case dir_down:
      /* the following test prevents promotions into different pices from adding
       * the same square to closest_flights more than once */
      if (current_state->current.requirement==no_requirement)
      {
        if (nr_reasons_for_staying_empty[move_generation_stack[nbcou].arrival]==0)
        {
          current_state->current.closest_flights[current_state->current.nr_flight_directions] = move_generation_stack[nbcou].arrival;
          ++current_state->current.nr_flight_directions;
          current_state->current.requirement = block_of_pawn_required;
        }
        else
          current_state->current.requirement = pin_required;
      }
      break;

    case 2*dir_down:
      /* dealt with by single step case */
      break;

    case dir_down+dir_left:
    case dir_down+dir_right:
      /* this works for both regular and en passant captures: */
      current_state->current.requirement = pin_required;
      break;

    default:
      assert(0);
      break;
  }
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type intelligent_immobilisation_counter_attack(slice_index si,
                                                           stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (move_generation_stack[nbcou].departure!=current_state->current.target_square)
  {
    next_trouble_maker();
    current_state->current = null_trouble_maker;
    current_state->current.target_square = move_generation_stack[nbcou].departure;
  }

  switch (pjoue[nbply])
  {
    case roin: /* unpinnable leaper */
      update_leaper_requirement(immobilisation_impossible);
      break;

    case dn: /* unpinnable rider */
      update_rider_requirement(immobilisation_impossible);
      break;

    case tn:
    case fn: /* pinnable rider */
      update_rider_requirement(pin_required);
      break;

    case cn: /* pinnable leaper */
      update_leaper_requirement(pin_required);
      break;

    case pn: /* pinnable rider, blockable by White */
      update_pawn_requirement();
      break;

    default:  /* no support for fairy chess */
      assert(0);
      break;
  }

  if (current_state->current.requirement==immobilisation_impossible)
    result = n; /* abort iteration over moves */
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
