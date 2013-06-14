#include "pieces/attributes/magic.h"
#include "pydata.h"
#include "pymsg.h"
#include "pieces/roses.h"
#include "position/pieceid.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>

/* magic pieces */
enum
{
  magicviews_size = 10000
};

typedef struct
{
  square pos_viewed;
  PieceIdType viewedid;
  PieceIdType magicpieceid;
  numvec vec_viewed_to_magic;
} magicview_type;

static magicview_type magicviews[magicviews_size];

unsigned int magic_views_top[maxply + 1];

static void PushMagicView(square pos_viewed, square pos_magic, numvec vec_viewed_to_magic)
{
  ply const parent = parent_ply[nbply];

  TraceFunctionEntry(__func__);
  TraceSquare(pos_viewed);
  TraceSquare(pos_magic);
  TraceValue("%d",vec_viewed_to_magic);
  TraceFunctionParamListEnd();

  assert(magic_views_top[parent]<magicviews_size);

  magicviews[magic_views_top[parent]].pos_viewed = pos_viewed;
  magicviews[magic_views_top[parent]].viewedid = GetPieceId(spec[pos_viewed]);
  magicviews[magic_views_top[parent]].magicpieceid = GetPieceId(spec[pos_magic]);
  magicviews[magic_views_top[parent]].vec_viewed_to_magic = vec_viewed_to_magic;
  ++magic_views_top[parent];

  TraceValue("%u",parent);
  TraceValue("%u\n",magic_views_top[parent]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void GetRoseAttackVectors(square from, square to)
{
  vec_index_type vec_index_start;
  for (vec_index_start = vec_knight_start; vec_index_start<=vec_knight_end; vec_index_start++)
  {
    if (detect_rosecheck_on_line(to,get_walk_of_piece_on_square(from),
                                 vec_index_start,0,rose_rotation_clockwise,
                                 eval_fromspecificsquare))
      PushMagicView(to, from, 200+vec[vec_index_start] );
    if (detect_rosecheck_on_line(to,get_walk_of_piece_on_square(from),
                                 vec_index_start,vec_knight_end-vec_knight_start+1,rose_rotation_counterclockwise,
                                 eval_fromspecificsquare))
      PushMagicView(to, from, 300+vec[vec_index_start]);
  }
}

void GetRoseLionAttackVectors(square from, square to)
{
  vec_index_type vec_index_start;
  for (vec_index_start = vec_knight_start; vec_index_start <= vec_knight_end; vec_index_start++)
  {
    if (detect_roselioncheck_on_line(to,get_walk_of_piece_on_square(from),
                                     vec_index_start,0,rose_rotation_clockwise,
                                     eval_fromspecificsquare))
      PushMagicView(to, from, 200+vec[vec_index_start] );
    if (detect_roselioncheck_on_line(to,get_walk_of_piece_on_square(from),
                                     vec_index_start,vec_knight_end-vec_knight_start+1,rose_rotation_counterclockwise,
                                     eval_fromspecificsquare))
      PushMagicView(to, from, 300+vec[vec_index_start]);
  }
}

void GetRoseHopperAttackVectors(square from, square to)
{
  vec_index_type vec_index_start;

  for (vec_index_start = vec_knight_start; vec_index_start <= vec_knight_end; vec_index_start++)
  {
    square const sq_hurdle= to+vec[vec_index_start];
    if (!is_square_empty(sq_hurdle) && !is_square_blocked(sq_hurdle))
    {
        /* k1==0 (and the equivalent
         * vec_knight_end-vec_knight_start+1) were already used for
         * sq_hurdle! */
      if (detect_rosehoppercheck_on_line(to,sq_hurdle,get_walk_of_piece_on_square(from),
                                         vec_index_start,1,rose_rotation_clockwise,
                                         eval_fromspecificsquare))
        PushMagicView(to, from, 200+vec[vec_index_start] );
      if (detect_rosehoppercheck_on_line(to,sq_hurdle,get_walk_of_piece_on_square(from),
                                         vec_index_start,vec_knight_end-vec_knight_start,rose_rotation_counterclockwise,
                                         eval_fromspecificsquare))
        PushMagicView(to, from, 300+vec[vec_index_start]);
    }
  }
}

void GetRoseLocustAttackVectors(square from, square to)
{
  /* detects check by a rose locust */
  vec_index_type  vec_index_start;

  for (vec_index_start = vec_knight_start; vec_index_start <= vec_knight_end; vec_index_start++)
  {
    square const sq_arrival= to-vec[vec_index_start];
    if (is_square_empty(sq_arrival))
    {
        /* k1==0 (and the equivalent
         * vec_knight_end-vec_knight_start+1) were already used for
         * sq_hurdle! */
      if (detect_roselocustcheck_on_line(to,sq_arrival,get_walk_of_piece_on_square(from),
                                         vec_index_start,1,rose_rotation_clockwise,
                                         eval_fromspecificsquare))
        PushMagicView(to, from, 200+vec[vec_index_start] );
      if (detect_roselocustcheck_on_line(to,sq_arrival,get_walk_of_piece_on_square(from),
                                         vec_index_start,vec_knight_end-vec_knight_start,rose_rotation_counterclockwise,
                                         eval_fromspecificsquare))
        PushMagicView(to, from, 300+vec[vec_index_start]);
    }
  }
}

static void GetRMHopAttackVectors(square from, square to,
                                  vec_index_type kend, vec_index_type kanf,
                                  angle_t angle)
{
  vec_index_type k;

  for (k= kend; k>=kanf; k--)
  {
    square const sq_hurdle = to+vec[k];
    if (get_walk_of_piece_on_square(sq_hurdle)>=King)
    {
      vec_index_type const k1 = 2*k;

      {
        square const sq_departure = find_end_of_line(sq_hurdle,mixhopdata[angle][k1]);
        PieNam const hopper = get_walk_of_piece_on_square(sq_departure);
        if (hopper==get_walk_of_piece_on_square(from)
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && eval_fromspecificsquare(sq_departure,to,to))
          PushMagicView(to,from,vec[k]);
      }

      {
        square const sq_departure = find_end_of_line(sq_hurdle,mixhopdata[angle][k1-1]);
        PieNam const hopper = get_walk_of_piece_on_square(sq_departure);
        if (hopper==get_walk_of_piece_on_square(from)
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && eval_fromspecificsquare(sq_departure,to,to))
          PushMagicView(to,from,vec[k]);
      }
    }
  }
}

void GetMooseAttackVectors(square from, square to)
{
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_45);
}

void GetRookMooseAttackVectors(square from, square to) {
  GetRMHopAttackVectors(from, to, vec_rook_end, vec_rook_start, angle_45);
}

void GetBishopMooseAttackVectors(square from, square to)
{
  GetRMHopAttackVectors(from, to, vec_bishop_end, vec_bishop_start, angle_45);
}

void GetEagleAttackVectors(square from, square to)
{
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_90);
}

