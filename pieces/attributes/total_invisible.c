#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "position/position.h"
#include "position/move_diff_code.h"
#include "position/effects/piece_creation.h"
#include "position/effects/null_move.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/goals/slice_insertion.h"
#include "solving/check.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/machinery/slack_length.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/move_effect_journal.h"
#include "solving/castling.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/pieces.h"
#include "optimisations/orthodox_square_observation.h"
#include "optimisations/orthodox_check_directions.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

unsigned int total_invisible_number;

static unsigned int nr_total_invisibles_left;

static unsigned int idx_next_placed_victim = 0;
static unsigned int idx_next_placed_mating_piece_attacker = 0;
static unsigned int idx_next_placed_interceptor = 0;

static ply ply_replayed;

static stip_length_type combined_result;

static boolean end_of_iteration;

static unsigned int taboo[nr_sides][maxsquare];

static struct
{
    Side side;
    piece_walk_type walk;
    square pos;
} piece_choice[nr_squares_on_board];

static enum
{
  regular_play,
  validating_mate,
  replaying_moves
} play_phase = regular_play;

static enum
{
  mate_unvalidated,
  no_mate,
  mate_attackable,
  mate_with_2_uninterceptable_doublechecks,
  mate_defendable_by_interceptors
} mate_validation_result;

static square sq_mating_piece_to_be_attacked = initsquare;

