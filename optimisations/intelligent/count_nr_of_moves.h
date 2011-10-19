#if !defined(OPTIMISATIONS_INTELLIGENT_COUNT_NR_OF_MOVES_H)
#define OPTIMISATIONS_INTELLIGENT_GUARDS_FLIGHTS_H

#include "py.h"
#include "pyboard.h"

unsigned int intelligent_count_moves_to_white_promotion(square from_square);

boolean intelligent_reserve_black_pawn_moves_from_to_no_promotion(square from_square,
                                                                  square to_square);
boolean intelligent_reserve_white_pawn_moves_from_to_no_promotion(square from_square,
                                                                  square to_square);
unsigned int intelligent_count_nr_of_moves_from_to_no_check(piece from_piece,
                                                            square from_square,
                                                            piece to_piece,
                                                            square to_square);
unsigned int intelligent_count_nr_of_moves_from_to_checking(piece from_piece,
                                                            square from_square,
                                                            piece to_piece,
                                                            square to_square);
unsigned int intelligent_estimate_min_nr_black_moves_to_square(square to_be_blocked);

/* Tests if a specific king move sequence is still possible.
 * @param side whose king to move
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 * @note modifies Nr_remaining_moves[side] if the move sequence is possible
 */
boolean intelligent_reserve_king_moves_from_to(Side side,
                                               square from_square,
                                               square to_square);

/* Tests if a specific checking sequence of moves by the same pawn including its
 * promotion is still possible.
 * @param from_square from
 * @param promotee_type type of piece that the pawn promotes to
 * @param to_square to
 * @return true iff the move sequence is still possible
 * @note modifies Nr_remaining_moves[side] if the move sequence is possible
 */
boolean intelligent_reserve_promoting_pawn_moves_from_to(square from_square,
                                                                        piece promotee_type,
                                                                        square to_square);

/* Tests if a specific checking white sequence of moves by the same pawn is
 * still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 * @note modifies Nr_remaining_moves[side] if the move sequence is possible
 */
boolean intelligent_reserve_white_pawn_moves_from_to_checking(square from_square,
                                                              square to_square);

/* Tests if a specific checking white sequence of moves by the same officer is
 * still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 * @note modifies Nr_remaining_moves[side] and Nr_unused_masses[Black] if the
 *       move sequence is possible
 */
boolean intelligent_reserve_white_officer_moves_from_to_checking(piece piece,
                                                                 square from_square,
                                                                 square to_square);

#endif
