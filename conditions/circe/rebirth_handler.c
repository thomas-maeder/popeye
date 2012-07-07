#include "conditions/circe/rebirth_handler.h"
#include "pydata.h"
#include "conditions/circe/cage.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "solving/legal_move_counter.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

extern piece pdisp[maxply+1];
extern Flags pdispspec[maxply+1];

#define setneutre(i)            do {if (neutral_side != get_side(i)) change_side(i);} while(0)

static void joueparrain(ply ply_id)
{
  numecoup const coup_id = ply_id==nbply ? current_move[nbply] : current_move[ply_id];
  piece const pi_captured = pprise[ply_id-1];
  Flags spec_captured = pprispec[ply_id-1];
  square sq_rebirth;
  if (CondFlag[parrain]) {
    sq_rebirth = (move_generation_stack[current_move[ply_id-1]].capture
                                 + move_generation_stack[coup_id].arrival
                                 - move_generation_stack[coup_id].departure);
  }
  if (CondFlag[contraparrain]) {
    sq_rebirth = (move_generation_stack[current_move[ply_id-1]].capture
                                 - move_generation_stack[coup_id].arrival
                                 + move_generation_stack[coup_id].departure);
  }

  if (e[sq_rebirth]==vide)
  {
    current_circe_rebirth_square[ply_id] = sq_rebirth;
    ren_parrain[ply_id] = pi_captured;
    e[sq_rebirth] = pi_captured;
    spec[sq_rebirth] = spec_captured;

    if ((is_forwardpawn(pi_captured)
         && PromSq(advers(trait[ply_id-1]), sq_rebirth))
        || (is_reversepawn(pi_captured)
            && ReversePromSq(advers(trait[ply_id-1]), sq_rebirth)))
    {
      /* captured white pawn on eighth rank: promotion ! */
      /* captured black pawn on first rank: promotion ! */
      piece pprom = current_promotion_of_reborn[ply_id];

      if (TSTFLAG(spec_captured,Chameleon))
        is_reborn_chameleon_promoted[ply_id] = true;

      if (pprom==vide)
      {
        current_promotion_of_reborn[ply_id] = getprompiece[vide];
        pprom = getprompiece[vide];
      }
      if (pi_captured<vide)
        pprom = -pprom;

      e[sq_rebirth]= pprom;
      nbpiece[pprom]++;
      if (is_reborn_chameleon_promoted[ply_id])
        SETFLAG(spec_captured,Chameleon);
      spec[sq_rebirth]= spec_captured;
    }
    else
      nbpiece[pi_captured]++;

    if (TSTFLAG(spec_captured,Neutral))
      setneutre(sq_rebirth);
  }
}

