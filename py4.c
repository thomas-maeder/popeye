/********************* MODIFICATIONS to py4.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/05/09 SE   New pieces Bouncer, Rookbouncer, Bishopbouncer (invented P.Wong)
 **
 ** 2006/05/14 SE   New Condition: TakeMake (invented H.Laue)
 **
 ** 2006/05/17 SE   Bug fix: querquisite
 **                 P moves to 1st rank disallowed for Take&Make on request of inventor
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher
 **
 ** 2007/01/28 SE   New condition: NormalPawn
 **
 ** 2007/01/28 SE   New condition: Annan Chess
 **
 ** 2007/05/01 SE   Extended Chopper types to eagles, mooses and sparrows
 **
 ** 2007/06/01 SE   New piece: Radial knight (invented: C.J.Feather)
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/12/20 SE   New condition: Lortap (invented: F.H. von Meyenfeldt)
 **
 ** 2007/12/26 SE   New piece: Reverse Pawn (for below but independent)
 **
 ** 2008/01/01 SE   Bug fix: Isardam + Maximummer (reported V.Crisan)
 **
 ** 2008/02/24 SE   Bug fix: Gridchess
 **
 ** 2008/02/19 SE   New piece: RoseLocust
 **
 ** 2008/03/13 SE   New condition: Castling Chess (invented: N.A.Bakke?)
 **
 ** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
 **
 ** 2009/02/24 SE   New pieces: 2,0-Spiralknight
 **                             4,0-Spiralknight
 **                             1,1-Spiralknight
 **                             3,3-Spiralknight
 **                             Quintessence (invented Joerg Knappen)
 **
 ** 2009/04/25 SE   New condition: Provacateurs
 **                 New piece type: Patrol pieces
 **
 ** 2009/06/27 SE   Extended imitators/hurdlecolorchanging to moose etc.
 **
 ** 2012/02/04 NG   New condition: Chess 8/1 (invented: Werner Keym, 5/2011)
 **
 **************************** End of List ******************************/

#if defined(macintosh)    /* is always defined on macintosh's  SB */
#   define SEGM2
#   include "platform/unix/mac.h"
#endif

#include "py.h"
#include "stipulation/stipulation.h"
#include "py1.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "stipulation/has_solution_type.h"
#include "solving/solve.h"
#include "solving/castling.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/hoppers.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "conditions/disparate.h"
#include "conditions/eiffel.h"
#include "conditions/madrasi.h"
#include "conditions/sat.h"
#include "conditions/duellists.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "conditions/singlebox/type3.h"
#include "conditions/beamten.h"
#include "conditions/patrol.h"
#include "conditions/central.h"
#include "conditions/koeko/koeko.h"
#include "conditions/phantom.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/plus.h"
#include "conditions/annan.h"
#include "conditions/wormhole.h"
#include "conditions/singlebox/type2.h"
#include "conditions/vaulting_kings.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/walks/leapers.h"
#include "pieces/walks/riders.h"
#include "pieces/walks/pawns/pawns.h"
#include "pieces/walks/pawns/pawn.h"
#include "pieces/walks/pawns/reverse.h"
#include "pieces/walks/pawns/berolina.h"
#include "pieces/walks/lions.h"
#include "pieces/walks/hunters.h"
#include "pieces/walks/roses.h"
#include "pieces/walks/spiral_springers.h"
#include "pieces/walks/marine.h"
#include "pieces/walks/ubiubi.h"
#include "pieces/walks/angle/angles.h"
#include "pieces/walks/angle/hoppers.h"
#include "pieces/walks/chinese/riders.h"
#include "pieces/walks/chinese/leapers.h"
#include "pieces/walks/chinese/pawn.h"
#include "pieces/walks/skylla_charybdis.h"
#include "pieces/walks/radial.h"
#include "pieces/walks/edgehog.h"
#include "pieces/walks/friend.h"
#include "pieces/walks/orphan.h"
#include "pieces/walks/pawns/super.h"
#include "pieces/walks/chinese/mao.h"
#include "pieces/walks/cat.h"
#include "pieces/walks/bouncy.h"
#include "pieces/walks/cardinal.h"
#include "pieces/walks/reflecting_bishop.h"
#include "pieces/walks/bob.h"
#include "pieces/walks/rabbit.h"
#include "pieces/walks/kangaroo.h"
#include "pieces/walks/locusts.h"
#include "pieces/walks/hamster.h"
#include "pieces/walks/bouncer.h"
#include "debugging/trace.h"
#include "debugging/measure.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int len_max(square sq_departure, square sq_arrival, square sq_capture)
{
  switch (sq_capture) {
  case messigny_exchange:
    return 0;

  case kingside_castling:
    return 16;

  case queenside_castling:
    return 25;

  default:  /* "ordinary move" */
    switch (get_walk_of_piece_on_square(sq_departure)) {

    case Mao:    /* special MAO move.*/
      return 6;

    case Moa:    /* special MOA move.*/
      return 6;

    default:
      if (CondFlag[castlingchess] && sq_capture > platzwechsel_rochade) {
        return (move_diff_code[abs(sq_arrival-sq_departure)]) +
          (move_diff_code[abs((sq_capture-maxsquare)-(sq_departure+sq_arrival)/2)]);
      }
      if (CondFlag[castlingchess] && sq_capture == platzwechsel_rochade) {
        return 2 * (move_diff_code[abs(sq_arrival-sq_departure)]);
      }
      else
       return (move_diff_code[abs(sq_arrival-sq_departure)]);
    }
    break;
  }
}

