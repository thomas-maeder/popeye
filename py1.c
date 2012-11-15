/********************* MODIFICATIONS to py1.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/01 SE   New Koeko conditions: GI-Koeko, AN-Koeko
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher
 **
 ** 2007/01/28 SE   New condition: Annan Chess
 **
 ** 2007/06/01 SE   New piece: Radial knight (invented: C.J.Feather)
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/12/26 SE   New condition: Protean Chess
 **
 ** 2008/01/11 SE   New variant: Special Grids
 **
 ** 2008/01/24 SE   New variant: Gridlines
 **
 ** 2008/02/24 SE   Bugfix: Koeko + Parrain
 **
 ** 2008/02/19 SE   New condition: AntiKoeko
 **
 ** 2008/02/25 SE   New piece type: Magic
 **
 ** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
 **
 ** 2009/04/25 SE   New condition: Provacateurs
 **                 New piece type: Patrol pieces
 **
 **************************** End of List ******************************/

#if defined(macintosh)          /* is always defined on macintosh's  SB */
#    define SEGM1
#    include "platform/unix/mac.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "py.h"
#include "pymsg.h"
#include "py1.h"
#include "pyproc.h"
#include "pydata.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "options/nontrivial.h"
#include "options/maxthreatlength.h"
#include "options/movenumbers.h"
#include "options/maxflightsquares.h"
#include "stipulation/stipulation.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "pieces/attributes/magic.h"
#include "platform/maxtime.h"
#include "solving/move_effect_journal.h"
#include "solving/battle_play/try.h"
#include "solving/castling.h"
#include "solving/en_passant.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "conditions/bgl.h"
#include "conditions/sat.h"
#include "conditions/oscillating_kings.h"
#include "conditions/duellists.h"
#include "conditions/kobul.h"
#include "conditions/anticirce/super.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/chameleon.h"
#include "conditions/circe/april.h"
#include "conditions/imitator.h"
#include "conditions/sentinelles.h"
#include "conditions/singlebox/type3.h"
#include "utilities/table.h"
#include "debugging/trace.h"

static numvec ortho_opt[4][2*(square_h8-square_a1)+1];

static numvec const * const check_dir_impl[4] = {
    ortho_opt[Queen-Queen]+(square_h8-square_a1),
    ortho_opt[Knight-Queen]+(square_h8-square_a1),
    ortho_opt[Rook-Queen]+(square_h8-square_a1),
    ortho_opt[Bishop-Queen]+(square_h8-square_a1)
};

numvec const * const * const CheckDir = check_dir_impl-Queen;

