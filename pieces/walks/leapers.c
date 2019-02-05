#include "pieces/walks/leapers.h"
#include "solving/move_generator.h"
#include "solving/fork.h"
#include "debugging/trace.h"

/* Generate moves for a leaper piece
 * @param kbeg start of range of vector indices to be used
 * @param kend end of range of vector indices to be used
 */
void leaper_generate_moves(vec_index_type kbeg, vec_index_type kend)
{
  /* generate leaper moves from vec[kbeg] to vec[kend] */
  vec_index_type k;

  for (k= kbeg; k<= kend; ++k)
  {
    curr_generation->arrival = curr_generation->departure+vec[k];
    if (is_square_empty(curr_generation->arrival))
      push_move_no_capture();
    else if (piece_belongs_to_opponent(curr_generation->arrival))
      push_move_regular_capture();
  }
}

boolean leapers_check(vec_index_type kanf, vec_index_type kend,
                      validator_id evaluate)
{
  square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(sq_target);
  TraceEOL();

  ++observation_context;

  for (interceptable_observation[observation_context].vector_index1 = kanf;
       interceptable_observation[observation_context].vector_index1<=kend;
       interceptable_observation[observation_context].vector_index1++)
  {
    square const sq_departure= sq_target+vec[interceptable_observation[observation_context].vector_index1];
    if (EVALUATE_OBSERVATION(evaluate,sq_departure,sq_target))
    {
      result = true;
      break;
    }
  }

  --observation_context;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean king_check(validator_id evaluate)
{
  return leapers_check(vec_queen_start,vec_queen_end, evaluate);
}

boolean knight_check(validator_id evaluate)
{
  return leapers_check(vec_knight_start,vec_knight_end, evaluate);
}

boolean vizir_check(validator_id evaluate)
{
  return leapers_check(vec_rook_start, vec_rook_end, evaluate);
}

boolean dabbaba_check(validator_id evaluate)
{
  return leapers_check(vec_dabbaba_start, vec_dabbaba_end, evaluate);
}

boolean fers_check(validator_id evaluate)
{
  return leapers_check(vec_bishop_start, vec_bishop_end, evaluate);
}


boolean alfil_check(validator_id evaluate)
{
  return leapers_check(vec_alfil_start, vec_alfil_end, evaluate);
}

boolean rccinq_check(validator_id evaluate)
{
  return leapers_check(vec_rccinq_start, vec_rccinq_end, evaluate);
}


boolean bucephale_check(validator_id evaluate)
{
  return leapers_check(vec_bucephale_start, vec_bucephale_end, evaluate);
}


boolean girafe_check(validator_id evaluate)
{
  return leapers_check(vec_girafe_start, vec_girafe_end, evaluate);
}

boolean camel_check(validator_id evaluate)
{
  return leapers_check(vec_chameau_start, vec_chameau_end, evaluate);
}

boolean zebra_check(validator_id evaluate)
{
  return leapers_check(vec_zebre_start, vec_zebre_end, evaluate);
}

boolean leap15_check(validator_id evaluate)
{
  return leapers_check(vec_leap15_start, vec_leap15_end, evaluate);
}

boolean leap16_check(validator_id evaluate)
{
  return leapers_check(vec_leap16_start, vec_leap16_end, evaluate);
}

boolean leap24_check(validator_id evaluate)
{
  return leapers_check(vec_leap24_start, vec_leap24_end, evaluate);
}

boolean leap25_check(validator_id evaluate)
{
  return leapers_check(vec_leap25_start, vec_leap25_end, evaluate);
}

boolean leap35_check(validator_id evaluate)
{
  return leapers_check(vec_leap35_start, vec_leap35_end, evaluate);
}

boolean leap36_check(validator_id evaluate)
{
  return leapers_check(vec_leap36_start, vec_leap36_end, evaluate);
}

boolean leap37_check(validator_id evaluate)
{
  return leapers_check(vec_leap37_start, vec_leap37_end, evaluate);
}

boolean okapi_check(validator_id evaluate)
{
  return leapers_check(vec_okapi_start, vec_okapi_end, evaluate);   /* knight+zebra */
}

boolean bison_check(validator_id evaluate)
{
  return leapers_check(vec_bison_start, vec_bison_end, evaluate);    /* camel+zebra */
}

boolean zebu_check(validator_id evaluate)
{
  return (leapers_check(vec_chameau_start,vec_chameau_end,evaluate)
          || leapers_check(vec_girafe_start,vec_girafe_end,evaluate));
}

boolean gnu_check(validator_id evaluate)
{
  return  leapers_check(vec_knight_start,vec_knight_end, evaluate)
      || leapers_check(vec_chameau_start, vec_chameau_end, evaluate);
}

boolean antilope_check(validator_id evaluate)
{
  return leapers_check(vec_antilope_start, vec_antilope_end, evaluate);
}

boolean squirrel_check(validator_id evaluate)
{
  return  leapers_check(vec_knight_start,vec_knight_end, evaluate)
      || leapers_check(vec_ecureuil_start, vec_ecureuil_end, evaluate);
}
