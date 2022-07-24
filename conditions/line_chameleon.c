#include "conditions/line_chameleon.h"
#include "pieces/walks/walks.h"
#include "position/position.h"
#include "position/effects/walk_change.h"
#include "position/effects/utils.h"
#include "stipulation/pipe.h"
#include "solving/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include "debugging/assert.h"

static piece_walk_type linechampiece(square sq_arrival)
{
  piece_walk_type walk_moving = get_walk_of_piece_on_square(sq_arrival);
  piece_walk_type walk_chameleonised = walk_moving;

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

  return walk_chameleonised;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void line_chameleon_arriving_adjuster_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
    PieceIdType const moving_id = GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec);
    square const pos = move_effect_journal_follow_piece_through_other_effects(nbply,
                                                                              moving_id,
                                                                              sq_arrival);
    piece_walk_type const substitute = linechampiece(pos);
    if (get_walk_of_piece_on_square(pos)!=substitute)
      move_effect_journal_do_walk_change(move_effect_reason_chameleon_movement,
                                          pos,
                                          substitute);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void solving_insert_line_chameleon_chess(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STLineChameleonArrivingAdjuster);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
