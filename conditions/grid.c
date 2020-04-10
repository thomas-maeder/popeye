#include "conditions/grid.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/pipe.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

grid_type_type grid_type;
static unsigned int gridlines[112][4];
static unsigned int numgridlines;

void IntialiseIrregularGridLines(void)
{
  grid_type = grid_irregular;
  numgridlines = 0;
}

boolean PushIrregularGridLine(unsigned int file,
                              unsigned int row,
                              unsigned int length,
                              gridline_direction dir)
{
  if (numgridlines<100)
  {
    gridlines[numgridlines][0] = 2*file;
    gridlines[numgridlines][1] = 2*row;
    gridlines[numgridlines][2] = 2*file;
    gridlines[numgridlines][3] = 2*row;
    gridlines[numgridlines][dir==gridline_horizonal ? 2 : 3] += 2*length;

    ++numgridlines;

    return true;
  }
  else
    return false;
}

boolean CrossesGridLines(square sq_departure, square sq_arrival)
{
  unsigned int const X1 = (unsigned int)(2*(sq_departure-nr_of_slack_files_left_of_board) % onerow + 1);
  unsigned int const Y1 = (unsigned int)(2*(sq_departure/onerow - nr_of_slack_rows_below_board) +1);
  unsigned int const X2 = (unsigned int)(2*(sq_arrival-nr_of_slack_files_left_of_board) % onerow + 1);
  unsigned int const Y2 = (unsigned int)(2*(sq_arrival/onerow - nr_of_slack_rows_below_board) +1);
  int const dX = (int)X2-(int)X1;
  int const dY = (int)Y2-(int)Y1;

  {
    unsigned int i;
    for (i = 0; i<numgridlines; i++)
    {
      unsigned int const x1 = gridlines[i][0];
      unsigned int const y1 = gridlines[i][1];
      unsigned int const x2 = gridlines[i][2];
      unsigned int const y2 = gridlines[i][3];
      int const dx = (int)(x2-x1);
      int const dy = (int)(y2-y1);

      int const v = dY*dx-dX*dy;
      if (v!=0)
      {
        int const diffx = (int)x1-(int)X1;
        int const diffy = (int)y1-(int)Y1;
        int const u1 = dX*diffy-dY*diffx;
        if (v<0 ? (u1<=0 && u1>=v) : (u1>=0 && u1<=v))
        {
          int const u2 = dx*diffy-dy*diffx;
          if (v<0 ? (u2<=0 && u2>=v) : (u2>=0 && u2<=v))
            return true;
        }
      }
    }
  }

  return false;
}

static boolean is_not_in_same_cell(numecoup n)
{
  return GridLegal(move_generation_stack[n].departure,
                   move_generation_stack[n].arrival);
}

/* Validate the geometry of observation according to Grid Chess
 * @return true iff the observation is valid
 */
boolean grid_validate_observation_geometry(slice_index si)
{
  return (is_not_in_same_cell(CURRMOVE_OF_PLY(nbply))
          && pipe_validate_observation_recursive_delegate(si));
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
void grid_remove_illegal_moves_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(MOVEBASE_OF_PLY(nbply),&is_not_in_same_cell);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STGridRemoveIllegalMoves);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise solving in Grid Chess
 * @param si identifies the root slice of the stipulation
 */
void grid_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  stip_instrument_observation_geometry_validation(si,
                                                  nr_sides,
                                                  STGridRemoveIllegalMoves);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
