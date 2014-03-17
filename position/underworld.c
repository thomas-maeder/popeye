#include "position/underworld.h"
#include "debugging/trace.h"

#include <string.h>

piece_type underworld[underworld_capacity];
underworld_index_type nr_ghosts;

/* Find the last spot referring to a particular square
 * @param pos the square
 * @return index of the spot; ghost_not_found if not found
 */
underworld_index_type underworld_find_last(square pos)
{
  underworld_index_type current = nr_ghosts;
  underworld_index_type result = ghost_not_found;

  TraceFunctionEntry(__func__);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  while (current>0)
  {
    --current;
    TraceSquare(underworld[current].on);
    TraceValue("%u\n",current);
    if (underworld[current].on==pos)
    {
      result = current;
      break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d",result);
  TraceFunctionResultEnd();
  return result;
}

/* Make space at some spot in the underworld
 * @param ghost_pos identifies the spot
 */
void underworld_make_space(underworld_index_type ghost_pos)
{
  memmove(underworld+ghost_pos+1, underworld+ghost_pos,
          (nr_ghosts-ghost_pos) * sizeof underworld[0]);
  ++nr_ghosts;
}

/* Get rid of a space at some spot in the underworld
 * @param ghost_pos identifies the spot
 */
void underworld_lose_space(underworld_index_type ghost_pos)
{
  --nr_ghosts;
  memmove(underworld+ghost_pos, underworld+ghost_pos+1,
          (nr_ghosts-ghost_pos) * sizeof underworld[0]);
}
