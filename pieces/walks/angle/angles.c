#include "pieces/walks/angle/angles.h"
#include "position/board.h"

numvec const angle_vectors[nr_angles][17] =
{
  { 0,
    dir_down+2*dir_right, dir_up+2*dir_right,
    2*dir_up+dir_left,    2*dir_up+dir_right,
    dir_up+2*dir_left,    dir_down+2*dir_left,
    2*dir_down+dir_right, 2*dir_down+dir_left,
    2*dir_left+dir_up,    2*dir_up+dir_left,
    2*dir_right+dir_up,   2*dir_up+dir_right,
    2*dir_right+dir_down, 2*dir_down+dir_right,
    2*dir_left+dir_down,  2*dir_down+dir_left
  },
  { 0,
    dir_down+dir_right,   dir_up+dir_right,
    dir_up+dir_left,      dir_up+dir_right,
    dir_up+dir_left,      dir_down+dir_left,
    dir_down+dir_right,   dir_down+dir_left,
    dir_left,             dir_up,
    dir_right,            dir_up,
    dir_right,            dir_down,
    dir_left,             dir_down
  },
  { 0,
    dir_up,               dir_down,
    dir_right,            dir_left,
    dir_up,               dir_down,
    dir_right,            dir_left,
    dir_up+dir_right,     dir_down+dir_left,
    dir_up+dir_left,      dir_down+dir_right,
    dir_up+dir_right,     dir_down+dir_left,
    dir_up+dir_left,      dir_down+dir_right
  },
  { 0,
    dir_up+dir_left,      dir_down+dir_left,
    dir_down+dir_right,   dir_down+dir_left,
    dir_down+dir_right,   dir_up+dir_right,
    dir_up+dir_left,      dir_up+dir_right,
    dir_right,            dir_down,
    dir_left,             dir_down,
    dir_left,             dir_up,
    dir_right,            dir_up
  }
};
