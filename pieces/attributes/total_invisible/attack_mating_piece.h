#if !defined(PIECES_ATTRIBUTES_TOTAL_INVISIBLE_ATTACK_MATING_PIECE_H)
#define PIECES_ATTRIBUTES_TOTAL_INVISIBLE_ATTACK_MATING_PIECE_H

#include "position/position.h"

extern square sq_mating_piece_to_be_attacked;

void attack_mating_piece(Side side_attacking,
                         square sq_mating_piece);

void done_placing_mating_piece_attacker(void);

#endif
