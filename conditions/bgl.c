#include "conditions/bgl.h"
#include "pypipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
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

/* Allocate a STBGLFilter slice.
 * @return index of allocated slice
 */
slice_index alloc_bgl_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STBGLFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type bgl_filter_attack(slice_index si, stip_length_type n)
{
  stip_length_type result;
  move_generation_elmt const * const move_gen_top = move_generation_stack+nbcou;
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
    result = attack(slices[si].u.pipe.next,n);
  else
    result = slack_length-2;

  if (BGL_values[White][nbply]!=BGL_infinity && (BGL_global || trait[nbply] == White))
    BGL_values[White][nbply] += diff;
  if (BGL_values[Black][nbply]!=BGL_infinity && (BGL_global || trait[nbply] == Black))
    BGL_values[Black][nbply] += diff;

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
stip_length_type bgl_filter_defend(slice_index si, stip_length_type n)
{
  stip_length_type result;
  move_generation_elmt const * const move_gen_top = move_generation_stack+nbcou;
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
    result = defend(slices[si].u.pipe.next,n);
  else
    result = slack_length-2;

  if (BGL_values[White][nbply]!=BGL_infinity && (BGL_global || trait[nbply] == White))
    BGL_values[White][nbply] += diff;
  if (BGL_values[Black][nbply]!=BGL_infinity && (BGL_global || trait[nbply] == Black))
    BGL_values[Black][nbply] += diff;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_bgl_filter(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_bgl_filter_slice();
    switch (st->context)
    {
      case stip_traversal_context_attack:
        attack_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_defense:
        defense_branch_insert_slices(si,&prototype,1);
        break;

      case stip_traversal_context_help:
        help_branch_insert_slices(si,&prototype,1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument slices with move tracers
 */
void stip_insert_bgl_filters(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,STMove,&insert_bgl_filter);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean eval_BGL(square sq_departure, square sq_arrival, square sq_capture)
{
  return BGL_move_diff_code[abs(sq_departure-sq_arrival)]
         <= (e[sq_capture]<vide ? BGL_values[White][nbply] : BGL_values[Black][nbply]);
}
