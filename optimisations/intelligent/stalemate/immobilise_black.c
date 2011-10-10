#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "pyint.h"
#include "pydata.h"
#include "pyslice.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/stalemate/white_block.h"
#include "optimisations/intelligent/stalemate/black_block.h"
#include "optimisations/intelligent/stalemate/pin_black_piece.h"
#include "stipulation/temporary_hacks.h"
#include "trace.h"

#include <assert.h>

static immobilisation_state_type const null_immobilisation_state;

immobilisation_state_type * current_immobilisation_state;

static boolean can_white_pin(void)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[White]; i++)
    if (!(white[i].usage!=piece_is_unused
          || white[i].type==cb
          || (white[i].type==pb && Nr_remaining_white_moves<moves_to_white_prom[i])))
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static
boolean can_we_block_all_necessary_squares(unsigned int const nr_blocks_needed[nr_sides])
{
  boolean result;
  unsigned int nr_unused_pieces[nr_sides] = { 0, 0 };
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_blocks_needed[White]);
  TraceFunctionParam("%u",nr_blocks_needed[Black]);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[Black]; ++i)
    if (black[i].usage==piece_is_unused)
      ++nr_unused_pieces[Black];
  TraceValue("%u\n",nr_unused_pieces[Black]);

  if (nr_unused_pieces[Black]<nr_blocks_needed[Black])
    result = false;
  else
  {
    for (i = 0; i<MaxPiece[White]; ++i)
      if (white[i].usage==piece_is_unused)
        ++nr_unused_pieces[White];
    TraceValue("%u\n",nr_unused_pieces[White]);

    result = (nr_unused_pieces[White]+nr_unused_pieces[Black]
              >=nr_blocks_needed[Black]+nr_blocks_needed[White]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the most expensive square (if any) that must be blocked by Black
 * @param nr_remaining_black_moves number of remaining black moves
 * @param block_requirement blocking requirements for each square
 * @return * nullsquare more squares need to be blocked than Black can in the
 *                      nr_remaining_black_moves remaining moves
 *         * initsquare no square is required to be blocked by Black
 *         * otherwise: most expensive square that must be blocked by Black
 */
static square find_most_expensive_square_to_be_blocked_by_black(block_requirement_type const block_requirement[maxsquare+4])
{
  square result = initsquare;
  int max_number_black_moves_to_squares_to_be_blocked = -1;
  unsigned int total_number_black_moves_to_squares_to_be_blocked = 0;

  square const *bnp;
  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
    if (block_requirement[*bnp]==black_block_needed_on_square)
    {
      int const nr_black_blocking_moves = intelligent_count_nr_black_moves_to_square(*bnp);
      total_number_black_moves_to_squares_to_be_blocked += nr_black_blocking_moves;
      if (total_number_black_moves_to_squares_to_be_blocked>Nr_remaining_black_moves)
      {
        result = nullsquare;
        break;
      }
      else if (nr_black_blocking_moves>max_number_black_moves_to_squares_to_be_blocked)
      {
        max_number_black_moves_to_squares_to_be_blocked = nr_black_blocking_moves;
        result = *bnp;
      }
    }


  return result;
}

/* @return true iff >=1 black pieces needed to be immobilised
 */
boolean intelligent_stalemate_immobilise_black(stip_length_type n)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  {
    immobilisation_state_type immobilisation_state = null_immobilisation_state;
    current_immobilisation_state = &immobilisation_state;
    slice_has_solution(slices[current_start_slice].u.fork.fork);
    current_immobilisation_state = 0;

    if (immobilisation_state.last_found_trouble_square_status>no_requirement)
    {
      assert(immobilisation_state.nr_of_trouble_makers>0);
      assert(immobilisation_state.positions_of_trouble_makers[immobilisation_state.nr_of_trouble_makers-1]!=initsquare);

      if (immobilisation_state.last_found_trouble_square_status<immobilisation_impossible)
      {
        if (immobilisation_state.last_found_trouble_square_status!=king_block_required
            && can_white_pin())
          intelligent_stalemate_immobilise_by_pinning_any_trouble_maker(n,
                                                                        &immobilisation_state);

        if (immobilisation_state.last_found_trouble_square_status<pin_required
            && can_we_block_all_necessary_squares(immobilisation_state.nr_blocks_needed))
        {
          square const most_expensive_square_to_be_blocked_by_black
            = find_most_expensive_square_to_be_blocked_by_black(immobilisation_state.block_requirement);
          switch (most_expensive_square_to_be_blocked_by_black)
          {
            case nullsquare:
              /* Black doesn't have time to provide all required blocks */
              break;

            case initsquare:
              assert(immobilisation_state.block_requirement[immobilisation_state.last_found_trouble_square]
                     ==white_block_sufficient_on_square);
            {
              /* All required blocks can equally well be provided by White or Black,
               * i.e. they all concern black pawns!
               * We could now try to find the most expensive one, but we assume that
               * there isn't much difference; so simply pick
               * immobilisation_state.last_found_trouble_square.
               */
              intelligent_stalemate_black_block(n,
                                                immobilisation_state.last_found_trouble_square);
              intelligent_stalemate_white_block(n,
                                                immobilisation_state.last_found_trouble_square);
              break;
            }

            default:
              if (nr_reasons_for_staying_empty[most_expensive_square_to_be_blocked_by_black]==0)
              {
                /* most_expensive_square_to_be_blocked_by_black is the most expensive
                 * square among those that Black must block */
                intelligent_stalemate_black_block(n,
                                                  most_expensive_square_to_be_blocked_by_black);
              }
              break;
          }
        }
      }

      result = true;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void update_block_requirements(immobilisation_state_type *state)
{
  switch (state->block_requirement[state->last_found_trouble_square])
  {
    case no_block_needed_on_square:
      if (pjoue[nbply]==pn)
      {
        state->block_requirement[state->last_found_trouble_square] = white_block_sufficient_on_square;
        ++state->nr_blocks_needed[White];
      }
      else
      {
        state->block_requirement[state->last_found_trouble_square] = black_block_needed_on_square;
        ++state->nr_blocks_needed[Black];
      }
      break;

    case white_block_sufficient_on_square:
      if (pjoue[nbply]!=pn)
      {
        state->block_requirement[state->last_found_trouble_square] = black_block_needed_on_square;
        --state->nr_blocks_needed[White];
        ++state->nr_blocks_needed[Black];
      }
      break;

    case black_block_needed_on_square:
      /* nothing */
      break;

    default:
      assert(0);
      break;
  }
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n number of half moves until end state has to be reached
 * @return length of solution found, i.e.:
 *         n+2 the move leading to the current position has turned out
 *             to be illegal
 *         n+1 no solution found
 *         n   solution found
 */
stip_length_type intelligent_immobilisation_counter_can_help(slice_index si,
                                                             stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (current_immobilisation_state->nr_of_trouble_makers==0
      || move_generation_stack[nbcou].departure
         !=current_immobilisation_state->positions_of_trouble_makers[current_immobilisation_state->nr_of_trouble_makers-1])
  {
    current_immobilisation_state->positions_of_trouble_makers[current_immobilisation_state->nr_of_trouble_makers] = move_generation_stack[nbcou].departure;
    ++current_immobilisation_state->nr_of_trouble_makers;
  }

  switch (e[move_generation_stack[nbcou].arrival])
  {
    case roin: /* unpinnable leaper */
      current_immobilisation_state->last_found_trouble_square = move_generation_stack[nbcou].arrival;
      current_immobilisation_state->last_found_trouble_square_status = pprise[nbply]==vide ? king_block_required : immobilisation_impossible;
      update_block_requirements(current_immobilisation_state);
      break;

    case cn: /* pinnable leaper */
      current_immobilisation_state->last_found_trouble_square = move_generation_stack[nbcou].arrival;
      current_immobilisation_state->last_found_trouble_square_status = pprise[nbply]==vide ? block_required : pin_required;
      update_block_requirements(current_immobilisation_state);
      break;

    case dn: /* unpinnable rider */
    {
      int const diff = (move_generation_stack[nbcou].arrival
                        -move_generation_stack[nbcou].departure);
      current_immobilisation_state->last_found_trouble_square = (move_generation_stack[nbcou].departure
                                                                 +CheckDirQueen[diff]);
      if (move_generation_stack[nbcou].arrival
          ==current_immobilisation_state->last_found_trouble_square)
      {
        if (pprise[nbply]==vide)
          current_immobilisation_state->last_found_trouble_square_status = block_required;
        else
          current_immobilisation_state->last_found_trouble_square_status = immobilisation_impossible;

        update_block_requirements(current_immobilisation_state);
      }
      break;
    }

    case tn:
    case fn:
    case pn: /* pinnable riders */
    {
      int const diff = (move_generation_stack[nbcou].arrival
                        -move_generation_stack[nbcou].departure);
      current_immobilisation_state->last_found_trouble_square = (move_generation_stack[nbcou].departure
                                                                 +CheckDirQueen[diff]);
      if (move_generation_stack[nbcou].arrival
          ==current_immobilisation_state->last_found_trouble_square)
      {
        if (pprise[nbply]==vide)
          current_immobilisation_state->last_found_trouble_square_status = block_required;
        else
          current_immobilisation_state->last_found_trouble_square_status = pin_required;

        update_block_requirements(current_immobilisation_state);
      }
      break;
    }

    default:  /* no support for fairy chess */
      assert(0);
      break;
  }

  if (current_immobilisation_state->last_found_trouble_square_status<king_block_required)
    result = n+2;
  else
    result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
