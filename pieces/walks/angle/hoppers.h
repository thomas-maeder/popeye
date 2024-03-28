#if !defined(PIECES_ANGLE_HOPPERS_H)
#define PIECES_ANGLE_HOPPERS_H

#include "pieces/walks/angle/angles.h"
#include "position/board.h"
#include "position/position.h"
#include "utilities/boolean.h"
#include "solving/observation.h"


/* Generated moves for an Elk
 */
void elk_generate_moves(void);

boolean moose_check(validator_id evaluate);

/* Generated moves for an Rook Moose
 */
void rook_moose_generate_moves(void);

boolean rookmoose_check(validator_id evaluate);

/* Generated moves for an Bishop Moose
 */
void bishop_moose_generate_moves(void);

boolean bishopmoose_check(validator_id evaluate);

/* Generated moves for an Eagle
 */
void eagle_generate_moves(void);

boolean eagle_check(validator_id evaluate);

/* Generated moves for a Rook Eagle
 */
void rook_eagle_generate_moves(void);

boolean rookeagle_check(validator_id evaluate);

/* Generated moves for a Bishop Eagle
 */
void bishop_eagle_generate_moves(void);

boolean bishopeagle_check(validator_id evaluate);

/* Generated moves for a Sparrow
 */
void sparrow_generate_moves(void);

boolean sparrow_check(validator_id evaluate);

/* Generated moves for a Rook Sparrow
 */
void rook_sparrow_generate_moves(void);

boolean rooksparrow_check(validator_id evaluate);

/* Generated moves for a Bishop Sparrow
 */
void bishop_sparrow_generate_moves(void);

boolean bishopsparrow_check(validator_id evaluate);

/* Generated moves for a Marguerite
 */
void marguerite_generate_moves(void);

boolean marguerite_check(validator_id evaluate);

void eagle_equihopper_generate_moves(void);
void moose_equihopper_generate_moves(void);
void sparrow_equihopper_generate_moves(void);

boolean eagle_equihopper_check(validator_id evaluate);
boolean moose_equihopper_check(validator_id evaluate);
boolean sparrow_equihopper_check(validator_id evaluate);

void eagle_nonstop_equihopper_generate_moves(void);
void moose_nonstop_equihopper_generate_moves(void);
void sparrow_nonstop_equihopper_generate_moves(void);

boolean eagle_nonstop_equihopper_check(validator_id evaluate);
boolean moose_nonstop_equihopper_check(validator_id evaluate);
boolean sparrow_nonstop_equihopper_check(validator_id evaluate);

#endif
