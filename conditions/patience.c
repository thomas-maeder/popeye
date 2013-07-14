#include "conditions/patience.h"
#include "pydata.h"
#include "solving/castling.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>

boolean PatienceB;
static square sqdep[maxply+1];

static boolean patience_legal()
{
  square bl_last_vacated = initsquare;
  square wh_last_vacated = initsquare;
  ply ply;
  /* n.b. inventor rules that R squares are forbidden after
     castling but not yet implemented */

  for (ply = nbply-1; ply>1 && !bl_last_vacated; --ply)
    if (trait[ply]==Black)
      bl_last_vacated = sqdep[ply];

  for (ply = nbply-1; ply>1 && !wh_last_vacated; --ply)
    if (trait[ply]==White)
      wh_last_vacated = sqdep[ply];

  return ((wh_last_vacated==initsquare || is_square_empty(wh_last_vacated))
          && (bl_last_vacated==initsquare || is_square_empty(bl_last_vacated)));
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
  solving_disable_castling(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
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
    result = previous_move_is_illegal;
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
