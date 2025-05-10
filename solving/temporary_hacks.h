#if !defined(SOLVING_TEMPORARY_HACKS_H)
#define SOLVING_TEMPORARY_HACKS_H

#include "stipulation/stipulation.h"

/* interface to some slices inserted as temporary hacks */

/* fork slice into mate tester */
extern slice_index temporary_hack_mate_tester[nr_sides];

/* fork slice into lost piece tester */
extern slice_index temporary_hack_lost_piece_tester[nr_sides];

/* fork slice into mating move counter */
extern slice_index temporary_hack_exclusive_mating_move_counter[nr_sides];

/* fork slice into branch finding Brunner Chess specific defenses */
extern slice_index temporary_hack_brunner_check_defense_finder[nr_sides];

/* fork slice into branch finding king captures without optimisations */
extern slice_index temporary_hack_ultra_mummer_length_measurer[nr_sides];

/* fork slice into validating movees */
extern slice_index temporary_hack_move_legality_tester[nr_sides];

/* fork slice into validating king captures */
extern slice_index temporary_hack_king_capture_legality_tester[nr_sides];

/* fork slice into branch finding non-capturing moves in Cage Circe */
extern slice_index temporary_hack_cagecirce_noncapture_finder[nr_sides];

/* fork slice into branch finding CirceTake&Make rebirth squares */
extern slice_index temporary_hack_circe_take_make_rebirth_squares_finder[nr_sides];

/* fork slice into branch that tests the legality of intermediate castling moves */
extern slice_index temporary_hack_castling_intermediate_move_legality_tester[nr_sides];

/* fork slice into branch that tests the legality of a candidate move */
extern slice_index temporary_hack_opponent_moves_counter[nr_sides];

/* fork slice into branch that finds a back home move */
extern slice_index temporary_hack_back_home_finder[nr_sides];

/* fork slice into branch that detects suffocation by paralysis */
extern slice_index temporary_hack_suffocation_by_paralysis_finder[nr_sides];

/* fork slice into branch that generates moves */
extern slice_index temporary_hack_move_generator[nr_sides];

/* fork slice into branch that generates Bul movements */
extern slice_index temporary_hack_bul_movement_generator[nr_sides];

/* fork slice into branch that tests square observation */
extern slice_index temporary_hack_is_square_observed[nr_sides];
extern slice_index temporary_hack_is_square_observed_specific[nr_sides];
extern slice_index temporary_hack_is_square_observed_by_non_king[nr_sides];

/* fork slice into check validation */
extern slice_index temporary_hack_check_validator[nr_sides];

/* fork slice into square observation validation */
extern slice_index temporary_hack_observation_validator[nr_sides];

/* fork slice into square observer validation */
extern slice_index temporary_hack_observer_validator[nr_sides];

/* fork slice into square observation geometry validation */
extern slice_index temporary_hack_observation_geometry_validator[nr_sides];

/* fork slice into branch that detects check*/
extern slice_index temporary_hack_check_tester;


/* Initialise temporary hack slices
 * @param root_slice identifies root slice of stipulation
 */
void insert_temporary_hacks(slice_index root_slice);

#endif
