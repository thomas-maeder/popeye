#include "solving/ply.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/pawns/promotion.h"
#include "solving/move_effect_journal.h"
#include "solving/post_move_iteration.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include "debugging/assert.h"
#include <string.h>

ply nbply = ply_nil;

ply parent_ply[maxply+1];

Side trait[maxply+1];

static ply ply_watermark;
static ply ply_stack[maxply+1];
static ply ply_stack_pointer;

/* reset the ply module */
void ply_reset(void)
{
  SET_CURRMOVE(nbply,nil_coup);
}

/* Start a new ply as the child of the current ply, making the child the new
 * current ply
 * @param side the side at the move in the child ply
 */
void nextply(Side side)
{
  ply const parent = nbply;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  assert(ply_watermark<maxply);

  ply_stack[ply_stack_pointer++] = nbply;
  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  current_move_id[nbply] = current_move_id[ply_watermark];
  ++ply_watermark;

  parent_ply[nbply] = parent;

  trait[nbply] = side;

  move_effect_journal_base[nbply+1] = move_effect_journal_base[parent+1];
  move_effect_journal_base[nbply] = move_effect_journal_base[nbply+1];

  TraceValue("%u",parent);
  TraceValue("%u",ply_watermark);
  TraceValue("%u",nbply);
  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceEOL();

  en_passant_top[nbply] = en_passant_top[nbply-1];
  promotion_horizon[nbply] = move_effect_journal_base[nbply]+move_effect_journal_index_offset_other_effects-1;

  post_move_iteration_init_ply();

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

  parent_ply[nbply] = parent_ply[elder];

  TraceValue("%u",elder);
  TraceValue("%u",parent_ply[nbply]);
  TraceValue("%u",nbply);
  TraceEOL();

  trait[nbply] = side;

  move_effect_journal_base[nbply+1] = move_effect_journal_base[elder+1];
  move_effect_journal_base[nbply] = move_effect_journal_base[nbply+1];
  en_passant_top[nbply] = en_passant_top[nbply-1];
  promotion_horizon[nbply] = move_effect_journal_base[nbply]+move_effect_journal_index_offset_other_effects-1;

  post_move_iteration_init_ply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Copy the current ply, making the copy the new current ply */
void copyply(void)
{
  ply const original = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",ply_stack_pointer);
  TraceValue("%u",ply_watermark);
  TraceEOL();

  ply_stack[ply_stack_pointer++] = nbply;
  nbply = ply_watermark+1;
  current_move[nbply] = current_move[ply_watermark];
  current_move_id[nbply] = current_move_id[ply_watermark];
  ++ply_watermark;

  parent_ply[nbply] = parent_ply[original];

  trait[nbply] = trait[original];

  move_effect_journal_base[nbply+1] = move_effect_journal_base[nbply];
  en_passant_top[nbply] = en_passant_top[nbply-1];
  promotion_horizon[nbply] = move_effect_journal_base[nbply]+move_effect_journal_index_offset_other_effects-1;

  post_move_iteration_init_ply();

  {
    unsigned int const nr_moves = CURRMOVE_OF_PLY(original)-MOVEBASE_OF_PLY(original);
    memcpy(&move_generation_stack[MOVEBASE_OF_PLY(nbply)+1],
           &move_generation_stack[MOVEBASE_OF_PLY(original)+1],
           nr_moves*sizeof move_generation_stack[0]);
    current_move[nbply] += nr_moves;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* End the current ply, making the previous current ply the current ply again */
void finply(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

//  assert(post_move_iteration_ply_was_ended());

  TraceValue("%u",nbply);
  TraceValue("%u",ply_watermark);
  TraceEOL();
//  assert(nbply==ply_watermark);
//  --ply_watermark;
  ply_watermark = nbply-1;

  nbply = ply_stack[--ply_stack_pointer];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
