#include "solving/find_square_observer_tracking_back_from_target.h"
#include "pieces/walks/leapers.h"
#include "pieces/walks/riders.h"
#include "pieces/walks/lions.h"
#include "pieces/walks/roses.h"
#include "pieces/walks/bouncer.h"
#include "pieces/walks/pawns/pawn.h"
#include "pieces/walks/pawns/berolina.h"
#include "pieces/walks/pawns/reverse.h"
#include "pieces/walks/chinese/riders.h"
#include "pieces/walks/chinese/mao.h"
#include "pieces/walks/chinese/pawn.h"
#include "pieces/walks/argentinian/riders.h"
#include "pieces/walks/argentinian/saltador.h"
#include "pieces/walks/locusts.h"
#include "pieces/walks/marine.h"
#include "pieces/walks/hoppers.h"
#include "pieces/walks/bouncy.h"
#include "pieces/walks/pawns/super.h"
#include "pieces/walks/cardinal.h"
#include "pieces/walks/reflecting_bishop.h"
#include "pieces/walks/combined.h"
#include "pieces/walks/angle/hoppers.h"
#include "pieces/walks/spiral_springers.h"
#include "pieces/walks/bob.h"
#include "pieces/walks/cat.h"
#include "pieces/walks/skylla_charybdis.h"
#include "pieces/walks/edgehog.h"
#include "pieces/walks/kangaroo.h"
#include "pieces/walks/querquisite.h"
#include "pieces/walks/ubiubi.h"
#include "pieces/walks/radial.h"
#include "pieces/walks/orphan.h"
#include "pieces/walks/friend.h"
#include "pieces/walks/hunters.h"
#include "pieces/walks/sting.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "stipulation/stipulation.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "pieces/walks/hamster.h"

#include "debugging/assert.h"

piece_walk_type observing_walk[maxply+1];

piece_walk_type checkpieces[nr_piece_walks-Leo+1]; /* only fairies ! */

static boolean never_check(validator_id evaluate)
{
  return false;
}

