#include "conditions/bgl.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

long int BGL_values[nr_sides][maxply+1];
boolean BGL_global;

static long int BGL_move_diff_code[square_h8 - square_a1 + 1] =
{
 /* left/right   */        0,   100,   200,   300,  400,  500,  600,  700,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 1 left  up   */            707,  608,  510,  412,  316,   224,   141,
 /* 1 right up   */        100,   141,   224,  316,  412,  510,  608,  707,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 2 left  up   */            728,  632,  539,  447,  361,   283,   224,
 /* 2 right up   */        200,   224,   283,  361,  447,  539,  632,  728,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 3 left  up   */            762,  671,  583,  500,  424,  361,  316,
 /* 3 right up   */        300,  316,  361,  424,  500,  583,  671,  762,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 4 left  up   */            806,  721,  640,  566,  500,  447,  412,
 /* 4 right up   */       400,  412,  447,  500,  566,  640,  721,  806,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 5 left  up   */            860,  781,  707,  640,  583,  539,  510,
 /* 5 right up   */       500,  510,  539,  583,  640,  707,  781,  860,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 6 left  up   */            922,  849,  781,  721,  671,  632,  608,
 /* 6 right up   */       600,  608,  632,  671,  721,  781,  849,  922,
 /* dummies      */       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,
 /* 7 left  up   */            990,  922,  860,  806,  762,  728,  707,
 /* 7 right up   */       700,  707,  728,  762,  806,  860,  922,  990
};

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
stip_length_type bgl_filter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  move_generation_elmt const * const move_gen_top = move_generation_stack+current_move[nbply];
  int const move_diff = move_gen_top->departure-move_gen_top->arrival;
  long int const diff = BGL_move_diff_code[abs(move_diff)];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (BGL_values[White][nbply]!=BGL_infinity && (BGL_global || trait[nbply] == White))
    BGL_values[White][nbply] -= diff;
  if (BGL_values[Black][nbply]!=BGL_infinity && (BGL_global || trait[nbply] == Black))
    BGL_values[Black][nbply] -= diff;

  if (BGL_values[trait[nbply]][nbply]>=0)
    result = solve(slices[si].next1,n);
  else
    result = previous_move_is_illegal;

  if (BGL_values[White][nbply]!=BGL_infinity && (BGL_global || trait[nbply] == White))
    BGL_values[White][nbply] += diff;
  if (BGL_values[Black][nbply]!=BGL_infinity && (BGL_global || trait[nbply] == Black))
    BGL_values[Black][nbply] += diff;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_bgl_filters(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STBGLFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean eval_BGL(square sq_departure, square sq_arrival, square sq_capture)
{
  return BGL_move_diff_code[abs(sq_departure-sq_arrival)]
         <= (e[sq_capture]<vide ? BGL_values[White][nbply] : BGL_values[Black][nbply]);
}
