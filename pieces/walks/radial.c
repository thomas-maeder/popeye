#include "pieces/walks/radial.h"
#include "solving/move_generator.h"
#include "solving/fork.h"
#include "position/position.h"
#include "debugging/trace.h"

static
struct
{
    vec_index_type start;
    vec_index_type end;
} const queen_radial_ranges[] =
{
    { vec_rook_start,    vec_rook_end },
    { vec_dabbaba_start, vec_dabbaba_end },
    { vec_leap03_start,  vec_leap03_end },
    { vec_leap04_start,  vec_leap04_end },
    { vec_leap05_start,  vec_leap05_end },
    { vec_leap06_start,  vec_leap06_end },
    { vec_leap07_start,  vec_leap07_end },
    { vec_bishop_start,  vec_bishop_end },
    { vec_alfil_start,   vec_alfil_end },
    { vec_leap33_start,  vec_leap33_end },
    { vec_leap44_start,  vec_leap44_end },
    { vec_leap55_start,  vec_leap55_end },
    { vec_leap66_start,  vec_leap66_end },
    { vec_leap77_start,  vec_leap77_end }
};

enum
{
  nr_queen_radial_ranges = sizeof queen_radial_ranges / sizeof queen_radial_ranges[0]
};

static
struct
{
    vec_index_type start;
    vec_index_type end;
} const radial_ranges[] =
{
    { vec_rook_start,      vec_rook_end },
    { vec_dabbaba_start,   vec_dabbaba_end },
    { vec_leap03_start,    vec_leap03_end },
    { vec_leap04_start,    vec_leap04_end },
    { vec_bucephale_start, vec_bucephale_end },
    { vec_leap06_start,    vec_leap06_end },
    { vec_leap07_start,    vec_leap07_end },
    { vec_bishop_start,    vec_bishop_end },
    { vec_knight_start,    vec_knight_end },
    { vec_chameau_start,   vec_chameau_end },
    { vec_girafe_start,    vec_girafe_end },
    { vec_leap15_start,    vec_leap15_end },
    { vec_leap16_start,    vec_leap16_end },
    { vec_rccinq_start,    vec_rccinq_end },
    { vec_alfil_start,     vec_alfil_end },
    { vec_zebre_start,     vec_zebre_end },
    { vec_leap24_start,    vec_leap24_end },
    { vec_leap25_start,    vec_leap25_end },
    { vec_leap26_start,    vec_leap26_end },
    { vec_leap27_start,    vec_leap27_end },
    { vec_leap33_start,    vec_leap33_end },
    { vec_leap35_start,    vec_leap35_end },
    { vec_leap36_start,    vec_leap36_end },
    { vec_leap37_start,    vec_leap37_end },
    { vec_leap44_start,    vec_leap44_end },
    { vec_leap45_start,    vec_leap45_end },
    { vec_leap46_start,    vec_leap46_end },
    { vec_leap47_start,    vec_leap47_end },
    { vec_leap56_start,    vec_leap56_end },
    { vec_leap57_start,    vec_leap57_end },
    { vec_leap66_start,    vec_leap66_end },
    { vec_leap67_start,    vec_leap67_end },
    { vec_leap77_start,    vec_leap77_end }
};

enum
{
  nr_radial_ranges = sizeof radial_ranges / sizeof radial_ranges[0]
};

static void generate(square sq_arrival)
{
  curr_generation->arrival = sq_arrival;
  if (is_square_empty(curr_generation->arrival))
    push_move_no_capture();
  else if (piece_belongs_to_opponent(curr_generation->arrival))
    push_move_regular_capture();
}

