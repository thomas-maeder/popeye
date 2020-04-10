#if !defined(OPTIMISATIONS_INTELLIGENT_COUNT_NR_OF_MOVES_H)
#define OPTIMISATIONS_INTELLIGENT_COUNT_NR_OF_MOVES_H

#include "pieces/pieces.h"
#include "position/board.h"
#include "position/side.h"
#include "optimisations/intelligent/intelligent.h"
#include "utilities/boolean.h"

extern unsigned int const minimum_number_knight_moves[];

/* Count the number of moves required for promotiong a white pawn
 * @param from_square start square of the pawn
 * @return number of moves required if promotion is possible
 *         an absurdly high number otherwise
 */
unsigned int intelligent_count_moves_to_white_promotion(square from_square);

/* Tests if a specific checking white sequence of moves by the same black pawn
 * without promotion is still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_black_pawn_moves_from_to_no_promotion(square from_square,
                                                                  square to_square);

/* Tests if a specific checking white sequence of moves by the same white pawn
 * without promotion is still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 * @note modifies reserve[curr_reserve].nr_remaining_moves[White] and reserve[curr_reserve].nr_unused_masses[Black] if the
 */
boolean intelligent_reserve_white_pawn_moves_from_to_no_promotion(square from_square,
                                                                  square to_square);

unsigned int intelligent_count_nr_of_moves_from_to_no_check(Side side,
                                                            piece_walk_type from_piece,
                                                            square from_square,
                                                            piece_walk_type to_piece,
                                                            square to_square);
unsigned int intelligent_count_nr_of_moves_from_to_checking(Side side,
                                                            piece_walk_type from_piece,
                                                            square from_square,
                                                            piece_walk_type to_piece,
                                                            square to_square);

/* A rough check whether it is worth thinking about promotions
 * @param index index of white pawn
 * @param to_square to be reached by the promotee
 * @return true iff to_square is theoretically reachable
 */
boolean intelligent_can_promoted_white_pawn_theoretically_move_to(unsigned int index,
                                                                  square to_square);

/* A rough check whether it is worth thinking about promotions
 * @param index index of white pawn
 * @param to_square to be reached by the promotee
 * @return true iff to_square is theoretically reachable
 */
boolean intelligent_can_promoted_black_pawn_theoretically_move_to(unsigned int index,
                                                                  square to_square);

/* Initialise the moves and masses reservation system
 * @param nr_remaining_white_moves
 * @param nr_remaining_black_moves
 * @param nr_unused_white_masses
 * @param nr_unused_black_masses
 */
void intelligent_init_reservations(unsigned int nr_remaining_white_moves,
                                   unsigned int nr_remaining_black_moves,
                                   unsigned int nr_unused_white_masses,
                                   unsigned int nr_unused_black_masses);

/* Retrieve the number of reservable masses for one or both sides
 * @param side Whose masses? Pass no_side to get both sides' masses.
 * @return number of reservable masses
 */
unsigned int intelligent_get_nr_reservable_masses(Side side);

/* Retrieve the number of remaining moves for a side
 * @param side Whose moves
 * @return number of remaining moves
 */
unsigned int intelligent_get_nr_remaining_moves(Side side);

/* Test whether there are available masses for a side
 * @param side whose masses to reserve
 * @param nr_of_masses number of masses
 * @param usage of the reserved masses
 * @return true iff nr_of_masses are available
 */
boolean intelligent_reserve_masses(Side side,
                                   unsigned int nr_of_masses,
                                   piece_usage usage);

/* Test whether a white pinner is available
 * @return true iff a white pinner is available
 */
boolean intelligent_reserve_pinner(void);

/* Roughly test whether Black can possibly block all flights; if yes, reserve
 * the necessary black masses
 * @param flights flights to be blocked
 * @param nr_flights length of flights
 * @return true if masses have been reserved
 */
boolean intelligent_reserve_black_masses_for_blocks(square const flights[],
                                                    unsigned int nr_flights);

/* Tests if a specific white king move sequence is still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_white_king_moves_from_to(square from_square,
                                                     square to_square);

/* Tests if a specific king move sequence is still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_black_king_moves_from_to(square from_square,
                                                     square to_square);

/* Tests if a specific checking sequence of moves by the same white pawn
 * including its promotion is still possible.
 * @param from_square from
 * @param promotee_type type of piece that the pawn promotes to
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_promoting_white_pawn_moves_from_to(square from_square,
                                                               piece_walk_type promotee_type,
                                                               square to_square);

/* Tests if a specific checking sequence of moves by the same pawn including its
 * promotion is still possible.
 * @param from_square from
 * @param promotee_type type of piece that the pawn promotes to
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_promoting_black_pawn_moves_from_to(square from_square,
                                                               piece_walk_type promotee_type,
                                                               square to_square);

/* Tests if a specific checking white sequence of moves by the same pawn is
 * still possible.
 * @param from_square from
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_white_pawn_moves_from_to_checking(square from_square,
                                                              square to_square);

/* Tests if a specific checking white sequence of moves by the same officer is
 * still possible.
 * @param from_square from
 * @param checker_type type of officer
 * @param to_square to
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_white_officer_moves_from_to_checking(square from_square,
                                                                 piece_walk_type checker_type,
                                                                 square to_square);

/* Tests if a white officer can be the front piece of a battery double check
 * using a specific route
 * @param from_square from
 * @param via departure square of the double checking move
 * @param checker_type type of officer
 * @param to_square destination square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_front_check_by_officer(square from_square,
                                                   square via,
                                                   piece_walk_type checker_type,
                                                   square to_square);

/* Tests if an officer can be the rear piece of a battery double check
 * @param side officer's side
 * @param from_square from
 * @param checker_type type of officer
 * @param to_square destination square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_officer_moves_from_to(Side side,
                                                  square from_square,
                                                  piece_walk_type officer_type,
                                                  square to_square);

/* Tests if a white pawn can be the front piece of a battery double check
 * using a specific route, where the last move is a capture
 * @param from_square from
 * @param via departure square of the double checking move
 * @param to_square destination square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_front_check_by_pawn_with_capture(square from_square,
                                                             square via,
                                                             square to_square);

/* Tests if a white pawn can be the front piece of a battery double check
 * using a specific route, where the last move is not a capture
 * @param from_square from
 * @param via departure square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_front_check_by_pawn_without_capture(square from_square,
                                                                square via);

/* Tests if the front check of a double check can be given by pomotee
 * @param from_square departure square of pawn
 * @param promotee_type type of piece that the pawn promotes to
 * @param via departure square of mating move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_front_check_by_promotee(square from_square,
                                                    piece_walk_type promotee_type,
                                                    square via);

/* Tests if a white pawn can be the front piece of a battery double check
 * using a specific route, where the last move is a capture
 * @param from_square from
 * @param via departure square of the double checking move
 * @param to_square destination square of the double checking move
 * @return true iff the move sequence is still possible
 */
boolean intelligent_reserve_double_check_by_enpassant_capture(square from_square,
                                                              square via);

/* Undo a reservation
 */
void intelligent_unreserve(void);

/* Dump the move and mass allocations for debugging
 */
void intelligent_dump_reservations(void);

#endif
