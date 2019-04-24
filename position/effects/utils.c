#include "position/effects/utils.h"
#include "conditions/actuated_revolving_centre.h"
#include "position/effects/piece_removal.h"
#include "position/effects/piece_creation.h"
#include "position/effects/piece_movement.h"
#include "position/effects/piece_exchange.h"
#include "position/effects/board_transformation.h"
#include "debugging/assert.h"

/* Follow the captured or a moved piece through the "other" effects of a move
 * @param ply ply in which the move was played
 * @param followed_id id of the piece to be followed
 * @param pos position of the piece after the inital capture removal and piece movement have taken place
 * @return the position of the piece with the "other" effect applied
 *         initsquare if the piece is not on the board after the "other" effects
 */
square move_effect_journal_follow_piece_through_other_effects(ply ply,
                                                              PieceIdType followed_id,
                                                              square pos)
{
  move_effect_journal_index_type const base = move_effect_journal_base[ply];
  move_effect_journal_index_type const top = move_effect_journal_base[ply+1];
  move_effect_journal_index_type other;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply);
  TraceFunctionParam("%x",followed_id);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  TraceValue("%u",followed_id);TraceEOL();
  assert(followed_id!=NullPieceId);

  for (other = base+move_effect_journal_index_offset_other_effects;
       other<top;
       ++other)
  {
    TraceValue("%u",move_effect_journal[other].type);
    TraceEOL();
    switch (move_effect_journal[other].type)
    {
      case move_effect_piece_removal:
        pos = position_piece_removal_follow_piece(followed_id,other,pos);
        break;

      case move_effect_piece_readdition:
      case move_effect_piece_creation:
        pos = position_piece_creation_follow_piece(followed_id,other,pos);
        break;

      case move_effect_piece_movement:
        pos = position_piece_movement_follow_piece(followed_id,other,pos);
        break;

      case move_effect_piece_exchange:
        pos = position_piece_exchange_follow_piece(followed_id,other,pos);
        break;

      case move_effect_board_transformation:
        pos = position_board_transformation_follow_piece(followed_id,other,pos);
        break;

      case move_effect_centre_revolution:
        pos = actuated_revolving_centre_revolve_square(pos);
        break;

      case move_effect_none:
      case move_effect_no_piece_removal:
      case move_effect_walk_change:
      case move_effect_side_change:
      case move_effect_king_square_movement:
      case move_effect_flags_change:
      case move_effect_imitator_addition:
      case move_effect_imitator_movement:
      case move_effect_remember_ghost:
      case move_effect_forget_ghost:
      case move_effect_half_neutral_deneutralisation:
      case move_effect_half_neutral_neutralisation:
      case move_effect_square_block:
      case move_effect_bgl_adjustment:
      case move_effect_strict_sat_adjustment:
      case move_effect_disable_castling_right:
      case move_effect_enable_castling_right:
      case move_effect_remember_ep_capture_potential:
      case move_effect_remember_duellist:
      case move_effect_remember_parachuted:
      case move_effect_remember_volcanic:
      case move_effect_swap_volcanic:
      case move_effect_revelation_of_new_invisible:
      case move_effect_revelation_of_castling_partner:
      case move_effect_revelation_of_placed_invisible:
        /* nothing */
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceSquare(pos);
  TraceFunctionResultEnd();
  return pos;
}

