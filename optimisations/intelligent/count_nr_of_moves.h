#if !defined(OPTIMISATIONS_INTELLIGENT_COUNT_NR_OF_MOVES_H)
#define OPTIMISATIONS_INTELLIGENT_GUARDS_FLIGHTS_H

#include "py.h"
#include "pyboard.h"

unsigned int intelligent_count_moves_to_white_promotion(square from_square);

unsigned int intelligent_count_nr_of_moves_from_to_pawn_promotion(square from_square,
                                                                  piece to_piece,
                                                                  square to_square);
unsigned int intelligent_count_nr_of_moves_from_to_pawn_no_promotion(piece pawn,
                                                                     square from_square,
                                                                     square to_square);
unsigned int intelligent_count_nr_of_moves_from_to_king(piece piece,
                                                        square from_square,
                                                        square to_square);
unsigned int intelligent_count_nr_of_moves_from_to_no_check(piece from_piece,
                                                            square from_square,
                                                            piece to_piece,
                                                            square to_square);
unsigned int intelligent_count_nr_of_moves_from_to_checking(piece from_piece,
                                                            square from_square,
                                                            piece to_piece,
                                                            square to_square);
unsigned int intelligent_count_nr_black_moves_to_square(square to_be_blocked);

#endif