static boolean is_rider_check_uninterceptable_on_vector(Side side_checking, square king_pos,
                                                        vec_index_type k, piece_walk_type rider_walk)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceValue("%u",k);
  TraceWalk(rider_walk);
  TraceFunctionParamListEnd();

  {
    square s = king_pos+vec[k];
    while (is_square_empty(s) && taboo[White][s]>0 && taboo[Black][s]>0)
      s += vec[k];

    {
      piece_walk_type const walk = get_walk_of_piece_on_square(s);
      result = ((walk==rider_walk || walk==Queen)
                && TSTFLAG(being_solved.spec[s],side_checking));
    }
    TraceSquare(s);
    TraceValue("%u",is_square_empty(s));
    TraceValue("%u",taboo[White][s]);
    TraceValue("%u",taboo[Black][s]);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static vec_index_type is_rider_check_uninterceptable(Side side_checking, square king_pos,
                                                     vec_index_type kanf, vec_index_type kend, piece_walk_type rider_walk)
{
  vec_index_type result = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceValue("%u",kanf);
  TraceValue("%u",kend);
  TraceWalk(rider_walk);
  TraceFunctionParamListEnd();

  {
    vec_index_type k;
    for (k = kanf; !result && k<=kend; k++)
      if (is_rider_check_uninterceptable_on_vector(side_checking,king_pos,k,rider_walk))
      {
        result = k;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static vec_index_type is_square_uninterceptably_attacked(Side side_under_attack, square sq_attacked)
{
  vec_index_type result = 0;
  Side const side_checking = advers(side_under_attack);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_under_attack);
  TraceSquare(sq_attacked);
  TraceFunctionParamListEnd();

  if (!result && being_solved.number_of_pieces[side_checking][King]>0)
    result = king_check_ortho(side_checking,sq_attacked);

  if (!result && being_solved.number_of_pieces[side_checking][Pawn]>0)
    result = pawn_check_ortho(side_checking,sq_attacked);

  if (!result && being_solved.number_of_pieces[side_checking][Knight]>0)
    result = knight_check_ortho(side_checking,sq_attacked);

  if (!result && being_solved.number_of_pieces[side_checking][Rook]+being_solved.number_of_pieces[side_checking][Queen]>0)
    result = is_rider_check_uninterceptable(side_checking,sq_attacked, vec_rook_start,vec_rook_end, Rook);

  if (!result && being_solved.number_of_pieces[side_checking][Bishop]+being_solved.number_of_pieces[side_checking][Queen]>0)
    result = is_rider_check_uninterceptable(side_checking,sq_attacked, vec_bishop_start,vec_bishop_end, Bishop);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void play_with_placed_invisibles(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TracePosition(being_solved.board,being_solved.spec);

  if (is_in_check(advers(SLICE_STARTER(si))))
    solve_result = previous_move_is_illegal;
  else
    pipe_solve_delegate(si);

  if (solve_result>combined_result)
    combined_result = solve_result;

  if (combined_result==previous_move_has_not_solved || play_phase==validating_mate)
    end_of_iteration = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void walk_interceptor(slice_index si,
                             unsigned int base,
                             unsigned int idx)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",idx);
  TraceFunctionParamListEnd();

  if (idx==idx_next_placed_interceptor)
    play_with_placed_invisibles(si);
  else
  {
    square const place = piece_choice[idx].pos;

    TraceSquare(place);TraceEOL();
    assert(is_square_empty(place));

    for (piece_choice[idx].walk = Pawn;
         piece_choice[idx].walk<=Bishop && !end_of_iteration;
         ++piece_choice[idx].walk)
    {
      ++being_solved.number_of_pieces[piece_choice[idx].side][piece_choice[idx].walk];
      occupy_square(place,piece_choice[idx].walk,BIT(piece_choice[idx].side));
      if (!is_square_uninterceptably_attacked(advers(piece_choice[idx].side),
                                              being_solved.king_square[advers(piece_choice[idx].side)]))
        walk_interceptor(si,base,idx+1);
      empty_square(place);
      --being_solved.number_of_pieces[piece_choice[idx].side][piece_choice[idx].walk];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void colour_interceptor(slice_index si, unsigned int base, unsigned int idx)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",base);
  TraceFunctionParam("%u",idx);
  TraceFunctionParamListEnd();

  if (idx==idx_next_placed_interceptor)
    walk_interceptor(si,base,base);
  else
  {
    /* remove the dummy */
    empty_square(piece_choice[idx].pos);

    if (taboo[piece_choice[idx].side][piece_choice[idx].pos]==0)
      colour_interceptor(si,base,idx+1);

    if (!end_of_iteration)
    {
      piece_choice[idx].side = advers(piece_choice[idx].side);
      if (taboo[piece_choice[idx].side][piece_choice[idx].pos]==0)
        colour_interceptor(si,base,idx+1);
    }

    /* re-place the dummy */
    occupy_square(piece_choice[idx].pos,Dummy,BIT(White)|BIT(Black));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_intercepting_checks(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (play_phase==validating_mate)
    play_with_placed_invisibles(si);
  else
    colour_interceptor(si,idx_next_placed_mating_piece_attacker,idx_next_placed_mating_piece_attacker);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_check_to_be_intercepted(ply current_ply, slice_index si);

static void unwrap_move_effects(ply current_ply, slice_index si)
{
  ply const save_nbply = nbply;
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_arrival = move_gen_top->arrival;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",current_ply);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ply_replayed = nbply;

  undo_move_effects();

  /* before we arrived on this square, it is not taboo for a total invisible
   * to have occupied it, provided it could be captured */
  // TODO sq_arrival or sq_capture?
  assert(taboo[advers(trait[nbply])][sq_arrival]>0);
  --taboo[advers(trait[nbply])][sq_arrival];

  nbply = parent_ply[nbply];
  deal_with_check_to_be_intercepted(current_ply,si);
  nbply = save_nbply;

  ++taboo[advers(trait[nbply])][sq_arrival];

  redo_move_effects();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_check_to_be_intercepted_diagonal(ply current_ply,
                                                       slice_index si,
                                                       vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",current_ply);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  if (kcurr>kend)
  {
    if (nbply==ply_retro_move)
    {
      ply const save_nbply = nbply;
      nbply = current_ply;
      done_intercepting_checks(si);
      nbply = save_nbply;
    }
    else
      unwrap_move_effects(current_ply,si);
  }
  else
  {
    Side const side_in_check = trait[nbply];
    Side const side_checking = advers(side_in_check);
    square const king_pos = being_solved.king_square[side_in_check];
    square const sq_end = find_end_of_line(king_pos,vec[kcurr]);
    piece_walk_type const walk_at_end = get_walk_of_piece_on_square(sq_end);

    TraceEnumerator(Side,side_in_check);
    TraceEnumerator(Side,side_checking);
    TraceSquare(king_pos);
    TraceSquare(sq_end);
    TraceWalk(walk_at_end);
    TraceEOL();

    if ((walk_at_end==Bishop || walk_at_end==Queen)
        && TSTFLAG(being_solved.spec[sq_end],side_checking))
    {
      TraceValue("%u",idx_next_placed_interceptor);
      TraceValue("%u",nr_total_invisibles_left);
      TraceEOL();
      if (idx_next_placed_interceptor<nr_total_invisibles_left)
      {
        square s;
        for (s = king_pos+vec[kcurr];
             is_square_empty(s) && !end_of_iteration;
             s += vec[kcurr])
        {
          TraceSquare(s);
          TraceValue("%u",taboo[White][s]);
          TraceValue("%u",taboo[Black][s]);
          TraceEOL();
          if (taboo[White][s]==0 || taboo[Black][s]==0)
          {
            assert(!is_rider_check_uninterceptable_on_vector(side_checking,king_pos,kcurr,walk_at_end));
            piece_choice[idx_next_placed_interceptor].pos = s;
            piece_choice[idx_next_placed_interceptor].side = side_in_check;
            TraceSquare(s);TraceEnumerator(Side,side_in_check);TraceEOL();

            /* occupy the square to avoid intercepting it again "2 half moves ago" */
            occupy_square(s,Dummy,BIT(White)|BIT(Black));
            ++idx_next_placed_interceptor;
            deal_with_check_to_be_intercepted_diagonal(current_ply,si,kcurr+1,kend);
            --idx_next_placed_interceptor;
            empty_square(s);
          }
        }
      }
      else
      {
        /* there are not enough total invisibles to intercept all checks */
      }
    }
    else
      deal_with_check_to_be_intercepted_diagonal(current_ply,
                                                 si,
                                                 kcurr+1,kend);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_check_to_be_intercepted_orthogonal(ply current_ply,
                                                         slice_index si,
                                                         vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",current_ply);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  if (kcurr>kend)
    deal_with_check_to_be_intercepted_diagonal(current_ply,
                                               si,
                                               vec_bishop_start,vec_bishop_end);
  else
  {
    Side const side_in_check = trait[nbply];
    Side const side_checking = advers(side_in_check);
    square const king_pos = being_solved.king_square[side_in_check];
    square const sq_end = find_end_of_line(king_pos,vec[kcurr]);
    piece_walk_type const walk_at_end = get_walk_of_piece_on_square(sq_end);

    TraceEnumerator(Side,side_in_check);
    TraceEnumerator(Side,side_checking);
    TraceSquare(king_pos);
    TraceSquare(sq_end);
    TraceWalk(walk_at_end);
    TraceEOL();

    if ((walk_at_end==Rook || walk_at_end==Queen)
        && TSTFLAG(being_solved.spec[sq_end],side_checking))
    {
      TraceValue("%u",idx_next_placed_interceptor);
      TraceValue("%u",nr_total_invisibles_left);
      TraceEOL();
      if (idx_next_placed_interceptor<nr_total_invisibles_left)
      {
        square s;
        for (s = king_pos+vec[kcurr];
             is_square_empty(s) && !end_of_iteration;
             s += vec[kcurr])
        {
          TraceSquare(s);
          TraceValue("%u",taboo[White][s]);
          TraceValue("%u",taboo[Black][s]);
          TraceEOL();
          if (taboo[White][s]==0 || taboo[Black][s]==0)
          {
            assert(!is_rider_check_uninterceptable_on_vector(side_checking,king_pos,kcurr,walk_at_end));
            piece_choice[idx_next_placed_interceptor].pos = s;
            piece_choice[idx_next_placed_interceptor].side = side_in_check;
            TraceSquare(s);TraceEnumerator(Side,side_in_check);TraceEOL();

            /* occupy the square to avoid intercepting it again "2 half moves ago" */
            occupy_square(s,Dummy,BIT(White)|BIT(Black));
            ++idx_next_placed_interceptor;
            deal_with_check_to_be_intercepted_orthogonal(current_ply,si,kcurr+1,kend);
            --idx_next_placed_interceptor;
            empty_square(s);
          }
        }
      }
      else
      {
        /* there are not enough total invisibles to intercept all checks */
      }
    }
    else
      deal_with_check_to_be_intercepted_orthogonal(current_ply,
                                                   si,
                                                   kcurr+1,kend);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void deal_with_check_to_be_intercepted(ply current_ply, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",current_ply);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  deal_with_check_to_be_intercepted_orthogonal(current_ply,
                                               si,
                                               vec_rook_start,vec_rook_end);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_taboo(int delta)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;

  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  piece_walk_type const walk = move_effect_journal[movement].u.piece_movement.moving;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceEOL();

  taboo[White][sq_arrival] += delta;
  taboo[Black][sq_arrival] += delta;

  switch (sq_capture)
  {
    case kingside_castling :
    case queenside_castling:
    case pawn_multistep:
    {
      square const sq_intermediate = (sq_departure+sq_arrival)/2;
      taboo[White][sq_intermediate] += delta;
      taboo[Black][sq_intermediate] += delta;
      break;
    }

    case messigny_exchange:
    case retro_capture_departure:
    case no_capture:
      break;

    default:
      taboo[White][sq_capture] += delta;
      taboo[Black][sq_capture] += delta;
      break;
  }

  if (is_rider(walk))
  {
    int const diff_move = sq_arrival-sq_departure;
    int const dir_move = CheckDir[walk][diff_move];

    square s;
    assert(dir_move!=0);
    for (s = sq_departure+dir_move; s!=sq_arrival; s += dir_move)
    {
      taboo[White][s] += delta;
      taboo[Black][s] += delta;
    }
  }
  else if (is_pawn(walk))
  {
    /* arrival square must not be blocked */
    taboo[White][sq_arrival] += delta;
    taboo[Black][sq_arrival] += delta;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacker(slice_index si,
                                        Side side_attacking,
                                        square s,
                                        piece_walk_type walk)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(s);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  ++idx_next_placed_mating_piece_attacker;
  ++being_solved.number_of_pieces[side_attacking][walk];
  occupy_square(s,walk,BIT(side_attacking));
  idx_next_placed_interceptor = idx_next_placed_mating_piece_attacker;
  deal_with_check_to_be_intercepted(nbply,si);
  empty_square(s);
  --being_solved.number_of_pieces[side_attacking][walk];
  --idx_next_placed_mating_piece_attacker;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_rider(slice_index si,
                                               Side side_attacking,
                                               square sq_mating_piece,
                                               piece_walk_type walk_rider,
                                               vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && combined_result<previous_move_has_not_solved; ++kcurr)
  {
    square s;
    for (s = sq_mating_piece+vec[kcurr];
         is_square_empty(s) && !end_of_iteration;
         s += vec[kcurr])
    {
      TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
      if (taboo[side_attacking][s]==0)
        place_mating_piece_attacker(si,side_attacking,s,walk_rider);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_leaper(slice_index si,
                                                Side side_attacking,
                                                square sq_mating_piece,
                                                piece_walk_type walk_leaper,
                                                vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square const s = sq_mating_piece+vec[kcurr];
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[side_attacking][s]==0)
      place_mating_piece_attacker(si,side_attacking,s,walk_leaper);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_pawn(slice_index si,
                                                Side side_attacking,
                                                square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceFunctionParamListEnd();

  if (!end_of_iteration)
  {
    square s = sq_mating_piece+dir_up+dir_left;
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[side_attacking][s]==0)
      place_mating_piece_attacker(si,side_attacking,s,Pawn);
  }

  if (!end_of_iteration)
  {
    square s = sq_mating_piece+dir_up+dir_right;
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[side_attacking][s]==0)
      place_mating_piece_attacker(si,side_attacking,s,Pawn);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void attack_mating_piece(slice_index si,
                                Side side_attacking,
                                square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(sq_mating_piece);
  TraceFunctionParamListEnd();

  /* We are defending against a verified mate. If we can't attack the mate,
   * we are dealing with a solution. */
  combined_result = previous_move_has_solved;

  place_mating_piece_attacking_rider(si,
                                     side_attacking,
                                     sq_mating_piece,
                                     Bishop,
                                     vec_bishop_start,vec_bishop_end);

  place_mating_piece_attacking_rider(si,
                                     side_attacking,
                                     sq_mating_piece,
                                     Rook,
                                     vec_rook_start,vec_rook_end);

  place_mating_piece_attacking_leaper(si,
                                      side_attacking,
                                      sq_mating_piece,
                                      Knight,
                                      vec_knight_start,vec_knight_end);

  place_mating_piece_attacking_pawn(si,
                                    side_attacking,
                                    sq_mating_piece);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_move_sequence_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply-ply_retro_move);TraceEOL();

  update_taboo(+1);

  /* necessary for detecting checks by pawns and leapers */
  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else
  {
    /* make sure that our length corresponds to the length of the tested move sequence
     * (which may vary if STFindShortest is used)
     */
    assert(slices[SLICE_NEXT2(si)].type==STHelpAdapter);
    slices[SLICE_NEXT2(si)].u.branch.length = slack_length+(nbply-ply_retro_move);

    TraceValue("%u",SLICE_NEXT2(si));
    TraceValue("%u",slices[SLICE_NEXT2(si)].u.branch.length);
    TraceEOL();

    mate_validation_result = mate_unvalidated;

    combined_result = previous_move_is_illegal;
    idx_next_placed_mating_piece_attacker = idx_next_placed_victim;
    idx_next_placed_interceptor = idx_next_placed_mating_piece_attacker;
    play_phase = validating_mate;
    end_of_iteration = false;
    deal_with_check_to_be_intercepted(nbply,si);

    play_phase = replaying_moves;
    end_of_iteration = false;

    switch (mate_validation_result)
    {
      case mate_unvalidated:
        assert(combined_result==previous_move_is_illegal);
        break;

      case no_mate:
        assert(combined_result==previous_move_has_not_solved);
        break;

      case mate_attackable:
        combined_result = previous_move_is_illegal;
        attack_mating_piece(si,advers(trait[nbply]),sq_mating_piece_to_be_attacked);
        break;

      case mate_defendable_by_interceptors:
        combined_result = previous_move_is_illegal;
        deal_with_check_to_be_intercepted(nbply,si);
        break;

      case mate_with_2_uninterceptable_doublechecks:
        combined_result = previous_move_has_solved;
        break;

      default:
        assert(0);
        break;
    }

    play_phase = regular_play;
    solve_result = combined_result==immobility_on_next_move ? previous_move_has_not_solved : combined_result;
  }

  update_taboo(-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_move_still_playable(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side = SLICE_STARTER(si);
    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].departure;
    square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].arrival;
    square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture;

    TraceValue("%u",ply_replayed);
    TraceSquare(sq_departure);
    TraceSquare(sq_arrival);
    TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture);
    TraceEOL();

    // TODO redo (and undo) the pre-capture effect?

    if (sq_capture==queenside_castling)
      SETCASTLINGFLAGMASK(side,ra_cancastle);
    else if (sq_capture==kingside_castling)
      SETCASTLINGFLAGMASK(side,rh_cancastle);
    else if (!is_no_capture(sq_capture) && is_square_empty(sq_capture))
      occupy_square(sq_capture,Dummy,BIT(White)|BIT(Black));

    assert(TSTFLAG(being_solved.spec[sq_departure],side));
    generate_moves_for_piece(sq_departure);

    if (sq_capture==queenside_castling)
      CLRCASTLINGFLAGMASK(side,ra_cancastle);
    else if (sq_capture==kingside_castling)
      CLRCASTLINGFLAGMASK(side,rh_cancastle);
    else if (!is_no_capture(sq_capture) && being_solved.board[sq_capture]==Dummy)
      empty_square(sq_capture);

    {
      numecoup start = MOVEBASE_OF_PLY(nbply);
      numecoup i;
      numecoup new_top = start;
      for (i = start+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
      {
        assert(move_generation_stack[i].departure==sq_departure);
        if (move_generation_stack[i].arrival==sq_arrival)
        {
          ++new_top;
          move_generation_stack[new_top] = move_generation_stack[i];
          break;
        }
      }

      SET_CURRMOVE(nbply,new_top);
    }

    result = CURRMOVE_OF_PLY(nbply)>MOVEBASE_OF_PLY(nbply);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void copy_move_effects(void)
{
  move_effect_journal_index_type const replayed_base = move_effect_journal_base[ply_replayed];
  move_effect_journal_index_type const replayed_top = move_effect_journal_base[ply_replayed+1];
  move_effect_journal_index_type replayed_curr;
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",ply_replayed);
  TraceValue("%u",move_effect_journal_base[ply_replayed]);
  TraceValue("%u",move_effect_journal_base[ply_replayed+1]);
  TraceValue("%u",nbply);
  TraceEOL();

  move_effect_journal_base[nbply+1] += (replayed_top-replayed_base);
  curr = move_effect_journal_base[nbply+1];
  replayed_curr = replayed_top;

  while (replayed_curr>replayed_base+move_effect_journal_index_offset_movement)
    move_effect_journal[--curr] = move_effect_journal[--replayed_curr];

  /* do we have to invent a removal effect for an inserted total invisible? */
  {
    square const to = move_effect_journal[replayed_curr].u.piece_movement.to;
    assert(move_effect_journal[replayed_curr].type==move_effect_piece_movement);
    if (is_square_empty(to))
      move_effect_journal[--curr] = move_effect_journal[--replayed_curr];
    else
    {
      --curr;
      --replayed_curr;
      move_effect_journal[curr].type = move_effect_piece_removal;
      move_effect_journal[curr].u.piece_removal.on = to;
      move_effect_journal[curr].u.piece_removal.walk = get_walk_of_piece_on_square(to);
      move_effect_journal[curr].u.piece_removal.flags = being_solved.spec[to];
    }
  }

  /* finally, copy our pre-capture effect */
  move_effect_journal[--curr] = move_effect_journal[--replayed_curr];

  assert(curr==move_effect_journal_base[nbply]);
  assert(replayed_curr==replayed_base);

  TraceValue("%u",move_effect_journal_base[nbply]);
  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_move_repeater_solve(slice_index si)
{
  boolean playable;
  numecoup const curr = CURRMOVE_OF_PLY(ply_replayed);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));

  /* With the current placement algorithm, the move is always playable unless it is a castling */

  switch (sq_capture)
  {
    case queenside_castling:
    case kingside_castling:
      playable = is_move_still_playable(si);
      break;

    default:
      assert(is_move_still_playable(si));
      playable = true;
      break;
  }

  if (playable)
  {
    copy_move_effects();
    redo_move_effects();
    ++ply_replayed;
    pipe_solve_delegate(si);
    --ply_replayed;
    undo_move_effects();
  }
  else
    solve_result = previous_move_is_illegal;

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_uninterceptable_selfcheck_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  update_taboo(+1);

  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else
    pipe_solve_delegate(si);

  update_taboo(-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean make_flight_ortho(Side side_in_check,
                                 square flight,
                                 vec_index_type k_start, vec_index_type k_end,
                                 piece_walk_type walk_rider)
{
  boolean result = false;
  Side const side_checking = advers(side_in_check);
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(flight);
  TraceFunctionParam("%u",k_start);
  TraceFunctionParam("%u",k_end);
  TraceWalk(walk_rider);
  TraceFunctionParamListEnd();

  for (k = k_start; k<=k_end && !result; ++k)
  {
    square const end = find_end_of_line(flight,vec[k]);
    piece_walk_type const walk = being_solved.board[end];
    TraceSquare(end);TraceWalk(walk);TraceEOL();
    if ((walk==Queen || walk==walk_rider) && TSTFLAG(being_solved.spec[end],side_checking))
    {
      TraceValue("%u",idx_next_placed_interceptor);
      TraceValue("%u",nr_total_invisibles_left);
      TraceEOL();
      if (idx_next_placed_interceptor<nr_total_invisibles_left)
      {
        square s;
        for (s = flight+vec[k]; s!=end; s += vec[k])
        {
          TraceSquare(s);TraceValue("%u",taboo[White][s]);TraceValue("%u",taboo[Black][s]);TraceEOL();
          if (taboo[White][s]==0 || taboo[Black][s]==0)
          {
            TraceText("intercepted guard");TraceEOL();
            result = true;
            break;
          }
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean make_flight(Side side_in_check, square s)
{
  boolean result = false;
  Side const side_checking = advers(side_in_check);

  TraceFunctionEntry(__func__);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  if (!is_square_uninterceptably_attacked(side_in_check,s))
  {
    if ((being_solved.number_of_pieces[side_checking][Rook]+being_solved.number_of_pieces[side_checking][Queen]>0
         && make_flight_ortho(side_in_check,s,vec_rook_start,vec_rook_end,Rook))
        || (being_solved.number_of_pieces[side_checking][Bishop]+being_solved.number_of_pieces[side_checking][Queen]>0
            && make_flight_ortho(side_in_check,s,vec_bishop_start,vec_bishop_end,Bishop)))
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean make_a_flight(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_in_check = advers(trait[nbply]);
    square const king_pos = being_solved.king_square[side_in_check];
    int dir_vert;
    int dir_horiz;

    piece_walk_type const walk = get_walk_of_piece_on_square(king_pos);
    Flags const flags = being_solved.spec[king_pos];

    empty_square(king_pos);
    ++taboo[White][king_pos];
    ++taboo[Black][king_pos];

    for (dir_vert = dir_down; dir_vert<=dir_up; dir_vert += dir_up)
      for (dir_horiz = dir_left; dir_horiz<=dir_right; dir_horiz += dir_right)
      {
        square const flight = king_pos+dir_vert+dir_horiz;
        if (flight!=king_pos && make_flight(side_in_check,flight))
        {
          result = true;
          break;
        }
      }

    --taboo[White][king_pos];
    --taboo[Black][king_pos];
    occupy_square(king_pos,walk,flags);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void attack_checks(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_delivering_check = trait[nbply];
    Side const side_in_check = advers(side_delivering_check);
    square const king_pos = being_solved.king_square[side_in_check];
    vec_index_type const k = is_square_uninterceptably_attacked(side_in_check,king_pos);
    if (k==0)
    {
      if (make_flight(side_in_check,king_pos))
      {
        /* the king square can be made a "flight" */
        mate_validation_result = no_mate;
        solve_result = previous_move_has_not_solved;
      }
      else
        mate_validation_result = mate_defendable_by_interceptors;
    }
    else if (idx_next_placed_interceptor<nr_total_invisibles_left)
    {
      square const sq_attacker = find_end_of_line(king_pos,vec[k]);
      TraceSquare(king_pos);TraceValue("%u",k);TraceValue("%d",vec[k]);TraceSquare(sq_attacker);TraceEOL();
      assert(TSTFLAG(being_solved.spec[sq_attacker],side_delivering_check));
      CLRFLAG(being_solved.spec[sq_attacker],side_delivering_check);
      if (is_square_uninterceptably_attacked(side_in_check,king_pos))
        mate_validation_result = mate_with_2_uninterceptable_doublechecks;
      else
      {
        mate_validation_result = mate_attackable;
        sq_mating_piece_to_be_attacked = sq_attacker;
      }
      SETFLAG(being_solved.spec[sq_attacker],side_delivering_check);
    }
    else
      mate_validation_result = mate_defendable_by_interceptors;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_goal_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* make sure that we don't generate pawn captures total invisible */
  assert(play_phase!=regular_play);

  pipe_solve_delegate(si);

  if (play_phase==validating_mate)
  {
    TraceValue("%u",idx_next_placed_interceptor);
    TraceEOL();

    if (solve_result==previous_move_has_not_solved)
      mate_validation_result = no_mate;
    else if (make_a_flight())
    {
      solve_result = previous_move_has_not_solved;
      mate_validation_result = no_mate;
    }
    else
      attack_checks();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_pawn_capture_right(slice_index si, int dir_vertical)
{
  square const s = curr_generation->departure+dir_vertical+dir_right;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_empty(s))
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black));
    pipe_move_generation_delegate(si);
    empty_square(s);
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_pawn_capture_left(slice_index si, int dir_vertical)
{
  square const s = curr_generation->departure+dir_vertical+dir_left;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_empty(s))
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black));
    generate_pawn_capture_right(si,dir_vertical);
    empty_square(s);
  }
  else
    generate_pawn_capture_right(si,dir_vertical);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepare_king_side_castling_generation(slice_index si)
{
  Side const side = trait[nbply];
  square const square_a = side==White ? square_a1 : square_a8;
  square const square_h = square_a+file_h;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (nr_total_invisibles_left>0 && is_square_empty(square_h))
  {
    ++being_solved.number_of_pieces[side][Rook];
    occupy_square(square_h,Rook,BIT(side));
    SETCASTLINGFLAGMASK(side,rh_cancastle);
    pipe_move_generation_delegate(si);
    CLRCASTLINGFLAGMASK(side,rh_cancastle);
    empty_square(square_h);
    --being_solved.number_of_pieces[side][Rook];
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepare_queen_side_castling_generation(slice_index si)
{
  Side const side = trait[nbply];
  square const square_a = side==White ? square_a1 : square_a8;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (nr_total_invisibles_left>0 && is_square_empty(square_a))
  {
    ++being_solved.number_of_pieces[side][Rook];
    occupy_square(square_a,Rook,BIT(side));
    SETCASTLINGFLAGMASK(side,ra_cancastle);
    prepare_king_side_castling_generation(si);
    CLRCASTLINGFLAGMASK(side,ra_cancastle);
    empty_square(square_a);
    --being_solved.number_of_pieces[side][Rook];
  }
  else
    prepare_king_side_castling_generation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void total_invisible_generate_special_moves(slice_index si)
{
  square const sq_departure = curr_generation->departure ;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (play_phase==regular_play)
  {
    switch (being_solved.board[sq_departure])
    {
      case Pawn:
        if (idx_next_placed_victim<nr_total_invisibles_left)
          generate_pawn_capture_left(si,trait[nbply]==White ? dir_up : dir_down);
        break;

      case King:
        if (TSTCASTLINGFLAGMASK(trait[nbply],castlings)>=k_cancastle)
          prepare_queen_side_castling_generation(si);
        else
          pipe_move_generation_delegate(si);
        break;

      default:
        pipe_move_generation_delegate(si);
        break;
    }
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_special_moves_player_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt * const move_gen_top = move_generation_stack+curr;
    square const sq_capture = move_gen_top->capture;

    switch (sq_capture)
    {
      case pawn_multistep:
        move_effect_journal_do_null_effect();
        pipe_solve_delegate(si);
        break;

      case messigny_exchange:
        move_effect_journal_do_null_effect();
        pipe_solve_delegate(si);
        break;

      case kingside_castling:
      {
        Side const side = trait[nbply];
        square const square_a = side==White ? square_a1 : square_a8;
        square const square_h = square_a+file_h;

        TraceText("kingside_castling\n");

        assert(nr_total_invisibles_left>0);
        if (is_square_empty(square_h))
        {
          move_effect_journal_do_piece_creation(move_effect_reason_revelation_of_invisible,
                                                square_h,
                                                Rook,
                                                BIT(side),
                                                side);

          piece_choice[idx_next_placed_victim].pos = square_h;

          ++idx_next_placed_victim;
          pipe_solve_delegate(si);
          --idx_next_placed_victim;
        }
        else
        {
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
        }
        break;
      }

      case queenside_castling:
      {
        Side const side = trait[nbply];
        square const square_a = side==White ? square_a1 : square_a8;

        TraceText("queenside_castling\n");

        if (is_square_empty(square_a))
        {
          move_effect_journal_do_piece_creation(move_effect_reason_revelation_of_invisible,
                                                square_a,
                                                Rook,
                                                BIT(side),
                                                side);

          piece_choice[idx_next_placed_victim].pos = square_a;

          ++idx_next_placed_victim;
          pipe_solve_delegate(si);
          --idx_next_placed_victim;
        }
        else
        {
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
        }
        break;
      }

      case no_capture:
        move_effect_journal_do_null_effect();
        pipe_solve_delegate(si);
        break;

      default:
        /* pawn captures total invisible? */
        if (is_square_empty(sq_capture))
        {
          Side const side_victim = advers(SLICE_STARTER(si));
          move_effect_journal_do_piece_creation(move_effect_reason_removal_of_invisible,
                                                sq_capture,
                                                Dummy,
                                                BIT(side_victim),
                                                side_victim);

          piece_choice[idx_next_placed_victim].pos = sq_capture;

          ++idx_next_placed_victim;
          pipe_solve_delegate(si);
          --idx_next_placed_victim;
        }
        else
        {
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
        }
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_generator(slice_index si,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleMoveSequenceMoveRepeater);
    SLICE_STARTER(prototype) = SLICE_STARTER(si);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_the_pipe(slice_index si,
                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_castling_player(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  dealloc_slice(SLICE_NEXT2(si));
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_self_check_guard(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_index * const remembered = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  *remembered = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_goal_guard(slice_index si,
                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index remembered = no_slice;

    stip_structure_traversal st_nested;
    stip_structure_traversal_init_nested(&st_nested,st,&remembered);
    stip_structure_traversal_override_single(&st_nested,
                                             STSelfCheckGuard,
                                             &remember_self_check_guard);
    stip_traverse_structure(SLICE_NEXT2(si),&st_nested);

    if (remembered!=no_slice)
    {
      slice_index prototype = alloc_pipe(STTotalInvisibleGoalGuard);
      SLICE_STARTER(prototype) = SLICE_STARTER(remembered);
      goal_branch_insert_slices(SLICE_NEXT2(si),&prototype,1);
      pipe_remove(remembered);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_replay_branch(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;

    stip_structure_traversal_init_nested(&st_nested,st,0);
    // TODO prevent instrumentation in the first place?
    stip_structure_traversal_override_single(&st_nested,
                                             STFindShortest,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STFindAttack,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveEffectJournalUndoer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPostMoveIterationInitialiser,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STCastlingPlayer,
                                             &remove_castling_player);
    stip_structure_traversal_override_single(&st_nested,
                                             STMovePlayer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPawnPromoter,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveGenerator,
                                             &subsitute_generator);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachedTester,
                                             &subsitute_goal_guard);
    /* self check is impossible with the current optimisations for orthodox pieces */
    stip_structure_traversal_override_single(&st_nested,
                                             STSelfCheckGuard,
                                             &remove_the_pipe);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_self_check_guard(slice_index si,
                                    stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* This iteration ends at STTotalInvisibleMoveSequenceTester. We can therefore
   * blindly tamper with all STSelfCheckGuard slices that we meet.
   */
  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_intro)
    pipe_remove(si);
  else
    SLICE_TYPE(si) = STTotalInvisibleUninterceptableSelfCheckGuard;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STTotalInvisibleMoveSequenceTester,
                                             &instrument_replay_branch);
    stip_structure_traversal_override_single(&st,
                                             STSelfCheckGuard,
                                             &remove_self_check_guard);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  output_plaintext_check_indication_disabled = true;

  solving_instrument_move_generation(si,nr_sides,STTotalInvisibleSpecialMoveGenerator);

  {
    square const *s;
    for (s = boardnum; *s; ++s)
      if (!is_square_empty(*s))
      {
        ++taboo[White][*s];
        ++taboo[Black][*s];
      }
  }

  pipe_solve_delegate(si);

  {
    square const *s;
    for (s = boardnum; *s; ++s)
      if (!is_square_empty(*s))
      {
        --taboo[White][*s];
        --taboo[Black][*s];
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean instrumenting;
    slice_index the_copy;
    stip_length_type length;
} insertion_state_type;

static void insert_copy(slice_index si,
                        stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->the_copy==no_slice)
    stip_traverse_structure_children(si,st);
  else
  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const substitute = alloc_pipe(STTotalInvisibleMoveSequenceTester);
    pipe_link(proxy,substitute);
    link_to_branch(substitute,state->the_copy);
    SLICE_NEXT2(substitute) = state->the_copy;
    state->the_copy = no_slice;
    dealloc_slices(SLICE_NEXT2(si));
    SLICE_NEXT2(si) = proxy;

    assert(state->length!=no_stip_length);
    if (state->length%2!=0)
      pipe_append(proxy,alloc_pipe(STMoveInverter));
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void copy_help_branch(slice_index si,
                             stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",state->instrumenting);
  TraceEOL();

  state->length = slices[si].u.branch.length;

  if (state->instrumenting)
    stip_traverse_structure_children(si,st);
  else
  {
    state->instrumenting = true;
    state->the_copy = stip_deep_copy(si);
    stip_traverse_structure_children(si,st);
    assert(state->the_copy==no_slice);

    {
      slice_index const prototypes[] = {
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer)
      };
      enum { nr_protypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert(si,prototypes,nr_protypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with support for Total Invisible pieces
 * @param si identifies the root slice of the stipulation
 */
void solving_instrument_total_invisible(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  // later:
  // - in original
  //   - insert revelation logic
  // - in copy
  //   - logic for iteration over all possibilities of invisibles
  //     - special case of invisible king
  //     - special case: position has no legal placement of all invisibles may have to be dealt with:
  //       - not enough empty squares :-)
  //   - substitute for STFindShortest

  // bail out at STAttackAdapter

  // input for nr_total_invisibles_left, initialize to 0

  // what about:
  // - structured stipulations?
  // - goals that don't involve immobility
  // ?

  // we shouldn't need to set the starter of
  // - STTotalInvisibleMoveSequenceMoveRepeater
  // - STTotalInvisibleGoalGuard

  // check indication should also be deactivated in tree output

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInstrumenter);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    stip_structure_traversal st;
    insertion_state_type state = { false, no_slice, no_stip_length };

    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override_single(&st,
                                             STHelpAdapter,
                                             &copy_help_branch);
    stip_structure_traversal_override_single(&st,
                                             STGoalReachedTester,
                                             &insert_copy);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  nr_total_invisibles_left = total_invisible_number;

  move_effect_journal_register_pre_capture_effect();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
