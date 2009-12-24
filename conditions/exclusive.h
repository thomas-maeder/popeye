#if !defined(PYEXCLUS_H)
#define PYEXCLUS_H

/* This module implements Exclusive Chess
 */

#include "boolean.h"
#include "pyposit.h"

/* Perform the necessary verification steps for solving an Exclusive
 * Chess problem
 * @return true iff verification passed
 */
boolean exclusive_verifie_position(void);

/* Do preparations before generating moves for a side in an Exclusive
 * Chess problem
 * @param side side for which to generate moves
 */
void exclusive_init_genmove(Side side);

/* Determine whether the current position is illegal because of
 * Exclusive Chess rules
 * @return true iff the position is legal
 */
boolean exclusive_pos_legal(void);

#endif
