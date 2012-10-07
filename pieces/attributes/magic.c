#include "pieces/attributes/magic.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static unsigned int nr_magic_views;

/* magic pieces */
enum
{
  magicviews_size = 10000
};

typedef struct
{
  square piecesquare;
  int pieceid;
  int magicpieceid;
  numvec vecnum;
} magicview_type;

static magicview_type magicviews[magicviews_size];

magicstate_type magicstate[maxply + 1];

static void PushMagicView(square sq, PieceIdType id1, PieceIdType id2, numvec v)
{
  assert(nr_magic_views<magicviews_size);

  magicviews[nr_magic_views].piecesquare = sq;
  magicviews[nr_magic_views].pieceid = id1;
  magicviews[nr_magic_views].magicpieceid = id2;
  magicviews[nr_magic_views].vecnum = v;
  ++nr_magic_views;
}

void GetRoseAttackVectors(square from, square to)
{
  numvec  k;
  for (k= vec_knight_start; k<=vec_knight_end; k++)
  {
    if (detect_rosecheck_on_line(to,e[from],
                                 k,0,+1,
                                 eval_fromspecificsquare))
      PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), 200+vec[k] );
    if (detect_rosecheck_on_line(to,e[from],
                                 k,vec_knight_end-vec_knight_start+1,-1,
                                 eval_fromspecificsquare))
      PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), 300+vec[k]);
  }
}

void GetRoseLionAttackVectors(square from, square to)
{
  numvec  k;
  for (k= vec_knight_start; k <= vec_knight_end; k++)
  {
    if (detect_roselioncheck_on_line(to,e[from],
                                     k,0,+1,
                                     eval_fromspecificsquare))
      PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), 200+vec[k] );
    if (detect_roselioncheck_on_line(to,e[from],
                                        k,vec_knight_end-vec_knight_start+1,-1,
                                        eval_fromspecificsquare))
      PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), 300+vec[k]);
  }
}

void GetRoseHopperAttackVectors(square from, square to)
{
  numvec  k;
  square sq_hurdle;

  for (k= vec_knight_start; k <= vec_knight_end; k++) {
    sq_hurdle= to+vec[k];
    if (e[sq_hurdle]!=vide && e[sq_hurdle]!=obs)
    {
        /* k1==0 (and the equivalent
         * vec_knight_end-vec_knight_start+1) were already used for
         * sq_hurdle! */
      if (detect_rosehoppercheck_on_line(to,sq_hurdle,e[from],
                                         k,1,+1,
                                         eval_fromspecificsquare))
        PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), 200+vec[k] );
      if (detect_rosehoppercheck_on_line(to,sq_hurdle,e[from],
                                         k,vec_knight_end-vec_knight_start,-1,
                                         eval_fromspecificsquare))
        PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), 300+vec[k]);
    }
  }
}

void GetRoseLocustAttackVectors(square from, square to)
{
  /* detects check by a rose locust */
  numvec  k;
  square sq_arrival;

  for (k= vec_knight_start; k <= vec_knight_end; k++)
  {
    sq_arrival= to-vec[k];
    if (e[sq_arrival]==vide)
    {
        /* k1==0 (and the equivalent
         * vec_knight_end-vec_knight_start+1) were already used for
         * sq_hurdle! */
      if (detect_roselocustcheck_on_line(to,sq_arrival,e[from],
                                         k,1,+1,
                                         eval_fromspecificsquare))
        PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), 200+vec[k] );
      if (detect_roselocustcheck_on_line(to,sq_arrival,e[from],
                                         k,vec_knight_end-vec_knight_start,-1,
                                         eval_fromspecificsquare))
        PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), 300+vec[k]);
    }
  }
}

static void GetRMHopAttackVectors(square from, square to, numvec kend, numvec kanf, angle_t angle)
{
  square sq_hurdle;
  numvec k, k1;
  piece hopper;

  square sq_departure;

  for (k= kend; k>=kanf; k--)
  {
    sq_hurdle= to+vec[k];
    if (abs(e[sq_hurdle])>=roib)
    {
      k1= 2*k;
      finligne(sq_hurdle,mixhopdata[angle][k1],hopper,sq_departure);
      if (hopper==e[from]) {
        if (eval_fromspecificsquare(sq_departure,to,to))
          PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), vec[k] );
      }
      finligne(sq_hurdle,mixhopdata[angle][k1-1],hopper,sq_departure);
      if (hopper==e[from]) {
        if (eval_fromspecificsquare(sq_departure,to,to))
          PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), vec[k] );
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
  if (scheck(to, e[from], eval_fromspecificsquare)) {
    numvec attackVec;
    if (to < from)
      attackVec = move_vec_code[from - to];
    else
      attackVec = -move_vec_code[to - from];
    if (attackVec)
      PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), attackVec);
  }
}

