#if !defined(PIECES_WALKS_RADIAL_H)
#define PIECES_WALKS_RADIAL_H

/* This module implements radial pieces */

#include "solving/observation.h"

/* Generate moves for a radial knight
 */
void radialknight_generate_moves(void);

boolean radialknight_check(evalfunction_t *evaluate);

/* Generate moves for a (lesser) tree hopper
 */
void treehopper_generate_moves(void);

boolean treehopper_check(evalfunction_t *evaluate);

/* Generate moves for a greater tree hopper
 */
void greater_treehopper_generate_moves(void);

boolean greatertreehopper_check(evalfunction_t *evaluate);

/* Generate moves for a (lesser) leaf hopper
 */
void leafhopper_generate_moves(void);

boolean leafhopper_check(evalfunction_t *evaluate);

/* Generate moves for a greater leaf hopper
 */
void greater_leafhopper_generate_moves(void);

boolean greaterleafhopper_check(evalfunction_t *evaluate);

#endif
