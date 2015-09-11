#if !defined(POSITION_UNDERWORLD_H)
#define POSITION_UNDERWORLD_H

#include "solving/move_effect_journal.h"

/* This module implements the Underworld.
 * It provides the possibility to temporarily store pieces under those on the
 * board.
 * This functionality is used in conditions such as Ghost Chess, Haunted Chess,
 * Circe Volcanic or Circe Parachute.
 */


enum
{
  underworld_capacity = 32,
  ghost_not_found = underworld_capacity
};

extern piece_type underworld[underworld_capacity];

typedef unsigned int underworld_index_type;

extern underworld_index_type nr_ghosts;

void underworld_resetter_solve(slice_index si);

/* Assign the ghosts ids
 * @param id id to be assigned to the first ghost
 * @return next id to be assigned to some piece
 */
PieceIdType underworld_set_piece_ids(PieceIdType id);

/* Make space at some spot in the underworld
 * @param ghost_pos identifies the spot
 */
void underworld_make_space(underworld_index_type ghost_pos);

/* Get rid of a space at some spot in the underworld
 * @param ghost_pos identifies the spot
 */
void underworld_lose_space(underworld_index_type ghost_pos);

/* Find the last spot referring to a particular square
 * @param pos the square
 * @return index of the spot; ghost_not_found if not found
 */
underworld_index_type underworld_find_last(square pos);

#endif
