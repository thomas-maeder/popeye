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

#include "pymsg.h"
#include "py1.h"
#include "pydata.h"
#include "pieces/pieces.h"
#include "options/options.h"
#include "conditions/conditions.h"
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
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/move_generator.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "solving/king_capture_avoider.h"
#include "solving/observation.h"
#include "conditions/madrasi.h"
#include "conditions/grid.h"
#include "conditions/protean.h"
#include "conditions/bgl.h"
#include "conditions/oscillating_kings.h"
#include "conditions/kobul.h"
#include "conditions/anticirce/super.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/chameleon.h"
#include "conditions/circe/april.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/football.h"
#include "conditions/anticirce/anticirce.h"
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

  anticirce_determine_rebirth_square = rennormal;
  circe_determine_rebirth_square = rennormal;
  immunrenai = rennormal;
  marscirce_determine_rebirth_square = rennormal;

  royal_square[White] = initsquare;
  royal_square[Black] = initsquare;

  CondFlag[circeassassin]= false;
  flagparasent= false;
  madrasi_is_rex_inclusive = false;
  circe_is_rex_inclusive = false;
  immune_is_rex_inclusive = false;
  phantom_chess_rex_inclusive = false;
  rex_geneva =false;
  messigny_rex_exclusive = false;
  woozles_rex_exclusive = false;
  protean_is_rex_exclusive = false;

  sentinelles_max_nr_pawns[Black] = 8;
  sentinelles_max_nr_pawns[White] = 8;
  sentinelles_max_nr_pawns_total = 16;
  sentinelle = Pawn;

  grid_type = grid_normal;
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

  ply_reset();

  flagfee = false;

  for (i= maxply; i > 0; i--)
  {
    killer_moves[i].departure = initsquare;
    killer_moves[i].arrival = initsquare;
    current_circe_rebirth_square[i] = initsquare;
    trait[i] = White;
    anticirce_current_rebirth_square[i] = initsquare;
  }

  reset_tables();

  reset_max_nr_solutions_per_target_position();

  king_capture_avoiders_reset();
}