int len_min(square sq_departure, square sq_arrival, square sq_capture) {
  return -len_max(sq_departure,sq_arrival,sq_capture);
}

int len_capt(square sq_departure, square sq_arrival, square sq_capture)
{
  return !is_square_empty(sq_capture);
}

int len_follow(square sq_departure, square sq_arrival, square sq_capture)
{
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
  move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[parent];
  if (movement<parent_top)
    return sq_arrival==move_effect_journal[movement].u.piece_movement.from;
  else
    return true;
}

int len_whduell(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_departure == duellists[White][parent_ply[nbply]]);
}

int len_blduell(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_departure == duellists[Black][parent_ply[nbply]]);
}

int len_alphabetic(square sq_departure, square sq_arrival, square sq_capture) {
  return -((sq_departure/onerow) + onerow*(sq_departure%onerow));
}

int len_synchron(square sq_departure, square sq_arrival, square sq_capture)
{
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
  move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[parent];
  if (movement<parent_top)
  {
    square const sq_parent_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_parent_arrival = move_effect_journal[movement].u.piece_movement.to;
    numvec const parent_diff = sq_parent_departure-sq_parent_arrival;
    numvec const diff = sq_departure-sq_arrival;
    return diff==parent_diff;
  }
  else
    return true;
}

int len_antisynchron(square sq_departure, square sq_arrival, square sq_capture)
{
  ply const parent = parent_ply[nbply];
  move_effect_journal_index_type const parent_base = move_effect_journal_top[parent-1];
  move_effect_journal_index_type const movement = parent_base+move_effect_journal_index_offset_movement;
  move_effect_journal_index_type const parent_top = move_effect_journal_top[parent];
  if (movement<parent_top)
  {
    square const sq_parent_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_parent_arrival = move_effect_journal[movement].u.piece_movement.to;
    numvec const parent_diff = sq_parent_departure-sq_parent_arrival;
    numvec const diff = sq_departure-sq_arrival;
    return diff==-parent_diff;
  }
  else
    return true;
}

