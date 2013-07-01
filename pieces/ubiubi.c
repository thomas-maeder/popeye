#include "pieces/ubiubi.h"
#include "pyproc.h"
#include "pydata.h"

typedef enum
{
  ubiubi_empty,
  ubiubi_opposibe,
  ubiubi_taboo
} ubiubi_square_state_type;

typedef ubiubi_square_state_type ubiubi_traversal_state_type[maxsquare+4];

static void utiubi_generate_moves_recursive(square orig_departure,
                                            square step_departure,
                                            Side side,
                                            ubiubi_traversal_state_type traversal_state)
{
  vec_index_type k;

  for (k = vec_knight_start; k<=vec_knight_end; ++k)
  {
    square const sq_arrival = step_departure+vec[k];
    if (!is_square_blocked(sq_arrival))
      switch (traversal_state[sq_arrival])
      {
        case ubiubi_empty:
          add_to_move_generation_stack(orig_departure,sq_arrival,sq_arrival);
          traversal_state[sq_arrival] = ubiubi_taboo;
          utiubi_generate_moves_recursive(orig_departure,sq_arrival,side,traversal_state);
          break;

        case ubiubi_opposibe:
          add_to_move_generation_stack(orig_departure,sq_arrival,sq_arrival);
          traversal_state[sq_arrival] = ubiubi_taboo;
          break;

        case ubiubi_taboo:
          break;
      }
  }
}

void ubiubi_generate_moves(square sq_departure, Side side)
{
  Side const opposite = advers(side);
  ubiubi_traversal_state_type board_state;

  square const *bnp;
  for (bnp = boardnum; *bnp; ++bnp)
    if (is_square_empty(*bnp))
      board_state[*bnp] = ubiubi_empty;
    else if (TSTFLAG(spec[*bnp],opposite))
      board_state[*bnp] = ubiubi_opposibe;
    else
      board_state[*bnp] = ubiubi_taboo;

  board_state[sq_departure] = ubiubi_taboo; /* for neutral UbiUbis */

  utiubi_generate_moves_recursive(sq_departure,sq_departure,side,board_state);
}

static boolean ubiubi_check_recursive(square intermediate_square,
                                      square sq_king,
                                      PieNam p,
                                      ubiubi_traversal_state_type traversal_state,
                                      evalfunction_t *evaluate)
{
  vec_index_type k;

  traversal_state[intermediate_square] = ubiubi_taboo;

  for (k = vec_knight_start; k<=vec_knight_end; k++)
  {
    square const sq_departure = intermediate_square+vec[k];
    if (!is_square_blocked(sq_departure))
    {
      if (traversal_state[sq_departure]==ubiubi_empty)
      {
        if (ubiubi_check_recursive(sq_departure,sq_king,p,traversal_state,evaluate))
          return true;
      }
      else
      {
        if (get_walk_of_piece_on_square(sq_departure)==p
            && TSTFLAG(spec[sq_departure],trait[nbply])
            && evaluate(sq_departure,sq_king,sq_king))
          return true;
      }
    }
  }

  return false;
}

boolean ubiubi_check(square sq_king, PieNam p, evalfunction_t *evaluate)
{
  ubiubi_traversal_state_type board_state;

  square const *bnp;
  for (bnp = boardnum; *bnp; ++bnp)
    board_state[*bnp] = is_square_empty(*bnp) ? ubiubi_empty : ubiubi_taboo;

  return ubiubi_check_recursive(sq_king,sq_king,p,board_state,evaluate);
}
