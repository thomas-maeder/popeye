#include "pieces/walks/generate_moves.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/hoppers.h"
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
#include "pyproc.h"
#include "pydata.h"

#include <assert.h>

/* Generate moves for a piece
 * @param sq_departure common departure square of the generated moves
 * @param p the piece's walk
 */
void generate_moves_for_piece_based_on_walk(square sq_departure, PieNam p)
{
  switch (p)
  {
    case King:
    case ErlKing:
      leaper_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

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
      break;

    case Zebra:
      leaper_generate_moves(sq_departure, vec_zebre_start,vec_zebre_end);
      break;

    case Camel:
      leaper_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      break;

    case Giraffe:
      leaper_generate_moves(sq_departure, vec_girafe_start,vec_girafe_end);
      break;

    case RootFiftyLeaper:
      leaper_generate_moves(sq_departure, vec_rccinq_start,vec_rccinq_end);
      break;

    case Bucephale:
      leaper_generate_moves(sq_departure, vec_bucephale_start,vec_bucephale_end);
      break;

    case Wesir:
      leaper_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case Alfil:
      leaper_generate_moves(sq_departure, vec_alfil_start,vec_alfil_end);
      break;

    case Fers:
      leaper_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case Dabbaba:
      leaper_generate_moves(sq_departure, vec_dabbaba_start,vec_dabbaba_end);
      break;

    case BerolinaPawn:
      berolina_pawn_generate_moves(sq_departure);
      break;

    case ReversePawn:
      reverse_pawn_generate_moves(sq_departure);
      break;

    case Amazone:
      rider_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case Empress:
      rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case Princess:
      rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case Gnu:
      leaper_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case Antilope:
      leaper_generate_moves(sq_departure, vec_antilope_start,vec_antilope_end);
      break;

    case Squirrel:
      leaper_generate_moves(sq_departure, vec_ecureuil_start,vec_ecureuil_end);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case Waran:
      rider_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case Dragon:
      pawn_generate_moves(sq_departure);
      leaper_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case Gryphon:
    {
      unsigned int const no_capture_length = pawn_get_no_capture_length(trait[nbply],sq_departure);

      if (no_capture_length>0)
      {
        int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
        pawns_generate_nocapture_moves(sq_departure,dir_forward,no_capture_length);
      }

      rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      break;
    }

    case Ship:
      if (pawn_get_no_capture_length(trait[nbply],sq_departure)>0)
      {
        int const dir_forward = trait[nbply]==White ? dir_up : dir_down;
        pawns_generate_capture_move(sq_departure,dir_forward+dir_left);
        pawns_generate_capture_move(sq_departure,dir_forward+dir_right);
      }

      rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case Camelrider:
      rider_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      break;

    case Zebrarider:
      rider_generate_moves(sq_departure, vec_zebre_start,vec_zebre_end);
      break;

    case Gnurider:
      rider_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      rider_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case SuperBerolinaPawn:
      super_berolina_pawn_generate_moves(sq_departure);
      break;

    case SuperPawn:
      super_pawn_generate_moves(sq_departure);
      break;

    case RookHunter:
      rook_hunter_generate_moves(sq_departure);
      break;

    case BishopHunter:
      bishop_hunter_generate_moves(sq_departure);
      break;

    case Okapi:
      leaper_generate_moves(sq_departure, vec_okapi_start,vec_okapi_end);
      break;

    case Leap37:
      leaper_generate_moves(sq_departure, vec_leap37_start,vec_leap37_end);
      break;

    case Leap16:
      leaper_generate_moves(sq_departure, vec_leap16_start,vec_leap16_end);
      break;

    case Leap24:
      leaper_generate_moves(sq_departure, vec_leap24_start,vec_leap24_end);
      break;

    case Leap35:
      leaper_generate_moves(sq_departure, vec_leap35_start,vec_leap35_end);
      break;

    case Leap15:
      leaper_generate_moves(sq_departure, vec_leap15_start,vec_leap15_end);
      break;

    case Leap25:
      leaper_generate_moves(sq_departure, vec_leap25_start,vec_leap25_end);
      break;

    case WesirRider:
      rider_generate_moves(sq_departure,   vec_rook_start,vec_rook_end);
      break;

    case FersRider:
      rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case Bison:
      leaper_generate_moves(sq_departure, vec_bison_start,vec_bison_end);
      break;

    case Zebu:
      leaper_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      leaper_generate_moves(sq_departure, vec_girafe_start,vec_girafe_end);
      break;

    case Elephant:
      rider_generate_moves(sq_departure, vec_elephant_start,vec_elephant_end);
      break;

    case Leap36:
      leaper_generate_moves(sq_departure, vec_leap36_start,vec_leap36_end);
      break;

    case ChinesePawn:
      chinese_pawn_generate_moves(sq_departure);
      break;

    case Mao:
      mao_generate_moves(sq_departure);
      break;

    case Pao:
      chinese_rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case Leo:
      chinese_rider_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case Vao:
      chinese_rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case Nao:
      chinese_rider_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case Rose:
      rose_generate_moves(sq_departure,vec_knight_start,vec_knight_end);
      break;

    case NonStopEquihopper:
      nonstop_equihopper_generate_moves(sq_departure);
      break;

    case Locust:
      locust_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case NightLocust:
      locust_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case BishopLocust:
      locust_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case RookLocust:
      locust_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case Kangaroo:
      kangaroo_generate_moves(sq_departure);
      break;

    case KangarooLion:
      kangaroo_lion_generate_moves(sq_departure);
      break;

    case Kao:
      chinese_leaper_generate_moves(sq_departure, vec_knight_start, vec_knight_end);
      break;

    case KnightHopper:
      leaper_hoppers_generate_moves(sq_departure, vec_knight_start, vec_knight_end);
      break;

    case SpiralSpringer:
      spiralspringer_generate_moves(sq_departure);
      break;

    case DiagonalSpiralSpringer:
      diagonalspiralspringer_generate_moves(sq_departure);
      break;

    case BoyScout:
      boyscout_generate_moves(sq_departure);
      break;

    case GirlScout:
      girlscout_generate_moves(sq_departure);
      break;

    case Hamster:
      hamster_generate_moves(sq_departure);
      break;

    case UbiUbi:
    {
      ubiubi_generate_moves(sq_departure);
      break;
    }

    case Elk:
      elk_generate_moves(sq_departure);
      break;

    case Eagle:
      eagle_generate_moves(sq_departure);
      break;

    case Sparrow:
      sparrow_generate_moves(sq_departure);
      break;

    case Marguerite:
      marguerite_generate_moves(sq_departure);
      break;

    case Archbishop:
      archbishop_generate_moves(sq_departure);
      break;

    case ReflectBishop:
      reflecting_bishop_generate_moves(sq_departure);
      break;

    case Cardinal:
      cardinal_generate_moves(sq_departure);
      break;

    case BouncyKnight:
      bouncy_knight_generate_moves(sq_departure);
      break;

    case BouncyNightrider:
      bouncy_nightrider_generate_moves(sq_departure);
      break;

    case EquiHopper:
      equihopper_generate_moves(sq_departure);
      break;

    case CAT:
      cat_generate_moves(sq_departure);
      break;

    case Sirene:
      marine_rider_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case Triton:
      marine_rider_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case Nereide:
      marine_rider_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case Orphan:
      orphan_generate_moves(sq_departure);
      break;

    case Friend:
      friend_generate_moves(sq_departure);
      break;

    case EdgeHog:
      edgehog_generate_moves(sq_departure);
      break;

    case Moa:
      moa_generate_moves(sq_departure);
      break;

    case MoaRider:
      moarider_generate_moves(sq_departure);
      break;

    case MaoRider:
      maorider_generate_moves(sq_departure);
      break;

    case Skylla:
      skylla_generate_moves(sq_departure);
      break;

    case Charybdis:
      charybdis_generate_moves(sq_departure);
      break;

    case Grasshopper:
      hoppers_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case Lion:
      lions_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case NightriderHopper:
      hoppers_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case CamelHopper:
      hoppers_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      break;

    case ZebraHopper:
      hoppers_generate_moves(sq_departure, vec_zebre_start,vec_zebre_end);
      break;

    case GnuHopper:
      hoppers_generate_moves(sq_departure, vec_chameau_start,vec_chameau_end);
      hoppers_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case RookLion:
      lions_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case BishopLion:
      lions_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case RookHopper:
      hoppers_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case BishopHopper:
      hoppers_generate_moves(sq_departure, vec_bishop_start,vec_bishop_end);
      break;

    case ContraGras:
      contra_grasshopper_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case RoseLion:
      roselion_generate_moves(sq_departure,vec_knight_start,vec_knight_end);
      break;

    case RoseHopper:
      rosehopper_generate_moves(sq_departure,vec_knight_start,vec_knight_end);
      break;

    case RoseLocust:
      roselocust_generate_moves(sq_departure,vec_knight_start,vec_knight_end);
      break;

    case GrassHopper2:
      grasshoppers_n_generate_moves(sq_departure, vec_queen_start,vec_queen_end, 2);
      break;

    case GrassHopper3:
      grasshoppers_n_generate_moves(sq_departure, vec_queen_start,vec_queen_end, 3);
      break;

    case KingHopper:
      leaper_hoppers_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case DoubleGras:
      doublehopper_generate_moves(sq_departure,vec_queen_start,vec_queen_end);
      break;

    case DoubleRookHopper:
      doublehopper_generate_moves(sq_departure,vec_rook_start,vec_rook_end);
      break;

    case DoubleBishopper:
      doublehopper_generate_moves(sq_departure,vec_bishop_start,vec_bishop_end);
      break;

    case Orix:
      orix_generate_moves(sq_departure);
      break;

     case NonStopOrix:
      nonstop_orix_generate_moves(sq_departure);
      break;

    case Gral:
      leaper_generate_moves(sq_departure, vec_alfil_start,vec_alfil_end);
      hoppers_generate_moves(sq_departure, vec_rook_start,vec_rook_end);
      break;

    case RookMoose:
      rook_moose_generate_moves(sq_departure);
      break;

    case RookEagle:
      rook_eagle_generate_moves(sq_departure);
      break;

    case RookSparrow:
      rook_sparrow_generate_moves(sq_departure);
      break;

    case BishopMoose:
      bishop_moose_generate_moves(sq_departure);
      break;

    case BishopEagle:
      bishop_eagle_generate_moves(sq_departure);
      break;

    case BishopSparrow:
      bishop_sparrow_generate_moves(sq_departure);
      break;

    case Rao:
      rao_generate_moves(sq_departure,vec_knight_start,vec_knight_end);
      break;

    case Scorpion:
      leaper_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      hoppers_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case NightRiderLion:
      lions_generate_moves(sq_departure, vec_knight_start,vec_knight_end);
      break;

    case MaoRiderLion:
      maoriderlion_generate_moves(sq_departure);
      break;

    case MoaRiderLion:
      moariderlion_generate_moves(sq_departure);
      break;

    case Dolphin:
      kangaroo_generate_moves(sq_departure);
      hoppers_generate_moves(sq_departure, vec_queen_start,vec_queen_end);
      break;

    case Rabbit:
      rabbit_generate_moves(sq_departure);
      break;

    case Bob:
      bob_generate_moves(sq_departure);
      break;

    case EquiEnglish:
      equistopper_generate_moves(sq_departure);
      break;

    case EquiFrench:
      nonstop_equistopper_generate_moves(sq_departure);
      break;

    case Querquisite:
      switch (sq_departure%onerow - nr_of_slack_files_left_of_board)
      {
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
      break;

    default:
      /* Since pieces like DUMMY fall through 'default', we have */
      /* to check exactly if there is something to generate ...  */
      if (p>=Hunter0 && p<Hunter0+maxnrhuntertypes)
        hunter_generate_moves(sq_departure,p);
      break;
  }
}
