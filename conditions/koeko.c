#include "conditions/koeko.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/move_generator.h"
#include "solving/castling.h"
#include "conditions/sentinelles.h"
#include "conditions/circe/chameleon.h"
#include "debugging/trace.h"

#include <stdlib.h>

nocontactfunc_t koekofunc;

boolean nocontact(square sq_departure, square sq_arrival, square sq_capture, nocontactfunc_t nocontactfunc)
{
  boolean   Result;
  square    cr = initsquare;
  piece pj, pp, pren;
  piece pc= obs;
  square sq_castle_from=initsquare, sq_castle_to=initsquare;

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
}

static boolean is_legal(square sq_departure,
                        square sq_arrival,
                        square sq_capture)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = !nocontact(sq_departure,sq_arrival,sq_capture,koekofunc);

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
stip_length_type koeko_remove_illegal_moves_solve(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&is_legal);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STKoekoRemoveIllegalMoves);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_koeko(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
