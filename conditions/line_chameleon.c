#include "conditions/line_chameleon.h"
#include "conditions/andernach.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/castling.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>


static piece linechampiece(piece p, square sq)
{
  piece pja = p;

  if (CondFlag[leofamily])
  {
    switch (abs(p))
    {
      case leob:
      case maob:
      case vaob:
      case paob:
        switch(sq%onerow)
        {
          case 8:  case 15:   pja= paob; break;
          case 9:  case 14:   pja= maob; break;
          case 10: case 13:   pja= vaob; break;
          case 11:     pja= leob; break;
        }
        break;

      default:
        break;
    }

    return (pja != p && p < vide) ? - pja : pja;
  }
  else if (CondFlag[cavaliermajeur])
  {
    switch (abs(p))
    {
      case db:
      case nb:
      case fb:
      case tb:
        switch(sq%onerow)
        {
          case 8:  case 15:   pja= tb;  break;
          case 9:  case 14:   pja= nb;  break;
          case 10: case 13:   pja= fb;  break;
          case 11:     pja= db;  break;
        }
        break;

      default:
        break;
    }

    return (pja != p && p < vide) ? - pja : pja;
  }
  else
  {
    switch (abs(p))
    {
      case db:
      case cb:
      case fb:
      case tb:
        switch(sq%onerow)
        {
          case 8:  case 15:   pja= tb;  break;
          case 9:  case 14:   pja= cb;  break;
          case 10: case 13:   pja= fb;  break;
          case 11:     pja= db;  break;
        }
        break;

      default:
        break;
    }

    return (pja != p && p < vide) ? - pja : pja;
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
stip_length_type line_chameleon_arriving_adjuster_attack(slice_index si,
                                                         stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  replace_arriving_piece(linechampiece(e[sq_arrival],sq_arrival));
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
stip_length_type line_chameleon_arriving_adjuster_defend(slice_index si,
                                                         stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  replace_arriving_piece(linechampiece(e[sq_arrival],sq_arrival));
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_line_chameleon_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STLineChameleonArrivingAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
