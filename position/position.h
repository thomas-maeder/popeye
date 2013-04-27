#if !defined(POSITION_POSITION_H)
#define POSITION_POSITION_H

#include "position/board.h"
#include "position/piece.h"
#include "utilities/boolean.h"

/* Declarations of types and functions related to chess positions
 */

/* Array containing an element for each square plus many slack square
 * for making move generation easier
 */
typedef piece echiquier[maxsquare+4];

/* Set of flags representing characteristics of pieces (e.g. being
 * Neutral, or being Uncapturable)
 */
typedef unsigned long Flags;

/* Some useful symbols for dealing with these flags
 */

/* Enumeration type for the two sides which move, deliver mate etc.
 */

#include "position/board.h"

#define ENUMERATION_TYPENAME Side
#define ENUMERATORS \
  ENUMERATOR(White), \
    ENUMERATOR(Black), \
                       \
    ENUMERATOR(nr_sides), \
    ASSIGNED_ENUMERATOR(no_side = nr_sides)

#define ENUMERATION_DECLARE

#include "utilities/enumeration.h"

extern echiquier e;
extern Flags spec[maxsquare+4];
extern square king_square[nr_sides];
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
typedef struct
{
    echiquier board;                     /* placement of the pieces */
    Flags spec[maxsquare+4];      /* spec[s] contains flags for piece board[i]*/
    square king_square[nr_sides];        /* placement of the kings */
    unsigned int inum;                   /* number of iterators */
    imarr isquare;                       /* placement of iterators */
    unsigned int zzznp[derbla-dernoi-1]; /* number of piece kind */
} position;

/* The number of pieces of kind k in position p is accessed using the
 * expression nr_piece(p)[k]
 */
#define nr_piece(p) ((p).zzznp-(dernoi+1))


/* Sequence of pieces corresponding to the game array (a1..h1, a2..h2
 * ... a8..h8)
 */
extern piece const PAS[nr_squares_on_board];

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

#endif
