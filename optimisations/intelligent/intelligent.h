/******************** MODIFICATIONS to pyint.h **************************
**
** Date       Who  What
**
** 1997/04/04 TLi  Original
**
**************************** End of List ******************************/

#if !defined(OPTIMISATIONS_INTELLIGENT_INTELLIGENT_H)
#define OPTIMISATIONS_INTELLIGENT_INTELLIGENT_H

#include "pieces/pieces.h"
#include "position/position.h"
#include "position/pieceid.h"
#include "stipulation/goals/goals.h"
#include "solving/machinery/solve.h"
#include "solving/ply.h"
#include "optimisations/intelligent/piece_usage.h"

typedef struct
{
    square diagram_square;
    Flags flags;
    piece_walk_type type;
    piece_usage usage;
} PIECE;

extern PIECE white[nr_squares_on_board];
extern PIECE black[nr_squares_on_board];
extern PIECE target_position[MaxPieceId+1];

enum { index_of_king = 0 };

extern unsigned int MaxPiece[nr_sides];
extern unsigned int CapturesLeft[maxply+1];

extern boolean solutions_found;

extern goal_type goal_to_be_reached;

extern unsigned int nr_reasons_for_staying_empty[maxsquare+4];

extern unsigned int moves_to_white_prom[nr_squares_on_board];

extern boolean testcastling;

extern unsigned int MovesRequired[nr_sides][maxply+1];

extern unsigned int PieceId2index[MaxPieceId+1];

void IntelligentRegulargoal_types(slice_index si);

void solve_target_position(slice_index si);

boolean black_pawn_attacks_king(square from);

void remember_to_keep_rider_line_open(square from, square to,
                                      int dir, int delta);

/* Detrmine whether some line is empty
 * @param start start of line
 * @param end end of line
 * @param dir direction from start to end
 * @return true iff the line is empty
 */
boolean is_line_empty(square start, square end, int dir);

/* Initialize intelligent mode if the user or the stipulation asks for
 * it
 * @param si identifies slice where to start
 * @return false iff the user asks for intelligent mode, but the
 * stipulation doesn't support it
 */
boolean init_intelligent_mode(slice_index si);

typedef enum {
  NO_EP,
  EP_ON_FIRST_MOVE,
  EP_CONSEQUENT
} EnPassantLegality;

typedef enum {
  NO_CASTLING = 0,
  CASTLING_KINGSIDE = 1,
  CASTLING_QUEENSIDE = 2,
  CASTLING_EITHER = (CASTLING_KINGSIDE | CASTLING_QUEENSIDE),
  CASTLING_CONSEQUENT = (4 | CASTLING_EITHER)
} CastlingLegality;

unsigned int series_h_distance_to_target(CastlingLegality cl, EnPassantLegality ep);

#endif