void GetRookEagleAttackVectors(square from, square to)
{
  GetRMHopAttackVectors(from, to, vec_rook_end, vec_rook_start, angle_90);
}

void GetBishopEagleAttackVectors(square from, square to)
{
  GetRMHopAttackVectors(from, to, vec_bishop_end, vec_bishop_start, angle_90);
}

void GetSparrowAttackVectors(square from, square to)
{
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_135);
}

void GetRookSparrowAttackVectors(square from, square to)
{
  GetRMHopAttackVectors(from, to, vec_rook_end, vec_rook_start, angle_135);
}

void GetBishopSparrowAttackVectors(square from, square to)
{
  GetRMHopAttackVectors(from, to, vec_bishop_end, vec_bishop_start, angle_135);
}

void GetMargueriteAttackVectors(square from, square to)
{
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_45);
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_90);
  GetRMHopAttackVectors(from, to, vec_queen_end, vec_queen_start, angle_135);
  if (scheck(to, get_walk_of_piece_on_square(from), eval_fromspecificsquare))
  {
    numvec attackVec;
    if (to < from)
      attackVec = move_vec_code[from - to];
    else
      attackVec = -move_vec_code[to - from];
    if (attackVec)
      PushMagicView(to, from, attackVec);
  }
}

static void GetZigZagAttackVectors(square from, square to,
                                   numvec  k,
                                   numvec  k1)
{
  square sq_departure= to+k;
  square sq_arrival= to;
  square sq_capture= to;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParam("%d",k);
  TraceFunctionParam("%d",k1);
  TraceFunctionParamListEnd();

  while (is_square_empty(sq_departure))
  {
    sq_departure += k1;
    if (is_square_empty(sq_departure))
      sq_departure += k;
    else
      break;
  }

  if (sq_departure==from)
  {
    fromspecificsquare = from;
    if (eval_fromspecificsquare(sq_departure,sq_arrival,sq_capture))
      PushMagicView(to, from, 500+k );
  }

  sq_departure = to+k;
  while (is_square_empty(sq_departure))
  {
    sq_departure -= k1;
    if (is_square_empty(sq_departure))
      sq_departure += k;
    else
      break;
  }

  if (sq_departure==from)
  {
    fromspecificsquare = from;
    if (eval_fromspecificsquare(sq_departure,sq_arrival,sq_capture))
      PushMagicView(to, from, 400+k );
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void GetBoyscoutAttackVectors(square from, square to)
{
  vec_index_type const top = vec_bishop_start+vec_bishop_end;
  vec_index_type k;

  for (k = vec_bishop_start; k<=vec_bishop_end; k++)
    GetZigZagAttackVectors(from, to, vec[k], vec[top-k]);
}

void GetGirlscoutAttackVectors(square from, square to)
{
  vec_index_type const top = vec_rook_start+vec_rook_end;
  vec_index_type k;

  for (k = vec_rook_start; k<=vec_rook_end; k++)
    GetZigZagAttackVectors(from, to, vec[k], vec[top-k]);
}

void GetSpiralSpringerAttackVectors(square from, square to)
{
  vec_index_type const top = vec_knight_start+vec_knight_end;
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; k++)
    GetZigZagAttackVectors(from, to, vec[k], vec[top-k]);
}

void GetDiagonalSpiralSpringerAttackVectors(square from, square to) {
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; k += 2)
  {
    GetZigZagAttackVectors(from, to, vec[k],vec[k+1]);
    GetZigZagAttackVectors(from, to, vec[k+1],vec[k]);
  }
}

