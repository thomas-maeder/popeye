#include "position/effects/total_side_exchange.h"
#include "position/position.h"
#include "debugging/assert.h"

static void do_total_side_exchange(void)
{
  {
    square const king_square_white = being_solved.king_square[White];
    being_solved.king_square[White] = being_solved.king_square[Black];
    being_solved.king_square[Black] = king_square_white;
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp; bnp++)
      if (!is_square_empty(*bnp))
      {
        Side const to = TSTFLAG(being_solved.spec[*bnp],White) ? Black : White;
        if (!TSTFLAG(being_solved.spec[*bnp],to))
        {
          --being_solved.number_of_pieces[advers(to)][get_walk_of_piece_on_square(*bnp)];
          piece_change_side(&being_solved.spec[*bnp]);
          occupy_square(*bnp,get_walk_of_piece_on_square(*bnp),being_solved.spec[*bnp]);
          ++being_solved.number_of_pieces[to][get_walk_of_piece_on_square(*bnp)];
        }
      }
  }
}

/* Execute a Polish type twinning
 */
void move_effect_journal_do_total_side_exchange(move_effect_reason_type reason)
{

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceFunctionParamListEnd();

  move_effect_journal_allocate_entry(move_effect_total_side_exchange,reason);
  do_total_side_exchange();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_twinning_total_side_exchange(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do_total_side_exchange();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_twinning_total_side_exchange(move_effect_journal_entry_type const *entry)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  do_total_side_exchange();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void position_total_side_exchange_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_total_side_exchange,
                                       &undo_twinning_total_side_exchange,
                                       &redo_twinning_total_side_exchange);
}
