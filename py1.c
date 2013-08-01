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
#include <string.h>
#include <assert.h>

#include "py.h"
#include "pymsg.h"
#include "py1.h"
#include "pyproc.h"
#include "pydata.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "optimisations/killer_move/killer_move.h"
#include "options/nontrivial.h"
#include "options/maxthreatlength.h"
#include "options/movenumbers.h"
#include "options/maxflightsquares.h"
#include "stipulation/stipulation.h"
#include "pieces/walks/hunters.h"
#include "pieces/walks/friend.h"
#include "position/pieceid.h"
#include "platform/maxtime.h"
#include "solving/move_effect_journal.h"
#include "solving/battle_play/try.h"
#include "solving/castling.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/king_capture_avoider.h"
#include "solving/observation.h"
#include "conditions/bgl.h"
#include "conditions/oscillating_kings.h"
#include "conditions/kobul.h"
#include "conditions/anticirce/super.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/chameleon.h"
#include "conditions/circe/april.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/imitator.h"
#include "conditions/sentinelles.h"
#include "conditions/mummer.h"
#include "conditions/singlebox/type3.h"
#include "conditions/immune.h"
#include "conditions/geneva.h"
#include "conditions/koeko/koeko.h"
#include "conditions/koeko/anti.h"
#include "conditions/phantom.h"
#include "conditions/annan.h"
#include "conditions/vaulting_kings.h"
#include "conditions/messigny.h"
#include "conditions/woozles.h"
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
  vec_index_type i;
  unsigned int j;

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

static ply ply_watermark;
static ply ply_stack[maxply+1];
static ply ply_stack_pointer;