static void radialknight_generate(vec_index_type kanf, vec_index_type kend)
{
  vec_index_type idx_to_hurdle;

  for (idx_to_hurdle = kanf; idx_to_hurdle<=kend; ++idx_to_hurdle)
  {
    square const sq_hurdle = curr_generation->departure+vec[idx_to_hurdle];
    if (piece_belongs_to_opponent(sq_hurdle))
    {
      vec_index_type idx_hurdle_to_arrival;
      for (idx_hurdle_to_arrival = kanf; idx_hurdle_to_arrival<=kend; idx_hurdle_to_arrival++)
        generate(sq_hurdle+vec[idx_hurdle_to_arrival]);
    }
  }
}

/* Generate moves for a radial knight
 */
void radialknight_generate_moves(void)
{
  unsigned int i;

  for (i = 0; i!=nr_radial_ranges; ++i)
    radialknight_generate(radial_ranges[i].start,radial_ranges[i].end);
}

static boolean test_radialknight_check(vec_index_type kanf, vec_index_type kend,
                                       validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type idx_to_hurdle;

  for (idx_to_hurdle = kanf; idx_to_hurdle<=kend; idx_to_hurdle++)
  {
    square const sq_hurdle = sq_target+vec[idx_to_hurdle];
    if (!is_square_blocked(sq_hurdle) && !is_square_empty(sq_hurdle)
        && TSTFLAG(being_solved.spec[sq_hurdle],advers(trait[nbply])))
    {
      vec_index_type idx_to_observer;
      for (idx_to_observer= kanf; idx_to_observer<= kend; idx_to_observer++)
      {
        square const sq_observer = sq_hurdle+vec[idx_to_observer];
        if (sq_observer!=sq_target
            && EVALUATE_OBSERVATION(evaluate,sq_observer,sq_target))
          return true;
      }
    }
  }

  return false;
}

boolean radialknight_check(validator_id evaluate)
{
  unsigned int i;

  for (i = 0; i!=nr_radial_ranges; ++i)
    if (test_radialknight_check(radial_ranges[i].start,radial_ranges[i].end, evaluate))
      return true;

  return false;
}

static boolean test_treehopper_check(vec_index_type kanf, vec_index_type kend,
                                     validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type idx_to_hurdle;

  for (idx_to_hurdle = kanf; idx_to_hurdle<=kend; idx_to_hurdle++)
  {
    square const sq_hurdle = sq_target+vec[idx_to_hurdle];
    if (!is_square_blocked(sq_hurdle) && !is_square_empty(sq_hurdle))
    {
      vec_index_type idx_hurdle_to_observer;
      for (idx_hurdle_to_observer= kanf; idx_hurdle_to_observer<= kend; idx_hurdle_to_observer++)
      {
        square const sq_observer = sq_hurdle+vec[idx_hurdle_to_observer];
        if (sq_observer!=sq_target
            && EVALUATE_OBSERVATION(evaluate,sq_observer,sq_target))
          return true;
      }
    }
  }

  return false;
}

static void treehopper_generate(vec_index_type kanf, vec_index_type kend)
{
  vec_index_type idx_to_hurdle;

  for (idx_to_hurdle = kanf; idx_to_hurdle<=kend; ++idx_to_hurdle)
  {
    square const sq_hurdle = curr_generation->departure+vec[idx_to_hurdle];
    if (!is_square_blocked(sq_hurdle) && !is_square_empty(sq_hurdle))
    {
      vec_index_type idx_hurdle_to_arrival;
      for (idx_hurdle_to_arrival = kanf; idx_hurdle_to_arrival<=kend; idx_hurdle_to_arrival++)
        generate(sq_hurdle+vec[idx_hurdle_to_arrival]);
    }
  }
}

/* Generate moves for a (lesser) tree hopper
 */
void treehopper_generate_moves(void)
{
  unsigned int i;

  for (i = 0; i!=nr_queen_radial_ranges; ++i)
    treehopper_generate(queen_radial_ranges[i].start,queen_radial_ranges[i].end);
}

/* Generate moves for a greater tree hopper
 */
