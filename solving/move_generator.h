#if !defined(SOLVING_MOVE_GENERATOR_H)
#define SOLVING_MOVE_GENERATOR_H

#include "solving/machinery/solve.h"
#include "solving/ply.h"
#include "stipulation/slice_type.h"
#include "stipulation/stipulation.h"
#include "pieces/pieces.h"
#include "solving/machinery/dispatch.h"
#include "debugging/trace.h"

/* This module provides functionality dealing with the attacking side
 * in STMoveGenerator stipulation slices.
 */

typedef unsigned int numecoup;

enum
{
  offset_en_passant_capture = 8*onerow, /* must refer to squares that are never empty */

  offset_platzwechsel_rochade = 2*maxsquare
};

typedef struct
{
    numecoup id;
    square departure;
    square arrival;
    square capture;
} move_generation_elmt;

enum
{
  toppile = 60*maxply
};

extern move_generation_elmt move_generation_stack[toppile + 1];

enum
{
  current_generation = toppile
};

extern piece_walk_type move_generation_current_walk;

extern numecoup current_move[maxply+1];
extern numecoup current_move_id[maxply+1];

#define MOVEBASE_OF_PLY(ply) (current_move[(ply)-1])
#define SET_MOVEBASE_OF_PLY(ply,n) (current_move[(ply)-1] = (n))

#define CURRMOVE_OF_PLY(ply) (current_move[(ply)])
#define SET_CURRMOVE(ply,cm) (current_move[(ply)] = (cm))

enum
{
  nil_coup = 1
};

#define encore() (CURRMOVE_OF_PLY(nbply)>MOVEBASE_OF_PLY(nbply))

void move_generator_write_history(void);


/* Insert slices into a move generation branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by help_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void move_generation_branch_insert_slices(slice_index si,
                                          slice_index const prototypes[],
                                          unsigned int nr_prototypes);

/* Instrument move generation with a slice type
 * @param identifies where to start instrumentation
 * @param side which side (pass nr_sides for both sides)
 * @param type type of slice with which to instrument moves
 */
void solving_instrument_move_generation(slice_index si,
                                        Side side,
                                        slice_type type);

/* Reject generated captures
 * @param si identifies the slice
 */
void move_generation_reject_captures(slice_index si);

/* Reject generated non-captures
 * @param si identifies the slice
 */
void move_generation_reject_non_captures(slice_index si);

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 */
void generate_moves_for_piece_two_paths(slice_index si);

/* Generate moves for the current piece, but based on a different walk
 * @param si identifies the generator slices
 * @param walk the basis to be used for the move generation
 */
void generate_moves_different_walk(slice_index si, piece_walk_type walk);

#if defined(DOTRACE)
#define generate_moves_delegate(si) \
  TraceWalk(move_generation_current_walk), TraceSquare(curr_generation->departure), TraceEOL(), \
  dispatch(si)
#else
#define generate_moves_delegate(si) dispatch(si)
#endif

/* Initiate the generation of moves for the piece occupying a specific square
 * @param sq_departure square occupied by the piece for which to generate moves
 */
void generate_moves_for_piece(square sq_departure);

/* Allocate a STMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_move_generator_slice(void);

/* Continue determining whether a side is in check
 * @param si identifies the check tester
 * @param side_in_check which side?
 * @return true iff side_in_check is in check according to slice si
 */
boolean observing_move_generator_is_in_check(slice_index si,
                                             Side side_observed);

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void move_generator_solve(slice_index si);

/* Instrument the solving machinery with move generator slices
 * @param si identifies root the solving machinery
 */
void solving_insert_move_generators(slice_index si);

/* Instrument the move generation machinery so that captures and non captures
 * are generated (and can be adapted) separately per piece.
 * @param si root slice of solving machinery
 * @param side side for which to instrument; pass nr_sides for both sides
 * @note inserts proxy slices STGeneratingNoncapturesForPiece and
 *       STGeneratingCapturesForPiece that can be used for adjusting the move
 *       generation
 */
void move_generator_instrument_for_alternative_paths(slice_index si,
                                                                    Side side);

typedef boolean (*move_filter_criterion_type)(numecoup n);

/* Only keep generated moves that fulfill some criterion
 * @param start identifies last move on stack that the criterion will not be applied to
 * @param criterion to be fulfilled by moves kept
 */
void move_generator_filter_moves(numecoup start,
                                 move_filter_criterion_type criterion);

/* Only keep generated captures that fulfill some criterion; non-captures are all kept
 * @param start identifies last move on stack that the criterion will not be applied to
 * @param criterion to be fulfilled by moves kept
 */
void move_generator_filter_captures(numecoup start,
                                    move_filter_criterion_type criterion);

/* Only keep generated non-captures that fulfill some criterion; captures are all kept
 * @param start identifies last move on stack that the criterion will not be applied to
 * @param criterion to be fulfilled by moves kept
 */
void move_generator_filter_noncaptures(numecoup start,
                                       move_filter_criterion_type criterion);

/* Invert the order of the moves generated for a ply
 * @param ply the ply
 */
void move_generator_invert_move_order(ply ply);

/* address of an element used for holding the move currently being generated */
extern move_generation_elmt *curr_generation;

/* Pop the topmost move */
void pop_move(void);

/* Push the move described in *curr_generation onto the stack */
void push_move_no_capture(void);
void push_move_regular_capture(void);

/* Push the capture move described in *curr_generation onto the stack.
 * @param sq_capture place of capture
 * @note use push_move_regular_capture() if sq_capture is equal to the arrival square
 */
void push_move_capture_extra(square sq_capture);

/* Push the special move described in *curr_generation onto the stack
 * @param sq_special special square value describing the speciality of the move
 */
void push_special_move(square sq_special);

boolean is_null_move(numecoup curr);

void push_null_move(void);

/* Push the copy of a move from a different move generation
 * @param original identifies the original of which to push a copy
 */
void push_move_copy(numecoup original);

/* Push an observation target
 * @param sq_target the target
 */
void push_observation_target(square sq_target);

/* Replace an observation target
 * @param sq_target the target
 */
void replace_observation_target(square sq_target);

/* Push the previous observation target again after having played it in order
 * to validate it
 */
#define PUSH_OBSERVATION_TARGET_AGAIN(ply) (++current_move[(ply)])

/* Pop all remaining moves
 */
void pop_all(void);

/* Remove duplicate moves at the top of the move_generation_stack.
 * @param start start position of range where to look for duplicates
 */
void remove_duplicate_moves_of_single_piece(numecoup start);

/* Priorise a move in the move generation stack
 * @param priorised index in the move generation stack of the move to be
 *                  priorised
 */
void move_generator_priorise(numecoup priorised);

#endif
