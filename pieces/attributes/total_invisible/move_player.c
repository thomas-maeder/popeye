#include "pieces/attributes/total_invisible/move_player.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible.h"
#include "stipulation/stipulation.h"
#include "position/position.h"
#include "position/effects/piece_readdition.h"
#include "position/effects/null_move.h"
#include "solving/ply.h"
#include "solving/pipe.h"
#include "solving/move_generator.h"
#include "solving/has_solution_type.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

static void play_castling_with_invisible_partner(slice_index si,
                                                 square sq_departure_partner,
                                                 square sq_arrival_partner)
{
  Side const side = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure_partner);
  TraceSquare(sq_arrival_partner);
  TraceFunctionParamListEnd();

  if (is_square_empty(sq_departure_partner))
  {
    dynamic_consumption_type const save_consumption = current_consumption;
    if (allocate_flesh_out_unclaimed(trait[nbply]))
    {
      PieceIdType const id_partner = initialise_motivation(purpose_castling_partner,sq_departure_partner,
                                                           purpose_castling_partner,sq_arrival_partner);
      Flags spec = BIT(side)|BIT(Chameleon);

      SetPieceId(spec,id_partner);
      move_effect_journal_do_piece_readdition(move_effect_reason_castling_partner,
                                              sq_departure_partner,Rook,spec,side);
      pipe_solve_delegate(si);
      uninitialise_motivation(id_partner);
    }
    else
    {
      /* we have checked acts_when we generated the castling */
      assert(0);
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }
  else
  {
    move_effect_journal_do_null_effect();
    pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void total_invisible_special_moves_player_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt * const move_gen_top = move_generation_stack+curr;
    square const sq_departure = move_gen_top->departure;
    square const sq_capture = move_gen_top->capture;

    TraceSquare(sq_departure);
    TraceSquare(move_gen_top->arrival);
    TraceSquare(sq_capture);
    TraceEOL();

    if (sq_departure==move_by_invisible)
    {
      Side const side = trait[nbply];
      boolean const save_move_after_victim = static_consumption.move_after_victing[side];

      static_consumption.move_after_victing[side] = true;
      if (nr_total_invisbles_consumed()<=total_invisible_number)
        pipe_solve_delegate(si);
      else
        solve_result = this_move_is_illegal;
      static_consumption.move_after_victing[side] = save_move_after_victim;
    }
    else if (sq_departure>=capture_by_invisible)
    {
      PieceIdType const id_capturer = initialise_motivation(purpose_capturer,sq_departure,
                                                            purpose_capturer,sq_departure);
      Side const side = trait[nbply];
      Flags spec = BIT(side)|BIT(Chameleon);
      boolean const save_move_after_victim = static_consumption.move_after_victing[side];

      SetPieceId(spec,id_capturer);
      motivation[top_invisible_piece_id].levels.walk = decision_level_latest;
      motivation[top_invisible_piece_id].levels.from = decision_level_latest;
      move_effect_journal_do_piece_readdition(move_effect_reason_removal_of_invisible,
                                              sq_departure,Dummy,spec,side);

      static_consumption.move_after_victing[side] = true;
      if (nr_total_invisbles_consumed()<=total_invisible_number)
      {
        /* No adjustment of current_consumption.placed here!
         * The capture may be done by an existing invisible. We can only do the
         * adjustment when we flesh out this capture by inserting a new invisible.
         */
        pipe_solve_delegate(si);
      }
      else
        solve_result = this_move_is_illegal;

      static_consumption.move_after_victing[side] = save_move_after_victim;

      uninitialise_motivation(id_capturer);
    }
    else
      switch (sq_capture)
      {
        case pawn_multistep:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        case messigny_exchange:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        case kingside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;
          square const square_h = square_a+file_h;
          square const square_f = square_a+file_f;

          TraceText("kingside_castling\n");
          play_castling_with_invisible_partner(si,square_h,square_f);
          break;
        }

        case queenside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;
          square const square_d = square_a+file_d;

          TraceText("queenside_castling\n");
          play_castling_with_invisible_partner(si,square_a,square_d);
          break;
        }

        case no_capture:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        default:
          /* pawn captures total invisible? */
          if (is_square_empty(sq_capture))
          {
            PieceIdType const id_victim = initialise_motivation(purpose_victim,sq_capture,
                                                                purpose_victim,sq_capture);
            Side const side_victim = advers(SLICE_STARTER(si));
            Flags spec = BIT(side_victim)|BIT(Chameleon);
            boolean const save_move_after_victim = static_consumption.move_after_victing[side_victim];

            SetPieceId(spec,id_victim);
            motivation[top_invisible_piece_id].levels.walk = decision_level_latest;
            motivation[top_invisible_piece_id].levels.from = decision_level_latest;

            move_effect_journal_do_piece_readdition(move_effect_reason_removal_of_invisible,
                                                    sq_capture,Dummy,spec,side_victim);

            /* No adjustment of current_consumption.placed here! Another invisible may
             * have moved to sq_capture and serve as a victim.
             */

            ++static_consumption.pawn_victims[side_victim];
            static_consumption.move_after_victing[side_victim] = false;

            if (nr_total_invisbles_consumed()<=total_invisible_number)
              pipe_solve_delegate(si);
            else
              solve_result = this_move_is_illegal;

            static_consumption.move_after_victing[side_victim] = save_move_after_victim;
            --static_consumption.pawn_victims[side_victim];

            uninitialise_motivation(id_victim);
          }
          else
          {
            move_effect_journal_do_null_effect();
            pipe_solve_delegate(si);
          }
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
