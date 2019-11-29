#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_TABOO_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_TABOO_H

#include "position/position.h"
#include "solving/ply.h"

/* remember taboos so that the move to be played in ply nbply won't be obstructed
 */
void remember_taboos_for_current_move(void);

/* forget taboos so that the move to be played in ply nbply won't be obstructed
 */
void forget_taboos_for_current_move(void);

/* remember a taboo on a particular square so that no other piece of side side
 * is added on or moves to s when the move of ply ply is to be played
 * @param s the taboo square
 * @param side the side for which to remember a taboo
 * @param ply the taboo applies before the move of this ply
 */
void remember_taboo_on_square(square s, Side side, ply ply);

/* forget a taboo on a particular square so that no other piece of side side
 * is added on or moves to s when the move of ply ply is to be played
 * @param s the taboo square
 * @param side the side for which to forget a taboo
 * @param ply the taboo applies before the move of this ply
 */
void forget_taboo_on_square(square s, Side side, ply ply);

/* Will moving a piece of a particular side to a particular square violate a taboo in the
 * future (i.e. after ply nbply)?
 * @param s the square
 * @param side the side
 * @return true iff a taboo will be violated
 */
boolean will_be_taboo(square s, Side side);

/* Would moving a piece of a particular side to a particular square have violated a taboo
 * in the past (i.e. before ply nbply)?
 * @param s the square
 * @param side the side
 * @return true iff a taboo will be violated
 */
boolean was_taboo(square s, Side side);

/* Does moving a piece of a particular side to a particular square violate a taboo currently
 * (i.e. in ply nbply)?
 * @param s the square
 * @param side the side
 * @return true iff a taboo will be violated
 */
boolean is_taboo(square s, Side side);

boolean is_taboo_violation_acceptable(square first_taboo_violation);

square find_taboo_violation(void);

#endif
