#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

static_consumption_type static_consumption = { 0 };

dynamic_consumption_type current_consumption = { 0 };

void TraceConsumption(void)
{
  TraceValue("%u",current_consumption.fleshed_out[White]);
  TraceValue("%u",current_consumption.fleshed_out[Black]);
  TraceValue("%u",current_consumption.placed[White]);
  TraceValue("%u",current_consumption.placed[Black]);
  TraceValue("%u",current_consumption.claimed[White]);
  TraceValue("%u",current_consumption.claimed[Black]);
  TraceValue("%u",current_consumption.is_king_unplaced[White]);
  TraceValue("%u",current_consumption.is_king_unplaced[Black]);
  TraceValue("%u",static_consumption.pawn_victims[White]);
  TraceValue("%u",static_consumption.pawn_victims[Black]);
  TraceValue("%u",static_consumption.king[White]);
  TraceValue("%u",static_consumption.king[Black]);
  TraceEOL();
}

unsigned int nr_total_invisbles_consumed_for_side(Side side)
{
  unsigned int result = (current_consumption.fleshed_out[side]
                         + current_consumption.placed[side]
                         + current_consumption.claimed[side]);

  if (!current_consumption.claimed[side]
      && current_consumption.placed[side]==0
      && current_consumption.is_king_unplaced[side])
    ++result;

  if ((static_consumption.pawn_victims[side]+static_consumption.king[side])
      >result)
    result = (static_consumption.pawn_victims[side]+static_consumption.king[side]);

  if ((static_consumption.pawn_victims[side]+static_consumption.move_after_victim[side])
      >result)
    result = (static_consumption.pawn_victims[side]+static_consumption.move_after_victim[side]);

  return result;
}

unsigned int nr_total_invisbles_consumed(void)
{
  unsigned int result;

  result = (nr_total_invisbles_consumed_for_side(White)
            +nr_total_invisbles_consumed_for_side(Black));

  return result;
}

/* Determine the maximum number of placement allocations possible for both sides
 * @return the maximum number
 */
unsigned int nr_placeable_invisibles_for_both_sides(void)
{
  assert(total_invisible_number
         >=(current_consumption.fleshed_out[White]
            +current_consumption.fleshed_out[Black]
            +current_consumption.placed[White]
            +current_consumption.placed[Black]
            + (current_consumption.claimed[White] && current_consumption.claimed[Black])
           )
        );

  return (total_invisible_number
          - ((current_consumption.fleshed_out[White]
              +current_consumption.fleshed_out[Black])
              +current_consumption.placed[White]
              +current_consumption.placed[Black]
             + (current_consumption.claimed[White] && current_consumption.claimed[Black])
            )
         );
}

/* Determine the maximum number of placement allocations possible for a specific side
 * @param side the side
 * @return the maximum number
 */
unsigned int nr_placeable_invisibles_for_side(Side side)
{
  assert(total_invisible_number
         >= (current_consumption.fleshed_out[White]
             +current_consumption.fleshed_out[Black]
             +current_consumption.placed[White]
             +current_consumption.placed[Black]
             +current_consumption.claimed[advers(side)]
            )
        );

  return (total_invisible_number
          - ((current_consumption.fleshed_out[White]
              +current_consumption.fleshed_out[Black]
              +current_consumption.placed[White]
              +current_consumption.placed[Black])
              +current_consumption.claimed[advers(side)]
            )
         );
}

/* Allocate placement of an invisible, which may have already been claimed by
 * the side for a random move
 * @return false iff we have exhausted the invisibles contingent
 * @note modifies bookkeeping even on failure
 *       so restore bookkeeping after both success and failure
 */
boolean allocate_flesh_out_unplaced(Side side)
{
  ++current_consumption.fleshed_out[side];
  current_consumption.claimed[side] = false;

  return nr_total_invisbles_consumed()<=total_invisible_number;
}

/* Allocate placement of an invisible, which may have already been claimed by
 * the side for a random move
 * @return false iff we have exhausted the invisibles contingent
 * @note modifies bookkeeping even on failure
 *       so restore bookkeeping after both success and failure
 */
boolean allocate_placed(Side side)
{
  ++current_consumption.placed[side];
  current_consumption.claimed[side] = false;

  return nr_total_invisbles_consumed()<=total_invisible_number;
}

/* Allocate placement of an unclaimed invisible
 * @return false iff we have exhausted the invisibles contingent
 * @note modifies bookkeeping even on failure
 *       so restore bookkeeping after both success and failure
 */
boolean allocate_flesh_out_unclaimed(Side side)
{
  ++current_consumption.fleshed_out[side];

  return nr_total_invisbles_consumed()<=total_invisible_number;
}

/* Adapt bookkeeping to the fleshing out of an invisible of a side */
void allocate_flesh_out_placed(Side side)
{
  assert(current_consumption.placed[side]>0);
  --current_consumption.placed[side];
  ++current_consumption.fleshed_out[side];
}
