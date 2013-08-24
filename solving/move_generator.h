#if !defined(SOLVING_MOVE_GENERATOR_H)
#define SOLVING_MOVE_GENERATOR_H

#include "solving/solve.h"
#include "solving/ply.h"
#include "stipulation/slice_type.h"
#include "stipulation/stipulation.h"
#include "pieces/pieces.h"

/* This module provides functionality dealing with the attacking side
 * in STMoveGenerator stipulation slices.
 */

typedef unsigned int numecoup;

enum
{
  /* the following values are used instead of capture square
   * to indicate special moves */
  messigny_exchange = maxsquare+1,

  min_castling = maxsquare+2,
  kingside_castling = min_castling,
  queenside_castling = maxsquare+3,
  max_castling = queenside_castling,

  pawn_multistep = maxsquare, /* must refer to a square that is always empty */
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

extern numecoup current_move[maxply+1];
extern numecoup current_move_id[maxply+1];

enum
{
  nil_coup = 1
};

#define encore() (current_move[nbply]-1>current_move[nbply-1]-1)

/* Instrument move generation with a slice type
 * @param identifies where to start instrumentation
 * @param side which side (pass nr_sides for both sides)
 * @param type type of slice with which to instrument moves
 */
void solving_instrument_move_generation(slice_index si,
                                        Side side,
                                        slice_type type);

/* Generate moves for a single piece
 * @param identifies generator slice
 * @param p walk to be used for generating
 */
void generate_moves_for_piece(slice_index si, PieNam p);

/* Allocate a STMoveGenerator slice.
 * @return index of allocated slice
 */
slice_index alloc_move_generator_slice(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type move_generator_solve(slice_index si, stip_length_type n);

/* Instrument a stipulation with move generator slices
 * @param si root of branch to be instrumented
 */
void stip_insert_move_generators(slice_index si);

/* From the moves just generated, remove all those that don't meet a certain
 * criterion
 * @param criterion criterion to be met by moves not to be removed
 */
typedef boolean (*move_filter_criterion_type)(numecoup n);
void move_generator_filter_moves(move_filter_criterion_type criterion);
void move_generator_filter_captures(move_filter_criterion_type criterion);

/* address of an element used for holding the move currently being generated */
extern move_generation_elmt *curr_generation;

/* Push the move described in *curr_generation onto the stack */
void push_move(void);

/* Push the capture move described in *curr_generation onto the stack.
 * @param sq_capture place of capture
 * @note use push_move() if sq_capture is equal to the arrival square
 */
void push_move_capture_extra(square sq_capture);

/* Push the special move described in *curr_generation onto the stack
 * @param sq_special special square value describing the speciality of the move
 */
void push_special_move(square sq_special);

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