static void handle_rebirth(Side trait_ply)
{
  square const pi_captured = pprise[nbply];
  square const pi_departing = pjoue[nbply];
  numecoup const coup_id = current_move[nbply];
  move_generation_elmt const * const move_gen_top = move_generation_stack+coup_id;
  square const sq_arrival = move_gen_top->arrival;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  Flags const spec_pi_moving = jouespec[nbply];
  Flags spec_pi_captured = pprispec[nbply];
  piece const pi_arriving = jouearr[nbply];
  square const prev_rb = RB_[nbply];
  square const prev_rn = RN_[nbply];

  /* circe-rebirth of moving kamikaze-piece */
  if (TSTFLAG(spec_pi_moving, Kamikaze) && (pi_captured != vide))
  {
    square sq_rebirth;
    if (CondFlag[couscous]) {
      sq_rebirth= (*circerenai)(nbply,
                                pi_captured,
                                spec_pi_captured,
                                sq_capture,
                                sq_departure,
                                sq_arrival,
                                trait_ply);
    }
    else {
      sq_rebirth= (*circerenai)(nbply,
                                pi_arriving,
                                spec_pi_moving,
                                sq_capture,
                                sq_departure,
                                sq_arrival,
                                advers(trait_ply));
    }

    e[sq_arrival]= vide;
    spec[sq_arrival]= 0;
    if ((e[sq_rebirth] == vide)
        && !(CondFlag[contactgrid]
             && nogridcontact(sq_rebirth)))
    {
      current_anticirce_rebirth_square[nbply]= sq_rebirth;
      e[sq_rebirth]= pi_arriving;
      spec[sq_rebirth]= spec_pi_moving;
      if (rex_circe) {
        if (sq_departure == prev_rb)
          king_square[White]= sq_rebirth;
        if (sq_departure == prev_rn)
          king_square[Black]= sq_rebirth;

        if (castling_supported
            && (abs(pi_arriving) == King)) {
          if (TSTFLAG(spec_pi_moving, White)
              && sq_rebirth == square_e1
              && (!CondFlag[dynasty] || nbpiece[roib]==1))
            /* white king reborn on e1 */
            SETCASTLINGFLAGMASK(nbply,White,k_cancastle);
          else if (TSTFLAG(spec_pi_moving, Black)
                   && sq_rebirth == square_e8
                   && (!CondFlag[dynasty] || nbpiece[roin]==1))
            /* black king reborn on e8 */
            SETCASTLINGFLAGMASK(nbply,Black,k_cancastle);
        }
      }
      if (castling_supported
          && (abs(pi_arriving) == Rook)) {
        if (TSTFLAG(spec_pi_moving, White)) {
          if (sq_rebirth == square_h1)
            /* white rook reborn on h1 */
            SETCASTLINGFLAGMASK(nbply,White,rh_cancastle);
          else if (sq_rebirth == square_a1)
            /* white rook reborn on a1 */
            SETCASTLINGFLAGMASK(nbply,White,ra_cancastle);
        }
        if (TSTFLAG(spec_pi_moving, Black)) {
          if (sq_rebirth == square_h8)
            /* black rook reborn on h8 */
            SETCASTLINGFLAGMASK(nbply,Black,rh_cancastle);
          else if (sq_rebirth == square_a8)
            /* black rook reborn on a8 */
            SETCASTLINGFLAGMASK(nbply,Black,ra_cancastle);
        }
      }
    } else
      nbpiece[pi_arriving]--;
  } /* Kamikaze */

  if (anyparrain)
  {
    if (pprise[nbply-1]!=vide)
      joueparrain(nbply);
  }
  else
  {
    if (pi_captured != vide)
    {
      square sq_rebirth;
      piece pi_reborn;
      if (anyclone
          && sq_departure != prev_rn && sq_departure != prev_rb) {
        /* Circe Clone - new implementation
        ** captured pieces are reborn as pieces
        ** of the same type as the capturing piece
        ** if the latter one is not royal.
        */
        /* change type of pieces according to colour */
        pi_reborn = ((pi_departing * pi_captured < 0)
                     ? -pi_departing
                     : pi_departing);
        /* If it is a pawn give it the pawn-attribut.
        ** Otherwise delete it - the captured piece may
        ** have been a pawn, but is not anymore.
        */
      }
      else if (anytraitor) {
        pdispspec[nbply]=spec_pi_captured;
        pi_reborn= -pi_captured;
        CHANGECOLOR(spec_pi_captured);
      }
      else {
        if (CondFlag[chamcirce]) {
          pi_reborn= ChamCircePiece(pi_captured);
        }
        else if (CondFlag[antieinstein]) {
          pi_reborn= inc_einstein(pi_captured);
        }
        else {
          pi_reborn= pi_captured;
        }
      }

      if (CondFlag[couscous])
        sq_rebirth= (*circerenai)(nbply,
                                  pi_arriving,
                                  spec_pi_moving,
                                  sq_capture,
                                  sq_departure,
                                  sq_arrival,
                                  advers(trait_ply));
      else
        sq_rebirth= (*circerenai)(nbply,
                                  pi_reborn,
                                  spec_pi_captured,
                                  sq_capture,
                                  sq_departure,
                                  sq_arrival,
                                  trait_ply);

      if (!rex_circe
          && (CondFlag[exclusive] || CondFlag[isardam] || CondFlag[ohneschach] || CondFlag[brunner])
          && (sq_capture == prev_rb || sq_capture == prev_rn))
      {
        /* ordinary circe and (isardam, brunner or
        ** ohneschach) it can happen that the king is
        ** captured while testing the legality of a
        ** move. Therefore prevent the king from being
        ** reborn.
        */
        sq_rebirth= initsquare;
      }

      if ( (e[sq_rebirth] == vide || CondFlag[circeassassin])
           && !( CondFlag[contactgrid]
                 && nogridcontact(sq_rebirth)))
      {
        current_circe_rebirth_square[nbply]= sq_rebirth;
        if (rex_circe) {
          /* neutral K */
          if (prev_rb == sq_capture) {
            king_square[White]= sq_rebirth;
          }
          if (prev_rn == sq_capture) {
            king_square[Black]= sq_rebirth;
          }

          if (castling_supported
              && (abs(pi_reborn) == King)) {
            if (TSTFLAG(spec_pi_captured, White)
                && sq_rebirth == square_e1
                && (!CondFlag[dynasty] || nbpiece[roib]==1))
              /* white king reborn on e1 */
              SETCASTLINGFLAGMASK(nbply,White,k_cancastle);
            else if (TSTFLAG(spec_pi_captured, Black)
                     && sq_rebirth == square_e8
                     && (!CondFlag[dynasty] || nbpiece[roin]==1))
              /* black king reborn on e8 */
              SETCASTLINGFLAGMASK(nbply,Black,k_cancastle);
          }
        }

        if (castling_supported
            && (abs(pi_reborn) == Rook)) {
          if (TSTFLAG(spec_pi_captured, White)) {
            if (sq_rebirth == square_h1)
              /* white rook reborn on h1 */
              SETCASTLINGFLAGMASK(nbply,White,rh_cancastle);
            else if (sq_rebirth == square_a1)
              /* white rook reborn on a1 */
              SETCASTLINGFLAGMASK(nbply,White,ra_cancastle);
          }
          if (TSTFLAG(spec_pi_captured, Black)) {
            if (sq_rebirth == square_h8)
              /* black rook reborn on h8 */
              SETCASTLINGFLAGMASK(nbply,Black,rh_cancastle);
            else if (sq_rebirth == square_a8)
              /* black rook reborn on a8 */
              SETCASTLINGFLAGMASK(nbply,Black,ra_cancastle);
          }
        }
        if (anycirprom
            && is_pawn(pi_captured)
            && PromSq(advers(trait_ply),sq_rebirth))
        {
          /* captured white pawn on eighth rank: promotion ! */
          /* captured black pawn on first rank: promotion ! */
          piece pprom = current_promotion_of_reborn[nbply];
          if (pprom==vide)
          {
            pprom = getprompiece[vide];
            current_promotion_of_reborn[nbply] = pprom;
          }
          pi_reborn = pi_reborn<vide ? -pprom : pprom;
          if (is_reborn_chameleon_promoted[nbply])
            SETFLAG(spec_pi_captured, Chameleon);
        }
        if (TSTFLAG(spec_pi_captured, Volage)
            && SquareCol(sq_rebirth) != SquareCol(sq_capture))
        {
          pi_reborn= -pi_reborn;
          CHANGECOLOR(spec_pi_captured);
          if (!CondFlag[hypervolage]) {
            CLRFLAG(spec_pi_captured, Volage);
          }
        }
        if (CondFlag[circeassassin]) {
          nbpiece[pdisp[nbply]=e[sq_rebirth]]--;
          pdispspec[nbply]=spec[sq_rebirth];
        }
        nbpiece[e[sq_rebirth]= pi_reborn]++;
        spec[sq_rebirth]= spec_pi_captured;
      }
    }
  }
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_rebirth_handler_attack(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  handle_rebirth(slices[si].starter);
  result = attack(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type circe_rebirth_handler_defend(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  handle_rebirth(slices[si].starter);
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void instrument_move(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STCirceRebirthHandler);
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_rebirth_handlers(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STMove,
                                           &instrument_move);
  stip_structure_traversal_override_single(&st,
                                           STReplayingMoves,
                                           &instrument_move);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