/* should never get called if validation works
(disallow magic + piecetype) */
void unsupported_uncalled_crosseyed_function(square from, square to) {}

typedef void (*crosseyed_views_function_t)(square, square);

/* magic pieces -
for most types a magic piece of that type can only
solve another unit from one direction in any given position. Therefore
all that is needed is to see if it checks, and use the relative diff to
calculate the vector. These types have NULL entries in the table below.

More complicated types can solve from more than one direction and need
special functions listed below to calculate each potential direction.

Unsupported types are listed below with the entry
unsupported_uncalled_crosseyed_function
*/
static crosseyed_views_function_t crosseyed_views_functions[PieceCount] = {
/*  0 */        0, /* not used */
/*  1 */        0, /* not used */
/*  2 */        0,
/*  3 */        0,
/*  4 */        0,
/*  5 */        0,
/*  6 */        0,
/*  7 */        0,
/*  8 */        0,
/*  9 */        0,
/* 10 */        0,
/* 11 */        0,
/* 12 */        &GetRoseAttackVectors,
/* 13 */        0,
/* 14 */        0,
/* 15 */        0,
/* 16 */        0,
/* 17 */        0,
/* 18 */        0,
/* 19 */        0,
/* 20 */        0,
/* 21 */        0,
/* 22 */        0,
/* 23 */        0,
/* 24 */        0,
/* 25 */        0,
/* 26 */        0,
/* 27 */        0,
/* 28 */        0,
/* 29 */        0,
/* 30 */        0,
/* 31 */        0,
/* 32 */        0,
/* 33 */        0,
/* 34 */        0,
/* 35 */        0,
/* 36 */        0,
/* 37 */        &GetSpiralSpringerAttackVectors,
/* 38 */        &unsupported_uncalled_crosseyed_function, /* ubiubi */
/* 39 */        0,
/* 40 */        &GetMooseAttackVectors,
/* 41 */        &GetEagleAttackVectors,
/* 42 */        &GetSparrowAttackVectors,
/* 43 */        &unsupported_uncalled_crosseyed_function,  /* archbishop */
/* 44 */        &unsupported_uncalled_crosseyed_function, /* ref B */
/* 45 */        &unsupported_uncalled_crosseyed_function, /* cardinal */
/* 46 */        0,
/* 47 */        0,
/* 48 */        0,
/* 49 */        0,
/* 50 */        0,
/* 51 */        0,
/* 52 */        0,
/* 53 */        0,
/* 54 */        &GetDiagonalSpiralSpringerAttackVectors,
/* 55 */        &unsupported_uncalled_crosseyed_function, /* bouncy knight */
/* 56 */        0,
/* 57 */        &unsupported_uncalled_crosseyed_function, /* cat */
/* 58 */        0,
/* 59 */        0,
/* 60 */        0,
/* 61 */        0,
/* 62 */        0,
/* 63 */        0,
/* 64 */        0,
/* 65 */        &unsupported_uncalled_crosseyed_function,  /* orphan */
/* 66 */        0,
/* 67 */        0,
/* 68 */        0,
/* 69 */        0,
/* 70 */        0,
/* 71 */        0,
/* 72 */        0,
/* 73 */        0,
/* 74 */        0,
/* 75 */        &GetBoyscoutAttackVectors, /* boyscout */
/* 76 */        &GetGirlscoutAttackVectors, /* girlscout */
/* 77 */        0, /* skylla - depends on vacant sq?? */
/* 78 */        0, /* charybdis - depends on vacant sq?? */
/* 79 */        0,
/* 80 */        &GetRoseLionAttackVectors,
/* 81 */        &GetRoseHopperAttackVectors,
/* 82 */        0,
/* 83 */        0,
/* 84 */        0,
/* 85 */        0,
/* 86 */        0,
/* 87 */        0,
/* 88 */        0,
/* 89 */        0,
/* 90 */        0,
/* 91 */        0,
/* 92 */        0,
/* 93 */        0,
/* 94 */        0,
/* 95 */        0,
/* 96 */        0,
/* 97 */        0,
/* 98 */        0,
/* 99 */        0,
/*100 */        0,
/*101 */        0,
/*102 */        0,
/*103 */        &GetRookMooseAttackVectors,
/*104 */        &GetRookEagleAttackVectors,
/*105 */        &GetRookSparrowAttackVectors,
/*106 */        &GetBishopMooseAttackVectors,
/*107 */        &GetBishopEagleAttackVectors,
/*108 */        &GetBishopSparrowAttackVectors,
/*109 */        &GetRoseLionAttackVectors,   /* rao checks like roselion */
/*110 */        0,
/*111 */        &GetMargueriteAttackVectors, /* = G+M+EA+SW; magic - believe ok to treat as combination of these */
/*112 */        0,
/*113 */        0,
/*114 */        0,
/*115 */        0,
/*116 */        &unsupported_uncalled_crosseyed_function,    /*friend*/
/*117 */        0,  /* dolphin - do g, g2 count as different vectors? */
/*118 */        0,
/*119 */        0,
/*120 */  0,
/*121 */  0,
/*122 */  0,
/*123 */  0,
/*124 */  0,
/*125 */  0,
/*126 */  0,
/*127 */  &unsupported_uncalled_crosseyed_function, /*radial k*/
/*128 */  0,
/*129 */  &GetRoseLocustAttackVectors,
/*130 */  &unsupported_uncalled_crosseyed_function,
/*131 */  &unsupported_uncalled_crosseyed_function,
/*132 */  &unsupported_uncalled_crosseyed_function,
/*133 */  &unsupported_uncalled_crosseyed_function,
/*134 */  &unsupported_uncalled_crosseyed_function,
/*135 */  &unsupported_uncalled_crosseyed_function,
/*136 */  &unsupported_uncalled_crosseyed_function,
/*137 */  &unsupported_uncalled_crosseyed_function,
/*138 */  &unsupported_uncalled_crosseyed_function,
/*139 */  &unsupported_uncalled_crosseyed_function
};

