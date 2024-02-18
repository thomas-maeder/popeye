#if !defined(OPTIMISATIONS_INTELLIGENT_GUARDS_FLIGHTS_H)
#define OPTIMISATIONS_INTELLIGENT_GUARDS_FLIGHTS_H

#include "pieces/walks/vectors.h"
#include "position/board.h"
#include "stipulation/stipulation.h"

#include <limits.h>

/* fast detection of guards by newly placed white pieces
 * */

enum
{
  guard_dir_check_uninterceptable = INT_MAX,
  guard_dir_guard_uninterceptable = INT_MAX-1
};

typedef struct
{
    /* direction from square to guarded flight; special values:
     * guard_dir_check_uninterceptable: uninterceptable check from square
     * guard_dir_guard_uninterceptable: uninterceptable guard from square
     */
    numvec dir;

    /* what square is guarded in direction dir? */
    square target;
} guard_dir_struct;

/* index of guarding piece currently being placed */
extern unsigned int index_of_guarding_piece;

/* lookup doing something like GuardDir[Queen][some_square] */
extern guard_dir_struct GuardDir[nr_piece_walks][maxsquare+4];

/* Initialise GuardDir
 * @param black_king_pos position of black king
 */
void init_guard_dirs(square black_king_pos);

/* continue guarding king flights */
void intelligent_continue_guarding_flights(slice_index si);

/* guard king flights */
void intelligent_guard_flights(slice_index si);

#endif
