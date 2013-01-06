#include "conditions/line_chameleon.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "pieces/walks.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static piece linechampiece(square sq_arrival)
{
  piece const pi_moving = e[sq_arrival];
  PieNam walk_moving = abs(pi_moving);
  PieNam walk_chameleonised = walk_moving;

  if (walk_moving==standard_walks[Queen]
      || walk_moving==standard_walks[Rook]
      || walk_moving==standard_walks[Bishop]
      || walk_moving==standard_walks[Knight])
    switch(sq_arrival%onerow-nr_of_slack_files_left_of_board)
    {
      case file_a:
      case file_h:
        walk_chameleonised = standard_walks[Rook];
        break;

      case file_b:
      case file_g:
        walk_chameleonised = standard_walks[Knight];
        break;

      case file_c:
      case file_f:
        walk_chameleonised = standard_walks[Bishop];
        break;

      case file_d:
        walk_chameleonised = standard_walks[Queen];
        break;

      case file_e:
        /* nothing */
        break;

      default:
        assert(0);
        break;
    }

  return pi_moving<vide ? -walk_chameleonised : walk_chameleonised;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     uninted immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type line_chameleon_arriving_adjuster_solve(slice_index si,
                                                         stip_length_type n)
{
  stip_length_type result;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;
  piece const substitute = linechampiece(sq_arrival);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (e[sq_arrival]!=substitute)
    move_effect_journal_do_piece_change(move_effect_reason_chameleon_movement,
                                        sq_arrival,
                                        substitute);

  result = solve(slices[si].next1,n);

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
