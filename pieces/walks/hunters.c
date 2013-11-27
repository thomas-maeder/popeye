#include "pieces/walks/hunters.h"
#include "pieces/walks/riders.h"
#include "pieces/walks/generate_moves.h"
#include "solving/move_generator.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>
#include <string.h>

HunterType huntertypes[maxnrhuntertypes];
unsigned int nrhuntertypes;

typedef boolean (*direction_validator_type)(numecoup n);

static direction_validator_type direction_validator[maxply+1];

PieNam hunter_make_type(PieNam away, PieNam home)
{
  unsigned int i;
  for (i = 0; i!=nrhuntertypes; ++i)
    if (huntertypes[i].away==away && huntertypes[i].home==home)
      return Hunter0+i;

  if (nrhuntertypes<maxnrhuntertypes)
  {
    PieNam const result = Hunter0+nrhuntertypes;
    HunterType * const huntertype = huntertypes+nrhuntertypes;
    huntertype->away = away;
    huntertype->home = home;
    ++nrhuntertypes;
    return result;
  }
  else
    return Invalid;
}

static boolean goes_up(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  square const sq_arrival = move_generation_stack[n].arrival;
  numvec const diff = sq_arrival-sq_departure;

  return diff>nr_files_on_board;
}

static boolean goes_down(numecoup n)
{
  square const sq_departure = move_generation_stack[n].departure;
  square const sq_arrival = move_generation_stack[n].arrival;
  numvec const diff = sq_arrival-sq_departure;

  return diff<-nr_files_on_board;
}

static void generate_one_dir(PieNam part, move_filter_criterion_type criterion)
{
  numecoup const savenbcou = CURRMOVE_OF_PLY(nbply);
  generate_moves_for_piece_based_on_walk(part);
  move_generator_filter_moves(savenbcou,criterion);
}

boolean hunter_check(validator_id evaluate)
{
  /* detect check by a hunter */
  boolean result;
  direction_validator_type const goes_away = trait[nbply]==Black ? &goes_down : &goes_up;
  direction_validator_type const goes_home = trait[nbply]==Black ? &goes_up : &goes_down;
  unsigned int const typeofhunter = observing_walk[nbply]-Hunter0;
  HunterType const * const huntertype = huntertypes+typeofhunter;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(typeofhunter<maxnrhuntertypes);

  direction_validator[nbply] = goes_home;
  if ((*checkfunctions[huntertype->home])(evaluate))
    result = true;
  else
  {
    direction_validator[nbply] = goes_away;
    result = (*checkfunctions[huntertype->away])(evaluate);
  }

  direction_validator[nbply] = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

void hunter_generate_moves(PieNam walk)
{
  TraceFunctionEntry(__func__);
  TracePiece(walk);
  TraceFunctionParamListEnd();

  assert(walk>=Hunter0);
  assert(walk<Hunter0+maxnrhuntertypes);

  {
    unsigned int const typeofhunter = walk-Hunter0;
    HunterType const * const huntertype = huntertypes+typeofhunter;

    if (trait[nbply]==White)
    {
      generate_one_dir(huntertype->home,&goes_down);
      generate_one_dir(huntertype->away,&goes_up);
    }
    else
    {
      generate_one_dir(huntertype->away,&goes_down);
      generate_one_dir(huntertype->home,&goes_up);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void rook_hunter_generate_moves(void)
{
  generate_one_dir(Bishop,&goes_down);
  generate_one_dir(Rook,&goes_up);
}

void bishop_hunter_generate_moves(void)
{
  generate_one_dir(Rook,&goes_down);
  generate_one_dir(Bishop,&goes_up);
}

boolean rookhunter_check(validator_id evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* always moves up as rook and down as bishop!! */
  direction_validator[nbply] = &goes_up;
  if ((*checkfunctions[Rook])(evaluate))
    result = true;
  else
  {
    direction_validator[nbply] = &goes_down;
    result = (*checkfunctions[Bishop])(evaluate);
  }

  direction_validator[nbply] = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean bishophunter_check(validator_id evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* always moves up as bishop and down as rook!! */
  direction_validator[nbply] = &goes_down;
  if ((*checkfunctions[Rook])(evaluate))
    result = true;
  else
  {
    direction_validator[nbply] = &goes_up;
    result = (*checkfunctions[Bishop])(evaluate);
  }

  direction_validator[nbply] = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Make sure that the observer has the expected direction
 * @return true iff the observation is valid
 */
boolean hunter_enforce_observer_direction(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = ((direction_validator[nbply]==0
             || (*direction_validator[nbply])(CURRMOVE_OF_PLY(nbply)))
            && validate_observation_recursive(slices[si].next1));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the solving machinery with hunter direction enforcement
 * @param root identifies the root slice of the solving machinery
 */
void solving_initialise_hunters(slice_index root)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",root);
  TraceFunctionParamListEnd();

  if (nrhuntertypes>0 || exist[RookHunter] || exist[BishopHunter])
  {
    stip_instrument_observation_validation(root,nr_sides,STEnforceHunterDirection);
    stip_instrument_observer_validation(root,nr_sides,STEnforceHunterDirection);
    stip_instrument_observation_geometry_validation(root,nr_sides,STEnforceHunterDirection);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
