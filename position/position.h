#if !defined(POSITION_POSITION_H)
#define POSITION_POSITION_H

#include "position/board.h"
#include "position/side.h"
#include "position/color.h"
#include "pieces/pieces.h"
#include "pieces/walks/vectors.h"
#include "position/castling_rights.h"
#include "utilities/boolean.h"

/* Declarations of types and functions related to chess positions
 */

/* Array containing an element for each square plus many slack square
 * for making move generation easier
 */
typedef piece_walk_type echiquier[maxsquare+5];

/* Some useful symbols for dealing with these flags
 */

extern boolean areColorsSwapped;
extern boolean isBoardReflected;

enum
{
  BorderSpec = 0,
  EmptySpec  = 0
};

enum
{
  maxinum    = 10    /* maximum supported number of imitators */
};

typedef square imarr[maxinum]; /* squares currently occupied by imitators */

/* Structure containing the pieces of data that together represent a
 * position.
 */
typedef struct position
{
    echiquier board;                     /* placement of the pieces */
    Flags spec[maxsquare+5];      /* spec[s] contains flags for piece board[i]*/
    square king_square[nr_sides];        /* placement of the kings */
    unsigned int number_of_imitators;    /* number of iterators */
    imarr isquare;                       /* placement of iterators */
    unsigned number_of_pieces[nr_sides][nr_piece_walks]; /* number of piece kind */
    castling_rights_type castling_rights;
} position;

extern position being_solved;

/* Sequence of pieces corresponding to the game array (a1..h1, a2..h2
 * ... a8..h8)
 */
extern piece_walk_type const PAS[nr_squares_on_board];
extern Side const PAS_sides[nr_squares_on_board];

/* Initial game position.
 *
 */
extern position const game_array;

/* Initialize the game array into a position object
 * @param pos address of position object
 */
void initialise_game_array(position *pos);

/* Swap the sides of all the pieces */
void swap_sides(void);

/* Reflect the position at the horizontal central line */
void reflect_position(void);

void empty_square(square s);
void block_square(square s);
void occupy_square(square s, piece_walk_type walk, Flags flags);
void replace_walk(square s, piece_walk_type walk);
#define is_square_empty(s) (being_solved.board[(s)]==Empty)
#define is_square_blocked(s) (being_solved.board[(s)]==Invalid)
#define get_walk_of_piece_on_square(s) (being_solved.board[(s)])
square find_end_of_line(square from, numvec dir);
#define piece_belongs_to_opponent(sq)    TSTFLAG(being_solved.spec[(sq)],advers(trait[nbply]))

/* Change the side of some piece specs
 * @param being_solved.spec address of piece specs where to change the side
 */
void piece_change_side(Flags *spec);

#endif
