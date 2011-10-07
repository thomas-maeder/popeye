#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_IMMOBILISE_BLACK_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_IMMOBILISE_BLACK_H

#include "py.h"

typedef enum
{
  no_requirement,
  block_required,
  king_block_required,
  pin_required,
  immobilisation_impossible
} last_found_trouble_square_status_type;

typedef enum
{
  no_block_needed_on_square,
  white_block_sufficient_on_square,
  black_block_needed_on_square
} block_requirement_type;

typedef struct
{
  square positions_of_trouble_makers[MaxPieceId];
  unsigned int nr_of_trouble_makers;
  square last_found_trouble_square;
  unsigned int nr_blocks_needed[nr_sides];
  block_requirement_type block_requirement[maxsquare+4];
  last_found_trouble_square_status_type last_found_trouble_square_status;
} immobilisation_state_type;

extern immobilisation_state_type * current_immobilisation_state;

void intelligent_stalemate_immobilise_black(unsigned int nr_remaining_black_moves,
                                            unsigned int nr_remaining_white_moves,
                                            unsigned int max_nr_allowed_captures_by_black_pieces,
                                            unsigned int max_nr_allowed_captures_by_white_pieces,
                                            stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type intelligent_immobilisation_counter_can_help(slice_index si,
                                                             stip_length_type n);

#endif