void InitCheckDir(void)
{
  int i, j;

  assert(Queen<Rook);
  assert(Rook-Queen<4);
  assert(Queen<Bishop);
  assert(Bishop-Queen<4);
  assert(Queen<Knight);
  assert(Knight-Queen<4);

  for (i = -(square_h8-square_a1); i<=square_h8-square_a1; i++)
  {
    ortho_opt[Queen-Queen][(square_h8-square_a1)+i] = 0;
    ortho_opt[Rook-Queen][(square_h8-square_a1)+i] = 0;
    ortho_opt[Bishop-Queen][(square_h8-square_a1)+i] = 0;
    ortho_opt[Knight-Queen][(square_h8-square_a1)+i] = 0;
  }

  for (i = vec_knight_start; i <= vec_knight_end; i++)
    ortho_opt[Knight-Queen][(square_h8-square_a1)+vec[i]] = vec[i];

  for (i = vec_rook_start; i<=vec_rook_end; i++)
    for (j = 1; j<=max_nr_straight_rider_steps; j++)
    {
      ortho_opt[Queen-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
      ortho_opt[Rook-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
    }

  for (i = vec_bishop_start; i<=vec_bishop_end; i++)
    for (j = 1; j<=max_nr_straight_rider_steps; j++)
    {
      ortho_opt[Queen-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
      ortho_opt[Bishop-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
    }
}

static void initply(ply parent, ply child)
{
  parent_ply[child] = parent;

  /* child -1 is correct and parent would be wrong! */
  move_effect_journal_top[child] = move_effect_journal_top[child-1];

  ep2[child] = initsquare;
  ep[child] = initsquare;

  pprise[child] = vide;

  prev_king_square[White][nbply] = king_square[White];
  prev_king_square[Black][nbply] = king_square[Black];

  /*
    start with the castling rights of the parent level
  */
  castling_flag[child] = castling_flag[parent];

  /*
    start with the SAT state of the parent level
  */
  StrictSAT[Black][child] = StrictSAT[Black][parent];
  StrictSAT[White][child] = StrictSAT[White][parent];
  BGL_values[White][child] = BGL_values[White][parent];
  BGL_values[Black][child] = BGL_values[Black][parent];

  magicstate[child] = magicstate[parent];

  platzwechsel_rochade_allowed[White][child] = platzwechsel_rochade_allowed[White][parent];
  platzwechsel_rochade_allowed[Black][child] = platzwechsel_rochade_allowed[Black][parent];

  ++post_move_iteration_id[child];
  TraceValue("%u",nbply);TraceValue("%u\n",post_move_iteration_id[nbply]);
}

static void do_copyply(ply original, ply copy)
{
  parent_ply[copy] = parent_ply[original];

  trait[copy] = trait[original];

  move_effect_journal_top[copy] = move_effect_journal_top[copy-1];

  ep2[copy] = ep2[parent_ply[original]];
  ep[copy] = ep[parent_ply[original]];

  pprise[copy] = vide;

  prev_king_square[White][nbply] = prev_king_square[White][parent_ply[original]];
  prev_king_square[Black][nbply] = prev_king_square[Black][parent_ply[original]];

  /*
    start with the castling rights of the parent level
  */
  castling_flag[copy] = castling_flag[parent_ply[original]];

  /*
    start with the SAT state of the original level
  */
  StrictSAT[Black][copy] = StrictSAT[Black][parent_ply[original]];
  StrictSAT[White][copy] = StrictSAT[White][parent_ply[original]];
  BGL_values[White][copy] = BGL_values[White][parent_ply[original]];
  BGL_values[Black][copy] = BGL_values[Black][parent_ply[original]];

  magicstate[copy] = magicstate[parent_ply[original]];

  platzwechsel_rochade_allowed[White][copy] = platzwechsel_rochade_allowed[White][parent_ply[original]];
  platzwechsel_rochade_allowed[Black][copy] = platzwechsel_rochade_allowed[Black][parent_ply[original]];

  {
    unsigned int const nr_moves = current_move[original]-current_move[original-1];
    memcpy(&move_generation_stack[current_move[copy]+1],
           &move_generation_stack[current_move[original-1]+1],
           nr_moves*sizeof move_generation_stack[0]);
    current_move[copy] += nr_moves;
  }

  ++post_move_iteration_id[copy];
  TraceValue("%u",nbply);TraceValue("%u\n",post_move_iteration_id[nbply]);
}

static ply ply_watermark;

void nextply(void)
{
  ply const parent = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  ++ply_watermark;
  initply(parent,nbply);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void copyply(void)
{
  ply const original = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  ++ply_watermark;
  do_copyply(original,nbply);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void finply()
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nbply==ply_watermark);
  --ply_watermark;
  nbply = parent_ply[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void InitCond(void)
{
  square const *bnp;
  square i, j;

  wh_exact = false;
  bl_exact = false;

  ultra_mummer[White] = false;
  ultra_mummer[Black] = false;

  anyclone = false;
  anycirprom = false;
  anycirce = false;
  anyimmun = false;
  anyanticirce = false;
  anyanticirprom = false;
  anymars = false;
  anyantimars = false;
  anygeneva = false;
  anyparrain= false;

  immrenroib = initsquare;
  immrenroin = initsquare;
  cirrenroib = initsquare;
  cirrenroin = initsquare;

  antirenai = rennormal;
  circerenai = rennormal;
  immunrenai = rennormal;
  marsrenai = rennormal;

  royal_square[White] = initsquare;
  royal_square[Black] = initsquare;

  CondFlag[circeassassin]= false;
  flagmummer[White] = false;
  flagmummer[Black] = false;
  flagparasent= false;
  rex_mad = false;
  rex_circe = false;
  rex_immun = false;
  rex_phan = false;
  rex_geneva =false;
  rex_mess_ex = false;
  rex_wooz_ex = false;
  rex_protean_ex = false;
  calctransmute = false;

  sentinelles_max_nr_pawns[Black] = 8;
  sentinelles_max_nr_pawns[White] = 8;
  sentinelles_max_nr_pawns_total = 16;
  sentinelle[White] = pb;
  sentinelle[Black] = pn;

  gridvar = grid_normal;
  numgridlines = 0;

  {
    PieceIdType id;
    for (id = MinPieceId; id<=MaxPieceId; ++id)
      PiecePositionsInDiagram[id] = initsquare;
  }

  for (bnp= boardnum; *bnp; bnp++) {
    int const file= *bnp%onerow - nr_of_slack_files_left_of_board;
    int const row= *bnp/onerow - nr_of_slack_rows_below_board;

    ClearPieceId(spec[*bnp]);
    CLEARFL(sq_spec[*bnp]);
    sq_num[*bnp]= (int)(bnp-boardnum);

    /* initialise sq_spec and set grid number */
    sq_spec[*bnp] += ((file/2)+4*(row/2)) << Grid;
    if (file!=0 && file!=nr_files_on_board-1
        && row!=0 && row!=nr_rows_on_board-1)
      SETFLAG(sq_spec[*bnp], NoEdgeSq);
  }

  for (i= square_a1; i < square_h8; i+= onerow)
  {
    if (i > square_a1)
      if (!TSTFLAG(sq_spec[i+dir_down], SqColor))
        SETFLAG(sq_spec[i], SqColor);
    for (j= i+1; j < i+nr_files_on_board; j++)
      if (!TSTFLAG(sq_spec[j+dir_left], SqColor))
        SETFLAG(sq_spec[j], SqColor);
  }

  for (i= 0; i < CondCount; ++i)
    CondFlag[i] = false;

  for (i= 0; i < ExtraCondCount; ++i)
    ExtraCondFlag[i] = false;

  number_of_imitators = 0;

  memset((char *) promonly, 0, sizeof(promonly));
  memset((char *) is_football_substitute, 0, sizeof(promonly));
  memset((char *) is_april_kind,0,sizeof(is_april_kind));
  checkhopim = false;
  koekofunc= nokingcontact;
  antikoekofunc= nokingcontact;
  OscillatingKingsTypeB[White]= false;
  OscillatingKingsTypeB[Black]= false;
  OscillatingKingsTypeC[White]= false;
  OscillatingKingsTypeC[Black]= false;

  measure_length[Black] = NULL;
  measure_length[White] = NULL;

  BGL_values[White][1] = BGL_infinity;
  BGL_values[Black][1] = BGL_infinity;
  BGL_global= false;

  calc_transmuting_king[White] = false;
  calc_transmuting_king[White] = false;

  calc_reflective_king[White] = false;
  calc_reflective_king[Black] = false;

  normaltranspieces[White] = true;
  normaltranspieces[Black] = true;

  obsgenre = false;

  kobulking[White] = false;
  kobulking[Black] = false;
} /* InitCond */

void InitOpt(void)
{
  {
    Side side;
    square castling;
    for (side = White; side<=Black; ++side)
      for (castling = min_castling; castling<=max_castling; ++castling)
        castling_mutual_exclusive[side][castling-min_castling] = 0;
  }

  castling_flag[castlings_flags_no_castling] = bl_castlings|wh_castlings;

  ep[nbply] = initsquare;
  ep2[nbply] = initsquare;

  resetOptionMaxtime();

  reset_max_flights();
  set_max_nr_refutations(0);
  reset_restart_number();
  reset_max_threat_length();
  reset_nontrivial_settings();

  {
    unsigned int i;
    for (i = 0; i<OptCount; i++)
      OptFlag[i] = false;
  }

  pprise[1] = vide;
  pprispec[1] = EmptySpec;
  move_generation_stack[1].capture = initsquare;
}

void InitBoard(void)
{
  square i;
  square const *bnp;

  ActTitle[0] = '\0';
  ActAuthor[0] = '\0';
  ActOrigin[0] = '\0';
  ActTwinning[0] = '\0';
  ActAward[0] = '\0';
  ActStip[0] = '\0';

  for (i= maxsquare-1; i>=0; i--)
  {
    e[i] = obs;
    e_ubi[i] = obs;
    e_ubi_mad[i] = obs;
    spec[i] = BorderSpec;
  }

  /* dummy squares for Messigny chess and castling -- must be empty */
  e[messigny_exchange] = vide;
  e[kingside_castling] = vide;
  e[queenside_castling] = vide;
  CLEARFL(spec[messigny_exchange]);
  CLEARFL(spec[kingside_castling]);
  CLEARFL(spec[queenside_castling]);

  for (bnp = boardnum; *bnp; bnp++)
    e[*bnp] = vide;

  king_square[White] = initsquare;
  king_square[Black] = initsquare;

  CLEARFL(PieSpExFlags);
  SETFLAG(PieSpExFlags,White);
  SETFLAG(PieSpExFlags,Black);
  SETFLAG(PieSpExFlags,Royal);

  nrhuntertypes = 0;
} /* InitBoard */

void InitAlways(void) {
  square i;

  memset((char *) exist, 0, sizeof(exist));

  move_generation_mode= move_generation_optimized_by_killer_move;
  TraceValue("->%u\n",move_generation_mode);

  nbply = nil_ply;
  current_move[nbply] = nil_coup;
  ply_watermark = nil_ply;

  CondFlag[circeassassin] = false;
  k_cap = false;
  flagfee = false;

  for (i= maxply; i > 0; i--)
  {
    duellists[White][i] = initsquare;
    duellists[Black][i] = initsquare;
    kpilcd[i] = initsquare;
    kpilca[i] = initsquare;
    current_circe_rebirth_square[i] = initsquare;
    trait[i] = White;
    current_anticirce_rebirth_square[i] = initsquare;
    pwcprom[i] = false;
  }

  initialise_neutrals(White);
  reset_tables();
  dont_generate_castling = false;

  takemake_takedeparturesquare= initsquare;
  takemake_takecapturesquare= initsquare;

  reset_max_nr_solutions_per_target_position();
}

square coinequis(square i)
{
  return 75 + (onerow*(((i/onerow)+3)/2) + (((i%onerow)+3)/2));
}

boolean leapcheck(square     sq_king,
                  numvec     kanf,
                  numvec     kend,
                  piece  p,
                  evalfunction_t *evaluate)
{
  /* detect "check" of leaper p */
  numvec  k;

  square sq_departure;

  for (k= kanf; k<=kend; k++) {
    sq_departure= sq_king+vec[k];
    if (e[sq_departure]==p
        && evaluate(sq_departure,sq_king,sq_king)
        && imcheck(sq_departure,sq_king))
      return true;
  }

  return false;
}

boolean leapleapcheck(square     sq_king,
                      numvec     kanf,
                      numvec     kend,
                      int hurdletype,
                      boolean leaf,
                      piece  p,
                      evalfunction_t *evaluate)
{
  /* detect "check" of leaper p */
  numvec  k, k1;
  square  sq_departure, sq_hurdle;

  for (k= kanf; k<= kend; k++) {
    sq_hurdle= sq_king + vec[k];
    if (hurdletype==0 && abs(e[sq_hurdle])>obs && e[sq_hurdle]*p<0 ||
       hurdletype ==1 && abs(e[sq_hurdle])>obs)
    {
      for (k1= kanf; k1<= kend; k1++) {
        sq_departure = sq_hurdle + vec[k1];
        if (e[sq_departure]==p && sq_departure!=sq_king
            && (*evaluate)(sq_departure,sq_king,sq_king)
            && imcheck(sq_departure,sq_king))
        {
          return true;
        }
      }
    }
  }

  return false;
}

boolean riderhoppercheck(square  sq_king,
                         numvec  kanf,
                         numvec  kend,
                         piece   p,
                         int     run_up,
                         int     jump,
                         evalfunction_t *evaluate)
{
  /* detect "check" of a generalised rider-hopper p that runs up
     run_up squares and jumps jump squares. 0 indicates an
     potentially infinite run_up or jump.
     examples:  grasshopper:         run_up: 0   jump: 1
     grasshopper2:      run_up: 0    jump: 2
     contragrasshopper: run_up: 1    jump: 0
     lion:           run_up: 0   jump: 0
  ********/

  piece   hurdle, hopper;
  square  sq_hurdle;
  numvec  k;

  square sq_departure;

  for (k= kanf; k <= kend; k++) {
    if (jump) {
      sq_hurdle= sq_king;
      if (jump>1) {
        int jumped= jump;
        while (--jumped) {
          sq_hurdle+= vec[k];
          if (e[sq_hurdle]!=vide)
            break;
        }

        if (jumped)
          continue;
      }
      sq_hurdle+= vec[k];
      hurdle= e[sq_hurdle];
    }
    else
      /* e.g. lion, contragrashopper */
      finligne(sq_king,vec[k],hurdle,sq_hurdle);

    if (abs(hurdle)>=roib) {
      if (run_up) {
        /* contragrashopper */
        sq_departure= sq_hurdle;
        if (run_up>1) {
          int ran_up= run_up;
          while (--ran_up) {
            sq_hurdle+= vec[k];
            if (e[sq_hurdle]!=vide)
              break;
          }
          if (ran_up)
            continue;
        }
        sq_departure+= vec[k];
        hopper= e[sq_departure];
      }
      else
        /* grashopper, lion */
        finligne(sq_hurdle,vec[k],hopper,sq_departure);

      if (hopper==p
          && evaluate(sq_departure,sq_king,sq_king)
          && (!checkhopim || hopimok(sq_departure,sq_king,sq_hurdle,-vec[k],-vec[k])))
        return true;
    }
  }
  return false;
} /* end of riderhoppercheck */

boolean ridcheck(square sq_king,
                 numvec kanf,
                 numvec kend,
                 piece  p,
                 evalfunction_t *evaluate)
{
  /* detect "check" of rider p */
  piece rider;
  numvec  k;
  square sq_departure;

  for (k= kanf; k<= kend; k++)
  {
    finligne(sq_king,vec[k],rider,sq_departure);
    if (rider==p
        && evaluate(sq_departure,sq_king,sq_king)
        && ridimcheck(sq_departure,sq_king,vec[k]))
      return true;
  }
  return false;
}

boolean marincheck(square   sq_king,
                   numvec   kanf,
                   numvec   kend,
                   piece    p,
                   evalfunction_t *evaluate)
{
  /* detect "check" of marin piece p or a locust */
  piece marine;
  numvec  k;

  square sq_departure;
  square sq_arrival;

  for (k= kanf; k<= kend; k++) {
    sq_arrival= sq_king-vec[k];
    if (e[sq_arrival]==vide) {
      finligne(sq_king,vec[k],marine,sq_departure);
      if (marine==p && evaluate(sq_departure,sq_arrival,sq_king))
        return true;
    }
  }
  return false;
}

boolean nogridcontact(square j)
{
  square  j1;
  numvec  k;
  piece   p;

  for (k= 8; k > 0; k--) {
    p= e[j1= j + vec[k]];
    if (p != vide && p != obs && GridLegal(j1, j)) {
      return false;
    }
  }
  return true;
}

static boolean noleapcontact(square sq_arrival, numvec kanf, numvec kend)
{
  boolean result = true;

  numvec k;
  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();
  for (k= kanf; k <= kend; k++)
  {
    piece const p = e[sq_arrival+vec[k]];
    /* this is faster than a call to abs() */
    if (p!=obs && p!=vide)
    {
      TraceSquare(sq_arrival+vec[k]);
      TracePiece(e[sq_arrival+vec[k]]);
      TraceText("\n");
      result = false;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean nokingcontact(square ia)
{
  return noleapcontact(ia, vec_queen_start, vec_queen_end);
}

boolean nowazircontact(square ia)
{
  return noleapcontact(ia, vec_rook_start, vec_rook_end);
}

boolean noferscontact(square ia)
{
  return noleapcontact(ia, vec_bishop_start, vec_bishop_end);
}

boolean noknightcontact(square ia)
{
  return noleapcontact(ia, vec_knight_start, vec_knight_end);
}

boolean nocamelcontact(square ia)
{
  return noleapcontact(ia, vec_chameau_start, vec_chameau_end);
}

boolean noalfilcontact(square ia)
{
  return noleapcontact(ia, vec_alfil_start, vec_alfil_end);
}

boolean nodabbabacontact(square ia)
{
  return noleapcontact(ia, vec_dabbaba_start, vec_dabbaba_end);
}

boolean nozebracontact(square ia)
{
  return noleapcontact(ia, vec_zebre_start, vec_zebre_end);
}

boolean nogiraffecontact(square ia)
{
  return noleapcontact(ia, vec_girafe_start, vec_girafe_end);
}

boolean noantelopecontact(square ia)
{
  return noleapcontact(ia, vec_antilope_start, vec_antilope_end);
}


boolean nocontact(square sq_departure, square sq_arrival, square sq_capture, nocontactfunc_t nocontactfunc) {
  boolean   Result;
  square    cr;
  piece pj, pp, pren;
  piece pc= obs;
  square sq_castle_from=initsquare, sq_castle_to=initsquare;

  VARIABLE_INIT(cr);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  nextply();

  pj= e[sq_departure];
  pp= e[sq_capture];
  /* does this work with neutral pieces ??? */
  if (CondFlag[haanerchess]) {
    e[sq_departure]= obs;
  }
  else if (CondFlag[sentinelles]
           && sq_departure>=square_a2 && sq_departure<=square_h7
           && !is_pawn(abs(pj)))
  {
    if ((pj<=roin) != SentPionAdverse) {
      if (nbpiece[sentinelle[Black]] < sentinelles_max_nr_pawns[Black]
          && nbpiece[sentinelle[White]]+nbpiece[sentinelle[Black]] < sentinelles_max_nr_pawns_total
          && (!flagparasent
              || (nbpiece[sentinelle[Black]]
                  <= nbpiece[sentinelle[White]]+(pp==sentinelle[White]?1:0))))
      {
        e[sq_departure]= sentinelle[Black];
      }
      else {
        e[sq_departure]= vide;
      }
    }
    else { /* we assume  pj >= roib */
      if (nbpiece[sentinelle[White]] < sentinelles_max_nr_pawns[White]
          && nbpiece[sentinelle[White]]+nbpiece[sentinelle[Black]] < sentinelles_max_nr_pawns_total
          && (!flagparasent
              || (nbpiece[sentinelle[White]]
                  <= nbpiece[sentinelle[Black]]+(pp==sentinelle[Black]?1:0))))
      {
        e[sq_departure]= sentinelle[White];
      }
      else {
        e[sq_departure]= vide;
      }
      /* don't think any change as a result of Sentinelles */
      /* PionNeutral is needed as piece specs not changed  */
    }
  }
  else {
    e[sq_departure]= vide;
    /* e[sq_departure] = CondFlag[haanerchess] ? obs : vide;       */
  }

  if (sq_capture == messigny_exchange) {
    e[sq_departure]= e[sq_arrival];
  }
  else {
    /* the pieces captured and reborn may be different: */
    /* Clone, Chameleon Circe               */
    pp= e[sq_capture];

    /* the pieces can be reborn at the square where it has been
     * captured. For example, when it is taken by a locust or a
     * similarly moving piece
     */
    e[sq_capture]= vide;

    TraceValue("%u",nbply);
    TracePiece(pprise[parent_ply[nbply]]);
    TracePiece(pprise[parent_ply[parent_ply[nbply]]]);
    TraceText("\n");
    if (anyparrain && pprise[parent_ply[parent_ply[nbply]]] != vide)
    {
      if (CondFlag[parrain]) {
        cr = (move_generation_stack[current_move[parent_ply[nbply]-1]].capture
            + sq_arrival - sq_departure);
      }
      if (CondFlag[contraparrain]) {
        cr = (move_generation_stack[current_move[parent_ply[nbply]-1]].capture
            - sq_arrival + sq_departure);
      }
      pc = e[cr];
      if (pc==vide)
      {
        e[cr]= pprise[parent_ply[parent_ply[nbply]]];
        TraceSquare(cr);
        TraceText("\n");
      }
    }

    if (pp != vide && pp != obs) {
      if (anycirce && abs(pp) > roib && !anyparrain) {
        /* This still doesn't work with neutral pieces.
        ** Eventually we must add the colour of the side making
        ** the move or potentially giving the check to the
        ** argument list!
        */
        if (anyclone && sq_departure != king_square[Black] && sq_departure != king_square[White]) {
          /* Circe Clone */
          pren = (pj * pp < 0) ? -pj : pj;
        }
        else {
          /* Chameleon Circe or ordinary Circe type */
          pren= CondFlag[chamcirce]
            ? chameleon_circe_get_reborn_piece(pp)
            : pp;
        }

        if (CondFlag[couscous]) {
          cr= (*circerenai)(pj, spec[sq_departure], sq_capture, sq_departure, sq_arrival, pp > vide ? White : Black);
        }
        else {
          cr= (*circerenai)(pren, spec[sq_capture], sq_capture, sq_departure, sq_arrival, pp > vide ? Black : White);
        }

        if ((pc= e[cr]) == vide) {
          e[cr]= pren;
        }
      } /* anycirce && abs(pp) > roib */
    } /* pp != vide && pp != obs */
    else { /* no capture move */
      if (abs(pj) == King)
      {
        if (castling_supported) {
              if (sq_capture == kingside_castling) {
            sq_castle_from = sq_arrival+dir_right;
            sq_castle_to = sq_arrival+dir_left;
              }
              else if (sq_capture == queenside_castling) {
            sq_castle_from = sq_arrival+2*dir_left;
            sq_castle_to = sq_arrival+dir_right;
              }
        }
        else if (CondFlag[castlingchess] && sq_capture > platzwechsel_rochade)
        {
          sq_castle_to = (sq_arrival + sq_departure) / 2;
          sq_castle_from = sq_capture - maxsquare;
        }
        else if (CondFlag[platzwechselrochade] && sq_capture == platzwechsel_rochade)
        {
          sq_castle_to = sq_arrival;
          sq_castle_from = sq_departure;
        }
        if (sq_castle_from != initsquare)
        {
          e[sq_castle_to]= e[sq_castle_from];
          e[sq_castle_from]= vide;
        }
      }
    }
  }

  if (CondFlag[contactgrid]) {
    Result= nogridcontact(sq_arrival);
  }
  else {
    Result= (*nocontactfunc)(sq_arrival);
  }

  if (pc != obs) {
    e[cr]= pc;
  }

  e[sq_capture]= pp;
  e[sq_departure]= pj;
  if (sq_castle_from != initsquare) {
      e[sq_castle_from]= e[sq_castle_to];
    e[sq_castle_to] = vide;
  }
  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",Result);
  TraceFunctionResultEnd();
  return Result;
} /* nocontact */

boolean ooorphancheck(square sq_king,
                      piece porph,
                      piece p,
                      evalfunction_t *evaluate) {
  boolean   flag= false;
  square    olist[63];
  square const *bnp;
  unsigned int j, k, nrp, co;

  if ((*checkfunctions[abs(porph)])(sq_king,porph,evaluate))
    return true;

  nrp= nbpiece[p];
  if (nrp == 0)
    return false;

  nbpiece[-p]--;
  e[sq_king]= dummyb;
  co= 0;
  for (bnp= boardnum; co < nrp; bnp++) {
    if (e[*bnp] == p) {
      olist[co++]= *bnp;
    }
  }
  for (k= 0; k < co; k++) {
    j= 0;
    while (j<co) {
      e[olist[j]]= k==j ? p : dummyb;
      j++;
    }
    if ((*checkfunctions[abs(porph)])(sq_king,p,evaluate)) {
      for (j= 0; j<co; j++)
        e[olist[j]]= p;
      flag= ooorphancheck(olist[k],-porph,-p,evaluate);
      if (flag)
        break;
    }
    else
      for (j= 0; j<co; j++)
        e[olist[j]]= p;
  }

  nbpiece[-p]++;
  e[sq_king]= -p;
  return flag;
}

boolean orphancheck(square   sq_king,
                    piece    p,
                    evalfunction_t *evaluate)
{
  PieNam const *porph;
  boolean   flag= false;
  boolean   inited= false;
  square    olist[63];
  square const *bnp;
  int   k, j, co= 0;

  for (porph = orphanpieces; *porph!=Empty; porph++)
  {
    if (nbpiece[(piece)*porph]>0 || nbpiece[-(piece)*porph]>0) {
      if (!inited)
      {
        inited = true;
        for (bnp= boardnum; *bnp; bnp++)
          if (e[*bnp] == p)
            olist[co++]= *bnp;
      }

      for (k= 0; k < co; k++)
      {
        j= 0;
        while (j < co) {
          e[olist[j]]= (k == j) ? p : dummyb;
          j++;
        }
        if ((*checkfunctions[*porph])(sq_king, p, evaluate)) {
          piece op;
          for (j= 0; j < co; e[olist[j++]]= p)
            ;
          if (p == orphanb)
            op = -*porph;
          else
            op = *porph;
          flag= ooorphancheck(olist[k], op, -p, evaluate);
          if (flag)
            break;
        }
        else {
          for (j= 0; j < co; e[olist[j++]]= p)
            ;
        }
      }
      if (flag)
        return true;
    }
  }
  return false;
}

boolean fffriendcheck(square    sq_king,
                      piece pfr,
                      piece p,
                      evalfunction_t *evaluate)
{
  boolean   flag= false;
  square    flist[63];
  square const *bnp;
  unsigned int j, k, nrp, cf= 0;

  if ((*checkfunctions[abs(pfr)])(sq_king, pfr, evaluate))
    return true;

  nrp= nbpiece[p]-1;
  if (nrp == 0)
    return false;

  nbpiece[p]--;
  e[sq_king]= dummyb;
  for (bnp= boardnum; cf < nrp; bnp++) {
    if (e[*bnp] == p) {
      flist[cf++]= *bnp;
    }
  }

  for (k= 0; k < cf; k++) {
    j= 0;
    while (j < cf) {
      e[flist[j]]= (k == j) ? p : dummyb;
      j++;
    }
    if ((*checkfunctions[abs(pfr)])(sq_king, p, evaluate)) {
      for (j= 0; j < cf; e[flist[j++]]= p)
        ;
      flag= fffriendcheck(flist[k], pfr, p, evaluate);
      if (flag) {
        break;
      }
    }
    else {
      for (j= 0; j < cf; e[flist[j++]]= p)
        ;
    }
  }

  nbpiece[p]++;
  e[sq_king]= p;
  return flag;
} /* fffriendcheck */

boolean friendcheck(square    i,
                    piece p,
                    evalfunction_t *evaluate)
{
  PieNam const *pfr;
  piece cfr;
  boolean   flag= false;
  boolean   initialized= false;
  square    flist[63];
  square const *bnp;
  int   k, j, cf= 0;

  for (pfr= orphanpieces; *pfr!=Empty; pfr++)
  {
    cfr = p== friendb ? (piece)*pfr : -(piece)*pfr;
    if (nbpiece[cfr]>0)
    {
      if (!initialized)
      {
        initialized= true;
        for (bnp= boardnum; *bnp; bnp++)
          if (e[*bnp] == p)
            flist[cf++]= *bnp;
      }

      for (k= 0; k < cf; k++) {
        j= 0;
        while (j < cf) {
          e[flist[j]]= (k == j) ? p : dummyb;
          j++;
        }
        if ((*checkfunctions[*pfr])(i, p, evaluate)) {
          for (j= 0; j < cf; e[flist[j++]]= p)
            ;
          flag= fffriendcheck(flist[k], cfr, p, evaluate);
          if (flag) {
            break;
          }
        }
        else {
          for (j= 0; j < cf; e[flist[j++]]= p)
            ;
        }
      }
      if (flag) {
        return true;
      }
    }
  }
  return false;
} /* friendcheck */

boolean whannan(square rear, square front)
{
  if (e[rear] <= obs)
    return false;
  switch(annanvar)
  {
  case 0:
    return true;
  case 1:
    return rear != king_square[White];
  case 2:
    return front != king_square[White];
  case 3:
    return rear != king_square[White] && front != king_square[White];
  }
  return true;
}

boolean blannan(square rear, square front)
{
  if (e[rear] >= vide)
    return false;
  switch(annanvar)
  {
  case 0:
    return true;
  case 1:
    return rear != king_square[Black];
  case 2:
    return front != king_square[Black];
  case 3:
    return rear != king_square[Black] && front != king_square[Black];
  }
  return true;
}

boolean CrossesGridLines(square dep, square arr)
{
  int i, x1, y1, x2, y2, X1, Y1, X2, Y2, dx, dy, dX, dY, u1, u2, v;

  X1= ((dep<<1) -15) % 24;
  Y1= ((dep/24)<<1) - 15;
  X2= ((arr<<1) -15) % 24;
  Y2= ((arr/24)<<1) - 15;
  dX= X2-X1;
  dY= Y2-Y1;
  for (i= 0; i < numgridlines; i++)
  {
    x1= gridlines[i][0];
    y1= gridlines[i][1];
    x2= gridlines[i][2];
    y2= gridlines[i][3];
    dx= x2-x1;
    dy= y2-y1;
    v=dY*dx-dX*dy;
    if (!v)
      continue;
    u1= dX*(y1-Y1)-dY*(x1-X1);
    if (v<0? (u1>0 || u1<v) : (u1<0 || u1>v))
      continue;
    u2= dx*(y1-Y1)-dy*(x1-X1);
    if (v<0? (u2>0 || u2<v) : (u2<0 || u2>v))
      continue;
    return true;
  }
  return false;
}

PieNam* GetPromotingPieces (square sq_departure,
                           piece pi_departing,
                           Side camp,
                           Flags spec_pi_moving,
                           square sq_arrival,
                           piece pi_captured)
{
    if (is_pawn(abs(pi_departing))
        && PromSq(is_reversepawn(abs(pi_departing))^camp,sq_arrival) &&
	    ((!CondFlag[protean] && !TSTFLAG(spec_pi_moving, Protean)) || pi_captured == vide)) {
    	return getprompiece;
    }

    return NULL;
}