int len_whforcedsquare(square sq_departure, square sq_arrival, square sq_capture)
{
  int const result = TSTFLAG(sq_spec[sq_arrival],WhForcedSq);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  TraceValue("%x\n",sq_spec[sq_arrival]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

int len_blforcedsquare(square sq_departure, square sq_arrival, square sq_capture)
{
  int const result = TSTFLAG(sq_spec[sq_arrival],BlForcedSq);

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

int len_schwarzschacher(square sq_departure, square sq_arrival, square sq_capture)
{
   return sq_arrival==nullsquare ? 0 : 1;
}

void generate_moves_for_piece_ortho(square sq_departure, PieNam p)
{
  switch (p)
  {
    case King:
    case ErlKing:
      leaper_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      return;

    case Pawn:
      pawn_generate_moves(sq_departure);
      break;

    case Knight:
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case Rook:
      rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case Queen:
      rider_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case Bishop:
      rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case NightRider:
      rider_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Zebra:
      leaper_generate_moves(sq_departure, vec_zebre_start,vec_zebre_end);
      return;

    case Camel:
      leaper_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      return;

    case Giraffe:
      leaper_generate_moves(sq_departure, vec_girafe_start,vec_girafe_end);
      return;

    case RootFiftyLeaper:
      leaper_generate_moves(sq_departure, vec_rccinq_start,vec_rccinq_end);
      return;

    case Bucephale:
      leaper_generate_moves(sq_departure, vec_bucephale_start,vec_bucephale_end);
      return;

    case Wesir:
      leaper_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      return;

    case Alfil:
      leaper_generate_moves(sq_departure, vec_alfil_start,vec_alfil_end);
      return;

    case Fers:
      leaper_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      return;

    case Dabbaba:
      leaper_generate_moves(sq_departure, vec_dabbaba_start,vec_dabbaba_end);
      return;

    case BerolinaPawn:
      berolina_pawn_generate_moves(sq_departure);
      return;

    case ReversePawn:
      reverse_pawn_generate_moves(sq_departure);
      return;

    case Amazone:
      rider_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Empress:
      rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Princess:
      rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Gnu:
      leaper_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Antilope:
      leaper_generate_moves(sq_departure, vec_antilope_start,vec_antilope_end);
      return;

    case Squirrel:
      leaper_generate_moves(sq_departure, vec_ecureuil_start,vec_ecureuil_end);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Waran:
      rider_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      return;

    case Dragon:
      pawn_generate_moves(sq_departure);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Gryphon:
    {
      unsigned int const no_capture_length = pawn_get_no_capture_length(trait[nbply],sq_departure);

      if (no_capture_length>0)
      {
        int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
        pawns_generate_nocapture_moves(sq_departure,dir_forward,no_capture_length);
      }

      rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      return;
    }

    case Ship:
      if (pawn_get_no_capture_length(trait[nbply],sq_departure)>0)
      {
        int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
        pawns_generate_capture_move(sq_departure,dir_forward+dir_left);
        pawns_generate_capture_move(sq_departure,dir_forward+dir_right);
      }

      rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      return;

    case Camelrider:
      rider_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      return;

    case Zebrarider:
      rider_generate_moves(sq_departure, vec_zebre_start,vec_zebre_end);
      return;

    case Gnurider:
      rider_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      rider_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case SuperBerolinaPawn:
      super_berolina_pawn_generate_moves(sq_departure);
      return;

    case SuperPawn:
      super_pawn_generate_moves(sq_departure);
      return;

    case RookHunter:
      rook_hunter_generate_moves(sq_departure);
      return;

    case BishopHunter:
      bishop_hunter_generate_moves(sq_departure);
      return;

    case Okapi:
      leaper_generate_moves(sq_departure, vec_okapi_start,vec_okapi_end);
      return;

    case Leap37:
      leaper_generate_moves(sq_departure, vec_leap37_start,vec_leap37_end);
      return;

    case Leap16:
      leaper_generate_moves(sq_departure, vec_leap16_start,vec_leap16_end);
      return;

    case Leap24:
      leaper_generate_moves(sq_departure, vec_leap24_start,vec_leap24_end);
      return;

    case Leap35:
      leaper_generate_moves(sq_departure, vec_leap35_start,vec_leap35_end);
      return;

    case Leap15:
      leaper_generate_moves(sq_departure, vec_leap15_start,vec_leap15_end);
      return;

    case Leap25:
      leaper_generate_moves(sq_departure, vec_leap25_start,vec_leap25_end);
      return;

    case WesirRider:
      rider_generate_moves(sq_departure,   vec_rook_start,vec_rook_end);
      return;

    case FersRider:
      rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      return;

    case Bison:
      leaper_generate_moves(sq_departure, vec_bison_start,vec_bison_end);
      return;

    case Zebu:
      leaper_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      leaper_generate_moves(sq_departure, vec_girafe_start,vec_girafe_end);
      return;

    case Elephant:
      rider_generate_moves(sq_departure, vec_elephant_start,vec_elephant_end);
      return;

    case Leap36:
      leaper_generate_moves(sq_departure, vec_leap36_start,vec_leap36_end);
      return;

    case ChinesePawn:
      chinese_pawn_generate_moves(sq_departure);
      return;

    case Mao:
      mao_generate_moves(sq_departure);
      return;

    case Pao:
      chinese_rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      return;

    case Leo:
      chinese_rider_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      return;

    case Vao:
      chinese_rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      return;

    case Nao:
      chinese_rider_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case Rose:
      rose_generate_moves(sq_departure,vec_knight_start,vec_knight_end);
      return;

    case NonStopEquihopper:
      nonstop_equihopper_generate_moves(sq_departure);
      return;

    case Locust:
      locust_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      return;

    case NightLocust:
      locust_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case BishopLocust:
      locust_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      return;

    case RookLocust:
      locust_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      return;

    case Kangaroo:
      kangaroo_generate_moves(sq_departure);
      return;

    case KangarooLion:
      kangaroo_lion_generate_moves(sq_departure);
      return;

    case Kao:
      chinese_leaper_generate_moves(sq_departure, vec_knight_start, vec_knight_end);
      return;

    case KnightHopper:
      leaper_hoppers_generate_moves(sq_departure, vec_knight_start, vec_knight_end);
      return;

    case SpiralSpringer:
      spiralspringer_generate_moves(sq_departure);
      return;

    case DiagonalSpiralSpringer:
      diagonalspiralspringer_generate_moves(sq_departure);
      return;

    case BoyScout:
      boyscout_generate_moves(sq_departure);
      return;

    case GirlScout:
      girlscout_generate_moves(sq_departure);
      return;

    case Hamster:
      hamster_generate_moves(sq_departure);
      return;

    case UbiUbi:
    {
      ubiubi_generate_moves(sq_departure);
      return;
    }

    case Elk:
      elk_generate_moves(sq_departure);
      return;

    case Eagle:
      eagle_generate_moves(sq_departure);
      return;

    case Sparrow:
      sparrow_generate_moves(sq_departure);
      return;

    case Marguerite:
      marguerite_generate_moves(sq_departure);
      return;

    case Archbishop:
      archbishop_generate_moves(sq_departure);
      return;

    case ReflectBishop:
      reflecting_bishop_generate_moves(sq_departure);
      return;

    case Cardinal:
      cardinal_generate_moves(sq_departure);
      return;

    case BouncyKnight:
      bouncy_knight_generate_moves(sq_departure);
      return;

    case BouncyNightrider:
      bouncy_nightrider_generate_moves(sq_departure);
      return;

    case EquiHopper:
      equihopper_generate_moves(sq_departure);
      return;

    case CAT:
      cat_generate_moves(sq_departure);
      return;

    case Sirene:
      marine_rider_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      return;

    case Triton:
      marine_rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      return;

    case Nereide:
      marine_rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      return;

    case Orphan:
      orphan_generate_moves(sq_departure);
      return;

    case Friend:
      friend_generate_moves(sq_departure);
      return;

    case EdgeHog:
      edgehog_generate_moves(sq_departure);
      return;

    case Moa:
      moa_generate_moves(sq_departure);
      return;

    case MoaRider:
      moarider_generate_moves(sq_departure);
      return;

    case MaoRider:
      maorider_generate_moves(sq_departure);
      return;

    case Skylla:
      skylla_generate_moves(sq_departure);
      return;

    case Charybdis:
      charybdis_generate_moves(sq_departure);
      return;

    case Grasshopper:
      hoppers_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      return;

    case Lion:
      lions_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      return;

    case NightriderHopper:
      hoppers_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case CamelHopper:
      hoppers_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      return;

    case ZebraHopper:
      hoppers_generate_moves(sq_departure, vec_zebre_start,vec_zebre_end);
      return;

    case GnuHopper:
      hoppers_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      hoppers_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case RookLion:
      lions_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      return;

    case BishopLion:
      lions_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      return;

    case RookHopper:
      hoppers_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      return;

    case BishopHopper:
      hoppers_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      return;

    case ContraGras:
      contra_grasshopper_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      return;

    case RoseLion:
      roselion_generate_moves(sq_departure,vec_knight_start,vec_knight_end);
      return;

    case RoseHopper:
      rosehopper_generate_moves(sq_departure,vec_knight_start,vec_knight_end);
      return;

    case RoseLocust:
      roselocust_generate_moves(sq_departure,vec_knight_start,vec_knight_end);
      return;

    case GrassHopper2:
      grasshoppers_n_generate_moves(sq_departure, vec_queen_start,vec_queen_end, 2);
      return;

    case GrassHopper3:
      grasshoppers_n_generate_moves(sq_departure, vec_queen_start,vec_queen_end, 3);
      return;

    case KingHopper:
      leaper_hoppers_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      return;

    case DoubleGras:
      doublehopper_generate_moves(sq_departure,vec_queen_start,vec_queen_end);
      return;

    case DoubleRookHopper:
      doublehopper_generate_moves(sq_departure,vec_rook_start,vec_rook_end);
      return;

    case DoubleBishopper:
      doublehopper_generate_moves(sq_departure,vec_bishop_start,vec_bishop_end);
      return;

    case Orix:
      orix_generate_moves(sq_departure);
      return;

     case NonStopOrix:
      nonstop_orix_generate_moves(sq_departure);
      return;

    case Gral:
      leaper_generate_moves(sq_departure, vec_alfil_start,vec_alfil_end);
      hoppers_generate_moves(sq_departure, vec_rook_start,vec_rook_end);      /* rookhopper */
      return;

    case RookMoose:
      rook_moose_generate_moves(sq_departure);
      return;

    case RookEagle:
      rook_eagle_generate_moves(sq_departure);
      return;

    case RookSparrow:
      rook_sparrow_generate_moves(sq_departure);
      return;

    case BishopMoose:
      bishop_moose_generate_moves(sq_departure);
      return;

    case BishopEagle:
      bishop_eagle_generate_moves(sq_departure);
      return;

    case BishopSparrow:
      bishop_sparrow_generate_moves(sq_departure);
      return;

    case Rao:
      rao_generate_moves(sq_departure,vec_knight_start,vec_knight_end);
      return;

    case Scorpion:
      leaper_generate_moves(sq_departure, vec_queen_start,vec_queen_end); /* eking */
      hoppers_generate_moves(sq_departure, vec_queen_start,vec_queen_end);     /* grashopper */
      return;

    case NightRiderLion:
      lions_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      return;

    case MaoRiderLion:
      maoriderlion_generate_moves(sq_departure);
      return;

    case MoaRiderLion:
      moariderlion_generate_moves(sq_departure);
      return;

    case Dolphin:
      kangaroo_generate_moves(sq_departure);
      hoppers_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      return;

    case Rabbit:
      rabbit_generate_moves(sq_departure);
      return;

    case Bob:
      bob_generate_moves(sq_departure);
      return;

    case EquiEnglish:
      equistopper_generate_moves(sq_departure);
      return;

    case EquiFrench:
      nonstop_equistopper_generate_moves(sq_departure);
      return;

    case Querquisite:
      switch (sq_departure%onerow - nr_of_slack_files_left_of_board) {
      case file_rook_queenside:
      case file_rook_kingside:
        rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
        break;
      case file_bishop_queenside:
      case file_bishop_kingside:
        rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
        break;
      case file_queen:
        rider_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
        break;
      case file_knight_queenside:
      case file_knight_kingside:
        leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
        break;
      case file_king:
        leaper_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
        break;
      }
      break;

    case Bouncer :
      bouncer_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case RookBouncer:
      bouncer_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case BishopBouncer :
      bouncer_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case RadialKnight:
      radialknight_generate_moves(sq_departure);
      break;

    case Treehopper:
      treehopper_generate_moves(sq_departure);
      break;

    case Leafhopper :
      leafhopper_generate_moves(sq_departure);
      break;

    case GreaterTreehopper:
      greater_treehopper_generate_moves(sq_departure);
      break;

    case GreaterLeafhopper:
      greater_leafhopper_generate_moves(sq_departure);
      break;

    case SpiralSpringer40:
      spiralspringer40_generate_moves(sq_departure);
      break;

    case SpiralSpringer20:
      spiralspringer20_generate_moves(sq_departure);
      break;

    case SpiralSpringer33:
      spiralspringer33_generate_moves(sq_departure);
      break;

    case SpiralSpringer11:
      spiralspringer11_generate_moves(sq_departure);
      break;

    case Quintessence:
      quintessence_generate_moves(sq_departure);
      break;

    case MarineKnight:
      marine_knight_generate_moves(sq_departure);
      break;

    case Poseidon:
      poseidon_generate_moves(sq_departure);
      break;

    case MarinePawn:
      marine_pawn_generate_moves(sq_departure);
      break;

    case MarineShip:
      marine_ship_generate_moves(sq_departure,vec_rook_start,vec_rook_end);
      return;

    default:
      /* Since pieces like DUMMY fall through 'default', we have */
      /* to check exactly if there is something to generate ...  */
      if (p>=Hunter0 && p<Hunter0+maxnrhuntertypes)
        hunter_generate_moves(sq_departure,p);
      break;
  }
}