checkfunction_t *checkfunctions[nr_piece_walks] =
{
/*  0 */        0, /* not used */
/*  1 */        0, /* not used */
/*  2 */        king_check,
/*  3 */        pawn_check,
/*  4 */        queen_check,
/*  5 */        knight_check,
/*  6 */        rook_check,
/*  7 */        bishop_check,
/*  8 */        leo_check,
/*  9 */        mao_check,
/* 10 */        pao_check,
/* 11 */        vao_check,
/* 12 */        rose_check,
/* 13 */        grasshopper_check,
/* 14 */        nightrider_check,
/* 15 */        zebra_check,
/* 16 */        camel_check,
/* 17 */        girafe_check,
/* 18 */        rccinq_check,
/* 19 */        bucephale_check,
/* 20 */        vizir_check,
/* 21 */        alfil_check,
/* 22 */        fers_check,
/* 23 */        dabbaba_check,
/* 24 */        lion_check,
/* 25 */        nonstop_equihopper_check,
/* 26 */        locust_check,
/* 27 */        berolina_pawn_check,
/* 28 */        amazone_check,
/* 29 */        empress_check,
/* 30 */        princess_check,
/* 31 */        gnu_check,
/* 32 */        antilope_check,
/* 33 */        squirrel_check,
/* 34 */        waran_check,
/* 35 */        dragon_check,
/* 36 */        kangaroo_check,
/* 37 */        spiralspringer_check,
/* 38 */        ubiubi_check,
/* 39 */        never_check, /* hamster cannot check */
/* 40 */        moose_check,
/* 41 */        eagle_check,
/* 42 */        sparrow_check,
/* 43 */        archbishop_check,
/* 44 */        reflecting_bishop_check,
/* 45 */        cardinal_check,
/* 46 */        nightrider_hopper_check,
/* 47 */        never_check, /* dummy cannot check */
/* 48 */        camel_rider_check,
/* 49 */        zebra_rider_check,
/* 50 */        gnu_rider_check,
/* 51 */        camelrider_hopper_check,
/* 52 */        zebrarider_hopper_check,
/* 53 */        gnurider_hopper_check,
/* 54 */        diagonalspiralspringer_check,
/* 55 */        bouncy_knight_check,
/* 56 */        equihopper_check,
/* 57 */        cat_check,
/* 58 */        superberolinapawn_check,
/* 59 */        superpawn_check,
/* 60 */        rooklion_check,
/* 61 */        bishoplion_check,
/* 62 */        locust_check,               /* sirene checks like locust */
/* 63 */        rooklocust_check,
/* 64 */        bishoplocust_check,
/* 65 */        orphan_check,
/* 66 */        edgehog_check,
/* 67 */        moa_check,
/* 68 */        rookhunter_check,
/* 69 */        bishophunter_check,
/* 70 */        maorider_check,
/* 71 */        moarider_check,
/* 72 */        rookhopper_check,
/* 73 */        bishopper_check,
/* 74 */        king_check,               /* erlking checks like king */
/* 75 */        boyscout_check,
/* 76 */        girlscout_check,
/* 77 */        skylla_check,
/* 78 */        charybdis_check,
/* 79 */        contragrasshopper_check,
/* 80 */        roselion_check,
/* 81 */        rosehopper_check,
/* 82 */        okapi_check,
/* 83 */        leap37_check,
/* 84 */        grasshopper_2_check,
/* 85 */        grasshopper_3_check,
/* 86 */        leap16_check,
/* 87 */        leap24_check,
/* 88 */        leap35_check,
/* 89 */        doublegrasshopper_check,
/* 90 */        kinghopper_check,
/* 91 */        orix_check,
/* 92 */        leap15_check,
/* 93 */        leap25_check,
/* 94 */        gral_check,
/* 95 */        rooklocust_check,
/* 96 */        bishoplocust_check,
/* 97 */        nightlocust_check,
/* 98 */        rook_check,              /* wazirrider checks like rook */
/* 99 */        bishop_check,               /* fersrider checks like bishop */
/*100 */        bison_check,
/*101 */        elephant_check,
/*102 */        nao_check,
/*103 */        rookmoose_check,
/*104 */        rookeagle_check,
/*105 */        rooksparrow_check,
/*106 */        bishopmoose_check,
/*107 */        bishopeagle_check,
/*108 */        bishopsparrow_check,
/*109 */        roselion_check,   /* rao checks like roselion */
/*110 */        scorpion_check,
/*111 */        marguerite_check,
/*112 */        leap36_check,
/*113 */        nightriderlion_check,
/*114 */        maoriderlion_check,
/*115 */        moariderlion_check,
/*116 */        friend_check,
/*117 */        dolphin_check,
/*118 */        kangaroolion_check,
/*119 */        bob_check,
/*120 */  equistopper_check,
/*121 */  nonstop_equistopper_check,
/*122 */  querquisite_check,
/*123 */  bouncer_check,
/*124 */  rookbouncer_check,
/*125 */  bishopbouncer_check,
/*126 */  chinese_pawn_check,
/*127 */  radialknight_check,
/*128 */  reversepawn_check,
/*129 */  roselocust_check,
/*130 */  zebu_check,
/*131 */  bouncy_nightrider_check,
/*132 */  spiralspringer20_check,
/*133 */  spiralspringer40_check,
/*134 */  spiralspringer11_check,
/*135 */  spiralspringer33_check,
/*136 */  quintessence_check,
/*137 */  doublerookhopper_check,
/*138 */  doublebishopper_check,
/*139 */  nonstop_orix_check,
/*140 */  treehopper_check,
/*141 */  leafhopper_check,
/*142 */  greatertreehopper_check,
/*143 */  greaterleafhopper_check,
/*144 */  kangaroolion_check,
/*145 */  knighthopper_check,
/*146 */  knighthopper_check,
/*147 */  ship_check,
/*148 */  gryphon_check,
/*149 */  marine_knight_check,
/*150 */  poseidon_check,
/*151 */  marine_pawn_check,
/*152 */  marine_ship_check,
/*153 */  sting_check,
/*154 */  senora_check,
/*155 */  faro_check,
/*156 */  loco_check,
/*157 */  saltador_check,
/*158 */  maohopper_check,
/*159 */  moahopper_check,
/*160*/   contrahamster_check,
/*161*/   &eagle_equihopper_check,
/*162*/   0,
/*163*/   &sparrow_equihopper_check,
/*164*/   0,
/*165*/   &moose_equihopper_check,
/*166*/   0,
/*167 */  hunter_check,
/*168 */  hunter_check,
/*169 */  hunter_check,
/*170 */  hunter_check,
/*171 */  hunter_check,
/*172 */  hunter_check,
/*173 */  hunter_check,
/*174 */  hunter_check,
/*175 */  hunter_check,
/*176 */  hunter_check,
};

void track_back_from_target_according_to_observer_walk(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceWalk(observing_walk[nbply]);
  TraceEOL();
  observation_result = (*checkfunctions[observing_walk[nbply]])(observation_validator);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static piece_walk_type const ortho_walks[] = { King, Pawn, Knight, Rook, Bishop, Queen };
enum { nr_ortho_walks = sizeof ortho_walks / sizeof ortho_walks[0] };

void determine_observer_walk(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    unsigned int i;

    for (i = 0; i!=nr_ortho_walks; ++i)
    {
      observing_walk[nbply] = ortho_walks[i];
      TraceWalk(observing_walk[nbply]);
      TraceEOL();
      pipe_is_square_observed_delegate(si);
      if (observation_result)
      {
        TraceFunctionExit(__func__);
        TraceFunctionResultEnd();
        return;
      }
    }
  }

  {
    piece_walk_type const *pcheck;

    for (pcheck = checkpieces; *pcheck; ++pcheck)
    {
      observing_walk[nbply] = *pcheck;
      TraceWalk(observing_walk[nbply]);
      TraceEOL();
      pipe_is_square_observed_delegate(si);
      if (observation_result)
      {
        TraceFunctionExit(__func__);
        TraceFunctionResultEnd();
        return;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
