/********************* MODIFICATIONS to py6.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/05/09 SE   Bug fix: Halfneutral + AntiCirce
 **
 ** 2006/05/09 SE   New conditions: SAT, StrictSAT, SAT X Y (invented L.Salai sr.)
 **
 ** 2006/05/14 SE   New Condition: TakeMake (invented H.Laue)
 **
 ** 2006/05/17 SE   Changes to allow half-move specification for helpmates using 0.5 notation
 **                 Reset of maxsolutions changed for set play
 **                 SOme combinations with Take&MAke disallowed
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2007/01/28 NG   New stipulation: help-reflexmate (hr)
 **
 ** 2006/07/30 SE   New condition: Schwarzschacher
 **
 ** 2006/07/30 SE   Extended movenum option to also print time when solving h#/sh# at next1 ply
 **
 ** 2007/01/28 SE   New condition: Annan Chess
 **
 ** 2007/05/01 SE   Extended Chopper types to eagles, mooses and sparrows
 **
 ** 2007/05/04 SE   Bugfix: SAT + BlackMustCapture
 **
 ** 2007/05/04 SE   Bugfix: SAT + Ultraschachzwang
 **
 ** 2007/07/04 SE   Bugfix: Ser-dia
 **
 ** 2007/11/08 SE   New conditions: Vaulting kings (invented: J.G.Ingram)
 **                 Transmuting/Reflecting Ks now take optional piece list
 **                 turning them into vaulting types
 **
 ** 2007/11/19 NG   Bugfix: intelligent mode with helpselfmate not supported
 **
 ** 2007/12/20 SE   New condition: Lortap (invented: F.H. von Meyenfeldt)
 **
 ** 2007/12/21 SE   Command-line switch: -b set low priority (Win32 only so far)
 **                 Command-line switch: -maxtime (same func as Option)
 **
 ** 2007/12/26 SE   New piece: Reverse Pawn (for below but independent)
 **                 New condition: Protean Chess
 **                 New piece type: Protean man (invent A.H.Kniest?)
 **                 (Differs from Frankfurt chess in that royal riders
 **                 are not 'non-passant'. Too hard to do but possibly
 **                 implement as an independent condition later).
 **
 ** 2008/01/02 NG   New condition: Geneva Chess
 **
 ** 2008/02/20 SE   Bugfix: Annan
 **
 ** 2008/02/10 SE   New condition: Cheameleon Pursuit (invented? : L.Grolman)
 **
 ** 2008/02/19 SE   New condition: AntiKoeko
 **
 ** 2008/02/25 SE   New piece type: Magic
 **                 Adjusted Masand code
 **
 ** 2008/03/13 SE   New condition: Castling Chess (invented: N.A.Bakke?)
 **
 ** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)
 **
 **                 New piece type: Patrol pieces
 **
 ** 2012/01/27 NG   AlphabeticChess now possible for white or black only.
 **
 ***************************** End of List ******************************/

#if defined(macintosh)    /* is always defined on macintosh's  SB */
#   define SEGM2
#   include "platform/unix/mac.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#if defined(__unix)
#   include <unistd.h>        /* alarm() prototype */
#endif  /* __unix */

/* TurboC and BorlandC   TLi */
#if defined(__TURBOC__)
#   include <mem.h>
#   include <alloc.h>
#   include <conio.h>
#endif  /* __TURBOC__ */

#if defined(_WIN32)
#include <process.h>
#endif  /* _WIN32 */