void nextply(Side side)
{
  ply const parent = nbply;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  assert(ply_watermark<maxply);

  ply_stack[ply_stack_pointer++] = nbply;
  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  ++ply_watermark;

  TraceValue("%u",parent);
  TraceValue("%u\n",nbply);

  parent_ply[nbply] = parent;

  trait[nbply] = side;

  move_effect_journal_top[nbply] = move_effect_journal_top[nbply-1];

  ++post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void siblingply(Side side)
{
  ply const elder = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(ply_watermark<maxply);

  ply_stack[ply_stack_pointer++] = nbply;
  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  ++ply_watermark;

  TraceValue("%u",elder);
  TraceValue("%u\n",nbply);

  parent_ply[nbply] = parent_ply[elder];

  trait[nbply] = side;

  move_effect_journal_top[nbply] = move_effect_journal_top[nbply-1];

  ++post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void copyply(void)
{
  ply const original = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ply_stack[ply_stack_pointer++] = nbply;
  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  ++ply_watermark;

  parent_ply[nbply] = parent_ply[original];

  trait[nbply] = trait[original];

  move_effect_journal_top[nbply] = move_effect_journal_top[nbply-1];

  ++post_move_iteration_id[nbply];

  {
    unsigned int const nr_moves = current_move[original]-current_move[original-1];
    memcpy(&move_generation_stack[current_move[nbply]+1],
           &move_generation_stack[current_move[original-1]+1],
           nr_moves*sizeof move_generation_stack[0]);
    current_move[nbply] += nr_moves;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void finply()
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nbply==ply_watermark);
  --ply_watermark;

  nbply = ply_stack[--ply_stack_pointer];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void InitCond(void)
{
  square const *bnp;
  square i, j;

  mummer_strictness[White] = mummer_strictness_none;
  mummer_strictness[Black] = mummer_strictness_none;

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

  antirenai = rennormal;
  circerenai = rennormal;
  immunrenai = rennormal;
  marsrenai = rennormal;

  royal_square[White] = initsquare;
  royal_square[Black] = initsquare;

  CondFlag[circeassassin]= false;
  flagparasent= false;
  rex_mad = false;
  rex_circe = false;
  immune_is_rex_inclusive = false;
  phantom_chess_rex_inclusive = false;
  rex_geneva =false;
  messigny_rex_exclusive = false;
  woozles_rex_exclusive = false;
  rex_protean_ex = false;

  sentinelles_max_nr_pawns[Black] = 8;
  sentinelles_max_nr_pawns[White] = 8;
  sentinelles_max_nr_pawns_total = 16;
  sentinelle = Pawn;

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
  koeko_nocontact= nokingcontact;
  antikoeko_nocontact= nokingcontact;
  OscillatingKingsTypeB[White]= false;
  OscillatingKingsTypeB[Black]= false;
  OscillatingKingsTypeC[White]= false;
  OscillatingKingsTypeC[Black]= false;

  BGL_values[White] = BGL_infinity;
  BGL_values[Black] = BGL_infinity;
  BGL_global= false;

  calc_reflective_king[White] = false;
  calc_reflective_king[Black] = false;

  reset_king_vaulters();

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

  castling_flags_no_castling = bl_castlings|wh_castlings;

  en_passant_forget_multistep();

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

  move_effect_journal_reset_retro_capture();
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
    empty_square(i);
    block_square(i);
  }

  /* dummy squares for Messigny chess and castling -- must be empty */
  empty_square(messigny_exchange);
  empty_square(kingside_castling);
  empty_square(queenside_castling);

  for (bnp = boardnum; *bnp; bnp++)
    empty_square(*bnp);

  king_square[White] = initsquare;
  king_square[Black] = initsquare;

  CLEARFL(all_pieces_flags);
  CLEARFL(all_royals_flags);
  CLEARFL(some_pieces_flags);

  nrhuntertypes = 0;
} /* InitBoard */

void InitAlways(void)
{
  ply i;

  nbply = nil_ply;
  current_move[nbply] = nil_coup;
  ply_watermark = nil_ply;

  flagfee = false;

  for (i= maxply; i > 0; i--)
  {
    killer_moves[i].departure = initsquare;
    killer_moves[i].arrival = initsquare;
    current_circe_rebirth_square[i] = initsquare;
    trait[i] = White;
    current_anticirce_rebirth_square[i] = initsquare;
  }

  reset_tables();

  reset_max_nr_solutions_per_target_position();

  king_capture_avoiders_reset();
}

square coinequis(square i)
{
  return 75 + (onerow*(((i/onerow)+3)/2) + (((i%onerow)+3)/2));
}

boolean leapcheck(vec_index_type kanf, vec_index_type kend,
                  PieNam p,
                  evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;

  /* detect "check" of leaper p */
  vec_index_type k;
  for (k= kanf; k<=kend; k++)
  {
    square const sq_departure= sq_target+vec[k];
    if (get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
      return true;
  }

  return false;
}

boolean leapleapcheck(vec_index_type kanf, vec_index_type kend,
                      int hurdletype,
                      boolean leaf,
                      PieNam p,
                      evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  /* detect "check" of leaper p */
  vec_index_type  k;

  for (k= kanf; k<= kend; k++)
  {
    square const sq_hurdle= sq_target + vec[k];
    if ((hurdletype==0 && get_walk_of_piece_on_square(sq_hurdle)>Invalid && TSTFLAG(spec[sq_hurdle],advers(trait[nbply])))
        || (hurdletype ==1 && get_walk_of_piece_on_square(sq_hurdle)>Invalid))
    {
      vec_index_type k1;
      for (k1= kanf; k1<= kend; k1++)
      {
        square const sq_departure = sq_hurdle + vec[k1];
        if (get_walk_of_piece_on_square(sq_departure)==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && sq_departure!=sq_target
            && INVOKE_EVAL(evaluate,sq_departure,sq_target))
          return true;
      }
    }
  }

  return false;
}

boolean ridcheck(vec_index_type kanf, vec_index_type kend,
                 PieNam p,
                 evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  /* detect "check" of rider p */
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(p);
  TraceFunctionParamListEnd();

  ++observation_context;

  TraceEnumerator(Side,trait[nbply],"\n");
  for (interceptable_observation_vector_index[observation_context] = kanf;
       interceptable_observation_vector_index[observation_context]<= kend;
       ++interceptable_observation_vector_index[observation_context])
  {
    square const sq_departure = find_end_of_line(sq_target,vec[interceptable_observation_vector_index[observation_context]]);
    PieNam const rider = get_walk_of_piece_on_square(sq_departure);
    TraceSquare(sq_departure);
    TracePiece(rider);
    TraceValue("%u\n",TSTFLAG(spec[sq_departure],trait[nbply]));
    if (rider==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_target))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean marine_rider_check(vec_index_type kanf, vec_index_type kend,
                           PieNam p,
                           evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  /* detect "check" of marin piece p or a locust */
  vec_index_type k;

  for (k= kanf; k<= kend; k++)
  {
    square const sq_arrival= sq_target-vec[k];
    if (is_square_empty(sq_arrival))
    {
      square const sq_departure = find_end_of_line(sq_target,vec[k]);
      PieNam const marine = get_walk_of_piece_on_square(sq_departure);
      if (marine==p
          && TSTFLAG(spec[sq_departure],trait[nbply])
          && INVOKE_EVAL(evaluate,sq_departure,sq_arrival))
        return true;
    }
  }

  return false;
}

boolean marine_leaper_check(vec_index_type kanf, vec_index_type kend,
                            PieNam p,
                            evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  vec_index_type k;
  for (k = kanf; k<=kend; ++k)
  {
    square const sq_arrival = sq_target-vec[k];
    square const sq_departure = sq_target+vec[k];
    if (is_square_empty(sq_arrival)
        && get_walk_of_piece_on_square(sq_departure)==p
        && TSTFLAG(spec[sq_departure],trait[nbply])
        && INVOKE_EVAL(evaluate,sq_departure,sq_arrival))
      return true;
  }

  return false;
}

static boolean marine_pawn_test_check(square sq_departure,
                                      square sq_hurdle,
                                      PieNam p,
                                      evalfunction_t *evaluate)
{
  boolean result;
  numvec const dir_check = sq_hurdle-sq_departure;
  square const sq_arrival = sq_hurdle+dir_check;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_hurdle);
  TraceFunctionParamListEnd();

  result = (get_walk_of_piece_on_square(sq_departure)==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && is_square_empty(sq_arrival)
            && INVOKE_EVAL(evaluate,sq_departure,sq_arrival));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean marine_pawn_check(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  numvec const dir_forward = trait[nbply]==White ? dir_up : dir_down;
  numvec const dir_forward_right = dir_forward+dir_right;
  numvec const dir_forward_left = dir_forward+dir_left;

  if (marine_pawn_test_check(sq_target-dir_forward_right,sq_target,p,evaluate))
    return true;
  else if (marine_pawn_test_check(sq_target-dir_forward_left,sq_target,p,evaluate))
    return true;
  else if (en_passant_test_check(dir_forward_right,&marine_pawn_test_check,p,evaluate))
    return true;
  else if (en_passant_test_check(dir_forward_left,&marine_pawn_test_check,p,evaluate))
    return true;

  return false;
}

boolean marine_ship_check(PieNam p, evalfunction_t *evaluate)
{
  return marine_pawn_check(p,evaluate) || tritoncheck(p,evaluate);
}

static boolean noleapcontact(square sq_arrival, vec_index_type kanf, vec_index_type kend)
{
  boolean result = true;
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParamListEnd();

  for (k = kanf; k<=kend; ++k)
  {
    square const sq_candidate = sq_arrival+vec[k];
    if (!is_square_empty(sq_candidate) && !is_square_blocked(sq_candidate))
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

static boolean find_next_orphan_in_chain(square sq_target,
                                         square const pos_orphans[],
                                         PieNam orphan_observer,
                                         evalfunction_t *evaluate)
{
  boolean result = false;
  unsigned int orphan_id;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(orphan_observer);
  TraceFunctionParamListEnd();

  for (orphan_id = 0; orphan_id<number_of_pieces[trait[nbply]][Orphan]; ++orphan_id)
  {
    boolean does_orphan_observe;

    isolate_observee(Orphan,pos_orphans,orphan_id);
    move_generation_stack[current_move[nbply]].capture = sq_target;
    does_orphan_observe = (*checkfunctions[orphan_observer])(Orphan,
                                                             evaluate);
    restore_observees(Orphan,pos_orphans);

    if (does_orphan_observe
        && orphan_find_observation_chain(pos_orphans[orphan_id],
                                         orphan_observer,
                                         evaluate))
    {
      result = true;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean orphan_find_observation_chain(square sq_target,
                                      PieNam orphan_observer,
                                      evalfunction_t *evaluate)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(orphan_observer);
  TraceFunctionParamListEnd();

  trait[nbply] = advers(trait[nbply]);

  move_generation_stack[current_move[nbply]].capture = sq_target;
  if ((*checkfunctions[orphan_observer])(orphan_observer,evaluate))
    result = true;
  else if (number_of_pieces[trait[nbply]][Orphan]==0)
    result = false;
  else
  {
    --number_of_pieces[advers(trait[nbply])][Orphan];
    occupy_square(sq_target,Dummy,spec[sq_target]);

    {
      square pos_orphans[63];
      locate_observees(Orphan,pos_orphans);
      result = find_next_orphan_in_chain(sq_target,
                                         pos_orphans,
                                         orphan_observer,
                                         evaluate);
    }

    occupy_square(sq_target,Orphan,spec[sq_target]);
    ++number_of_pieces[advers(trait[nbply])][Orphan];
  }

  trait[nbply] = advers(trait[nbply]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean orphancheck(PieNam orphan_type,
                    evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  boolean result = false;
  PieNam const *orphan_observer;
  square pos_orphans[63];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(orphan_type);
  TraceFunctionParamListEnd();

  locate_observees(Orphan,pos_orphans);

  siblingply(trait[nbply]);
  current_move[nbply] = current_move[nbply-1]+1;
  move_generation_stack[current_move[nbply]].capture = sq_target;
  move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = initsquare;

  for (orphan_observer = orphanpieces; *orphan_observer!=Empty; orphan_observer++)
    if (number_of_pieces[White][*orphan_observer]+number_of_pieces[Black][*orphan_observer]>0
        && find_next_orphan_in_chain(sq_target,
                                     pos_orphans,
                                     *orphan_observer,
                                     evaluate))
    {
      result = true;
      break;
    }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean friendcheck(PieNam p, evalfunction_t *evaluate)
{
  square const sq_target = move_generation_stack[current_move[nbply]].capture;
  PieNam const *pfr;
  boolean result = false;
  square pos_friends[63];

  TraceFunctionEntry(__func__);
  TraceSquare(sq_target);
  TracePiece(p);
  TraceFunctionParamListEnd();

  locate_observees(Friend,pos_friends);

  siblingply(trait[nbply]);
  current_move[nbply] = current_move[nbply-1]+1;
  move_generation_stack[current_move[nbply]].auxiliary.hopper.sq_hurdle = initsquare;

  for (pfr = orphanpieces; *pfr!=Empty; pfr++)
    if (number_of_pieces[trait[nbply]][*pfr]>0)
    {
      unsigned int k;
      for (k = 0; k<number_of_pieces[trait[nbply]][Friend]; ++k)
      {
        boolean does_friend_observe;

        isolate_observee(Friend,pos_friends,k);
        move_generation_stack[current_move[nbply]].capture = sq_target;
        does_friend_observe = (*checkfunctions[*pfr])(Friend,evaluate);
        restore_observees(Friend,pos_friends);

        if (does_friend_observe
            && find_next_friend_in_chain(pos_friends[k],*pfr,p,evaluate))
        {
          result = true;
          break;
        }
      }

      if (result)
        break;
    }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