static void GetZigZagAttackVectors(square from, square to,
                                   numvec  k,
                                   numvec  k1)
{
  square sq_departure= to+k;
  square sq_arrival= to;
  square sq_capture= to;

  while (e[sq_departure] == vide)
  {
    sq_departure+= k1;
    if (e[sq_departure] != vide)
      break;
    else
      sq_departure+= k;
  }

  if (e[sq_departure]==e[from]
      && eval_fromspecificsquare(sq_departure,sq_arrival,sq_capture))
    PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), vec[500+k] );

  sq_departure = to+k;
  while (e[sq_departure]==vide)
  {
    sq_departure-= k1;
    if (e[sq_departure]!=vide)
      break;
    else
      sq_departure+= k;
  }

  if (e[sq_departure]==e[from]
      && eval_fromspecificsquare(sq_departure,sq_arrival,sq_capture))
    PushMagicView(to, GetPieceId(spec[to]), GetPieceId(spec[from]), vec[400+k] );
}

void GetBoyscoutAttackVectors(square from, square to) {
  numvec  k;

  for (k= vec_bishop_start; k <= vec_bishop_end; k++)
    GetZigZagAttackVectors(from, to, vec[k], vec[13 - k]);
}

void GetGirlscoutAttackVectors(square from, square to) {
  numvec  k;

  for (k= vec_rook_start; k <= vec_rook_end; k++)
    GetZigZagAttackVectors(from, to, vec[k], vec[5 - k]);
}

void GetSpiralSpringerAttackVectors(square from, square to) {
  numvec  k;

  for (k= vec_knight_start; k <= vec_knight_end; k++)
    GetZigZagAttackVectors(from, to, vec[k], vec[25 - k]);
}

void GetDiagonalSpiralSpringerAttackVectors(square from, square to) {
  numvec  k;

  for (k= vec_knight_start; k <= 14; k++)
    GetZigZagAttackVectors(from, to, vec[k], vec[23 - k]);
  for (k= 15; k <= vec_knight_end; k++)
    GetZigZagAttackVectors(from, to, vec[k], vec[27 - k]);
}

/* should never get called if validation works
(disallow magic + piecetype) */
void unsupported_uncalled_attackfunction(square from, square to) {}

/* magic pieces -
for most types a magic piece of that type can only
solve another unit from one direction in any given position. Therefore
all that is needed is to see if it checks, and use the relative diff to
calculate the vector. These types have NULL entries in the table below.

More complicated types can solve from more than one direction and need
special functions listed below to calculate each potential direction.

Unsupported types are listed below with the entry
unsupported_uncalled_attackfunction
*/
static attackfunction_t *attackfunctions[derbla + 1] = {
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
/* 12 */        GetRoseAttackVectors,
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
/* 37 */        GetSpiralSpringerAttackVectors,
/* 38 */        unsupported_uncalled_attackfunction, /* ubiubi */
/* 39 */        0,
/* 40 */        GetMooseAttackVectors,
/* 41 */        GetEagleAttackVectors,
/* 42 */        GetSparrowAttackVectors,
/* 43 */        unsupported_uncalled_attackfunction,  /* archbishop */
/* 44 */        unsupported_uncalled_attackfunction, /* ref B */
/* 45 */        unsupported_uncalled_attackfunction, /* cardinal */
/* 46 */        0,
/* 47 */        0,
/* 48 */        0,
/* 49 */        0,
/* 50 */        0,
/* 51 */        0,
/* 52 */        0,
/* 53 */        0,
/* 54 */        GetDiagonalSpiralSpringerAttackVectors,
/* 55 */        unsupported_uncalled_attackfunction, /* bouncy knight */
/* 56 */        0,
/* 57 */        unsupported_uncalled_attackfunction, /* cat */
/* 58 */        0,
/* 59 */        0,
/* 60 */        0,
/* 61 */        0,
/* 62 */        0,
/* 63 */        0,
/* 64 */        0,
/* 65 */        unsupported_uncalled_attackfunction,  /* orphan */
/* 66 */        0,
/* 67 */        0,
/* 68 */        0,
/* 69 */        0,
/* 70 */        0,
/* 71 */        0,
/* 72 */        0,
/* 73 */        0,
/* 74 */        0,
/* 75 */        GetBoyscoutAttackVectors, /* boyscout */
/* 76 */        GetGirlscoutAttackVectors, /* girlscout */
/* 77 */        0, /* skylla - depends on vacant sq?? */
/* 78 */        0, /* charybdis - depends on vacant sq?? */
/* 79 */        0,
/* 80 */        GetRoseLionAttackVectors,
/* 81 */        GetRoseHopperAttackVectors,
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
/*103 */        GetRookMooseAttackVectors,
/*104 */        GetRookEagleAttackVectors,
/*105 */        GetRookSparrowAttackVectors,
/*106 */        GetBishopMooseAttackVectors,
/*107 */        GetBishopEagleAttackVectors,
/*108 */        GetBishopSparrowAttackVectors,
/*109 */        GetRoseLionAttackVectors,   /* rao checks like roselion */
/*110 */        0,
/*111 */        GetMargueriteAttackVectors, /* = G+M+EA+SW; magic - believe ok to treat as combination of these */
/*112 */        0,
/*113 */        0,
/*114 */        0,
/*115 */        0,
/*116 */        unsupported_uncalled_attackfunction,    /*friend*/
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
/*127 */  unsupported_uncalled_attackfunction, /*radial k*/
/*128 */  0,
/*129 */  GetRoseLocustAttackVectors,
/*130 */  unsupported_uncalled_attackfunction,
/*131 */  unsupported_uncalled_attackfunction,
/*132 */  unsupported_uncalled_attackfunction,
/*133 */  unsupported_uncalled_attackfunction,
/*134 */  unsupported_uncalled_attackfunction,
/*135 */  unsupported_uncalled_attackfunction,
/*136 */  unsupported_uncalled_attackfunction,
/*137 */  unsupported_uncalled_attackfunction,
/*138 */  unsupported_uncalled_attackfunction,
/*139 */  unsupported_uncalled_attackfunction
};