#include "pieces/pieces.h"
#include "options/options.h"
#include "conditions/conditions.h"
#include "pymsg.h"
#include "DHT/dhtbcmem.h"
#include "solving/proofgames.h"
#include "solving/selfcheck_guard.h"
#include "solving/moving_pawn_promotion.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "solving/solve.h"
#include "solving/observation.h"
#include "output/output.h"
#include "output/latex/latex.h"
#include "output/plaintext/language_dependant.h"
#include "input/plaintext/token.h"
#include "conditions/annan.h"
#include "conditions/bgl.h"
#include "conditions/koeko/contact_grid.h"
#include "conditions/koeko/koeko.h"
#include "conditions/koeko/anti.h"
#include "conditions/koeko/new.h"
#include "conditions/exclusive.h"
#include "conditions/republican.h"
#include "conditions/blackchecks.h"
#include "conditions/extinction.h"
#include "conditions/madrasi.h"
#include "conditions/maff/immobility_tester.h"
#include "conditions/owu/immobility_tester.h"
#include "conditions/ohneschach.h"
#include "conditions/singlebox/type1.h"
#include "conditions/singlebox/type2.h"
#include "conditions/singlebox/type3.h"
#include "conditions/patience.h"
#include "conditions/isardam.h"
#include "conditions/masand.h"
#include "conditions/disparate.h"
#include "conditions/dynasty.h"
#include "conditions/eiffel.h"
#include "conditions/circe/assassin.h"
#include "conditions/ultraschachzwang/legality_tester.h"
#include "conditions/sat.h"
#include "conditions/oscillating_kings.h"
#include "conditions/messigny.h"
#include "conditions/actuated_revolving_centre.h"
#include "conditions/actuated_revolving_board.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/cage.h"
#include "conditions/circe/chameleon.h"
#include "conditions/circe/clone.h"
#include "conditions/circe/couscous.h"
#include "conditions/circe/double_agents.h"
#include "conditions/circe/kamikaze.h"
#include "conditions/circe/parrain.h"
#include "conditions/circe/volage.h"
#include "conditions/circe/promotion.h"
#include "conditions/circe/frischauf.h"
#include "conditions/circe/super.h"
#include "conditions/circe/april.h"
#include "conditions/circe/turncoats.h"
#include "conditions/circe/takemake.h"
#include "conditions/circe/rex_inclusive.h"
#include "conditions/anticirce/cheylan.h"
#include "conditions/anticirce/couscous.h"
#include "conditions/anticirce/anticirce.h"
#include "conditions/anticirce/promotion.h"
#include "conditions/anticirce/strict.h"
#include "conditions/anticirce/relaxed.h"
#include "conditions/anticirce/super.h"
#include "conditions/anticirce/magic_square.h"
#include "conditions/sentinelles.h"
#include "conditions/duellists.h"
#include "conditions/haunted_chess.h"
#include "conditions/ghost_chess.h"
#include "conditions/kobul.h"
#include "conditions/andernach.h"
#include "conditions/antiandernach.h"
#include "conditions/chameleon_pursuit.h"
#include "conditions/norsk.h"
#include "conditions/protean.h"
#include "conditions/einstein/einstein.h"
#include "conditions/einstein/reverse.h"
#include "conditions/einstein/anti.h"
#include "conditions/einstein/en_passant.h"
#include "conditions/traitor.h"
#include "conditions/volage.h"
#include "conditions/magic_square.h"
#include "conditions/tibet.h"
#include "conditions/degradierung.h"
#include "conditions/phantom.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/plus.h"
#include "conditions/line_chameleon.h"
#include "conditions/haan.h"
#include "conditions/castling_chess.h"
#include "conditions/exchange_castling.h"
#include "conditions/transmuting_kings/super.h"
#include "conditions/transmuting_kings/reflective_kings.h"
#include "conditions/amu/attack_counter.h"
#include "conditions/imitator.h"
#include "conditions/football.h"
#include "conditions/castling_chess.h"
#include "conditions/mummer.h"
#include "conditions/woozles.h"
#include "conditions/immune.h"
#include "conditions/provocateurs.h"
#include "conditions/lortap.h"
#include "conditions/patrol.h"
#include "conditions/nocapture.h"
#include "conditions/nopromotion.h"
#include "conditions/geneva.h"
#include "conditions/monochrome.h"
#include "conditions/bichrome.h"
#include "conditions/edgemover.h"
#include "conditions/grid.h"
#include "conditions/take_and_make.h"
#include "conditions/superguards.h"
#include "conditions/central.h"
#include "conditions/beamten.h"
#include "conditions/eiffel.h"
#include "conditions/madrasi.h"
#include "conditions/brunner.h"
#include "conditions/wormhole.h"
#include "conditions/transmuting_kings/transmuting_kings.h"
#include "conditions/vaulting_kings.h"
#include "conditions/backhome.h"
#include "conditions/vogtlaender.h"
#include "conditions/bicolores.h"
#include "conditions/antikings.h"
#include "conditions/sting.h"
#include "conditions/follow_my_leader.h"
#include "conditions/alphabetic.h"
#include "conditions/forced_squares.h"
#include "conditions/synchronous.h"
#include "conditions/must_capture.h"
#include "platform/maxmem.h"
#include "platform/maxtime.h"
#include "platform/pytime.h"
#include "platform/priority.h"
#include "conditions/shielded_kings.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/branch.h"
#include "stipulation/end_of_branch.h"
#include "stipulation/end_of_branch_tester.h"
#include "stipulation/dead_end.h"
#include "stipulation/move_inverter.h"
#include "stipulation/boolean/and.h"
#include "stipulation/boolean/or.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/constraint.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/goals/prerequisite_guards.h"
#include "stipulation/temporary_hacks.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/temporary_hacks.h"
#include "solving/king_capture_avoider.h"
#include "solving/testers.h"
#include "solving/find_shortest.h"
#include "solving/for_each_move.h"
#include "solving/battle_play/try.h"
#include "solving/battle_play/threat.h"
#include "solving/battle_play/continuation.h"
#include "solving/battle_play/setplay.h"
#include "solving/battle_play/min_length_guard.h"
#include "solving/trivial_end_filter.h"
#include "solving/avoid_unsolvable.h"
#include "solving/play_suppressor.h"
#include "solving/castling.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/orphan.h"
#include "pieces/walks/classification.h"
#include "solving/move_generator.h"
#include "solving/moving_pawn_promotion.h"
#include "solving/post_move_iteration.h"
#include "pieces/walks/walks.h"
#include "pieces/attributes/magic.h"
#include "pieces/attributes/paralysing/paralysing.h"
#include "pieces/attributes/kamikaze/kamikaze.h"
#include "pieces/attributes/neutral/neutral.h"
#include "pieces/attributes/neutral/half.h"
#include "pieces/attributes/hurdle_colour_changing.h"
#include "pieces/attributes/chameleon.h"
#include "pieces/attributes/chameleon.h"
#include "pieces/attributes/jigger.h"
#include "pieces/attributes/uncapturable.h"
#include "pieces/walks/pawns/promotion.h"
#include "pieces/walks/hunters.h"
#include "conditions/amu/mate_filter.h"
#include "conditions/circe/goal_filters.h"
#include "conditions/anticirce/goal_filters.h"
#include "pieces/attributes/chameleon.h"
#include "conditions/ultraschachzwang/goal_filter.h"
#include "options/no_short_variations/no_short_variations.h"
#include "optimisations/hash.h"
#include "optimisations/intelligent/intelligent.h"
#include "optimisations/keepmating.h"
#include "optimisations/goals/optimisation_guards.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_move_generator.h"
#include "optimisations/killer_move/killer_move.h"
#include "optimisations/killer_move/final_defense_move.h"
#include "optimisations/ohneschach/non_checking_first.h"
#include "optimisations/ohneschach/redundant_immobility_tests.h"
#include "optimisations/ohneschach/immobility_tests_help.h"
#include "options/quodlibet.h"
#include "options/goal_is_end.h"
#include "options/maxtime.h"
#include "options/degenerate_tree.h"
#include "options/maxthreatlength.h"
#include "options/maxflightsquares.h"
#include "options/nontrivial.h"
#include "options/movenumbers.h"
#include "options/maxsolutions/maxsolutions.h"
#include "options/stoponshortsolutions/stoponshortsolutions.h"
#include "optimisations/count_nr_opponent_moves/move_generator.h"
#include "optimisations/orthodox_mating_moves/orthodox_mating_moves_generation.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "optimisations/goals/remove_non_reachers.h"
#include "optimisations/detect_retraction.h"
#include "optimisations/orthodox_square_observation.h"
#include "debugging/trace.h"
#include "debugging/measure.h"
#ifdef _SE_
#include "se.h"
#endif

