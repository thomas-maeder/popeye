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
 **************************** End of List ******************************/

#if defined(macintosh)    /* is always defined on macintosh's  SB */
#   define SEGM2
#   include "pymac.h"
#endif

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "py.h"
#include "py1.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"

#define MAX_OTHER_LEN 1000 /* needs to be at least the max of any value that can be returned in the len functions */

short len_max(square sq_departure, square sq_arrival, square sq_capture)
{
  switch (sq_capture) {
  case messigny_exchange:
    return 0;

  case kingside_castling:
    return 16;

  case queenside_castling:
    return 25;

  default:  /* "ordinary move" */
    switch (abs(e[sq_departure])) {

    case Mao:    /* special MAO move.*/
      return 6;

    case Moa:    /* special MOA move.*/
      return 6;

    default:
      return (move_diff_code[abs(sq_arrival-sq_departure)]);
    }
  }
}

short len_min(square sq_departure, square sq_arrival, square sq_capture) {
  return -len_max(sq_departure,sq_arrival,sq_capture);
}

short len_capt(square sq_departure, square sq_arrival, square sq_capture) {
  return (e[sq_capture] != vide);
}

short len_follow(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_arrival == move_generation_stack[repere[nbply]].departure);
}

short len_whduell(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_departure == whduell[nbply - 1]);
}

short len_blduell(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_departure == blduell[nbply - 1]);
}

short len_alphabetic(square sq_departure, square sq_arrival, square sq_capture) {
  return -((sq_departure/onerow) + onerow*(sq_departure%onerow));
}

short len_synchron(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_departure-sq_arrival
          == (move_generation_stack[repere[nbply]].departure
              - move_generation_stack[repere[nbply]].arrival));
}

short len_antisynchron(square sq_departure, square sq_arrival, square sq_capture) {
  return (sq_arrival-sq_departure
          == (move_generation_stack[repere[nbply]].departure
              - move_generation_stack[repere[nbply]].arrival));
}

