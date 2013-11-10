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

static evalfunction_t *next_evaluate;

static boolean eval_up(void)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  return (sq_arrival-sq_departure>8
          && INVOKE_EVAL(next_evaluate,sq_departure,sq_arrival));
}

static boolean eval_down(void)
{
  square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(nbply)].arrival;
  return (sq_arrival-sq_departure<-8
          && INVOKE_EVAL(next_evaluate,sq_departure,sq_arrival));
}

boolean hunter_check(evalfunction_t *evaluate)
{
  /* detect check by a hunter */
  boolean result;
  evalfunction_t * const eval_away = trait[nbply]==Black ? &eval_down : &eval_up;
  evalfunction_t * const eval_home = trait[nbply]==Black ? &eval_up : &eval_down;
  unsigned int const typeofhunter = observing_walk[nbply]-Hunter0;
  HunterType const * const huntertype = huntertypes+typeofhunter;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(typeofhunter<maxnrhuntertypes);
  next_evaluate = evaluate;
  result = ((*checkfunctions[huntertype->home])(eval_home)
            || (*checkfunctions[huntertype->away])(eval_away));

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

boolean rookhunter_check(evalfunction_t *evaluate)
{
  /* detect check of a rook/bishop-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (rook), down (bishop) !! */
  return riders_check(4, 4, evaluate) || riders_check(5, 6, evaluate);
}

boolean bishophunter_check(evalfunction_t *evaluate)
{
  /* detect check of a bishop/rook-hunter */
  /* it's not dependent of the piece-color !! */
  /* always moves up (bishop), down (rook) !! */
  return riders_check(2, 2, evaluate)
      || riders_check(7, 8, evaluate);
}
