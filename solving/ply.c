#include "solving/ply.h"
#include "pieces/walks/pawns/en_passant.h"
#include "solving/move_effect_journal.h"
#include "solving/post_move_iteration.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include <assert.h>
#include <string.h>

ply nbply = nil_ply;

ply parent_ply[maxply+1];

Side trait[maxply+1];

static ply ply_watermark;
static ply ply_stack[maxply+1];
static ply ply_stack_pointer;

/* resset the ply module */
void ply_reset(void)
{
  nbply = nil_ply;
  SET_CURRMOVE(nbply,nil_coup);
  ply_watermark = nil_ply;
}

/* Start a new ply as the child of the current ply, making the child the new
 * current ply
 * @param side the side at the move in the child ply
 */
void nextply(Side side)
{
  ply const parent = nbply;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side,"");
  TraceFunctionParamListEnd();

  assert(ply_watermark<maxply);

  ply_stack[ply_stack_pointer++] = nbply;
  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  current_move_id[nbply] = current_move_id[ply_watermark];
  ++ply_watermark;

  TraceValue("%u",parent);
  TraceValue("%u\n",nbply);

  parent_ply[nbply] = parent;

  trait[nbply] = side;

  move_effect_journal_base[nbply+1] = move_effect_journal_base[nbply];
  en_passant_top[nbply] = en_passant_top[nbply-1];

  ++post_move_iteration_id[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Start a new ply as a sibling of the current ply, making the child the new
 * current ply
 * @param side the side at the move in the child ply
 */
void siblingply(Side side)
{
  ply const elder = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(ply_watermark<maxply);

  ply_stack[ply_stack_pointer++] = nbply;
  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  current_move_id[nbply] = current_move_id[ply_watermark];
  ++ply_watermark;

  TraceValue("%u",elder);
  TraceValue("%u\n",nbply);

  parent_ply[nbply] = parent_ply[elder];

  trait[nbply] = side;

  move_effect_journal_base[nbply+1] = move_effect_journal_base[nbply];
  en_passant_top[nbply] = en_passant_top[nbply-1];

  ++post_move_iteration_id[nbply];

  INIT_CURRMOVE(nbply);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Copy the current ply, making the copy the new current ply */
void copyply(void)
{
  ply const original = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ply_stack[ply_stack_pointer++] = nbply;
  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  current_move_id[nbply] = current_move_id[ply_watermark];
  ++ply_watermark;

  parent_ply[nbply] = parent_ply[original];

  trait[nbply] = trait[original];

  move_effect_journal_base[nbply+1] = move_effect_journal_base[nbply];
  en_passant_top[nbply] = en_passant_top[nbply-1];

  ++post_move_iteration_id[nbply];

  {
    unsigned int const nr_moves = current_move[original]-current_move[original-1];
    memcpy(&move_generation_stack[CURRMOVE_OF_PLY(nbply)+1],
           &move_generation_stack[CURRMOVE_OF_PLY(original-1)+1],
           nr_moves*sizeof move_generation_stack[0]);
    current_move[nbply] += nr_moves;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* End the current ply, making the previous current ply the current ply again */
void finply()
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nbply==ply_watermark);
  --ply_watermark;

  nbply = ply_stack[--ply_stack_pointer];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
