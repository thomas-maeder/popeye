#if !defined(POSITION_POSITION_H)
#define POSITION_POSITION_H

#include "position/board.h"
#include "pieces/pieces.h"
#include "pieces/walks/vectors.h"
#include "utilities/boolean.h"

/* Declarations of types and functions related to chess positions
 */

/* Array containing an element for each square plus many slack square
 * for making move generation easier
 */
typedef PieNam echiquier[maxsquare+4];

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

#define advers(side)    ((side)==Black ? White : Black)


typedef enum
{
  color_white,
  color_black,
  color_neutral,

  nr_colors
} Colors;

#define COLORFLAGS      (BIT(color_black)+BIT(color_white)+BIT(color_neutral))
#define SETCOLOR(a,b)   (a)=((a)&~COLORFLAGS)+((b)&COLORFLAGS)

extern echiquier e;
extern Flags spec[maxsquare+4];
extern square king_square[nr_sides];
extern boolean areColorsSwapped;
extern boolean isBoardReflected;
extern unsigned int number_of_pieces[nr_sides][PieceCount];

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
    unsigned number_of_pieces[nr_sides][PieceCount]; /* number of piece kind */
} position;


/* Sequence of pieces corresponding to the game array (a1..h1, a2..h2
 * ... a8..h8)
 */
extern PieNam const PAS[nr_squares_on_board];
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
void occupy_square(square s, PieNam piece, Flags flags);
void replace_piece(square s, PieNam piece);
#define is_square_empty(s) (e[(s)]==Empty)
#define is_square_blocked(s) (e[(s)]==Invalid)
#define get_walk_of_piece_on_square(s) (e[(s)])
square find_end_of_line(square from, numvec dir);
#define piece_belongs_to_opponent(sq)    TSTFLAG(spec[(sq)],advers(trait[nbply]))

/* Change the side of some piece specs
 * @param spec address of piece specs where to change the side
 */
void piece_change_side(Flags *spec);

#endif
