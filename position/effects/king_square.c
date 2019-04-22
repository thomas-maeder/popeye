#include "position/effects/king_square.h"
#include "position/position.h"
#include "conditions/actuated_revolving_centre.h"
#include "solving/pipe.h"
#include "debugging/assert.h"

/* Update the king squares according to the effects since king_square_horizon
 * @note Updates king_square_horizon; solvers invoking this function should
 *       reset king_square_horizon to its previous value before returning
 */
void update_king_squares(void)
{
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  square new_king_square[nr_sides] = { being_solved.king_square[0], being_solved.king_square[1] };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (; king_square_horizon<top; ++king_square_horizon)
    switch (move_effect_journal[king_square_horizon].type)
    {
      case move_effect_piece_removal:
      {
        Flags const spec_captured = move_effect_journal[king_square_horizon].u.piece_removal.flags;
        if (TSTFLAG(spec_captured,Royal))
        {
          if (TSTFLAG(spec_captured,White))
            new_king_square[White] = initsquare;
          if (TSTFLAG(spec_captured,Black))
            new_king_square[Black] = initsquare;
        }
        break;
      }

      case move_effect_piece_readdition:
      case move_effect_piece_creation:
      {
        Flags const addedspec = move_effect_journal[king_square_horizon].u.piece_addition.added.flags;
        if (TSTFLAG(addedspec,Royal))
        {
          square const on = move_effect_journal[king_square_horizon].u.piece_addition.added.on;
          if (new_king_square[White]==initsquare && TSTFLAG(addedspec,White))
            new_king_square[White] = on;
          if (new_king_square[Black]==initsquare && TSTFLAG(addedspec,Black))
            new_king_square[Black] = on;
        }
        break;
      }

      case move_effect_piece_movement:
      {
        square const from = move_effect_journal[king_square_horizon].u.piece_movement.from;
        square const to = move_effect_journal[king_square_horizon].u.piece_movement.to;
        if (to!=from)
        {
          Flags const movingspec = move_effect_journal[king_square_horizon].u.piece_movement.movingspec;
          if (TSTFLAG(movingspec,Royal))
          {
            if (TSTFLAG(movingspec,White))
              new_king_square[White] = to;
            if (TSTFLAG(movingspec,Black))
              new_king_square[Black] = to;
          }
        }
        break;
      }

      case move_effect_piece_exchange:
      {
        square const from = move_effect_journal[king_square_horizon].u.piece_exchange.from;
        Flags const fromflags = move_effect_journal[king_square_horizon].u.piece_exchange.fromflags;
        square const to = move_effect_journal[king_square_horizon].u.piece_exchange.to;
        Flags const toflags = move_effect_journal[king_square_horizon].u.piece_exchange.toflags;

        if (TSTFLAG(fromflags,Royal))
        {
          if (TSTFLAG(fromflags,White))
            new_king_square[White] = to;
          if (TSTFLAG(fromflags,Black))
            new_king_square[Black] = to;
        }

        if (TSTFLAG(toflags,Royal))
        {
          if (TSTFLAG(toflags,White))
            new_king_square[White] = from;
          if (TSTFLAG(toflags,Black))
            new_king_square[Black] = from;
        }

        TraceSquare(new_king_square[White]);
        TraceSquare(new_king_square[Black]);
        TraceEOL();
        break;
      }

      case move_effect_flags_change:
      {
        square const on = move_effect_journal[king_square_horizon].u.flags_change.on;
        Flags const from = move_effect_journal[king_square_horizon].u.flags_change.from;
        Flags const to = move_effect_journal[king_square_horizon].u.flags_change.to;

        if (!TSTFLAG(from,Royal) && TSTFLAG(to,Royal))
        {
          if (TSTFLAG(to,White))
            new_king_square[White] = on;
          if (TSTFLAG(to,Black))
            new_king_square[Black] = on;
        }
        if (TSTFLAG(from,Royal) && !TSTFLAG(to,Royal))
        {
          if (TSTFLAG(to,White))
            new_king_square[White] = initsquare;
          if (TSTFLAG(to,Black))
            new_king_square[Black] = initsquare;
        }
        break;
      }

      case move_effect_board_transformation:
      {
        SquareTransformation const transformation = move_effect_journal[king_square_horizon].u.board_transformation.transformation;
        if (new_king_square[White]!=initsquare)
          new_king_square[White] = transformSquare(new_king_square[White],
                                                   transformation);
        if (new_king_square[Black]!=initsquare)
          new_king_square[Black] = transformSquare(new_king_square[Black],
                                                   transformation);
        break;
      }

      case move_effect_centre_revolution:
      {
        {
          square revolved = actuated_revolving_centre_revolve_square(new_king_square[White]);
          if (revolved!=initsquare)
            new_king_square[White] = revolved;
        }
        {
          square revolved = actuated_revolving_centre_revolve_square(new_king_square[Black]);
          if (revolved!=initsquare)
            new_king_square[Black] = revolved;
        }
        break;
      }

      case move_effect_half_neutral_neutralisation:
      {
        Side const from = move_effect_journal[king_square_horizon].u.half_neutral_phase_change.side;
        square const on = move_effect_journal[king_square_horizon].u.half_neutral_phase_change.on;
        if (new_king_square[from]==on)
          new_king_square[advers(from)] = on;
        break;
      }

      case move_effect_half_neutral_deneutralisation:
      {
        Side const to = move_effect_journal[king_square_horizon].u.half_neutral_phase_change.side;
        square const on = move_effect_journal[king_square_horizon].u.half_neutral_phase_change.on;
        if (new_king_square[advers(to)]==on)
          new_king_square[advers(to)] = initsquare;
        break;
      }

      default:
        break;
    }

  if (new_king_square[White]!=being_solved.king_square[White])
    move_effect_journal_do_king_square_movement(move_effect_no_reason,
                                                White,
                                                new_king_square[White]);
  if (new_king_square[Black]!=being_solved.king_square[Black])
    move_effect_journal_do_king_square_movement(move_effect_no_reason,
                                                Black,
                                                new_king_square[Black]);

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
void king_square_updater_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    move_effect_journal_index_type const save_horizon = king_square_horizon;
    update_king_squares();
    pipe_solve_delegate(si);
    king_square_horizon = save_horizon;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Add king square piece_movement to the current move of the current ply
 * @param reason reason for moving the king square
 * @param side whose king square to move
 * @param to where to move the king square
 */
void move_effect_journal_do_king_square_movement(move_effect_reason_type reason,
                                                 Side side,
                                                 square to)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_king_square_movement,reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",reason);
  TraceEnumerator(Side,side);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  entry->u.king_square_movement.side = side;
  entry->u.king_square_movement.from = being_solved.king_square[side];
  entry->u.king_square_movement.to = to;

  being_solved.king_square[side] = to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_king_square_movement(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.king_square_movement.side;
  square const from = entry->u.king_square_movement.from;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side);
  TraceSquare(from);
  TraceSquare(entry->u.king_square_movement.to);
  TraceEOL();

  assert(being_solved.king_square[side]==entry->u.king_square_movement.to);

  being_solved.king_square[side] = from;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_king_square_movement(move_effect_journal_entry_type const *entry)
{
  Side const side = entry->u.king_square_movement.side;
  square const to = entry->u.king_square_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side);
  TraceSquare(entry->u.king_square_movement.from);
  TraceSquare(to);
  TraceSquare(being_solved.king_square[side]);
  TraceEOL();

  assert(being_solved.king_square[side]==entry->u.king_square_movement.from);

  being_solved.king_square[side] = to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void king_square_initialise(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  being_solved.king_square[White] = initsquare;
  being_solved.king_square[Black] = initsquare;

  move_effect_journal_set_effect_doers(move_effect_king_square_movement,
                                       &undo_king_square_movement,
                                       &redo_king_square_movement);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