/* Can a specific type of (fairy) piece be magic?
 * @param p type of piece
 * @return true iff pieces of type p can be magic
 */
boolean magic_is_piece_supported(PieNam p)
{
  return crosseyed_views_functions[p]!=&unsupported_uncalled_crosseyed_function;
}

static void PushMagicViewsByOnePiece(square pos_magic)
{
  PieNam const pi_magic = get_walk_of_piece_on_square(pos_magic);
  square const *pos_viewed;

  TraceFunctionEntry(__func__);
  TraceSquare(pos_magic);
  TraceFunctionParamListEnd();

  for (pos_viewed = boardnum; *pos_viewed; pos_viewed++)
    if (get_walk_of_piece_on_square(*pos_viewed)>Invalid
        && !TSTFLAGMASK(spec[*pos_viewed],BIT(Magic)|BIT(Royal)|BIT(Neutral)))
    {
      /* for each non-magic piece
         (n.b. check *pos_magic != *pos_viewed redundant above) */
      if (crosseyed_views_functions[pi_magic]!=0)
        (*crosseyed_views_functions[pi_magic])(pos_magic,*pos_viewed);
      else
      {
        /* piece is not cross-eyed - use regular check function */
        fromspecificsquare = pos_magic;
        if ((*checkfunctions[pi_magic])(*pos_viewed,
                                        pi_magic,
                                        eval_fromspecificsquare))
        {
          numvec vec_viewed_to_magic;
          if (*pos_viewed<pos_magic)
            vec_viewed_to_magic = move_vec_code[pos_magic-*pos_viewed];
          else
            vec_viewed_to_magic = -move_vec_code[*pos_viewed-pos_magic];
          if (vec_viewed_to_magic!=0)
            PushMagicView(*pos_viewed,pos_magic,vec_viewed_to_magic);
        }
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* avoid unnecessary recursion if checkfunction has to play the observation */
static boolean pushing_magic_views = false;

static void PushMagicViews(void)
{
  square const *pos_magic;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  pushing_magic_views = true;

  magic_views_top[nbply] = magic_views_top[nbply-1];

  nextply();

  for (pos_magic = boardnum; *pos_magic; pos_magic++)
    if (TSTFLAG(spec[*pos_magic], Magic))
    {
      if (TSTFLAG(spec[*pos_magic],White))
      {
        trait[nbply] = White;
        PushMagicViewsByOnePiece(*pos_magic);
      }
      if (TSTFLAG(spec[*pos_magic],Black))
      {
        trait[nbply] = Black;
        PushMagicViewsByOnePiece(*pos_magic);
      }
      /* TODO: remove double views my neutral magic pieces
       * apply same logic as for cross-eyed pieces? */
    }

  finply();

  pushing_magic_views = false;

  TraceValue("%u",nbply);
  TraceValue("%u\n",magic_views_top[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean find_view(ply ply_id, int j)
{
  PieceIdType const currid = magicviews[j].viewedid;
  PieceIdType const magicpieceid = magicviews[j].magicpieceid;
  numvec const vec_viewed_to_magic = magicviews[j].vec_viewed_to_magic;
  boolean result = false;
  unsigned int k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceFunctionParam("%d",j);
  TraceFunctionParamListEnd();

  for (k = magic_views_top[ply_id-1]; k<magic_views_top[ply_id]; ++k)
    if (magicviews[k].viewedid==currid
        && magicviews[k].magicpieceid==magicpieceid
        && magicviews[k].vec_viewed_to_magic==vec_viewed_to_magic)
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_changed_views(square sq_viewed)
{
  unsigned int result = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_viewed);
  TraceFunctionParamListEnd();

  for (i = magic_views_top[nbply-1]; i<magic_views_top[nbply]; ++i)
    if (magicviews[i].pos_viewed==sq_viewed
        && !find_view(parent_ply[nbply],i))
      ++result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void ChangeMagic(void)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; ++bnp)
    /* only change if viewee suffers odd-no. new views */
    if (count_changed_views(*bnp)%2==1)
    {
      assert(!TSTFLAG(spec[*bnp],Neutral));
      move_effect_journal_do_side_change(move_effect_reason_magic_piece,*bnp);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type magic_views_initialiser_solve(slice_index si,
                                                stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(nbply==1);
  assert(!pushing_magic_views);
  PushMagicViews();
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
stip_length_type magic_pieces_recolorer_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (!pushing_magic_views)
    PushMagicViews();
  ChangeMagic();
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_magic_pieces_recolorers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMagicPiecesRecolorer);

  /* make sure that we compare the correct magic views in threats */
  stip_instrument_dummy_moves(si,STMagicPiecesRecolorer);

  {
    slice_index const prototype = alloc_pipe(STMagicViewsInitialiser);
    branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

#ifdef DEBUG
static void WriteMagicViews(int ply)
{
  int i;
  for (i= magictop[ply-1]; i < magictop[ply]; i++)
  {
    char buf[10];
    WriteSquare(magicviews[i].pos_viewed);
    StdChar(' ');
    WriteSquare(magicviews[i].viewedid);
    StdChar(' ');
    WriteSquare(magicviews[i].magicpieceid);
    StdChar(' ');
    sprintf(buf, "%i", magicviews[i].vec_viewed_to_magic);
    StdString(buf);
    StdChar('\n');
  }
}
#endif
