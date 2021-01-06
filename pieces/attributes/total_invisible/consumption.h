#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_CONSUMPTION_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_CONSUMPTION_H

#include "position/position.h"

typedef struct
{
    unsigned int pawn_victims[nr_sides];
    boolean king[nr_sides];
    boolean move_after_victim[nr_sides];
} static_consumption_type;

extern static_consumption_type static_consumption;

typedef struct
{
    unsigned int fleshed_out[nr_sides];
    unsigned int placed[nr_sides];
    boolean claimed[nr_sides];
} dynamic_consumption_type;

void TraceConsumption(void);

extern dynamic_consumption_type current_consumption;

unsigned int nr_total_invisbles_consumed_for_side(Side side);

unsigned int nr_total_invisbles_consumed(void);

/* Determine the maximum number of placement allocations possible for both sides
 * @return the maximum number
 */
unsigned int nr_placeable_invisibles_for_both_sides(void);

/* Determine the maximum number of placement allocations possible for a specific side
 * @param side the side
 * @return the maximum number
 */
unsigned int nr_placeable_invisibles_for_side(Side side);

/* Allocate placement of an invisible, which may have already been claimed by
 * the side for a random move
 * @return false iff we have exhausted the invisibles contingent
 * @note modifies bookkeeping even on failure
 *       so restore bookkeeping after both success and failure
 */
boolean allocate_flesh_out_unplaced(Side side);

/* Allocate placement of an invisible, which may have already been claimed by
 * the side for a random move
 * @return false iff we have exhausted the invisibles contingent
 * @note modifies bookkeeping even on failure
 *       so restore bookkeeping after both success and failure
 */
boolean allocate_placed(Side side);

/* Allocate placement of an unclaimed invisible
 * @return false iff we have exhausted the invisibles contingent
 * @note modifies bookkeeping even on failure
 *       so restore bookkeeping after both success and failure
 */
boolean allocate_flesh_out_unclaimed(Side side);

/* Adapt bookkeeping to the fleshing out of an invisible of a side */
void allocate_flesh_out_placed(Side side);

#endif