boolean WriteSpec(Flags sp, PieNam p, boolean printcolours)
{
  boolean ret = false;
  PieSpec spname;

  if (is_piece_neutral(sp))
  {
    StdChar(tolower(ColorString[UserLanguage][color_neutral][0]));
    ret = true;
  }
  else if (printcolours)
  {
    if (areColorsSwapped)
    {
      if (TSTFLAG(sp,White))
        StdChar(tolower(ColorString[UserLanguage][color_black][0]));
      if (TSTFLAG(sp,Black))
        StdChar(tolower(ColorString[UserLanguage][color_white][0]));
    }
    else
    {
      if (TSTFLAG(sp,White))
        StdChar(tolower(ColorString[UserLanguage][color_white][0]));
      if (TSTFLAG(sp,Black))
        StdChar(tolower(ColorString[UserLanguage][color_black][0]));
    }
  }

  for (spname = nr_sides; spname<PieSpCount; ++spname)
    if ((spname!=Volage || !CondFlag[volage])
        && (spname!=Patrol || !CondFlag[patrouille])
        && (spname!=Beamtet || !CondFlag[beamten])
        && (spname!=Royal || (p!=King && p!=Poseidon))
        && TSTFLAG(sp, spname))
    {
      StdChar(tolower(PieSpString[UserLanguage][spname-nr_sides][0]));
      ret = true;
    }

  return ret;
}

int parseCommandlineOptions(int argc, char *argv[])
{
  int idx = 1;

  while (idx<argc)
  {
    if (idx+1<argc && strcmp(argv[idx], "-maxpos")==0)
    {
      char *end;
      idx++;
      hash_max_number_storable_positions = strtoul(argv[idx], &end, 10);
      if (argv[idx]==end)
      {
        /* conversion failure
         * -> set to 0 now and to default value later */
        hash_max_number_storable_positions = 0;
      }
      idx++;
      continue;
    }
    else if (idx+1<argc && strcmp(argv[idx], "-maxtime")==0)
    {
      char *end;
      maxtime_type value;
      idx++;
      value = strtoul(argv[idx], &end, 10);
      if (argv[idx]==end)
        ; /* conversion failure -> assume no max time */
      else
        setCommandlineMaxtime(value);

      idx++;
      continue;
    }
    else if (idx+1<argc && strcmp(argv[idx],"-maxmem")==0)
    {
      readMaxmem(argv[idx+1]);
      idx += 2;
      continue;
    }
    else if (strcmp(argv[idx], "-regression")==0)
    {
      flag_regression = true;
      idx++;
      continue;
    }
    else if (strcmp(argv[idx], "-maxtrace")==0)
    {
      trace_level max_trace_level;
      char *end;

      idx++;
      if (idx<argc)
      {
        max_trace_level = strtoul(argv[idx], &end, 10);
#if defined(DOTRACE)
        if (*end==0)
          TraceSetMaxLevel(max_trace_level);
        else
        {
          /* conversion failure  - ignore option */
        }
#else
      /* ignore */
#endif
      }

      idx++;
      continue;
    }
    else
      break;
  }

  return idx;
}