void greater_treehopper_generate_moves(void)
{
  unsigned int i;

  for (i = 0; i!=nr_radial_ranges; ++i)
    treehopper_generate(radial_ranges[i].start,radial_ranges[i].end);
}

boolean treehopper_check(validator_id evaluate)
{
  unsigned int i;

  for (i = 0; i!=nr_queen_radial_ranges; ++i)
    if (test_treehopper_check(queen_radial_ranges[i].start,queen_radial_ranges[i].end, evaluate))
      return true;

  return false;
}

boolean greatertreehopper_check(validator_id evaluate)
{
  unsigned int i;

  for (i = 0; i!=nr_radial_ranges; ++i)
    if (test_treehopper_check(radial_ranges[i].start,radial_ranges[i].end, evaluate))
      return true;

  return false;
}

static void leafhopper_generate(vec_index_type kanf, vec_index_type kend)
{
  vec_index_type idx_to_hurdle;

  for (idx_to_hurdle = kanf; idx_to_hurdle<=kend; ++idx_to_hurdle)
  {
    square const sq_hurdle = curr_generation->departure+vec[idx_to_hurdle];
    if (!is_square_blocked(sq_hurdle) && !is_square_empty(sq_hurdle))
    {
      vec_index_type idx_to_arrival;
      for (idx_to_arrival = kanf; idx_to_arrival<=kend; idx_to_arrival++)
        if (idx_to_arrival!=idx_to_hurdle)
          generate(curr_generation->departure+vec[idx_to_arrival]);
    }
  }
}

/* Generate moves for a (lesser) leaf hopper
 */
void leafhopper_generate_moves(void)
{
  unsigned int i;

  for (i = 0; i!=nr_queen_radial_ranges; ++i)
    leafhopper_generate(queen_radial_ranges[i].start,queen_radial_ranges[i].end);
}

/* Generate moves for a greater leaf hopper
 */
void greater_leafhopper_generate_moves(void)
{
  unsigned int i;

  for (i = 0; i!=nr_radial_ranges; ++i)
    leafhopper_generate(radial_ranges[i].start,radial_ranges[i].end);
}

static boolean test_leafhopper_check(vec_index_type kanf, vec_index_type kend,
                                     validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type  idx_to_observer;

  TraceSquare(sq_target);
  TraceEOL();
  for (idx_to_observer = kanf; idx_to_observer<=kend; idx_to_observer++)
  {
    square const sq_observer = sq_target+vec[idx_to_observer];
    TraceSquare(sq_observer);
    TraceValue("%u",is_square_blocked(sq_observer));
    TraceValue("%u",is_square_empty(sq_observer));
    TraceEOL();
    if (!is_square_blocked(sq_observer) && !is_square_empty(sq_observer))
    {
      vec_index_type idx_to_hurdle;
      for (idx_to_hurdle = kanf; idx_to_hurdle<=kend; idx_to_hurdle++)
      {
        square const sq_hurdle = sq_observer+vec[idx_to_hurdle];
        TraceSquare(sq_hurdle);
        TraceEOL();
        if (sq_hurdle!=sq_target
            && !is_square_blocked(sq_hurdle) && !is_square_empty(sq_hurdle))
        {
          if (EVALUATE_OBSERVATION(evaluate,sq_observer,sq_target))
            return true;
          else
            /* there won't be any more observation using a different hurdle */
            break;
        }
      }
    }
  }

  return false;
}

boolean leafhopper_check(validator_id evaluate)
{
  unsigned int i;

  for (i = 0; i!=nr_queen_radial_ranges; ++i)
    if (test_leafhopper_check(queen_radial_ranges[i].start,queen_radial_ranges[i].end, evaluate))
      return true;

  return false;
}

boolean greaterleafhopper_check(validator_id evaluate)
{
  unsigned int i;

  for (i = 0; i!=nr_radial_ranges; ++i)
    if (test_leafhopper_check(radial_ranges[i].start,radial_ranges[i].end, evaluate))
      return true;

  return false;
}
