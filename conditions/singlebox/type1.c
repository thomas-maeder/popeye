#include "conditions/singlebox/type1.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>

PieNam next_singlebox_prom(PieNam p, Side c)
{
  PieNam pprom;
  PieNam result = Empty;

  TraceFunctionEntry(__func__);
  TracePiece(p);
  TraceEnumerator(Side,c,"");
  TraceFunctionParamListEnd();

  for (pprom = getprompiece[p]; pprom!=Empty; pprom = getprompiece[pprom])
  {
    piece const colored = c==White ? pprom : -pprom;
    if (pprom!=Pawn && nbpiece[colored]<nr_piece(game_array)[pprom])
    {
      result = pprom;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TracePiece(result);
  TraceFunctionResultEnd();
  return result;
}

static boolean singlebox_officer_out_of_box(void)
{
  boolean result = false;
  piece p;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (p = roib; p<=fb; ++p)
    if (nbpiece[p]>nr_piece(game_array)[p]
        || nbpiece[-p]>nr_piece(game_array)[-p])
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean singlebox_pawn_out_of_box(void)
{
  boolean const result = (nbpiece[pb]>nr_piece(game_array)[pb]
                          || nbpiece[pn]>nr_piece(game_array)[pn]);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the move just played is legal according to Singlebox Type 1
 * @return true iff the move is legal
 */
boolean singlebox_type1_illegal(void)
{
  return singlebox_officer_out_of_box() || singlebox_pawn_out_of_box();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_singlebox_type1(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STSingleBoxType1LegalityTester);

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
stip_length_type singlebox_type1_legality_tester_solve(slice_index si,
                                                        stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (singlebox_type1_illegal())
    result = slack_length-2;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
