#include "position/board_transformation.h"
#include "position/position.h"
#include "debugging/assert.h"

static void transformBoard(SquareTransformation transformation)
{
  piece_walk_type t_e[nr_squares_on_board];
  Flags t_spec[nr_squares_on_board];
  imarr t_isquare;
  int i;

  /* save the position to be mirrored/rotated */
  for (i = 0; i<nr_squares_on_board; i++)
  {
    t_e[i] = get_walk_of_piece_on_square(boardnum[i]);
    t_spec[i] = being_solved.spec[boardnum[i]];
  }

  for (i = 0; i<maxinum; i++)
    t_isquare[i] = being_solved.isquare[i];

  /* now rotate/mirror */
  /* pieces */
  for (i = 0; i<nr_squares_on_board; i++)
  {
    square const sq = transformSquare(boardnum[i],transformation);

    switch (t_e[i])
    {
      case Empty:
        empty_square(sq);
        break;

      case Invalid:
        block_square(sq);
        break;

      default:
        occupy_square(sq,t_e[i],t_spec[i]);
        break;
    }
  }

  /* imitators */
  for (i= 0; i<maxinum; i++)
    being_solved.isquare[i]= transformSquare(t_isquare[i], transformation);
}

/* Add transforming the board to the current move of the current ply
 * @param reason reason for moving the king square
 * @param transformation how to transform the board
 */
void move_effect_journal_do_board_transformation(move_effect_reason_type reason,
                                                 SquareTransformation transformation)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_board_transformation,reason);

  TraceFunctionEntry(__func__);
  TraceValue("%u",transformation);
  TraceFunctionParamListEnd();

  entry->u.board_transformation.transformation = transformation;

  transformBoard(transformation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static SquareTransformation const inverse_transformation[] =
{
    rot270,
    rot180,
    rot90,
    mirra1h1,
    mirra1a8,
    mirra1h8,
    mirra8h1
};

static void undo_board_transformation(move_effect_journal_entry_type const *entry)
{
  SquareTransformation const transformation = entry->u.board_transformation.transformation;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  transformBoard(inverse_transformation[transformation]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_board_transformation(move_effect_journal_entry_type const *entry)
{
  SquareTransformation const transformation = entry->u.board_transformation.transformation;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  transformBoard(transformation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initalise the module */
void position_board_transformation_initialise(void)
{
  move_effect_journal_set_effect_doers(move_effect_board_transformation,
                                       &undo_board_transformation,
                                       &redo_board_transformation);
}
