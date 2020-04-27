#include "optimisations/orthodox_check_directions.h"
#include "position/position.h"
#include "solving/pipe.h"

static numvec ortho_opt[4][2*(square_h8-square_a1)+1];

static numvec const * const check_dir_impl[4] = {
    ortho_opt[Queen-Queen]+(square_h8-square_a1),
    ortho_opt[Knight-Queen]+(square_h8-square_a1),
    ortho_opt[Rook-Queen]+(square_h8-square_a1),
    ortho_opt[Bishop-Queen]+(square_h8-square_a1)
};

numvec const * CheckDir(piece_walk_type t)
{
    return check_dir_impl[t-Queen];
}

static void InitCheckDir(void)
{
  vec_index_type i;
  unsigned int j;

  {
    enum
    {
      ensure_Queen_lt_Rook = 1/(Queen<Rook),
      ensure_Rook_minus_Queen_lt_4 = 1/((Rook-Queen)<4),
      ensure_Queen_lt_Bishop = 1/(Queen<Bishop),
      ensure_Bishop_minus_Queen_lt_4 = 1/((Bishop-Queen)<4),
      ensure_Queen_lt_Knight = 1/(Queen<Knight),
      ensure_Knight_minus_Queen_lt_4 = 1/((Knight-Queen)<4)
    };
  }

  for (i = -(square_h8-square_a1); i<=square_h8-square_a1; i++)
  {
    ortho_opt[Queen-Queen][(square_h8-square_a1)+i] = 0;
    ortho_opt[Rook-Queen][(square_h8-square_a1)+i] = 0;
    ortho_opt[Bishop-Queen][(square_h8-square_a1)+i] = 0;
    ortho_opt[Knight-Queen][(square_h8-square_a1)+i] = 0;
  }

  for (i = vec_knight_start; i <= vec_knight_end; i++)
    ortho_opt[Knight-Queen][(square_h8-square_a1)+vec[i]] = vec[i];

  for (i = vec_rook_start; i<=vec_rook_end; i++)
    for (j = 1; j<=max_nr_straight_rider_steps; j++)
    {
      ortho_opt[Queen-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
      ortho_opt[Rook-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
    }

  for (i = vec_bishop_start; i<=vec_bishop_end; i++)
    for (j = 1; j<=max_nr_straight_rider_steps; j++)
    {
      ortho_opt[Queen-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
      ortho_opt[Bishop-Queen][(square_h8-square_a1)+j*vec[i]] = vec[i];
    }
}

void check_dir_initialiser_solve(slice_index si)
{
  InitCheckDir();
  pipe_solve_delegate(si);
}
