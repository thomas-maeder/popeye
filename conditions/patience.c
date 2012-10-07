#include "conditions/patience.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>

boolean PatienceB;
static square sqdep[maxply+1];

static boolean patience_legal()
{
  square bl_last_vacated= initsquare, wh_last_vacated= initsquare;
  ply nply;
  /* n.b. inventor rules that R squares are forbidden after
     castling but not yet implemented */

  for (nply= nbply - 1 ; nply > 1 && !bl_last_vacated ; nply--)
    if (trait[nply] == Black)
      bl_last_vacated= sqdep[nply];
  for (nply= nbply - 1 ; nply > 1 && !wh_last_vacated ; nply--)
    if (trait[nply] == White)
      wh_last_vacated= sqdep[nply];
  return !((wh_last_vacated && e[wh_last_vacated]) ||
           (bl_last_vacated && e[bl_last_vacated]));
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_patience_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STPatienceChessLegalityTester);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type patience_chess_legality_tester_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  /* don't call patience_legal if TypeB as obs > vide ! */
  if (!PatienceB && !patience_legal())
    result = slack_length-2;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/*
verify_position
  if (!CondFlag[patience]) {
    PatienceB = false;
  }

play_move
  sqdep[nbply] = sq_departure;

if (PatienceB) {
  ply nply;
  e[sq_departure]= obs;
  for (nply= nbply - 1 ; nply > 1 ; nply--) {
    if (trait[nply] == trait_ply) {
      e[sqdep[nply]]= vide;
    }
  }
}

retract_move

  if (PatienceB) {
    ply nply;
    for (nply= nbply - 1 ; nply > 1 ; nply--) {
      if (trait[nply] == trait[nbply]) {
        e[sqdep[nply]]= obs;
      }
    }
  }

WriteConditions

    if ((cond == patience) && PatienceB) {
      strcat(CondLine, "    ");
      strcat(CondLine, VariantTypeString[UserLanguage][TypeB]);
    }

ParseCond
      case patience:
        tok = ParseVariant(&PatienceB, gpType);
        break;
 */