/* Can a specific type of (fairy) piece be magic?
 * @param p type of piece
 * @return true iff pieces of type p can be magic
 */
boolean magic_is_piece_supported(piece p)
{
  return attackfunctions[p]!=unsupported_uncalled_attackfunction;
}

static void PushMagicViews(void)
{
  square const *bnp;

  /*new stack */
  nr_magic_views = magicstate[parent_ply[nbply]].top;
  magicstate[nbply].bottom = nr_magic_views;

  for (bnp= boardnum; *bnp; bnp++)
    if (TSTFLAG(spec[*bnp], Magic))
    {
      /* for each magic piece */
      piece const p = e[*bnp];
      square * const royal = p<=roin ? &king_square[White] : &king_square[Black];
      square const royal_save = *royal;
      square const *bnp1;
      fromspecificsquare= *bnp;
      for (bnp1 = boardnum; *bnp1; bnp1++)
      {
        if (abs(e[*bnp1])>obs
            && !TSTFLAG(spec[*bnp1],Magic)
            && !TSTFLAG(spec[*bnp1],Royal))
        {
          /* for each non-magic piece
             (n.b. check *bnp != *bnp1 redundant above) */
          *royal = *bnp1;

          if (!attackfunctions[abs(p)])
          {
            /* if single solve at most */
            if ((*checkfunctions[abs(p)])(*royal,
                                          p,
                                          eval_fromspecificsquare))
            {
              numvec attackVec;
              if (*royal<*bnp)
                attackVec = move_vec_code[*bnp-*royal];
              else
                attackVec = -move_vec_code[*royal-*bnp];
              if (attackVec!=0)
                PushMagicView(*royal,
                              GetPieceId(spec[*royal]),
                              GetPieceId(spec[fromspecificsquare]),
                              attackVec);
            }
          }
          else
            /* call special function to determine all attacks */
            (*attackfunctions[abs(p)])(fromspecificsquare,*royal);
        }
      }

      *royal= royal_save;
    }

  magicstate[nbply].top = nr_magic_views;
}

static boolean find_view(ply ply_id, int j)
{
  int const currid = magicviews[j].pieceid;
  int const currmagid = magicviews[j].magicpieceid;
  numvec const currvec = magicviews[j].vecnum;
  boolean result = false;
  int k;

  for (k = magicstate[ply_id].bottom; k<magicstate[ply_id].top; ++k)
    if (magicviews[k].pieceid==currid
        && magicviews[k].magicpieceid==currmagid
        && magicviews[k].vecnum==currvec)
    {
      result = true;
      break;
    }

  return result;
}

static unsigned int count_changed_views(square sq_viewed)
{
  unsigned int result = 0;
  int i;
  for (i = magicstate[nbply].bottom; i<magicstate[nbply].top; i++)
    if (magicviews[i].piecesquare==sq_viewed && !find_view(parent_ply[nbply],i))
      ++result;
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
      move_effect_journal_do_side_change(move_effect_reason_magic_piece,
                                         *bnp,
                                         e[*bnp]<vide ? White : Black);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
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
  nr_magic_views = 0;
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
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type magic_pieces_recolorer_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

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
  for (i= magictop[parent_ply[ply]]; i < magictop[ply]; i++)
  {
    char buf[10];
    WriteSquare(magicviews[i].piecesquare);
    StdChar(' ');
    WriteSquare(magicviews[i].pieceid);
    StdChar(' ');
    WriteSquare(magicviews[i].magicpieceid);
    StdChar(' ');
    sprintf(buf, "%i", magicviews[i].vecnum);
    StdString(buf);
    StdChar('\n');
  }
}
#endif