short len_whforcedsquare(square sq_departure, square sq_arrival, square sq_capture) {
  if (we_generate_exact) {
    if (TSTFLAG(sq_spec[sq_arrival], WhConsForcedSq)) {
      there_are_consmoves = true;
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    return (TSTFLAG(sq_spec[sq_arrival], WhForcedSq));
  }
}

short len_blforcedsquare(square sq_departure, square sq_arrival, square sq_capture) {
  if (we_generate_exact) {
    if (TSTFLAG(sq_spec[sq_arrival], BlConsForcedSq)) {
      there_are_consmoves = true;
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    return (TSTFLAG(sq_spec[sq_arrival], BlForcedSq));
  }
}

short len_schwarzschacher(square id, square ia, square ip)
{
   return ia==nullsquare ? 0 : 1;
}

static boolean count_opponent_moves(int *nr_opponent_moves, couleur camp) {
  boolean       flag= false;
  numecoup      sic_nbc= nbcou;

  do {
    if (jouecoup()) {
      if (!flag && !echecc(camp)) {
        couleur ad= advers(camp);
        flag= true;
        *nr_opponent_moves= 0;
        move_generation_mode= move_generation_not_optimized;
        genmove(ad);
        move_generation_mode= move_generation_optimized_by_nr_opponent_moves;
        while (encore()) {
          if (jouecoup()) {
            if (!echecc(ad))
              (*nr_opponent_moves)++;
          }
          repcoup();
        }
        finply();
      }
    }
    repcoup();
  } while (sic_nbc == nbcou);
  nbcou= sic_nbc;
  return flag;
}

void init_move_generation_optimizer(void) {
  switch (move_generation_mode) {
  case move_generation_optimized_by_nr_opponent_moves:
    empile_optimization_table_count= 0;
    break;
  case move_generation_optimized_by_killer_move:
    current_killer_state.move.departure = kpilcd[nbply];
    current_killer_state.move.arrival = kpilca[nbply];
    current_killer_state.found = false;
    break;
  case move_generation_not_optimized:
    /* nothing */
    break;
  }
}

boolean is_killer_move(square sq_departure, square sq_arrival, square sq_capture) {
  return current_killer_state.move.departure==sq_departure
    && current_killer_state.move.arrival==sq_arrival;
}

static int compare_nr_opponent_moves(const void *a, const void *b) {
  return  (((empile_optimization_table_elmt *)a)->nr_opponent_moves
           - ((empile_optimization_table_elmt *)b)->nr_opponent_moves);
}

void finish_move_generation_optimizer(void) {
  switch (move_generation_mode) {
  case move_generation_optimized_by_nr_opponent_moves:
  {
    empile_optimization_table_elmt *
      curr_elmt = empile_optimization_table+empile_optimization_table_count;
    qsort(empile_optimization_table,
          empile_optimization_table_count,
          sizeof(empile_optimization_table_elmt),
          &compare_nr_opponent_moves);
    nbcou= repere[nbply];
    while (curr_elmt!=empile_optimization_table) {
      nbcou++;
      --curr_elmt;
      move_generation_stack[nbcou]= curr_elmt->move;
    }
    break;
  }
  case move_generation_optimized_by_killer_move:
    if (current_killer_state.found) {
      nbcou++;
      move_generation_stack[nbcou] = current_killer_state.move;
    }
    break;
  case move_generation_not_optimized:
    /* nothing */
    break;
  }
}

void add_to_move_generation_stack(square sq_departure, square sq_arrival, square sq_capture, square mren) {
  nbcou++;
  move_generation_stack[nbcou].departure= sq_departure;
  move_generation_stack[nbcou].arrival= sq_arrival;
  move_generation_stack[nbcou].capture= sq_capture;
  cmren[nbcou]= mren;
  ctrans[nbcou]=current_trans_gen;
}

void add_to_empile_optimization_table(square sq_departure, square sq_arrival, square sq_capture) {
  int   nr_opponent_moves = 0;

  /* for testempile() - mren shouldn't be relevant if we optimize by
   * number of opponent moves */
  add_to_move_generation_stack(sq_departure,sq_arrival,sq_capture,initsquare);
  
  if (count_opponent_moves(&nr_opponent_moves, trait[nbply])) {
    empile_optimization_table_elmt * const
      curr_elmt = empile_optimization_table+empile_optimization_table_count;
    if (is_killer_move(sq_departure,sq_arrival,sq_capture))
      nr_opponent_moves-= empile_optimization_priorize_killmove_by;
    curr_elmt->move.departure= sq_departure;
    curr_elmt->move.arrival= sq_arrival;
    curr_elmt->move.capture= sq_capture;
    curr_elmt->nr_opponent_moves= nr_opponent_moves;
    empile_optimization_table_count++;
  }
}

void save_as_killer_move(square capture) {
  current_killer_state.found= true;
  current_killer_state.move.capture= capture;
}

boolean empile(square sq_departure, square sq_arrival, square sq_capture) {
  square  hcr, mren= initsquare;
  couleur traitnbply;

  if (sq_departure==sq_arrival
      && (!nullgenre || sq_arrival != nullsquare))
    return true;

  if (empilegenre) {
    if (CondFlag[messigny]
        && sq_capture == messigny_exchange
        /* a swapping move */
        && (move_generation_stack[repere[nbply]].capture==messigny_exchange)
        /* last move was a swapping one too */
        && (sq_departure == move_generation_stack[repere[nbply]].arrival
            || sq_departure == move_generation_stack[repere[nbply]].departure
            || sq_arrival == move_generation_stack[repere[nbply]].arrival
            || sq_arrival == move_generation_stack[repere[nbply]].departure))
      /* No single piece must be involved in
       * two consecutive swappings, so reject move.
       */
    {
      return false;
    }

    if (anymars||anyantimars) {
      if (CondFlag[phantom]) {
        if (flagactive) {
          if ((sq_departure= marsid) == sq_arrival) {
            return true;
          }
        }
      }
      else {
        if (flagpassive!=anyantimars && e[sq_capture]!=vide)
          return true;
        if (flagcapture!=anyantimars && e[sq_capture]==vide)
          return true;
        if (flagcapture) {
          mren= sq_departure;
          sq_departure=marsid;
        }
      }
    }

    if (flaglegalsquare
        && !legalsquare(sq_departure,sq_arrival,sq_capture))
    {
      return true;
    }

    traitnbply= trait[nbply];

    if (CondFlag[takemake])
    {
      /* Take & make is implemented as follows: When the take part of
       * a move is attempted to be written into the moves table (ca,
       * cd, cp) by empile(), empile() causes moves to be generated
       * from the arrival square of the take part, but according to
       * the movement of the captured piece. This will cause empile()
       * to be invoked recursively.
       *
       * The actual take&make moves are empile()d in the nested
       * invocation; empile()ing them in the outer invocation would
       * break more or less everything that follows the if
       * (CondFlag[takemake]) clause, including Maximummer.
       *
       * The global variable takemake_departuresquare is used to
       * distinguish between the two cases; if its equal to
       * initsquare, we are generating a non-capturing move or the
       * take part of a capturing move; otherwise, we are generating
       * the make part of a capture where the capturing piece came
       * from takemake_departuresquare.
       */
      if (takemake_departuresquare==initsquare)
      {
        /* We are not generating the make part */

        piece takemake_takenpiece= e[sq_capture];
        if (takemake_takenpiece == vide) {
          /* Non-capturing move - go on as in orthodox chess */
        }
        else
        {
          /* this is the take part */
          Flags takemake_takenspec= spec[sq_capture];

          /* Save the departure square for signaling the we are now
           * generating the make part and for using the value when we
           * will be recursively invoked during the generation of the
           * make part. */
          takemake_departuresquare= sq_departure;

          /* At first, it may seem that saving ip isn't necessary
           * because the arrival square of the take part is the
           * departure square of the make part. Yet there are
           * situations where capture square and arrival square are
           * different (the raison d'etre of ip, after all) - most
           * notably en passant captures. */
          takemake_capturesquare= sq_capture;

          /* Execute the take part. The order ip, ia, id avoids losing
           * information and elegantly deals with the case where
           * ip==ia. */
          e[sq_capture]= vide;
          spec[sq_capture]= EmptySpec;

          e[sq_arrival]= e[sq_departure];
          spec[sq_arrival]= spec[sq_departure];

          e[sq_departure]= vide;    /* for sentinelles, need to calculate... */
          spec[sq_departure]= EmptySpec;

          if (traitnbply == blanc)
            gen_bl_piece_aux(sq_arrival, takemake_takenpiece);
          else
            gen_wh_piece_aux(sq_arrival, takemake_takenpiece);

          /* Take back the take part, reverse order of executing
           * it. */
          e[sq_departure]= e[sq_arrival];
          spec[sq_departure]= spec[sq_arrival];

          e[sq_arrival]= vide;
          spec[sq_arrival]= EmptySpec;

          e[sq_capture]= takemake_takenpiece;
          spec[sq_capture]= takemake_takenspec;

          takemake_capturesquare= initsquare;
          takemake_departuresquare= initsquare;

          /* This is the take part - actual moves were generated
           * during the recursive invokation for the make part, so
           * let's bail out.
           */
          return true;
        }
      }
      else if (e[sq_capture]==vide) {
        /* We are generating the make part */
        
        /* Extra rule: pawns must not 'make' to their base line */
        if (is_pawn(e[sq_departure] && !CondFlag[normalp])
            && ((e[sq_departure]>0 && sq_arrival<=square_h1)
                || (e[sq_departure]<0 && sq_arrival>=square_a8)))
          return true;

        sq_capture= takemake_capturesquare;
        sq_departure= takemake_departuresquare;
      }
      else
        /* We must not capture in the make part */
        return true;
    }

    if (((CondFlag[nowhiteprom]
          && traitnbply==blanc
          && PromSq(blanc,sq_arrival)
           )
         || (CondFlag[noblackprom]
             && traitnbply==noir
             && PromSq(noir,sq_arrival)
           )
          )
        && is_forwardpawn(e[sq_departure]))
    {
      return true;
    }

    if (((CondFlag[nowhiteprom]
          && traitnbply==blanc
          && ReversePromSq(blanc,sq_arrival)
           )
         || (CondFlag[noblackprom]
             && traitnbply==noir
             && ReversePromSq(noir,sq_arrival)
           )
          )
        && is_reversepawn(e[sq_departure]))
    {
      return true;
    }

    if (  TSTFLAG(spec[sq_departure], Beamtet)
          || CondFlag[beamten]
          || CondFlag[central]
          || CondFlag[ultrapatrouille])
    {
      if (!soutenu(sq_departure,sq_arrival,sq_capture))
        return true;
    }

    if (e[sq_capture] != vide) {
      if (CondFlag[woozles]
          && !woohefflibre(sq_arrival, sq_departure))
      {
        return  true;
      }

      if (CondFlag[norsk]
          && (sq_departure == rb || sq_departure == rn || abs(e[sq_capture]) != abs(e[sq_departure])))
      {
        return  true;
      }

      if (CondFlag[nocapture]
          || (CondFlag[nowhcapture]
              && traitnbply==blanc)
          || (CondFlag[noblcapture]
              && traitnbply==noir)
          || TSTFLAG(spec[sq_departure], Paralyse))
      {
        return true;
      }

      if (anyimmun) {
        hcr= (*immunrenai)(e[sq_capture], spec[sq_capture], sq_capture, sq_departure, sq_arrival, traitnbply);
        if (hcr != sq_departure && e[hcr] != vide) {
          return true;
        }
      }
      if (CondFlag[patrouille]
          && !(CondFlag[beamten]
               || TSTFLAG(PieSpExFlags, Beamtet)))
      {
        if (!soutenu(sq_departure,sq_arrival,sq_capture)) {
          return true;
        }
      }
      if (CondFlag[lortap]) 
      {
        if (soutenu(sq_departure,sq_arrival,sq_capture)) {
          return true;
        }
      }

      if (!k_cap) {
        /* We have to avoid captures by the rb because he
         * wouldn't be reborn! This might also be placed in
         * the function genrb(), but here, it works for all
         * royal pieces.
         * wegen neuer Funktionen genweiss/schwarz aus
         * gennoir/blanc hierher verschoben
         */
        /* capturing kamikaze pieces without circe condition are possible now */ 
        if (TSTFLAG(spec[sq_departure], Kamikaze)
            &&  ((traitnbply == blanc)
                 ? ((sq_departure == rb) && (!anycirce ||  (!rex_circe || e[(*circerenai)(e[rb], spec[rb], sq_capture, sq_departure, sq_arrival, noir)] != vide)))
                 : ((sq_departure == rn) && (!anycirce ||  (!rex_circe || e[(*circerenai)(e[rn], spec[rn], sq_capture, sq_departure, sq_arrival, blanc)] != vide)))))
        {
          return true;
        }

        if ((CondFlag[vogt]
             || CondFlag[antikings])
            && ((traitnbply == noir)
                ? ((sq_capture == rb) && (!rex_circe || e[(*circerenai)(e[rb], spec[rb], sq_capture, sq_departure, sq_arrival, noir)] != vide))
                : ((sq_capture == rn) && (!rex_circe || e[(*circerenai)(e[rn], spec[rn], sq_capture, sq_departure, sq_arrival, blanc)] != vide))))
        {
          return true;
        }

        if (SATCheck &&
            ((traitnbply == noir) ?
             ((sq_capture == rb) && (!rex_circe || e[(*circerenai)(e[rb], spec[rb], sq_capture, sq_departure, sq_arrival, noir)] != vide)) :
             ((sq_capture == rn) && (!rex_circe || e[(*circerenai)(e[rn], spec[rn], sq_capture, sq_departure, sq_arrival, blanc)] != vide))))
          return true;

        if (anyanticirce
            && (traitnbply==blanc
                ? !rnanticircech(sq_departure,sq_arrival,sq_capture)
                : !rbanticircech(sq_departure,sq_arrival,sq_capture)))
        {
          return true;
        }
      } /* k_cap */

    } /* e[sq_capture] != vide */

    if (CondFlag[imitators]
        && ((sq_capture==kingside_castling || sq_capture==queenside_castling)
            ? !castlingimok(sq_departure, sq_arrival)
            : !imok(sq_departure, sq_arrival))) {
      return false;
    }

    /* the following is used for conditions where checktest is
     *  needed for generation like follow my leader,
     * maximummer.....
     */
    if (!k_cap && traitnbply == noir ? flagblackmummer : flagwhitemummer) {
      boolean       flag= true, flag_dontaddk_sic;
      numecoup      test;
      boolean is_new_longest_move;

      if (encore()
          || (move_generation_mode==move_generation_optimized_by_killer_move
              && current_killer_state.found)) {
        /*
        ** There are moves stored. Check whether the
        ** length of the new one is shorter or equal
        ** to the currently longest move.
        */
        short len, curleng;

        if ( traitnbply == noir ? black_length : white_length)
        {
          len = traitnbply == noir
            ?  (*black_length)(sq_departure,sq_arrival,sq_capture)
            : (*white_length)(sq_departure,sq_arrival,sq_capture);
          curleng =
            (move_generation_mode==move_generation_optimized_by_killer_move
             && current_killer_state.found)
            ? (traitnbply == noir
               ? (*black_length)(current_killer_state.move.departure,
                                 current_killer_state.move.arrival,
                                 current_killer_state.move.capture)
               : (*white_length)(current_killer_state.move.departure,
                                 current_killer_state.move.arrival,
                                 current_killer_state.move.capture))
            : (traitnbply == noir
               ? (*black_length)(move_generation_stack[nbcou].departure,
                                 move_generation_stack[nbcou].arrival,
                                 move_generation_stack[nbcou].capture)
               : (*white_length)(move_generation_stack[nbcou].departure,
                                 move_generation_stack[nbcou].arrival,
                                 move_generation_stack[nbcou].capture));
        }
        else
        {
          len= 0;
          curleng=0;
        }
        
        if (traitnbply == blanc ? CondFlag[whsupertrans_king] : CondFlag[blsupertrans_king])
        {
                
          len+= MAX_OTHER_LEN * (current_trans_gen!=vide ? 1 : 0);
          curleng+= MAX_OTHER_LEN * (ctrans[nbcou]!=vide ? 1 : 0);
        }  

        if (curleng > len) {
          return true;
        }

        is_new_longest_move= curleng<len;
      }
      else
        is_new_longest_move= true;

      if (is_new_longest_move) {
        if (!we_generate_exact) {
          /* not exact-maxi -> test for selfcheck */
          couleur oneutcoul= neutcoul;
          test= nbcou++;
          move_generation_stack[nbcou].departure= sq_departure;
          move_generation_stack[nbcou].arrival= sq_arrival;
          move_generation_stack[nbcou].capture= sq_capture;
          cmren[nbcou]= mren;
          ctrans[nbcou]=current_trans_gen;
          flag_dontaddk_sic= flag_dontaddk;
          flag_dontaddk=true;
          while (test < nbcou) {
            if (jouecoup())
              flag= flag && echecc(traitnbply);
            repcoup();
          }
          flag_dontaddk= flag_dontaddk_sic;
          initneutre(oneutcoul);
          if (flag)
            return true;
        }

        nbcou= repere[nbply];
        current_killer_state.found= false;
      }
    }
  }

  if (FlagGenMatingMove) {
    if (FlagMoveOrientatedStip) {
      if (stipulation == stip_ep) {
        if (sq_arrival != ep[nbply-1] && sq_arrival != ep2[nbply-1])
          return true;
      }
      else if (stipulation == stip_target) {
        if (sq_arrival != TargetSquare)
          return true;
      }
      else if (stipulation == stip_capture
               || stipulation == stip_steingewinn)
      {
        if (e[sq_capture] == vide)
          return true;
      }
      else if (stipulation == stip_castling)
      {
        if (abs(e[sq_departure]) != King || abs(sq_departure-sq_arrival) != 2)
          return true;
      }
    }
    else if (totalortho
             && TSTFLAG(spec[sq_departure], Neutral)
             && stipulation != stip_check)
    {
      /* Check if a mating move by a neutral piece can be
      ** retracted by the opponent.
      ** This works also in more general cases, but which?
      */
      if (rb == rn) {
        if (rb == sq_departure) {
          /*     if (e[sq_capture] != vide)    nK-bug ! */
          if (e[sq_capture] == vide)
            return true;
        }
        else if ( (e[sq_departure] != Pawn && e[sq_departure] != -Pawn)
                  || e[sq_capture] == vide)
        {
          return true;
        }
      }
      else if (e[sq_capture] == vide && e[sq_departure] != Pawn && e[sq_departure] != -Pawn)
        return true;
    }
  }

  switch (move_generation_mode) {
  case move_generation_optimized_by_nr_opponent_moves:
    add_to_empile_optimization_table(sq_departure,sq_arrival,sq_capture);
    break;
  case move_generation_optimized_by_killer_move:
    if (!is_killer_move(sq_departure,sq_arrival,sq_capture)
        || nonkilgenre
        || flag_testlegality)
      add_to_move_generation_stack(sq_departure,sq_arrival,sq_capture,mren);
    else
      save_as_killer_move(sq_capture);
    break;
  case move_generation_not_optimized:
    add_to_move_generation_stack(sq_departure,sq_arrival,sq_capture,mren);
  }

  return true;
} /* empile */

void gemaooarider(square sq_departure,
                  numvec tomiddle,
                  numvec todest,
                  couleur camp)
{
  square middle= sq_departure+tomiddle;
  square sq_arrival= sq_departure+todest;
  while (e[middle]==vide
         && e[sq_arrival]==vide
         && empile(sq_departure,sq_arrival,sq_arrival)) {
    middle += todest;
    sq_arrival += todest;
  }
  if (e[middle]==vide && rightcolor(e[sq_arrival],camp))
    empile(sq_departure,sq_arrival,sq_arrival);
}

void gemoarider(square i, couleur camp) {
  gemaooarider(i,+dir_up+dir_left,+2*dir_up+dir_left, camp);
  gemaooarider(i,+dir_up+dir_left,+dir_up+2*dir_left, camp);
  gemaooarider(i,+dir_down+dir_right,+2*dir_down+dir_right, camp);
  gemaooarider(i,+dir_down+dir_right,+dir_down+2*dir_right, camp);
  gemaooarider(i,+dir_up+dir_right,+dir_up+2*dir_right, camp);
  gemaooarider(i,+dir_up+dir_right,+2*dir_up+dir_right, camp);
  gemaooarider(i,+dir_down+dir_left,+dir_down+2*dir_left, camp);
  gemaooarider(i,+dir_down+dir_left,+2*dir_down+dir_left, camp);
}

void gemaorider(square i, couleur camp) {
  gemaooarider(i,+dir_right,+dir_up+2*dir_right, camp);
  gemaooarider(i,+dir_right,+dir_down+2*dir_right, camp);
  gemaooarider(i,+dir_down,+2*dir_down+dir_right, camp);
  gemaooarider(i,+dir_down,+2*dir_down+dir_left, camp);
  gemaooarider(i,+dir_left,+dir_down+2*dir_left, camp);
  gemaooarider(i,+dir_left,+dir_up+2*dir_left, camp);
  gemaooarider(i,+dir_up,+2*dir_up+dir_left, camp);
  gemaooarider(i,+dir_up,+2*dir_up+dir_right, camp);
}

void gemaooariderlion(square sq_departure,
                      numvec tomiddle,
                      numvec todest,
                      couleur camp) {
  square middle= sq_departure + tomiddle;
  square sq_arrival= sq_departure+todest;

  while (e[middle]==vide && e[sq_arrival]==vide) {
    middle += todest;
    sq_arrival += todest;
  }
  if (e[middle] != obs && e[sq_arrival] != obs) {
    if (e[middle]!=vide
        && (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp)))
      empile(sq_departure,sq_arrival,sq_arrival);
    if (e[middle]==vide || e[sq_arrival]==vide) {
      middle += todest;
      sq_arrival += todest;
      while (e[middle]==vide && e[sq_arrival]==vide
             && empile(sq_departure,sq_arrival,sq_arrival)) {
        middle += todest;
        sq_arrival += todest;
      }
    }
    if (e[middle]==vide && rightcolor(e[sq_arrival],camp))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void gemaoriderlion(square i, couleur camp) {
  gemaooariderlion(i,+dir_right,+dir_up+2*dir_right, camp);
  gemaooariderlion(i,+dir_right,+dir_down+2*dir_right, camp);
  gemaooariderlion(i,+dir_down,+2*dir_down+dir_right, camp);
  gemaooariderlion(i,+dir_down,+2*dir_down+dir_left, camp);
  gemaooariderlion(i,+dir_left,+dir_down+2*dir_left, camp);
  gemaooariderlion(i,+dir_left,+dir_up+2*dir_left, camp);
  gemaooariderlion(i,+dir_up,+2*dir_up+dir_left, camp);
  gemaooariderlion(i,+dir_up,+2*dir_up+dir_right, camp);
}

void gemoariderlion(square i, couleur camp) {
  gemaooariderlion(i,+dir_up+dir_left,+2*dir_up+dir_left, camp);
  gemaooariderlion(i,+dir_up+dir_left,+dir_up+2*dir_left, camp);
  gemaooariderlion(i,+dir_down+dir_right,+2*dir_down+dir_right, camp);
  gemaooariderlion(i,+dir_down+dir_right,+dir_down+2*dir_right, camp);
  gemaooariderlion(i,+dir_up+dir_right,+dir_up+2*dir_right, camp);
  gemaooariderlion(i,+dir_up+dir_right,+2*dir_up+dir_right, camp);
  gemaooariderlion(i,+dir_down+dir_left,+dir_down+2*dir_left, camp);
  gemaooariderlion(i,+dir_down+dir_left,+2*dir_down+dir_left, camp);
}

static square generate_moves_on_line_segment(square sq_departure,
                                             square sq_base,
                                             smallint k) {
  square arr= sq_base+vec[k];
  while (e[arr]==vide && empile(sq_departure,arr,arr))
    arr+= vec[k];

  return arr;
}

void gebrid(square sq_departure, numvec kbeg, numvec kend) {
  /* generate white rider moves from vec[kbeg] to vec[kend] */
  numvec k;

  square sq_arrival;

  for (k= kbeg; k<= kend; k++) {
    sq_arrival= generate_moves_on_line_segment(sq_departure,sq_departure,k);
    if (e[sq_arrival]<=roin)
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void genrid(square sq_departure, numvec kbeg, numvec kend) {
  /* generate black rider moves from vec[kbeg] to vec[kend] */
  numvec k;
  square sq_arrival;

  for (k= kbeg; k<= kend; k++) {
    sq_arrival= generate_moves_on_line_segment(sq_departure,sq_departure,k);
    if (e[sq_arrival]>=roib)
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void genbouncer(square sq_departure, numvec kbeg, numvec kend, couleur camp) {

  square sq_arrival;

  numvec  k;
  for (k= kend; k >= kbeg; k--) {      
    piece   p1;
    square  bounce_where;
    finligne(sq_departure,vec[k],p1,bounce_where);

    {
      square const bounce_to= 2*sq_departure-bounce_where;

      sq_arrival= sq_departure-vec[k];
      while (sq_arrival!=bounce_to && e[sq_arrival]==vide)
        sq_arrival-= vec[k];

      if (sq_arrival==bounce_to
          && (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp)))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

boolean testempile(square sq_departure, square sq_arrival, square sq_capture) {
  numecoup k;

  if (!TSTFLAG(spec[sq_departure], ColourChange))
    for (k= nbcou; k > testdebut; k--)
      if (move_generation_stack[k].arrival==sq_arrival)
        return true;

  return empile(sq_departure,sq_arrival,sq_capture);
}

void gebleap(square sq_departure, numvec kbeg, numvec kend) {
  /* generate white leaper moves from vec[kbeg] to vec[kend] */

  square sq_arrival;

  numvec  k;
  for (k= kbeg; k<= kend; k++) {
    sq_arrival= sq_departure+vec[k];
    if (e[sq_arrival]<=vide)
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void genleap(square sq_departure, numvec kbeg, numvec kend) {
  /* generate black leaper moves from vec[kbeg] to vec[kend] */

  square sq_arrival;

  numvec  k;
  for (k= kbeg; k<= kend; k++) {
    sq_arrival= sq_departure+vec[k];
    if (e[sq_arrival]==vide || e[sq_arrival] >= roib)
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void geriderhopper(square   sq_departure,
                   numvec   kbeg, numvec    kend,
                   smallint run_up,
                   smallint jump,
                   couleur  camp)
{
  /* generate rider-hopper moves from vec[kbeg] to vec[kend] */

  piece   hurdle;
  square  sq_hurdle;

  square sq_arrival;

  numvec  k;
  for (k= kbeg; k <= kend; k++) {
    if (run_up) {
      /* run up of fixed length */
      /* contragrashopper type */
      sq_hurdle= sq_departure;
      if (run_up>1) {
        /* The run up is longer.
           Check if there is an obstacle between the hopper
           and the hurdle
        */
        smallint ran_up= run_up;
        while (--ran_up) {
          sq_hurdle += vec[k];
          if (e[sq_hurdle]!=vide)
            break;
        }
        if (ran_up) {
          /* there is an obstacle -> next line */
          continue;
        }
      }
      sq_hurdle+= vec[k];
      hurdle= e[sq_hurdle];
      if (hurdle==vide) {
        /* there is no hurdle -> next line */
        continue;
      }
    }
    else
      /* run up of flexible length
       * lion, grashopper type
       */
      finligne(sq_departure,vec[k],hurdle,sq_hurdle);

    if (hurdle!=obs) {
      sq_arrival= sq_hurdle;
      if (jump) {
        /* jump of fixed length */
        /* grashopper or grashopper-2 type */
        if (jump>1) {
          /* The jump up is a longer one.
             Check if there is an obstacle between
             the hurdle and the target square
          */
          smallint jumped= jump;
          while (--jumped) {
            sq_arrival+= vec[k];
            if (e[sq_arrival]!=vide)
              break;
          }
          if (jumped) {
            /* there is an obstacle -> next line */
            continue;
          }
        }
        sq_arrival+= vec[k];
        if ((rightcolor(e[sq_arrival],camp) || (e[sq_arrival]==vide))
            && hopimcheck(sq_departure,sq_arrival,sq_hurdle,vec[k]))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
      else {
        /* jump of flexible length */
        /* lion, contragrashopper type */
        sq_arrival+= vec[k];
        while (e[sq_arrival]==vide) {
          if (hopimcheck(sq_departure,sq_arrival,sq_hurdle,vec[k]))
            empile(sq_departure,sq_arrival,sq_arrival);
          sq_arrival+= vec[k];
        }
        
        if (rightcolor(e[sq_arrival],camp)
            && hopimcheck(sq_departure,sq_arrival,sq_hurdle,vec[k]))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

/* Generation of moves for Rose and derived pieces.
 * The circle lines are generated in a clever way (which leads to
 * simple code):
 *
 * At the positions vec_knight_start to
 * vec_knight_start+(vec_knight_end-vec_knight_start) of the array
 * vec, we have the knight vectors twice in a row, in the order of the
 * knight's wheel. Each rose line is generated by sequentally adding
 * vec[k1], vec[k1+1] ... vec[k1+(vec_knight_end-vec_knight_start)]
 * (or the same vectors in opposite order) to the initial square.
 *
 * In calls to the generator functions, k2 should either be 0 or
 * (vec_knight_end-vec_knight_start)+1, and delta_k +1 or -1,
 * respectively, to cause k2, k2+delta_k, ... to assume values 0 ...
 * (vec_knight_end-vec_knight_start) or
 * (vec_knight_end-vec_knight_start)+1 ... 1.
 */

static square generate_moves_on_circle_segment(square sq_departure,
                                               square sq_base,
                                               smallint k1,
                                               smallint *k2,
                                               smallint delta_k) {
  square sq_arrival= sq_base;
  do {
    sq_arrival+= vec[k1+*k2];
    *k2+= delta_k;
  } while (e[sq_arrival]==vide
           && testempile(sq_departure,sq_arrival,sq_arrival));

  return sq_arrival;
}

square fin_circle_line(square sq_departure,
                       numvec k1, numvec *k2, numvec delta_k) {
  square sq_result= sq_departure;
  do {
    sq_result+= vec[k1+*k2];
    *k2+= delta_k;
  } while (e[sq_result]==vide);

  return sq_result;
}

void grose(square sq_departure,
           numvec k1, numvec k2, numvec delta_k,
           couleur camp) {
  square sq_end= generate_moves_on_circle_segment(sq_departure,sq_departure,
                                                  k1,&k2,delta_k);
  if (rightcolor(e[sq_end],camp))
    testempile(sq_departure,sq_end,sq_end);
}

void grao(square sq_departure,
          numvec k1, numvec k2, numvec delta_k,
          couleur camp) {
  square sq_hurdle= generate_moves_on_circle_segment(sq_departure,sq_departure,
                                                     k1,&k2,delta_k);
  if (sq_hurdle!=sq_departure && e[sq_hurdle]!=obs) {
    square sq_arrival= fin_circle_line(sq_hurdle,k1,&k2,delta_k);
    if (rightcolor(e[sq_arrival],camp))

      testempile(sq_departure,sq_arrival,sq_arrival);
  }
}

void groselion(square sq_departure,
               numvec k1, numvec k2, numvec delta_k,
               couleur camp) {
  square sq_hurdle= fin_circle_line(sq_departure,k1,&k2,delta_k);
  if (sq_hurdle!=sq_departure && e[sq_hurdle]!=obs) {
#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    /* cf. issue 1747928 */
    /* temporarily remove the moving piece to prevent it from blocking
     * itself */
    piece save_piece = e[sq_departure];
    e[sq_departure] = vide;
#endif
    square sq_end= generate_moves_on_circle_segment(sq_departure,sq_hurdle,
                                                    k1,&k2,delta_k);
#if defined(ROSE_LION_HURDLE_CAPTURE_POSSIBLE)
    e[sq_departure] = save_piece;
#endif
    if (rightcolor(e[sq_end],camp))
      testempile(sq_departure,sq_end,sq_end);
  }
}

void grosehopper(square sq_departure,
                 numvec k1, numvec k2, numvec delta_k,
                 couleur camp) {
  square sq_hurdle= fin_circle_line(sq_departure,k1,&k2,delta_k);
  if (sq_hurdle!=sq_departure && e[sq_hurdle]!=obs) {
    square sq_arrival= sq_hurdle+vec[k1+k2];
    if (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
      testempile(sq_departure,sq_arrival,sq_arrival);
  }
}

void ghamst(square sq_departure) {
  piece hurdle;
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_arrival);
    if (hurdle!=obs) {
      sq_arrival-= vec[k];
      empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

void gmhop(square   sq_departure,
           numvec   kanf,
           numvec   kend,
           smallint m,
           couleur  camp)
{
  piece hurdle;
  square sq_hurdle;
  numvec k, k1;

  /* ATTENTION:
   *    m == 0: moose    45 degree hopper
   *    m == 1: eagle    90 degree hopper
   *    m == 2: sparrow 135 degree hopper
   *
   *    kend == 8, kanf == 1: queen types  (moose, eagle, sparrow)
   *    kend == 8, kanf == 5: bishop types
   *                  (bishopmoose, bishopeagle, bishopsparrow)
   *    kend == 4, kanf == 1: rook types
   *                  (rookmoose, rookeagle, rooksparrow)
   */

  /* different moves from one piece to the same square are possible.
   * so we have to use testempile !!
   */

  square sq_arrival;

  for (k= kend; k>=kanf; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    
    if (hurdle!=obs) {
      k1= k<<1;
      
      sq_arrival= sq_hurdle+mixhopdata[m][k1];
      if (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
        testempile(sq_departure,sq_arrival,sq_arrival);
      
      sq_arrival= sq_hurdle+mixhopdata[m][k1-1];
      if (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
        testempile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

static void generate_locust_capture(square sq_departure, square sq_capture,
                                    smallint k,
                                    couleur camp) {
  square sq_arrival;
  if (rightcolor(e[sq_capture],camp)) {
    sq_arrival= sq_capture+vec[k];
    if (e[sq_arrival]==vide)
      empile(sq_departure,sq_arrival,sq_capture);
  }
}

void glocust(square sq_departure,
             numvec kbeg,
             numvec kend,
             couleur    camp)
{
  piece hurdle;
  square sq_capture;

  numvec k;
  for (k= kbeg; k <= kend; k++) {
    finligne(sq_departure,vec[k],hurdle,sq_capture);
    generate_locust_capture(sq_departure,sq_capture,k,camp);
  }
} /* glocust */

void gmarin(square  sq_departure,
            numvec  kbeg,
            numvec  kend,
            couleur camp)
{
  /* generate marin-pieces moves from vec[kbeg] to vec[kend] */
  numvec k;
  square sq_capture;

  for (k= kbeg; k<=kend; k++) {
    sq_capture= generate_moves_on_line_segment(sq_departure,sq_departure,k);
    generate_locust_capture(sq_departure,sq_capture,k,camp);
  }
}

void gchin(square   sq_departure,
           numvec   kbeg, numvec    kend,
           couleur  camp)
{
  /* generate chinese-rider moves from vec[kbeg] to vec[kend] */

  piece   hurdle;
  numvec  k;

  square sq_arrival;

  for (k= kbeg; k<=kend; k++) {
    sq_arrival= generate_moves_on_line_segment(sq_departure,sq_departure,k);
    
    if (e[sq_arrival]!=obs) {
      finligne(sq_arrival,vec[k],hurdle,sq_arrival);
      if (rightcolor(hurdle,camp))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

void gnequi(square sq_departure, couleur camp) {
  /* Non-Stop-Equihopper */
  square sq_hurdle;
  square sq_arrival;
  numvec delta_horiz, delta_vert;
  numvec vector;

  square const coin= coinequis(sq_departure);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)
    
    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {

      sq_hurdle= coin+delta_horiz+delta_vert;

      if (e[sq_hurdle]!=vide) {

        vector= sq_hurdle-sq_departure;
        sq_arrival= sq_hurdle+vector;

        if ((e[sq_arrival]==vide
             || rightcolor(e[sq_arrival],camp))
            && hopimok(sq_departure,
                       sq_arrival,
                       sq_hurdle,
                       vector))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
}

void gnequiapp(square sq_departure, couleur camp) {
  /* Non-Stop-Equistopper */
  square sq_hurdle;
  square sq_arrival;
  numvec delta_horiz, delta_vert;
  numvec vector;

  square const coin= coinequis(sq_departure);

  for (delta_horiz= 3*dir_right;
       delta_horiz!=dir_left;
       delta_horiz+= dir_left)

    for (delta_vert= 3*dir_up;
         delta_vert!=dir_down;
         delta_vert+= dir_down) {

      sq_arrival= coin+delta_horiz+delta_vert;
      vector= sq_arrival-sq_departure;
      sq_hurdle= sq_arrival+vector;

      if (sq_arrival!=sq_departure
          && e[sq_hurdle]!=vide
          && (e[sq_arrival]==vide
              || rightcolor(e[sq_arrival],camp)))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
}

void gkang(square sq_departure, couleur camp) {
  piece hurdle;
  square sq_hurdle;
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {
      finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
      if (hurdle!=obs) {
        sq_arrival+= vec[k];
        if (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

void grabbit(square sq_departure, couleur camp) {
  piece hurdle;
  square sq_hurdle;
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k >=vec_queen_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {

      finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
      if (hurdle!=obs) {
        sq_arrival= generate_moves_on_line_segment(sq_departure,sq_arrival,k);
        if (rightcolor(e[sq_arrival],camp))
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

void gbob(square sq_departure, couleur camp) {
  piece hurdle;
  square sq_hurdle;
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {

      finligne(sq_hurdle,vec[k],hurdle,sq_hurdle);
      if (hurdle!=obs) {

        finligne(sq_hurdle,vec[k],hurdle,sq_hurdle);
        if (hurdle!=obs) {

          finligne(sq_hurdle,vec[k],hurdle,sq_arrival);
          if (hurdle!=obs) {

            sq_arrival= generate_moves_on_line_segment(sq_departure,
                                                       sq_arrival,
                                                       k);
            if (rightcolor(e[sq_arrival],camp))
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }
    }
  }
}

void gcs(square sq_departure,
         numvec k1, numvec k2,
         couleur camp)
{
  square sq_arrival= sq_departure+k1;

  while (e[sq_arrival]==vide) {
    testempile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k2;
    if (e[sq_arrival]==vide) {
      testempile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= k1;
    }
    else
      break;
  }
  if (rightcolor(e[sq_arrival],camp))
    testempile(sq_departure,sq_arrival,sq_arrival);
  
  sq_arrival= sq_departure+k1;
  while (e[sq_arrival]==vide) {
    testempile(sq_departure,sq_arrival,sq_arrival);
    sq_arrival-= k2;
    if (e[sq_arrival]==vide) {
      testempile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= k1;
    }
    else
      break;
  }
  if (rightcolor(e[sq_arrival],camp))
    testempile(sq_departure,sq_arrival,sq_arrival);
}

void gubi(square orig_departure,
          square step_departure,
          couleur camp) {
  numvec k;

  square sq_departure= orig_departure;
  square sq_arrival;

  e_ubi[step_departure]= obs;
  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    sq_arrival= step_departure+vec[k];
    if (e_ubi[sq_arrival]==vide) {
      empile(sq_departure,sq_arrival,sq_arrival);
      gubi(orig_departure,sq_arrival,camp);
    }
    else if (rightcolor(e_ubi[sq_arrival],camp))
      empile(sq_departure,sq_arrival,sq_arrival);
    e_ubi[sq_arrival]= obs;
  }
}

void grfou(square   orig_departure,
           square   in,
           numvec   k,
           smallint x,
           couleur  camp,
           evalfunction_t *generate)
{
  /* ATTENTION:
     if first call of x is 1 and boolnoedge[i]
     use empile() for generate,
     else
     use testempile() for generate !!
  */
  smallint k1;

  square sq_departure= orig_departure;
  square sq_arrival= in+k;

  if (e[sq_arrival]==obs)
    return;

  while (e[sq_arrival]==vide) {
    (*generate)(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k;
  }
  
  if (rightcolor(e[sq_arrival],camp))
    (*generate)(sq_departure,sq_arrival,sq_arrival);
  else if (x && e[sq_arrival]==obs) {
    sq_arrival-= k;
    k1= 5;
    while (vec[k1]!=k)
      k1++;
    k1*= 2;
    grfou(orig_departure,sq_arrival,mixhopdata[1][k1],x-1,camp,generate);
    k1--;
    grfou(orig_departure,sq_arrival,mixhopdata[1][k1],x-1,camp,generate);
  }
}

void gcard(square   orig_departure,
           square   in,
           numvec   k,
           smallint x,
           couleur  camp,
           evalfunction_t *generate)
{
  /* ATTENTION:
     if first call of x is 1
     use    empile() for generate,
     else
     use testempile() for generate !!
  */
  smallint k1;

  square sq_departure= orig_departure;
  square sq_arrival= in+k;

  while (e[sq_arrival]==vide) {
    (*generate)(sq_departure,sq_arrival,sq_arrival);
    sq_arrival+= k;
  }
  if (rightcolor(e[sq_arrival],camp))
    (*generate)(sq_departure,sq_arrival,sq_arrival);
  else if (x && e[sq_arrival]==obs) {
    for (k1= 1; k1<=4; k1++) {
      if (e[sq_arrival+vec[k1]]!=obs) {
        break;
      }
    }
    if (k1<=4) {
      sq_arrival+= vec[k1];
      if (rightcolor(e[sq_arrival],camp)) {
        (*generate)(sq_departure,sq_arrival,sq_arrival);
      }
      else if (e[sq_arrival]==vide) {
        (*generate)(sq_departure,sq_arrival,sq_arrival);
        k1= 5;
        while (vec[k1]!=k)
          k1++;
        k1*= 2;
        if (e[sq_arrival+mixhopdata[1][k1]]==obs)
          k1--;

        gcard(orig_departure,
              sq_arrival,
              mixhopdata[1][k1],
              x-1,
              camp,
              generate);
      }
    }
  }
}

void  grefc(square orig_departure,
            square step_departure,
            smallint x,
            couleur camp) {
  numvec k;

  /* ATTENTION:   first call of grefcn: x must be 2 !!   */

  square sq_departure= orig_departure;
  square sq_arrival;

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    if (x) {
      sq_arrival= step_departure+vec[k];
      if (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        if (!NoEdge(sq_arrival))
          grefc(orig_departure,sq_arrival,x-1,camp);
      }
      else if (rightcolor(e[sq_arrival], camp))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
    else
      for (k= vec_knight_start; k<=vec_knight_end; k++) {
        sq_arrival= step_departure+vec[k];
        if (e[sq_arrival]==vide
            || rightcolor(e[sq_arrival],camp))
          testempile(sq_departure,sq_arrival,sq_arrival);
      }
  }
} /* grefc */

void gequi(square sq_departure, couleur camp) {
  /* Equihopper */
  numvec  k;
  piece   hurdle;
  square  sq_hurdle, end_of_line;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {     /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {
      finligne(sq_hurdle,vec[k],hurdle,end_of_line);
      {
        smallint const dist_hurdle_end= abs(end_of_line-sq_hurdle);
        smallint const dist_hurdle_dep= abs(sq_hurdle-sq_departure);
        if (dist_hurdle_end>dist_hurdle_dep) {
          sq_arrival= sq_hurdle+sq_hurdle-sq_departure;
          if (hopimcheck(sq_departure,sq_arrival,sq_hurdle,vec[k]))
            empile(sq_departure,sq_arrival,sq_arrival);
        }
        else if (dist_hurdle_end==dist_hurdle_dep) {
          sq_arrival= end_of_line;
          if (rightcolor(hurdle,camp)
              && hopimcheck(sq_departure,sq_arrival,sq_hurdle,vec[k]))
            empile(sq_departure,sq_arrival,sq_arrival);
        }
      }
    }
  }
  
  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++) {     /* 2,4; 2,6; 4,6; */
    sq_hurdle= sq_departure+vec[k];
    sq_arrival= sq_departure + 2*vec[k];
    if (abs(e[sq_hurdle])>=roib
        && (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
        && hopimcheck(sq_departure,sq_arrival,sq_hurdle,vec[k]))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void gequiapp(square sq_departure, couleur camp) {
  /* (interceptable) Equistopper */
  numvec  k;
  piece   hurdle;
  square  sq_hurdle1, sq_hurdle2;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {     /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
    finligne(sq_departure,vec[k],hurdle,sq_hurdle1);
    if (hurdle!=obs) {
      sq_arrival= (sq_hurdle1+sq_departure)/2;
      if (!((sq_hurdle1/onerow+sq_departure/onerow)%2
            || (sq_hurdle1%onerow+sq_departure%onerow)%2)) /* is sq_arrival a square? */ 
        empile(sq_departure,sq_arrival,sq_arrival);

      finligne(sq_hurdle1,vec[k],hurdle,sq_hurdle2);
      if (hurdle!=obs
          && (abs(sq_hurdle2-sq_hurdle1)
              ==abs(sq_hurdle1-sq_departure))
          && rightcolor(hurdle,camp)) {
        sq_arrival= sq_hurdle1;
        empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
  for (k= vec_equi_nonintercept_start; k<=vec_equi_nonintercept_end; k++) {     /* 2,4; 2,6; 4,6; */
    sq_arrival= sq_departure + vec[k];
    sq_hurdle1= sq_departure+2*vec[k];
    if (abs(e[sq_hurdle1])>=roib
        && (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp)))
      empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void gcat(square sq_departure, couleur camp) {
  /* generate moves of a CAT */
  numvec  k;

  square sq_arrival;

  for (k= vec_knight_start; k<=vec_knight_end; k++) {
    sq_arrival= sq_departure+vec[k];
    if (rightcolor(e[sq_arrival], camp))
      empile(sq_departure,sq_arrival,sq_arrival);
    else {
      while (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= mixhopdata[3][k];
      }

      if (rightcolor(e[sq_arrival], camp))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
}

void gmaooa(square  sq_departure,
            square  pass,
            square  arrival1,
            square  arrival2,
            couleur camp)
{
  if (e[pass] == vide) {
    if (e[arrival1]==vide || rightcolor(e[arrival1],camp))
      if (maooaimcheck(sq_departure,arrival1,pass))
        empile(sq_departure,arrival1,arrival1);
    
    if (e[arrival2]==vide || rightcolor(e[arrival2],camp))
      if (maooaimcheck(sq_departure,arrival2,pass))
        empile(sq_departure,arrival2,arrival2);
  }
}

void gmao(square i, couleur camp) {
  gmaooa(i, i+dir_up, i+2*dir_up+dir_left, i+2*dir_up+dir_right, camp);
  gmaooa(i, i+dir_down, i+2*dir_down+dir_right, i+2*dir_down+dir_left, camp);
  gmaooa(i, i+dir_right, i+dir_up+2*dir_right, i+dir_down+2*dir_right, camp);
  gmaooa(i, i+dir_left, i+dir_down+2*dir_left, i+dir_up+2*dir_left, camp);
}

void gmoa(square i, couleur camp) {
  gmaooa(i, i+dir_up+dir_left, i+2*dir_up+dir_left, i+dir_up+2*dir_left, camp);
  gmaooa(i, i+dir_down+dir_right, i+2*dir_down+dir_right, i+dir_down+2*dir_right, camp);
  gmaooa(i, i+dir_up+dir_right, i+dir_up+2*dir_right, i+2*dir_up+dir_right, camp);
  gmaooa(i, i+dir_down+dir_left, i+dir_down+2*dir_left, i+2*dir_down+dir_left, camp);
}

void gdoubleg(square sq_departure, couleur camp) {
  numvec k,k1;
  piece hurdle;
  square sq_hurdle, past_sq_hurdle;

  square sq_arrival;

  for (k=vec_queen_end; k>=vec_queen_start; k--) {
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {
      past_sq_hurdle= sq_hurdle+vec[k];
      if (e[past_sq_hurdle]==vide)
        for (k1=vec_queen_end; k1>=vec_queen_start; k1--) {
          finligne(past_sq_hurdle,vec[k1],hurdle,sq_hurdle);
          if (hurdle!=obs) {
            sq_arrival= sq_hurdle+vec[k1];
            if (e[sq_arrival]==vide || rightcolor(e[sq_arrival],camp))
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
    }
  }
}

typedef enum
{
  UP,
  DOWN
} UPDOWN;

void filter(square i, numecoup prevnbcou, UPDOWN u)
{
  numecoup s = prevnbcou+1;
  while (s<=nbcou)
    if ((u==DOWN && move_generation_stack[s].arrival-i>-8)
        || (u==UP && move_generation_stack[s].arrival-i<8))
    {
      memmove(move_generation_stack+s,
              move_generation_stack+s+1,
              (nbcou-s) * sizeof move_generation_stack[s]);
      --nbcou;
    }
    else
      ++s;
}

void genhunt(square i, piece p, PieNam pabs)
{
  /*   PieNam const pabs = abs(p);  */
  assert(pabs>=Hunter0);
  assert(pabs<Hunter0+maxnrhuntertypes);
  
  {
    unsigned int const typeofhunter = pabs-Hunter0;
    HunterType const * const huntertype = huntertypes+typeofhunter;

    if (p>0) {
      numecoup savenbcou = nbcou;
      gen_wh_piece(i,huntertype->home);
      filter(i,savenbcou,DOWN);

      savenbcou = nbcou;
      gen_wh_piece(i,huntertype->away);
      filter(i,savenbcou,UP);
    }
    else {
      numecoup savenbcou = nbcou;
      gen_bl_piece(i,-huntertype->away);
      filter(i,savenbcou,DOWN);

      savenbcou = nbcou;
      gen_bl_piece(i,-huntertype->home);
      filter(i,savenbcou,UP);
    }
  }
}

void gfeerrest(square i, piece p, couleur camp) {
  numvec k;
  square *bnp;
  PieNam const pabs = abs(p);

  switch  (abs(p)) {
  case maob:
    gmao(i, camp);
    return;

  case paob:
    gchin(i, vec_rook_start,vec_rook_end, camp);
    return;

  case leob:
    gchin(i, vec_queen_start,vec_queen_end, camp);
    return;

  case vaob:
    gchin(i, vec_bishop_start,vec_bishop_end, camp);
    return;
  case naob:
    gchin(i, vec_knight_start,vec_knight_end, camp);
    return;

  case roseb:
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      grose(i, k, 0,+1, camp);
      grose(i, k, vec_knight_end-vec_knight_start+1,-1, camp);
    }
    return;

  case nequib:
    gnequi(i, camp);
    return;

  case locb:
    glocust(i, vec_queen_start,vec_queen_end, camp);
    return;

  case nightlocustb:
    glocust(i, vec_knight_start,vec_knight_end, camp);
    return;

  case bishoplocustb:
    glocust(i, vec_bishop_start,vec_bishop_end, camp);
    return;

  case rooklocustb:
    glocust(i, vec_rook_start,vec_rook_end, camp);
    return;

  case kangoub:
    gkang(i, camp);
    return;

  case csb:
    for (k= vec_knight_start; k<=vec_knight_end; k++)
      gcs(i, vec[k], vec[25 - k], camp);
    return;

  case hamstb:
    ghamst(i);
    return;

  case ubib:
    for (bnp= boardnum; *bnp; bnp++)
      e_ubi[*bnp]= e[*bnp];
    gubi(i, i, camp);
    return;

  case mooseb:
    gmhop(i, vec_queen_start,vec_queen_end, 0, camp);
    return;

  case eagleb:
    gmhop(i, vec_queen_start,vec_queen_end, 1, camp);
    return;

  case sparrb:
    gmhop(i, vec_queen_start,vec_queen_end, 2, camp);
    return;

  case margueriteb:
    gmhop(i, vec_queen_start,vec_queen_end, 0, camp);
    gmhop(i, vec_queen_start,vec_queen_end, 1, camp);
    gmhop(i, vec_queen_start,vec_queen_end, 2, camp);
    gerhop(i, vec_queen_start,vec_queen_end, camp);
    ghamst(i);
    return;

  case archb:
    if (NoEdge(i)) {
      for (k= vec_bishop_start; k <= vec_bishop_end; k++)
        grfou(i, i, vec[k], 1, camp, empile);
    }
    else {
      for (k= vec_bishop_start; k <= vec_bishop_end; k++)
        grfou(i, i, vec[k], 1, camp, testempile);
    }
    return;

  case reffoub:
    for (k= vec_bishop_start; k <= vec_bishop_end; k++)
      grfou(i, i, vec[k], 4, camp, testempile);
    return;

  case cardb:
    for (k= vec_bishop_start; k <= vec_bishop_end; k++)
      gcard(i, i, vec[k], 1, camp, empile);
    return;

  case dcsb:
    for (k= 9; k <= 14; k++)
      gcs(i, vec[k], vec[23 - k], camp);
    for (k= 15; k <= 16; k++)
      gcs(i, vec[k], vec[27 - k], camp);
    return;

  case refcb:
    grefc(i, i, 2, camp);
    return;

  case equib:
    gequi(i, camp);
    return;

  case catb:
    gcat(i, camp);
    return;

  case sireneb:
    gmarin(i, vec_queen_start,vec_queen_end, camp);
    return;

  case tritonb:
    gmarin(i, vec_rook_start,vec_rook_end, camp);
    return;

  case nereidb:
    gmarin(i, vec_bishop_start,vec_bishop_end, camp);
    return;

  case orphanb:
    gorph(i, camp);
    return;

  case friendb:
    gfriend(i, camp);
    return;

  case edgehb:
    gedgeh(i, camp);
    return;

  case moab:
    gmoa(i, camp);
    return;

  case moaridb:
    gemoarider(i, camp);
    return;

  case maoridb:
    gemaorider(i, camp);
    return;

  case bscoutb:
    for (k= vec_bishop_start; k <= vec_bishop_end; k++)
      gcs(i, vec[k], vec[13 - k], camp);
    return;

  case gscoutb:
    for (k= vec_rook_end; k >=vec_rook_start; k--)
      gcs(i, vec[k], vec[5 - k], camp);
    return;

  case skyllab:
    geskylla(i, camp);
    return;

  case charybdisb:
    gecharybdis(i, camp);
    return;

  case andergb:
  case sb:
    gerhop(i, vec_queen_start,vec_queen_end, camp);
    return;

  case lionb:
    gelrhop(i, vec_queen_start,vec_queen_end, camp);
    return;

  case nsautb:
    gerhop(i, vec_knight_start,vec_knight_end, camp);
    return;

  case camhopb:
    gerhop(i, vec_chameau_start,vec_chameau_end, camp);
    return;

  case zebhopb:
    gerhop(i, vec_zebre_start,vec_zebre_end, camp);
    return;

  case gnuhopb:
    gerhop(i, vec_chameau_start,vec_chameau_end, camp);
    gerhop(i, vec_knight_start,vec_knight_end, camp);
    return;

  case tlionb:
    gelrhop(i, vec_rook_start,vec_rook_end, camp);
    return;

  case flionb:
    gelrhop(i, vec_bishop_start,vec_bishop_end, camp);
    return;

  case rookhopb:
    gerhop(i, vec_rook_start,vec_rook_end, camp);
    return;

  case bishophopb:
    gerhop(i, vec_bishop_start,vec_bishop_end, camp);
    return;
  case contragrasb:
    gecrhop(i, vec_queen_start,vec_queen_end, camp);
    return;

  case roselionb:
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      groselion(i, k, 0,+1, camp);
      groselion(i, k, vec_knight_end-vec_knight_start+1,-1, camp);
    }
    return;

  case rosehopperb:
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      grosehopper(i, k, 0,+1, camp);
      grosehopper(i, k, vec_knight_end-vec_knight_start+1,-1, camp);
    }
    return;

  case g2b:
    gerhop2(i, vec_queen_start,vec_queen_end, camp);
    return;

  case g3b:
    gerhop3(i, vec_queen_start,vec_queen_end, camp);
    return;
  case khb:
    geshop(i, vec_queen_start,vec_queen_end, camp);
    return;

  case doublegb:
    gdoubleg(i, camp);
    return;

  case orixb:
    gorix(i, camp);
    return;

  case gralb:
    if (camp==blanc)
      gebleap(i, vec_alfil_start,vec_alfil_end);    /* alfilb */
    else
      genleap(i, vec_alfil_start,vec_alfil_end);    /* alfiln */
    gerhop(i, vec_rook_start,vec_rook_end, camp);      /* rookhopper */
    return;

  case rookmooseb:
    gmhop(i, vec_rook_start,vec_rook_end, 0, camp);
    return;

  case rookeagleb:
    gmhop(i, vec_rook_start,vec_rook_end, 1, camp);
    return;

  case rooksparrb:
    gmhop(i, vec_rook_start,vec_rook_end, 2, camp);
    return;

  case bishopmooseb:
    gmhop(i, vec_bishop_start,vec_bishop_end, 0, camp);
    return;

  case bishopeagleb:
    gmhop(i, vec_bishop_start,vec_bishop_end, 1, camp);
    return;

  case bishopsparrb:
    gmhop(i, vec_bishop_start,vec_bishop_end, 2, camp);
    return;

  case raob:
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      grao(i, k, 0,+1, camp);
      grao(i, k, vec_knight_end-vec_knight_start+1,-1, camp);
    }
    return;

  case scorpionb:
    if (camp==blanc)
      gebleap(i, vec_queen_start,vec_queen_end);        /* ekingb */
    else
      genleap(i, vec_queen_start,vec_queen_end);        /* ekingn */
    gerhop(i, vec_queen_start,vec_queen_end, camp);     /* grashopper */
    return;

  case nrlionb:
    gelrhop(i, vec_knight_start,vec_knight_end, camp);
    return;

  case mrlionb:
    gemaoriderlion(i, camp);
    return;

  case molionb:
    gemoariderlion(i, camp);
    return;

  case dolphinb:
    gkang(i, camp);
    gerhop(i, vec_queen_start,vec_queen_end, camp);
    return;

  case rabbitb:
    grabbit(i, camp);
    return;

  case bobb:
    gbob(i, camp);
    return;

  case equiengb:
    gequiapp(i, camp);
    return;

  case equifrab:
    gnequiapp(i, camp);
    return;

  case querqub:
    switch (i%onerow - nr_of_slack_files_left_of_board) {
    case file_rook_queenside:
    case file_rook_kingside:    if (camp == blanc)
        gebrid(i, vec_rook_start,vec_rook_end);
      else
        genrid(i, vec_rook_start,vec_rook_end);
      break;
    case file_bishop_queenside:
    case file_bishop_kingside:  if (camp == blanc)
        gebrid(i, vec_bishop_start,vec_bishop_end);
      else
        genrid(i, vec_bishop_start,vec_bishop_end);
      break;
    case file_queen:    if (camp == blanc)
        gebrid(i, vec_queen_start,vec_queen_end);
      else
        genrid(i, vec_queen_start,vec_queen_end);
      break;
    case file_knight_queenside:
    case file_knight_kingside:  if (camp == blanc)
        gebleap(i, vec_knight_start,vec_knight_end);
      else
        genleap(i, vec_knight_start,vec_knight_end);
      break;
    case file_king: if (camp == blanc)
        gebleap(i, vec_queen_start,vec_queen_end);
      else
        genleap(i, vec_queen_start,vec_queen_end);
      break;
    }
    break;

  case bouncerb : 
    genbouncer(i, vec_queen_start,vec_queen_end, camp);
    break;

  case rookbouncerb : 
    genbouncer(i, vec_rook_start,vec_rook_end, camp);
    break;

  case bishopbouncerb : 
    genbouncer(i, vec_bishop_start,vec_bishop_end, camp);
    break;

  case radialknightb :
    genradialknight(i, camp);
    break;

  default:
    /* Since pieces like DUMMY fall through 'default', we have */
    /* to check exactly if there is something to generate ...  */
    if ((pabs>=Hunter0) && (pabs<Hunter0+maxnrhuntertypes))
      genhunt(i,p,pabs);
  }
} /* gfeerrest */

/* Two auxiliary functions for generating super pawn moves */
void gen_sp_nocaptures(square sq_departure, numvec dir) {
  /* generates non capturing moves of a super pawn in direction dir */

  square sq_arrival= sq_departure+dir;

  /* it can move from first rank */
  for (; e[sq_arrival]==vide; sq_arrival+= dir)
    empile(sq_departure,sq_arrival,sq_arrival);
}

void gen_sp_captures(square sq_departure, numvec dir, couleur camp) {
  /* generates capturing moves of a super pawn of colour camp in
     direction dir.  */

  piece   hurdle;

  square sq_arrival;

  /* it can move from first rank */
  finligne(sq_departure,dir,hurdle,sq_arrival);
  if (rightcolor(hurdle,camp))
    empile(sq_departure,sq_arrival,sq_arrival);
}

void gencpn(square i) {
  genleap(i, 4, 4);
  if (2*i < haut+bas) {
    genleap(i, 1, 1);
    genleap(i, 3, 3);
  }
}

void gencpb(square i) {
  gebleap(i, 2, 2);
  if (2*i > haut+bas) {
    gebleap(i, 1, 1);
    gebleap(i, 3, 3);
  }
}

void gfeerblanc(square i, piece p) {
  testdebut= nbcou;
  switch(p) {
  case nb:
    gebrid(i, vec_knight_start,vec_knight_end);
    return;

  case zb:
    gebleap(i, vec_zebre_start,vec_zebre_end);
    return;

  case chb:
    gebleap(i, vec_chameau_start,vec_chameau_end);
    return;

  case gib:
    gebleap(i, vec_girafe_start,vec_girafe_end);
    return;

  case rccinqb:
    gebleap(i, vec_rccinq_start,vec_rccinq_end);
    return;

  case bub:
    gebleap(i, vec_bucephale_start,vec_bucephale_end);
    return;

  case vizirb:
    gebleap(i, vec_rook_start,vec_rook_end);
    return;

  case alfilb:
    gebleap(i, vec_alfil_start,vec_alfil_end);
    return;

  case fersb:
    gebleap(i, vec_bishop_start,vec_bishop_end);
    return;

  case dabb:
    gebleap(i, vec_dabbaba_start,vec_dabbaba_end);
    return;

  case pbb:
    genpbb(i);
    return;

  case reversepb:
    genreversepb(i);
    return;

  case amazb:
    gebrid(i, vec_queen_start,vec_queen_end);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case impb:
    gebrid(i, vec_rook_start,vec_rook_end);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case princb:
    gebrid(i, vec_bishop_start,vec_bishop_end);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case gnoub:
    gebleap(i, vec_chameau_start,vec_chameau_end);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case antilb:
    gebleap(i, vec_antilope_start,vec_antilope_end);
    return;

  case ecurb:
    gebleap(i, vec_ecureuil_start,vec_ecureuil_end);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case waranb:
    gebrid(i, vec_knight_start,vec_knight_end);
    gebrid(i, vec_rook_start,vec_rook_end);
    return;

  case dragonb:
    genpb(i);
    gebleap(i, vec_knight_start,vec_knight_end);
    return;

  case camridb:
    gebrid(i, vec_chameau_start,vec_chameau_end);
    return;

  case zebridb:
    gebrid(i, vec_zebre_start,vec_zebre_end);
    return;

  case gnuridb:
    gebrid(i, vec_chameau_start,vec_chameau_end);
    gebrid(i, vec_knight_start,vec_knight_end);
    return;

  case bspawnb:
    gen_sp_nocaptures(i,+dir_up+dir_left);
    gen_sp_nocaptures(i,+dir_up+dir_right);
    gen_sp_captures(i,+dir_up, blanc);
    return;

  case spawnb:
    gen_sp_nocaptures(i,+dir_up);
    gen_sp_captures(i,+dir_up+dir_left, blanc);
    gen_sp_captures(i,+dir_up+dir_right, blanc);
    return;

  case rhuntb:
    gebrid(i, 2, 2);
    gebrid(i, 7, 8);
    return;

  case bhuntb:
    gebrid(i, 4, 4);
    gebrid(i, 5, 6);
    return;

  case ekingb:
    gebleap(i, vec_queen_start,vec_queen_end);
    return;

  case okapib:
    gebleap(i, vec_okapi_start,vec_okapi_end);
    return;

  case leap37b:
    gebleap(i, vec_leap37_start,vec_leap37_end);
    return;

  case leap16b:
    gebleap(i, vec_leap16_start,vec_leap16_end);
    return;

  case leap24b:
    gebleap(i, vec_leap24_start,vec_leap24_end);
    return;

  case leap35b:
    gebleap(i, vec_leap35_start,vec_leap35_end);
    return;

  case leap15b:
    gebleap(i, vec_leap15_start,vec_leap15_end);
    return;

  case leap25b:
    gebleap(i, vec_leap25_start,vec_leap25_end);
    return;

  case vizridb:
    gebrid(i,   vec_rook_start,vec_rook_end);
    return;

  case fersridb:
    gebrid(i, vec_bishop_start,vec_bishop_end);
    return;

  case bisonb:
    gebleap(i, vec_bison_start,vec_bison_end);
    return;

  case elephantb:
    gebrid(i, vec_elephant_start,vec_elephant_end);
    return;

  case leap36b:
    gebleap(i, vec_leap36_start,vec_leap36_end);
    return;

  case chinesepawnb:
    gencpb(i);
    return;

  default:
    gfeerrest(i, p, blanc);
  }
}

void gfeernoir(square i, piece p) {
  testdebut= nbcou;
  switch(p) {
  case nn:
    genrid(i, vec_knight_start,vec_knight_end);
    return;

  case zn:
    genleap(i, vec_zebre_start,vec_zebre_end);
    return;

  case chn:
    genleap(i, vec_chameau_start,vec_chameau_end);
    return;

  case gin:
    genleap(i, vec_girafe_start,vec_girafe_end);
    return;

  case rccinqn:
    genleap(i, vec_rccinq_start,vec_rccinq_end);
    return;

  case bun:
    genleap(i, vec_bucephale_start,vec_bucephale_end);
    return;

  case vizirn:
    genleap(i, vec_rook_start,vec_rook_end);
    return;

  case alfiln:
    genleap(i, vec_alfil_start,vec_alfil_end);
    return;

  case fersn:
    genleap(i, vec_bishop_start,vec_bishop_end);
    return;

  case dabn:
    genleap(i, vec_dabbaba_start,vec_dabbaba_end);
    return;

  case pbn:
    genpbn(i);
    return;

  case reversepn:
    genreversepn(i);
    return;

  case amazn:
    genrid(i, vec_queen_start,vec_queen_end);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case impn:
    genrid(i, vec_rook_start,vec_rook_end);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case princn:
    genrid(i, vec_bishop_start,vec_bishop_end);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case gnoun:
    genleap(i, vec_chameau_start,vec_chameau_end);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case antiln:
    genleap(i, vec_antilope_start,vec_antilope_end);
    return;

  case ecurn:
    genleap(i, vec_ecureuil_start,vec_ecureuil_end);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case warann:
    genrid(i, vec_knight_start,vec_knight_end);
    genrid(i, vec_rook_start,vec_rook_end);
    return;

  case dragonn:
    genpn(i);
    genleap(i, vec_knight_start,vec_knight_end);
    return;

  case camridn:
    genrid(i, vec_chameau_start,vec_chameau_end);
    return;

  case zebridn:
    genrid(i, vec_zebre_start,vec_zebre_end);
    return;

  case gnuridn:
    genrid(i, vec_chameau_start,vec_chameau_end);
    genrid(i, vec_knight_start,vec_knight_end);
    return;

  case bspawnn:
    gen_sp_nocaptures(i,+dir_down+dir_right);
    gen_sp_nocaptures(i,+dir_down+dir_left);
    gen_sp_captures(i,+dir_down, noir);
    return;

  case spawnn:
    gen_sp_nocaptures(i,+dir_down);
    gen_sp_captures(i,+dir_down+dir_right, noir);
    gen_sp_captures(i,+dir_down+dir_left, noir);
    return;

  case rhuntn:
    genrid(i, 2, 2);
    genrid(i, 7, 8);
    return;

  case bhuntn:
    genrid(i, 4, 4);
    genrid(i, 5, 6);
    return;

  case ekingn:
    genleap(i, vec_queen_start,vec_queen_end);
    return;

  case okapin:
    genleap(i, vec_okapi_start,vec_okapi_end);
    return;

  case leap37n:
    genleap(i, vec_leap37_start,vec_leap37_end);
    return;

  case leap16n:
    genleap(i, vec_leap16_start,vec_leap16_end);
    return;

  case leap24n:
    genleap(i, vec_leap24_start,vec_leap24_end);
    return;

  case leap35n:
    genleap(i, vec_leap35_start,vec_leap35_end);
    return;

  case leap15n:
    genleap(i, vec_leap15_start,vec_leap15_end);
    return;

  case leap25n:
    genleap(i, vec_leap25_start,vec_leap25_end);
    return;

  case vizridn:
    genrid(i, vec_rook_start,vec_rook_end);
    return;

  case fersridn:
    genrid(i, vec_bishop_start,vec_bishop_end);
    return;

  case bisonn:
    genleap(i, vec_bison_start,vec_bison_end);
    return;

  case elephantn:
    genrid(i, vec_elephant_start,vec_elephant_end);
    return;

  case leap36n:
    genleap(i, vec_leap36_start,vec_leap36_end);
    return;

  case chinesepawnn:
    gencpn(i);
    return;

  default:
    gfeerrest(i, p, noir);
  }
} /* end of gfeernoir */

void genrb_cast(void) {
  /* It works only for castling_supported == TRUE
     have a look at funtion verifieposition() in py6.c
  */

  boolean is_castling_possible;

  if (dont_generate_castling)
    return;

  if (TSTFLAGMASK(castling_flag[nbply],wh_castlings)>ke1_cancastle
      && e[square_e1]==roib 
      /* then the king on e1 and at least one rook can castle !! */
      && !echecc(blanc))
  {
    /* 0-0 */
    if (TSTFLAGMASK(castling_flag[nbply],whk_castling)==whk_castling
        && e[square_h1]==tb
        && e[square_f1]==vide
        && e[square_g1]==vide)
    {
      if (complex_castling_through_flag)
      {
        numecoup sic_nbcou= nbcou;

        /* temporarily deactivate maximummer etc. */
        boolean sic_flagwhitemummer = flagwhitemummer;
        flagwhitemummer = false;
        empile(square_e1,square_f1,square_f1);
        flagwhitemummer = sic_flagwhitemummer;
        if (nbcou>sic_nbcou)
        {
          boolean ok= jouecoup() && !echecc(blanc);
          repcoup();
          if (ok)
            empile(square_e1,square_g1,kingside_castling);
        }
      }
      else
      {
        e[square_e1]= vide;
        e[square_f1]= roib;
        rb= square_f1;

        is_castling_possible= !echecc(blanc);

        e[square_e1]= roib;
        e[square_f1]= vide;
        rb= square_e1;

        if (is_castling_possible)
          empile(square_e1,square_g1,kingside_castling);
      }
    }

    /* 0-0-0 */
    if (TSTFLAGMASK(castling_flag[nbply],whq_castling)==whq_castling
        && e[square_a1]==tb
        && e[square_d1]==vide
        && e[square_c1]==vide
        && e[square_b1]==vide)
    {
      if (complex_castling_through_flag)
      {
        numecoup sic_nbcou= nbcou;

        /* temporarily deactivate maximummer etc. */
        boolean sic_flagwhitemummer = flagwhitemummer;
        flagwhitemummer = false;
        empile(square_e1,square_d1,square_d1);
        flagwhitemummer = sic_flagwhitemummer;
        if (nbcou>sic_nbcou)
        {
          boolean ok= (jouecoup() && !echecc(blanc));
          repcoup();
          if (ok)
            empile(square_e1,square_c1,queenside_castling);
        }
      }
      else
      {
        e[square_e1]= vide;
        e[square_d1]= roib;
        rb= square_d1;
        
        is_castling_possible= !echecc(blanc);
        
        e[square_e1]= roib;
        e[square_d1]= vide;
        rb= square_e1;

        if (is_castling_possible)
          empile(square_e1,square_c1,queenside_castling);
      }
    }
  }
} /* genrb_cast */

void genrb(square sq_departure) {
  numvec    k;
  boolean   flag = false;       /* K im Schach ? */
  numecoup  anf, l1, l2;

  square sq_arrival;

  VARIABLE_INIT(anf);

  if (calc_whrefl_king && !calctransmute) {
    /* K im Schach zieht auch */
    piece   *ptrans;

    anf= nbcou;
    calctransmute= true;
    if (!whitenormaltranspieces && echecc(blanc))
    {
      for (ptrans= whitetransmpieces; *ptrans; ptrans++) {
        flag = true;
        current_trans_gen=*ptrans;
        gen_wh_piece(sq_departure,*ptrans);
        current_trans_gen=vide;
      }
    }
    else if (whitenormaltranspieces)
    {
      for (ptrans= whitetransmpieces; *ptrans; ptrans++) {
        if (nbpiece[-*ptrans]
            && (*checkfunctions[*ptrans])(sq_departure,-*ptrans,eval_white))
        {
          flag = true;
          current_trans_gen=*ptrans;
          gen_wh_piece(sq_departure,*ptrans);
          current_trans_gen=vide;
        }
      }
    }
    calctransmute= false;

    if (flag && nbpiece[orphann]) {
      piece king= e[rb];
      e[rb]= dummyb;
      if (!echecc(blanc)) {
        /* white king checked only by an orphan
        ** empowered by the king */
        flag= false;
      }
    e[rb]= king;
  }

  /* K im Schach zieht nur */
  if (calc_whtrans_king && flag)
    return;
  }

  if (CondFlag[sting])
    gerhop(sq_departure,vec_queen_start,vec_queen_end,blanc);

  for (k= vec_queen_end; k >=vec_queen_start; k--) {
    sq_arrival= sq_departure+vec[k];
    if (e[sq_arrival] <= vide)
      empile(sq_departure,sq_arrival,sq_arrival);
  }
  
  if (flag) {
    /* testempile nicht nutzbar */
    /* VERIFY: has anf always a propper value??
     */
    for (l1= anf+1; l1<=nbcou; l1++)
      if (move_generation_stack[l1].arrival != initsquare)
        for (l2= l1+1; l2<=nbcou; l2++)
          if (move_generation_stack[l1].arrival
              ==move_generation_stack[l2].arrival)
            move_generation_stack[l2].arrival= initsquare;
  }

  /* Now we test castling */
  if (castling_supported)
    genrb_cast();
}

void gen_wh_ply(void) {
  square i, j, z;
  piece p;

  /* Don't try to "optimize" by hand. The double-loop is tested as
     the fastest way to compute (due to compiler-optimizations !)
     V3.14  NG
  */
  z= bas;
  for (i= nr_rows_on_board; i > 0; i--, z+= onerow-nr_files_on_board)
    for (j= nr_files_on_board; j > 0; j--, z++) {
      if ((p = e[z]) != vide) {
        if (TSTFLAG(spec[z], Neutral))
          p = -p;
        if (p > obs)
          gen_wh_piece(z, p);
      }
    }
}

void gen_wh_piece_aux(square z, piece p) {

  if (CondFlag[annan]) {
    piece annan_p= e[z-onerow];
    if (whannan(z-onerow, z))
      p= annan_p;
  }

  switch(p) {
  case roib:
    genrb(z);
    break;

  case pb:
    genpb(z);
    break;

  case cb:
    gebleap(z, vec_knight_start,vec_knight_end);
    break;

  case tb:
    gebrid(z, vec_rook_start,vec_rook_end);
    break;

  case db:
    gebrid(z, vec_queen_start,vec_queen_end);
    break;

  case fb:
    gebrid(z, vec_bishop_start,vec_bishop_end);
    break;

  default:
    gfeerblanc(z, p);
    break;
  }
}

static void orig_gen_wh_piece(square sq_departure, piece p) {
  piece pp;


  if (flag_madrasi) {
    if (!libre(sq_departure, true)) {
      return;
    }
  }

  if (TSTFLAG(PieSpExFlags,Paralyse)) {
    if (paralysiert(sq_departure)) {
      return;
    }
  }

  if (anymars||anyantimars) {
    square mren;
    Flags psp;

    if (CondFlag[phantom]) {
      numecoup      anf1, anf2, l1, l2;
      anf1= nbcou;
      /* generate standard moves first */
      flagactive= false;
      flagpassive= false;
      flagcapture= false;

      gen_wh_piece_aux(sq_departure,p);

      /* Kings normally don't move from their rebirth-square */
      if (p == e[rb] && !rex_phan) {
        return;
      }
      /* generate moves from rebirth square */
      flagactive= true;
      psp= spec[sq_departure];
      mren= (*marsrenai)(p,psp,sq_departure,initsquare,initsquare,noir);
      /* if rebirth square is where the piece stands,
         we've already generated all the relevant moves.
      */
      if (mren==sq_departure) {
        return;
      }
      if (e[mren] == vide) {
        anf2= nbcou;
        pp=e[sq_departure];      /* Mars/Neutral bug */
        e[sq_departure]= vide;
        spec[sq_departure]= EmptySpec;
        spec[mren]= psp;
        e[mren]= p;
        marsid= sq_departure;

        gen_wh_piece_aux(mren, p);

        e[mren]= vide;
        spec[sq_departure]= psp;
        e[sq_departure]= pp;
        flagactive= false;
        /* Unfortunately we have to check for
           duplicate generated moves now.
           there's only ONE duplicate per arrival square
           possible !
        */
        for (l1= anf1 + 1; l1 <= anf2; l1++) {
          for (l2= anf2 + 1; l2 <= nbcou; l2++) {
            if (move_generation_stack[l1].arrival
                == move_generation_stack[l2].arrival) {
              move_generation_stack[l2].arrival= initsquare;
              break;  /* remember: ONE duplicate ! */
            }
          }
        }
      }
    }
    else {
      /* generate noncapturing moves first */
      flagpassive= true;
      flagcapture= false;

      gen_wh_piece_aux(sq_departure, p);

      /* generate capturing moves now */
      flagpassive= false;
      flagcapture= true;
      more_ren=0;
      do {    /* Echecs Plus */
        psp=spec[sq_departure];
        mren= (*marsrenai)(p,psp,sq_departure,initsquare,initsquare,noir);
        if (mren==sq_departure || e[mren]==vide) {
          pp= e[sq_departure];      /* Mars/Neutral bug */
          e[sq_departure]= vide;
          spec[sq_departure]= EmptySpec;
          spec[mren]= psp;
          e[mren]= p;
          marsid= sq_departure;

          gen_wh_piece_aux(mren,p);

          e[mren]= vide;
          spec[sq_departure]= psp;
          e[sq_departure]= pp;
        }
      } while (more_ren);
      flagcapture= false;
    }
  }
  else
    gen_wh_piece_aux(sq_departure,p);

  if (CondFlag[messigny] && !(rb==sq_departure && rex_mess_ex)) {
    square *bnp;
    for (bnp= boardnum; *bnp; bnp++)
      if (e[*bnp]==-p) {
        empile(sq_departure,*bnp,messigny_exchange);
      }
  }
} /* orig_gen_wh_piece */

void singleboxtype3_gen_wh_piece(square z, piece p) {
  numecoup save_nbcou = nbcou;
  unsigned int latent_prom = 0;
  square sq;
  for (sq = next_latent_pawn(initsquare,blanc);
       sq!=initsquare;
       sq = next_latent_pawn(sq,blanc))
  {
    piece pprom;
    for (pprom = next_singlebox_prom(vide,blanc);
         pprom!=vide;
         pprom = next_singlebox_prom(pprom,blanc))
    {
      numecoup save_nbcou = nbcou;
      ++latent_prom;
      e[sq] = pprom;
      orig_gen_wh_piece(z, sq==z ? pprom : p);
      e[sq] = pb;
      for (++save_nbcou; save_nbcou<=nbcou; ++save_nbcou) {
        sb3[save_nbcou].where = sq;
        sb3[save_nbcou].what = pprom;
      }
    }
  }

  if (latent_prom==0) {
    orig_gen_wh_piece(z,p);

    for (++save_nbcou; save_nbcou<=nbcou; ++save_nbcou) {
      sb3[save_nbcou].where = initsquare;
      sb3[save_nbcou].what = vide;
    }
  }
}

void (*gen_wh_piece)(square z, piece p)
  = &orig_gen_wh_piece;


#define ColourSpec Flags
#define OppsiteColour(s)  (s == White ? Black : White)

boolean IsABattery(
  square    KingSquare,
  square    FrontSquare,
  numvec    Direction,
  ColourSpec    ColourMovingPiece,
  piece BackPiece1,
  piece BackPiece2)
{
  square sq;
  piece p;

  /* is the line between king and front piece empty? */
  EndOfLine(FrontSquare, Direction, sq);
  if (sq == KingSquare) {
    /* the line is empty, but is there really an appropriate back
    ** battery piece? */
    EndOfLine(FrontSquare, -Direction, sq);
    p= e[sq];
    if (p < vide)
      p= -p;
    if (   (p == BackPiece1 || p == BackPiece2)
           && TSTFLAG(spec[sq], ColourMovingPiece))
    {
      /* So, it is a battery. */
      return true;
    }
  }
  return false;
} /* IsABattery */

void GenMatingPawn(square   sq_departure,
                   square   sq_king,
                   ColourSpec   ColourMovingPiece)
{
  boolean Battery = false;
  numvec k;
  square sq_arrival;

  k= CheckDirBishop[sq_king-sq_departure];
  if (k!=0)
    Battery=
      IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
  else {
    k= CheckDirRook[sq_king-sq_departure];
    if (k!=0)
      Battery=
        IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Rook,Queen);
  }

  /* if the pawn is not the front piece of a battery reset k,
     otherwise normalise it to be positiv. This is necessary to
     avoid moves along the battery line subsequently.
  */
  if (Battery) {
    if (k<0)
      k= -k;
  }
  else
    k= 0;

  if (ColourMovingPiece==White) {
    if (sq_departure<=square_h1)
      return;
    else {
      /* not first rank */
      /* ep captures */
      if (ep[nbply-1]!=initsquare
          && trait[nbply-1]!=trait[nbply]
          && (sq_departure+dir_up+dir_right==ep[nbply-1]
              || sq_departure+dir_up+dir_left==ep[nbply-1])) {
        empile(sq_departure,
               ep[nbply-1],
               move_generation_stack[repere[nbply]].arrival);
      }

      /* single step */
      if (k!=24) {
        /* suppress moves along a battery line */
        sq_arrival= sq_departure+dir_up;
        if (e[sq_arrival]==vide) {
          if (Battery
              || sq_arrival+dir_up+dir_left == sq_king
              || sq_arrival+dir_up+dir_right == sq_king
              || (PromSq(blanc,sq_arrival)
                  && (CheckDirQueen[sq_king-sq_arrival]
                      || CheckDirKnight[sq_king-sq_arrival])))
            empile(sq_departure,sq_arrival,sq_arrival);

          /* double step */
          if (sq_departure<=square_h2) {
            sq_arrival+= onerow;
            if (e[sq_arrival]==vide
                && (Battery
                    || sq_arrival+dir_up+dir_left==sq_king
                    || sq_arrival+dir_up+dir_right==sq_king))
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }

      /* capture+dir_up+dir_left */
      sq_arrival= sq_departure+dir_up+dir_left;
      if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],Black))
        if (Battery
            || sq_arrival+dir_up+dir_left == sq_king
            || sq_arrival+dir_up+dir_right == sq_king
            || (PromSq(blanc,sq_arrival)
                && (CheckDirQueen[sq_king-sq_arrival]
                    || CheckDirKnight[sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);
      
      /* capture+dir_up+dir_right */
      sq_arrival= sq_departure+dir_up+dir_right;
      if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],Black))
        if (Battery
            || sq_arrival+dir_up+dir_left==sq_king
            || sq_arrival+dir_up+dir_right==sq_king
            || (PromSq(blanc,sq_arrival)
                && (CheckDirQueen[sq_king-sq_arrival]
                    || CheckDirKnight[sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else {
    if (sq_departure>=square_a8)
      return;

    /* not last rank */
    /* ep captures */
    if (ep[nbply-1]!=initsquare
        && trait[nbply-1] != trait[nbply]
        && (sq_departure+dir_down+dir_left==ep[nbply-1]
            || sq_departure+dir_down+dir_right==ep[nbply-1])) {
      empile(sq_departure,
             ep[nbply-1],
             move_generation_stack[repere[nbply]].arrival);
    }

    /* single step */
    if (k!=24) {    /* suppress moves along a battery line */
      sq_arrival= sq_departure+dir_down;
      if (e[sq_arrival]==vide) {
        if (Battery
            || sq_arrival+dir_down+dir_right==sq_king
            || sq_arrival+dir_down+dir_left==sq_king
            || (PromSq(noir,sq_arrival)
                && (CheckDirQueen[sq_king-sq_arrival]
                    || CheckDirKnight[sq_king-sq_arrival])))
          empile(sq_departure,sq_arrival,sq_arrival);

        /* double step */
        if (sq_departure>=square_a7) {
          sq_arrival-= onerow;
          if (e[sq_arrival] == vide
              && (Battery
                  || sq_arrival+dir_down+dir_right==sq_king
                  || sq_arrival+dir_down+dir_left==sq_king))
            empile(sq_departure,sq_arrival,sq_arrival);
        }
      }
    }

    /* capture+dir_up+dir_left */
    sq_arrival= sq_departure+dir_down+dir_right;
    if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],White)) {
      if (Battery
          || sq_arrival+dir_down+dir_right==sq_king
          || sq_arrival+dir_down+dir_left==sq_king
          || (PromSq(noir,sq_arrival)
              && (CheckDirQueen[sq_king-sq_arrival]
                  || CheckDirKnight[sq_king-sq_arrival])))
        empile(sq_departure,sq_arrival,sq_arrival);
    }

    /* capture+dir_up+dir_right */
    sq_arrival= sq_departure+dir_down+dir_left;
    if (e[sq_arrival]!=vide && TSTFLAG(spec[sq_arrival],White)) {
      if (Battery
          || sq_arrival+dir_down+dir_right==sq_king
          || sq_arrival+dir_down+dir_left==sq_king
          || (PromSq(noir,sq_arrival)
              && (CheckDirQueen[sq_king-sq_arrival]
                  || CheckDirKnight[sq_king-sq_arrival])))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
} /* GenMatingPawn */

void GenMatingKing(square   sq_departure,
                   square   sq_king,
                   ColourSpec   ColourMovingPiece)
{
  numvec    k, k2;
  boolean   Generate = false;
  ColourSpec    ColourCapturedPiece = OppsiteColour(ColourMovingPiece);

  square sq_arrival;

  if (rb==rn) {
    /* neutral kings */
    for (k2= vec_queen_start; k2<=vec_queen_end; k2++) {
      sq_arrival= sq_departure+vec[k2];
      /* they must capture to mate the opponent */
      if (e[sq_arrival]!=vide
          && TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else {
    /* check if the king is the front piece of a battery
       that can fire */
    k= CheckDirBishop[sq_king-sq_departure];
    if (k!=0)
      Generate=
        IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
    else {
      k= CheckDirRook[sq_king-sq_departure];
      if (k!=0)
        Generate= IsABattery(sq_king,sq_departure,k,ColourMovingPiece,
                             Rook,Queen);
    }

    if (Generate)
      for (k2= vec_queen_start; k2<=vec_queen_end; k2++) {
        /* prevent the king from moving along the battery line*/
        if (k2==k || k2==-k)
          continue;
        sq_arrival= sq_departure+vec[k2];
        if ((e[sq_arrival]==vide
             || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
            && move_diff_code[abs(sq_king-sq_arrival)]>2)
          empile(sq_departure,sq_arrival,sq_arrival);
      }

    if (CondFlag[ColourCapturedPiece==White ? whiteedge : blackedge]
        || DoubleMate)
      for (k2= vec_queen_start; k2<=vec_queen_end; k2++) {
        sq_arrival= sq_departure + vec[k2];
        if ((e[sq_arrival]==vide
             || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
            && move_diff_code[abs(sq_king-sq_arrival)]<3)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
  }

  /* castling */
  if (castling_supported) {
    if (ColourMovingPiece==White)
      /* white to play */
      genrb_cast();
    else
      /* black to play */
      genrn_cast();
  }
}

void GenMatingKnight(square sq_departure,
                     square sq_king,
                     ColourSpec ColourMovingPiece)
{
  numvec    k;
  boolean   Generate = false;
  ColourSpec    ColourCapturedPiece = OppsiteColour(ColourMovingPiece);

  square sq_arrival;

  /* check if the knight is the front piece of a battery that can
     fire
  */
  if ((k = CheckDirBishop[sq_king-sq_departure])!=0)
    Generate=
      IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Bishop,Queen);
  else if ((k = CheckDirRook[sq_king-sq_departure])!=0)
    Generate= IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Rook,Queen);

  k= abs(sq_king-sq_departure);
  if (Generate
      || (SquareCol(sq_departure) == SquareCol(sq_king)
          && move_diff_code[k]<21
          && move_diff_code[k]!=8))
    for (k= vec_knight_start; k<=vec_knight_end; k++) {
      sq_arrival= sq_departure+vec[k];
      if (e[sq_arrival]==vide
          || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        if (Generate || CheckDirKnight[sq_arrival-sq_king]!=0)
          empile(sq_departure,sq_arrival,sq_arrival);
    }
}

void GenMatingRook(square   sq_departure,
                   square   sq_king,
                   ColourSpec   ColourMovingPiece)
{
  square    sq2;
  numvec    k, k2;
  ColourSpec    ColourCapturedPiece = OppsiteColour(ColourMovingPiece);

  square sq_arrival;

  /* check if the rook is the front piece of a battery that can fire
   */
  k= CheckDirBishop[sq_king-sq_departure];
  if (k != 0
      && IsABattery(sq_king, sq_departure, k, ColourMovingPiece, Bishop, Queen))
  {
    for (k= vec_rook_start; k<=vec_rook_end; k++) {
      sq_arrival= sq_departure+vec[k];
      while (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= vec[k];
      }
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else {
    short OriginalDistance = move_diff_code[abs(sq_departure-sq_king)];

    k2= CheckDirRook[sq_king-sq_departure];
    if (k2!=0) {
      /* the rook is already on a line with the king */
      EndOfLine(sq_departure,k2,sq_arrival);
      /* We are at the end of the line */
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece)) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
    else {
      for (k= vec_rook_start; k<=vec_rook_end; k++) {
        sq_arrival= sq_departure+vec[k];
        if (e[sq_arrival]==obs)
          continue;
        if (move_diff_code[abs(sq_arrival-sq_king)]<OriginalDistance) {
          /* The rook must move closer to the king! */
          k2= CheckDirRook[sq_king-sq_arrival];
          while (k2==0 && e[sq_arrival]==vide) {
            sq_arrival+= vec[k];
            k2= CheckDirRook[sq_king-sq_arrival];
          }

          /* We are at the end of the line or in checking
             distance
          */
          if (k2==0)
            continue;
          if (e[sq_arrival]==vide
              || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
          {
            EndOfLine(sq_arrival,k2,sq2);
            if (sq2==sq_king)
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }
    }
  }
}

void GenMatingQueen(square  sq_departure,
                    square  sq_king,
                    ColourSpec  ColourMovingPiece)
{
  square sq2;
  numvec  k, k2;
  ColourSpec ColourCapturedPiece = OppsiteColour(ColourMovingPiece);

  square sq_arrival;

  for (k= vec_queen_start; k<=vec_queen_end; k++) {
    sq_arrival= sq_departure+vec[k];
    while (e[sq_arrival]==vide) {
      k2= CheckDirQueen[sq_king-sq_arrival];
      if (k2) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
      sq_arrival+= vec[k];
    }
    if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece)) {
      k2= CheckDirQueen[sq_king-sq_arrival];
      if (k2) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

void GenMatingBishop(square sq_departure,
                     square sq_king,
                     ColourSpec ColourMovingPiece)
{
  square    sq2;
  numvec    k, k2;
  ColourSpec    ColourCapturedPiece = OppsiteColour(ColourMovingPiece);

  square sq_arrival;

  /* check if the bishop is the front piece of a battery that can
     fire
  */
  k = CheckDirRook[sq_king-sq_departure];
  if (k!=0
      && IsABattery(sq_king,sq_departure,k,ColourMovingPiece,Rook,Queen))
  {
    for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
      sq_arrival= sq_departure+vec[k];
      while (e[sq_arrival]==vide) {
        empile(sq_departure,sq_arrival,sq_arrival);
        sq_arrival+= vec[k];
      }
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
        empile(sq_departure,sq_arrival,sq_arrival);
    }
  }
  else if (SquareCol(sq_departure)==SquareCol(sq_king)) {
    short OriginalDistance = move_diff_code[abs(sq_departure-sq_king)];

    k2= CheckDirBishop[sq_king-sq_departure];
    if (k2) {
      /* the bishop is already on a line with the king */
      EndOfLine(sq_departure,k2,sq_arrival);
      /* We are at the end of the line */
      if (TSTFLAG(spec[sq_arrival],ColourCapturedPiece)) {
        EndOfLine(sq_arrival,k2,sq2);
        if (sq2==sq_king)
          empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
    else {
      for (k= vec_bishop_start; k<=vec_bishop_end; k++) {
        sq_arrival= sq_departure+vec[k];
        if (e[sq_arrival]==obs)
          continue;
        if (move_diff_code[abs(sq_arrival-sq_king)]
            <OriginalDistance) {
          /* The bishop must move closer to the king! */
          k2= CheckDirBishop[sq_king-sq_arrival];
          while (k2==0 && e[sq_arrival]==vide) {
            sq_arrival+= vec[k];
            k2= CheckDirBishop[sq_king-sq_arrival];
          }

          /* We are at the end of the line or in checking
             distance */
          if (k2==0)
            continue;
          if (e[sq_arrival]==vide
              || TSTFLAG(spec[sq_arrival],ColourCapturedPiece))
          {
            EndOfLine(sq_arrival,k2,sq2);
            if (sq2==sq_king)
              empile(sq_departure,sq_arrival,sq_arrival);
          }
        }
      }
    }
  }
} /* GenMatingBishop */

void GenMatingMove(couleur camp) {
  if (totalortho) {
    ColourSpec ColourMovingPiece = camp == blanc ? White : Black;
    square     i, j, z, OpponentsKing = camp == blanc ? rn : rb;
    piece      p;

    nextply();
    trait[nbply]= camp;
    init_move_generation_optimizer();

    FlagGenMatingMove= TSTFLAG(PieSpExFlags, Neutral);

    /* Don't try to "optimize" by hand. The double-loop is tested
       as the fastest way to compute (due to
       compiler-optimizations!) */

    z= bas;
    for (i= 8; i > 0; i--, z+= 16) {
      for (j= 8; j > 0; j--, z++) {
        p= e[z];
        if (p != vide && TSTFLAG(spec[z], ColourMovingPiece)) {
          if (CondFlag[gridchess] && !GridLegal(z, OpponentsKing))
          {
            if (camp == blanc)
              gen_wh_piece(z, p);
            else
              gen_bl_piece(z, p);
          }
          else
          {
            switch(abs(p)) {
            case King:
              GenMatingKing(z,
                            OpponentsKing, ColourMovingPiece);
              break;
            case Pawn:
              GenMatingPawn(z,
                            OpponentsKing, ColourMovingPiece);
              break;
            case Knight:
              GenMatingKnight(z,
                              OpponentsKing, ColourMovingPiece);
              break;
            case Rook:
              GenMatingRook(z,
                            OpponentsKing, ColourMovingPiece);
              break;
            case Queen:
              GenMatingQueen(z,
                             OpponentsKing, ColourMovingPiece);
              break;
            case Bishop:
              GenMatingBishop(z,
                              OpponentsKing, ColourMovingPiece);
              break;
            }
          }
        }
      }
    }
    finish_move_generation_optimizer();
  }
  else {
    if (FlagMoveOrientatedStip) {
      if (stipulation == stip_ep) {
        if (  ep[nbply] == initsquare
              && ep2[nbply] == initsquare)
        {
          nextply();
          return;
        }
      }
      else if (stipulation == stip_castling) {
        if (camp == blanc
            ? TSTFLAGMASK(castling_flag[nbply],wh_castlings)<=ke1_cancastle
            : TSTFLAGMASK(castling_flag[nbply],bl_castlings)<=ke8_cancastle)
        {
          nextply();
          return;
        }
      }
      FlagGenMatingMove= ! (camp == blanc
                            ? flagwhitemummer
                            : flagblackmummer);
    }
    genmove(camp);
  }
  FlagGenMatingMove= False;
} /* GenMatingMove */

void gorph(square i, couleur camp) {
  piece *porph;
  numecoup  anf, l1, l2;

  anf= nbcou;
  for (porph= orphanpieces; *porph; porph++) {
    if (nbpiece[*porph] || nbpiece[-*porph]) {
      if (camp == blanc) {
        if (ooorphancheck(i, -*porph, orphann, eval_white))
          gen_wh_piece(i, *porph);
      }
      else {
        if (ooorphancheck(i, *porph, orphanb, eval_black))
          gen_bl_piece(i, -*porph);
      }
    }
  }
  for (l1= anf + 1; l1 <= nbcou; l1++) {
    if (move_generation_stack[l1].arrival != initsquare) {
      for (l2= l1 + 1; l2 <= nbcou; l2++) {
        if (move_generation_stack[l1].arrival == move_generation_stack[l2].arrival) {
          move_generation_stack[l2].arrival= initsquare;
        }
      }
    }
  }
}

void gfriend(square i, couleur camp) {
  piece *pfr;
  numecoup  anf, l1, l2;

  anf= nbcou;
  for (pfr= orphanpieces; *pfr; pfr++) {
    if (nbpiece[*pfr]) {
      if (camp == blanc) {
        if (fffriendcheck(i, *pfr, friendb, eval_white)) {
          gen_wh_piece(i, *pfr);
        }
      }
      else {
        if (fffriendcheck(i, -*pfr, friendn, eval_black)) {
          gen_bl_piece(i, -*pfr);
        }
      }
    }
  }
  for (l1= anf + 1; l1 <= nbcou; l1++) {
    if (move_generation_stack[l1].arrival != initsquare) {
      for (l2= l1 + 1; l2 <= nbcou; l2++) {
        if (move_generation_stack[l1].arrival == move_generation_stack[l2].arrival) {
          move_generation_stack[l2].arrival= initsquare;
        }
      }
    }
  }
}


void gedgeh(square sq_departure, couleur camp) {
  numvec k;

  square sq_arrival;

  for (k= vec_queen_end; k >=vec_queen_start; k--) {
    sq_arrival= sq_departure+vec[k];
    while (e[sq_arrival]==vide) {
      if (NoEdge(sq_arrival)!=NoEdge(sq_departure))
        empile(sq_departure,sq_arrival,sq_arrival);
      sq_arrival+= vec[k];
    }
    
    if (rightcolor(e[sq_arrival],camp))
      if (NoEdge(sq_arrival)!=NoEdge(sq_departure))
        empile(sq_departure,sq_arrival,sq_arrival);
  }
}

void geskylchar(square sq_departure, square sq_arrival, square sq_capture,
                couleur camp) {
  if (e[sq_arrival] == vide) {
    if (e[sq_capture]==vide)
      empile(sq_departure,sq_arrival,sq_arrival);
    else if (rightcolor(e[sq_capture],camp))
      empile(sq_departure,sq_arrival,sq_capture);
  }
}

void geskylla(square i, couleur camp) {
  geskylchar(i, i+dir_up+2*dir_right, i+dir_right, camp);
  geskylchar(i, i+2*dir_up+dir_right, i+dir_up, camp);
  geskylchar(i, i+2*dir_up+dir_left, i+dir_up, camp);
  geskylchar(i, i+dir_up+2*dir_left, i+dir_left, camp);
  geskylchar(i, i+dir_down+2*dir_left, i+dir_left, camp);
  geskylchar(i, i+2*dir_down+dir_left, i+dir_down, camp);
  geskylchar(i, i+2*dir_down+dir_right, i+dir_down, camp);
  geskylchar(i, i+dir_down+2*dir_right, i+dir_right, camp);
}

void gecharybdis(square i, couleur camp) {
  geskylchar(i, i+dir_up+2*dir_right, i+dir_up+dir_right, camp);
  geskylchar(i, i+2*dir_up+dir_right, i+dir_up+dir_right, camp);
  geskylchar(i, i+2*dir_up+dir_left, i+dir_up+dir_left, camp);
  geskylchar(i, i+dir_up+2*dir_left, i+dir_up+dir_left, camp);
  geskylchar(i, i+dir_down+2*dir_left, i+dir_down+dir_left, camp);
  geskylchar(i, i+2*dir_down+dir_left, i+dir_down+dir_left, camp);
  geskylchar(i, i+2*dir_down+dir_right, i+dir_down+dir_right, camp);
  geskylchar(i, i+dir_down+2*dir_right, i+dir_down+dir_right, camp);
}

/***********************************************************************
 **                                    **
 **       generating functions for pawns               **
 **                                    **
 ***********************************************************************/

/* Two auxiliary function for generating pawn moves */
void gen_p_captures(square sq_departure, square sq_arrival, couleur camp) {
  /* generates move of a pawn of colour camp on departure capturing a
     piece on arrival
  */

  if (rightcolor(e[sq_arrival],camp))
    /* normal capture */
    empile(sq_departure,sq_arrival,sq_arrival);
  else {
    /* ep capture */
    if ((abs(e[sq_departure])!=Orphan)   /* orphans cannot capture ep */
        && (sq_arrival==ep[nbply-1] || sq_arrival==ep2[nbply - 1])
        /* a pawn has just done a critical move */
        && trait[nbply-1]!=camp)     /* the opp. moved before */
    {
      square prev_arrival;

      if (nbply==2) {    /* ep.-key  standard pawn */
        if (camp==blanc)
          move_generation_stack[repere[2]].arrival= sq_arrival+dir_down;
        else
          move_generation_stack[repere[2]].arrival= sq_arrival+dir_up;
      }

      prev_arrival= move_generation_stack[repere[nbply]].arrival;
      if (rightcolor(e[prev_arrival],camp))
        /* the pawn has the right color */
        empile(sq_departure,sq_arrival,prev_arrival);
    }
  }
} /* end gen_p_captures */

void gen_p_nocaptures(square sq_departure, numvec dir, short steps)
{
  /* generates moves of a pawn in direction dir where steps single
     steps are possible.
  */

  square sq_arrival= sq_departure+dir;

  while (steps--)
    if (e[sq_arrival]==vide && empile(sq_departure,sq_arrival,sq_arrival))
      sq_arrival+= dir;
    else
      break;
}

/****************************  white pawn  ****************************/
void genpb(square sq_departure) {
  if (sq_departure<=square_h1) {
    /* pawn on first rank */
    if (CondFlag[parrain]
        || CondFlag[einstein]
    || CondFlag[normalp]
        || abs(e[sq_departure]) == orphanb)
    {
      gen_p_captures(sq_departure, sq_departure+dir_up+dir_left, blanc);
      gen_p_captures(sq_departure, sq_departure+dir_up+dir_right, blanc);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,+dir_up, CondFlag[einstein] ? 3 : 1);
    }
    else {
      return;
    }
  }
  else {
    /* not first rank */
    if ( CondFlag[singlebox] && SingleBoxType==singlebox_type1
         && PromSq(blanc,sq_departure+dir_up)
         && next_singlebox_prom(vide,blanc)==vide)
    {
      return;
    }
    gen_p_captures(sq_departure, sq_departure+dir_up+dir_left, blanc);
    gen_p_captures(sq_departure, sq_departure+dir_up+dir_right, blanc);
    /* double or single step? */
    gen_p_nocaptures(sq_departure,+dir_up, sq_departure<=square_h2 ? 2 : 1);
  }
} /* end of genpb */

/****************************  black pawn  ****************************/
void genpn(square sq_departure) {
  if (sq_departure>=square_a8) {
    /* pawn on last rank */
    if (CondFlag[parrain]
        || CondFlag[normalp]
        || CondFlag[einstein]
        || abs(e[sq_departure])==orphanb)
    {
      gen_p_captures(sq_departure, sq_departure+dir_down+dir_right, noir);
      gen_p_captures(sq_departure, sq_departure+dir_down+dir_left, noir);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,+dir_down, CondFlag[einstein] ? 3 : 1);
    }
    else {
      return;
    }
  }
  else {
    /* not last rank */
    if (CondFlag[singlebox] && SingleBoxType==singlebox_type1
        && PromSq(noir,sq_departure+dir_down)
        && next_singlebox_prom(vide,noir)==vide)
    {
      return;
    }
    gen_p_captures(sq_departure, sq_departure+dir_down+dir_right, noir);
    gen_p_captures(sq_departure, sq_departure+dir_down+dir_left, noir);
    /* double or single step? */
    gen_p_nocaptures(sq_departure,+dir_down, sq_departure>=square_a7 ? 2 : 1);
  }
}

void genreversepb(square sq_departure) {
  if (sq_departure > haut - 24) {
    /* pawn on last rank */
    if (CondFlag[parrain]
        || CondFlag[einstein]
    || CondFlag[normalp]
        || abs(e[sq_departure]) == orphanb)
    {
      gen_p_captures(sq_departure, sq_departure - 23, blanc);
      gen_p_captures(sq_departure, sq_departure - 25, blanc);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure, -24, CondFlag[einstein] ? 3 : 1);
    }
    else {
      return;
    }
  }
  else {
    /* not last rank */
    gen_p_captures(sq_departure, sq_departure - 23, blanc);
    gen_p_captures(sq_departure, sq_departure - 25, blanc);
    /* double or single step? */
    gen_p_nocaptures(sq_departure, -24, (sq_departure > haut - 32) ? 2 : 1);
  }
}

void genreversepn(square sq_departure) {
  if (sq_departure < bas + 24) {
    /* pawn on last rank */
    if ( CondFlag[parrain]
         || CondFlag[normalp]
         || CondFlag[einstein]
         || abs(e[sq_departure]) == orphanb)
    {
      gen_p_captures(sq_departure, sq_departure + 23, noir);
      gen_p_captures(sq_departure, sq_departure + 25, noir);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure, 24, CondFlag[einstein] ? 3 : 1);
    }
    else {
      return;
    }
  }
  else {
    gen_p_captures(sq_departure, sq_departure + 23, noir);
    gen_p_captures(sq_departure, sq_departure + 25, noir);
    /* double or single step? */
    gen_p_nocaptures(sq_departure, 24, (sq_departure < bas + 32) ? 2 : 1);
  }
}

/************************  white berolina pawn  ***********************/
void genpbb(square sq_departure) {
  if (sq_departure<=square_h1) {
    /* pawn on first rank */
    if ( CondFlag[parrain]
         || CondFlag[normalp]
         || CondFlag[einstein]
         || abs(e[sq_departure]) == orphanb)
    {
      gen_p_captures(sq_departure, sq_departure+dir_up, blanc);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,+dir_up+dir_left, CondFlag[einstein] ? 3 : 1);
      gen_p_nocaptures(sq_departure,+dir_up+dir_right, CondFlag[einstein] ? 3 : 1);
    }
    else {
      return;
    }
  }
  else {
    /* not first rank */
    gen_p_captures(sq_departure, sq_departure+dir_up, blanc);
    /* double or single step? */
    gen_p_nocaptures(sq_departure,+dir_up+dir_left, sq_departure<=square_h2 ? 2 : 1);
    gen_p_nocaptures(sq_departure,+dir_up+dir_right, sq_departure<=square_h2 ? 2 : 1);
  }
}

/************************  black berolina pawn  ***********************/
void genpbn(square sq_departure) {
  if (sq_departure>=square_a8) {
    /* pawn on last rank */
    if ( CondFlag[parrain]
         || CondFlag[normalp]
         || CondFlag[einstein]
         || abs(e[sq_departure]) == orphanb)
    {
      gen_p_captures(sq_departure, sq_departure+dir_down, noir);
      /* triple or single step? */
      gen_p_nocaptures(sq_departure,+dir_down+dir_right, CondFlag[einstein] ? 3 : 1);
      gen_p_nocaptures(sq_departure,+dir_down+dir_left, CondFlag[einstein] ? 3 : 1);
    }
    else {
      return;
    }
  }
  else {
    /* not last rank */
    gen_p_captures(sq_departure, sq_departure+dir_down, noir);
    /* double or single step? */
    gen_p_nocaptures(sq_departure,+dir_down+dir_right,
                     sq_departure>=square_a7 ? 2 : 1);
    gen_p_nocaptures(sq_departure,+dir_down+dir_left,
                     sq_departure>=square_a7 ? 2 : 1);
  }
}


void gorix(square sq_departure, couleur camp) {
  /* Orix */
  numvec  k;
  piece   hurdle, at_end_of_line;
  square  sq_hurdle, end_of_line;

  square sq_arrival;

  for (k= vec_queen_end; k>=vec_queen_start; k--) {     /* 0,2; 0,4; 0,6; 2,2; 4,4; 6,6; */
    finligne(sq_departure,vec[k],hurdle,sq_hurdle);
    if (hurdle!=obs) {
      finligne(sq_hurdle,vec[k],at_end_of_line,end_of_line);
      sq_arrival= sq_hurdle+sq_hurdle-sq_departure;
      if (abs(end_of_line-sq_hurdle) > abs(sq_hurdle-sq_departure)
          && hopimcheck(sq_departure,sq_arrival,sq_hurdle,vec[k]))
        empile(sq_departure,sq_arrival,sq_arrival);
      else if (abs(end_of_line-sq_hurdle) == abs(sq_hurdle-sq_departure)
               && rightcolor(at_end_of_line,camp)
               && hopimcheck(sq_departure,end_of_line,sq_hurdle,vec[k])) {
        sq_arrival= end_of_line;
        empile(sq_departure,sq_arrival,sq_arrival);
      }
    }
  }
}

void genleapleap(square sq_departure, numvec kanf, numvec kend, smallint hurdletype, couleur camp)
{
  square  sq_arrival, sq_hurdle;
  numvec  k, k1;

  for (k= kanf; k <= kend; k++) {
    sq_hurdle= sq_departure + vec[k];
    if (hurdletype == 0 && rightcolor(e[sq_hurdle], camp))
    {
      for (k1= kanf; k1 <= kend; k1++) {
        sq_arrival = sq_hurdle + vec[k1];
        if (e[sq_arrival] == vide || rightcolor(e[sq_arrival], camp))
          empile(sq_departure, sq_arrival, sq_arrival);
      }
    }
  }
}

void genradialknight(square sq_departure, couleur camp) 
{
  genleapleap(sq_departure, vec_rook_start, vec_rook_end, 0, camp);
  genleapleap(sq_departure, vec_dabbaba_start, vec_dabbaba_end, 0, camp);
  genleapleap(sq_departure, vec_leap03_start, vec_leap03_end, 0, camp);
  genleapleap(sq_departure, vec_leap04_start, vec_leap04_end, 0, camp);
  genleapleap(sq_departure, vec_bucephale_start, vec_bucephale_end, 0, camp);
  genleapleap(sq_departure, vec_leap06_start, vec_leap06_end, 0, camp);
  genleapleap(sq_departure, vec_leap07_start, vec_leap07_end, 0, camp);
  genleapleap(sq_departure, vec_bishop_start, vec_bishop_end, 0, camp);
  genleapleap(sq_departure, vec_knight_start, vec_knight_end, 0, camp);
  genleapleap(sq_departure, vec_chameau_start, vec_chameau_end, 0, camp);
  genleapleap(sq_departure, vec_girafe_start, vec_girafe_end, 0, camp);
  genleapleap(sq_departure, vec_leap15_start, vec_leap15_end, 0, camp);
  genleapleap(sq_departure, vec_leap16_start, vec_leap16_end, 0, camp);
  genleapleap(sq_departure, vec_rccinq_start, vec_rccinq_end, 0, camp);
  genleapleap(sq_departure, vec_alfil_start, vec_alfil_end, 0, camp);
  genleapleap(sq_departure, vec_zebre_start, vec_zebre_end, 0, camp);
  genleapleap(sq_departure, vec_leap24_start, vec_leap24_end, 0, camp);
  genleapleap(sq_departure, vec_leap25_start, vec_leap25_end, 0, camp);
  genleapleap(sq_departure, vec_leap26_start, vec_leap26_end, 0, camp);
  genleapleap(sq_departure, vec_leap27_start, vec_leap27_end, 0, camp);
  genleapleap(sq_departure, vec_leap33_start, vec_leap33_end, 0, camp);
  genleapleap(sq_departure, vec_leap35_start, vec_leap35_end, 0, camp);
  genleapleap(sq_departure, vec_leap36_start, vec_leap36_end, 0, camp);
  genleapleap(sq_departure, vec_leap37_start, vec_rccinq_end, 0, camp);
  genleapleap(sq_departure, vec_leap44_start, vec_leap44_end, 0, camp);
  genleapleap(sq_departure, vec_leap45_start, vec_leap45_end, 0, camp);
  genleapleap(sq_departure, vec_leap46_start, vec_leap46_end, 0, camp);
  genleapleap(sq_departure, vec_leap47_start, vec_leap47_end, 0, camp);
  genleapleap(sq_departure, vec_leap56_start, vec_leap56_end, 0, camp);
  genleapleap(sq_departure, vec_leap57_start, vec_leap57_end, 0, camp);
  genleapleap(sq_departure, vec_leap66_start, vec_leap66_end, 0, camp);
  genleapleap(sq_departure, vec_leap67_start, vec_leap67_end, 0, camp);
  genleapleap(sq_departure, vec_leap77_start, vec_leap77_end, 0, camp);
}
