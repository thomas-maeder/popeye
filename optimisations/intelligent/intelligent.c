/******************** MODIFICATIONS to pyint.c **************************
 **
 ** Date       Who  What
 **
 ** 2006/06/14 TLi  bug fix in function guards_black_flight()
 **
 ** 2007/12/27 TLi  bug fix in function stalemate_immobilise_black()
 **
 **************************** End of List ******************************/

#include "optimisations/intelligent/intelligent.h"

#include "pieces/walks/pawns/en_passant.h"
#include "pyproof.h"
#include "solving/solve.h"
#include "solving/castling.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/fork.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/temporary_hacks.h"
#include "optimisations/intelligent/count_nr_of_moves.h"
#include "optimisations/intelligent/guard_flights.h"
#include "optimisations/intelligent/moves_left.h"
#include "optimisations/intelligent/stalemate/filter.h"
#include "optimisations/intelligent/proof.h"
#include "optimisations/intelligent/duplicate_avoider.h"
#include "optimisations/intelligent/limit_nr_solutions_per_target.h"
#include "optimisations/intelligent/place_black_piece.h"
#include "optimisations/intelligent/mate/filter.h"
#include "optimisations/intelligent/mate/generate_checking_moves.h"
#include "optimisations/intelligent/mate/generate_doublechecking_moves.h"
#include "platform/maxtime.h"
#include "debugging/trace.h"
#include "pyproc.h"
#include "pieces/pieces.h"
#include "options/options.h"

#include <assert.h>
#include <stdio.h>

typedef unsigned int index_type;

#define piece_usageENUMERATORS \
    ENUMERATOR(piece_is_unused), \
    ENUMERATOR(piece_pins), \
    ENUMERATOR(piece_is_fixed_to_diagram_square), \
    ENUMERATOR(piece_intercepts), \
    ENUMERATOR(piece_intercepts_check_from_guard), \
    ENUMERATOR(piece_blocks), \
    ENUMERATOR(piece_guards), \
    ENUMERATOR(piece_gives_check), \
    ENUMERATOR(piece_is_missing), \
    ENUMERATOR(piece_is_captured), \
    ENUMERATOR(piece_is_king)

#define ENUMERATORS piece_usageENUMERATORS
#define ENUMERATION_TYPENAME piece_usage
#define ENUMERATION_MAKESTRINGS
#include "utilities/enumeration.h"
#undef piece_usageENUMERATORS


goal_type goal_to_be_reached;

unsigned int MaxPiece[nr_sides];

PIECE white[nr_squares_on_board];
PIECE black[nr_squares_on_board];
unsigned int moves_to_white_prom[nr_squares_on_board];

PIECE target_position[MaxPieceId+1];

slice_index current_start_slice = no_slice;

boolean solutions_found;

boolean testcastling;

unsigned int MovesRequired[nr_sides][maxply+1];
unsigned int CapturesLeft[maxply+1];

unsigned int PieceId2index[MaxPieceId+1];

unsigned int nr_reasons_for_staying_empty[maxsquare+4];

static stip_length_type nr_of_moves;

typedef struct
{
  Flags       spec[nr_squares_on_board];
  PieNam      e[nr_squares_on_board];
  square      rn_sic, rb_sic;
} stored_position_type;

static stored_position_type initial_position;

static void StorePosition(stored_position_type *store)
{
  store->rn_sic = king_square[Black];
  store->rb_sic = king_square[White];

  {
    unsigned int i;
    for (i = 0; i<nr_squares_on_board; i++)
    {
      store->e[i] = get_walk_of_piece_on_square(boardnum[i]);
      store->spec[i] = spec[boardnum[i]];
    }
  }
}

static void ResetPosition(stored_position_type const *store)
{
  {
    PieNam p;
    for (p = King; p<PieceCount; ++p)
    {
      number_of_pieces[White][p] = 0;
      number_of_pieces[Black][p] = 0;
    }
  }

  king_square[Black] = store->rn_sic;
  king_square[White] = store->rb_sic;

  {
    unsigned int i;
    for (i = 0; i<nr_squares_on_board; i++)
      switch (store->e[i])
      {
        case Empty:
          empty_square(boardnum[i]);
          break;

        case Invalid:
          block_square(boardnum[i]);
          break;

        default:
        {
          Side const side = TSTFLAG(store->spec[i],White) ? White : Black;
          ++number_of_pieces[side][store->e[i]];
          occupy_square(boardnum[i],store->e[i],store->spec[i]);
        }
      }
  }
}

void remember_to_keep_rider_line_open(square from, square to,
                                      int dir, int delta)
{
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParam("%d",dir);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  for (s = from+dir; s!=to; s+=dir)
  {
    assert(is_square_empty(s));
    nr_reasons_for_staying_empty[s] += delta;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
/* Detrmine whether some line is empty
 * @param start start of line
 * @param end end of line
 * @param dir direction from start to end
 * @return true iff the line is empty
 */
boolean is_line_empty(square start, square end, int dir)
{
  boolean result = true;
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(start);
  TraceSquare(end);
  TraceFunctionParam("%d",dir);
  TraceFunctionParamListEnd();

  for (s = start+dir; s!=end; s += dir)
    if (!is_square_empty(s))
    {
      result = false;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

boolean black_pawn_attacks_king(square from)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  assert(!TSTFLAG(sq_spec[from],BlPromSq));
  assert(!TSTFLAG(sq_spec[from],BlBaseSq));

  if (king_square[White]==initsquare)
    result = false;
  else
  {
    int const diff = king_square[White]-from;
    result = diff==dir_down+dir_right || diff==dir_down+dir_left;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/*#define DETAILS*/
#if defined(DETAILS)
static void trace_target_position(PIECE const position[MaxPieceId+1],
                                  unsigned int nr_required_captures)
{
  unsigned int moves_per_side[nr_sides] = { 0, 0 };
  square const *bnp;

  for (bnp = boardnum; *bnp!=initsquare; bnp++)
    if (!is_square_empty(*bnp))
    {
      Flags const sp = spec[*bnp];
      PieceIdType const id = GetPieceId(sp);
      PIECE const * const target = &position[id];
      if (target->square!=vide)
      {
        unsigned int const time = intelligent_count_nr_of_moves_from_to_no_check(e[*bnp],
                                                                     *bnp,
                                                                     target->type,
                                                                     target->square);
        moves_per_side[TSTFLAG(spec[*bnp],White) ? White : Black] += time;
        TracePiece(e[*bnp]);
        TraceSquare(*bnp);
        TracePiece(target->type);
        TraceSquare(target->square);
        TraceEnumerator(piece_usage,target->usage,"");
        TraceValue("%u\n",time);
      }
    }

  TraceValue("%u",nr_required_captures);
  TraceValue("%u",moves_per_side[Black]);
  TraceValue("%u\n",moves_per_side[White]);
}

static piece_usage find_piece_usage(PieceIdType id)
{
  piece_usage result = piece_is_unused;

  unsigned int i;
  for (i = 0; i<MaxPiece[White]; ++i)
    if (id==GetPieceId(white[i].flags))
    {
      result = white[i].usage;
      break;
    }
  for (i = 0; i<MaxPiece[Black]; ++i)
    if (id==GetPieceId(black[i].flags))
    {
      result = black[i].usage;
      break;
    }

  assert(result!=piece_is_unused);

  return result;
}
#endif

void solve_target_position(void)
{
#if (defined(_WIN32) && !defined(_MSC_VER))|| defined(__CYGWIN__)
  /* Windows executables generated with gcc (both cross-compiling from Linux and
   * using cygwin) appear to have an optimiser error which may cause the value
   * of the expression save_king_square[Black] (but not the underlying memory!!)
   * to be modified from here to the end of the function (where
   * the value of king_square[Black] is to be restored).
   *
   * This error doesn't manifest itself if save_king_square is volatile.
   *
   * Cf. bug report 3489394, which gives this problem as an example:
   * AnfangProblem
   * Steine Weiss  kh1 lh3 bh2
   * Steine Schwarz  ka2 dh8 ta1h6 la5f1 sg7a8 bc2e2f2b3c3a4h4h5
   * Forderung H=8
   * Option Intelligent
   * EndeProblem
   */
  volatile
#endif
  square const save_king_square[nr_sides] = { king_square[White],
                                              king_square[Black] };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    PieceIdType id;
    for (id = 0; id<=MaxPieceId; ++id)
      target_position[id].diagram_square = initsquare;
  }

  {
    square const *bnp;
    for (bnp = boardnum; *bnp!=initsquare; bnp++)
    {
      PieNam const type = get_walk_of_piece_on_square(*bnp);
      if (type!=Empty && type!=Invalid)
      {
        Flags const flags = spec[*bnp];
        PieceIdType const id = GetPieceId(flags);
        target_position[id].type = type;
        target_position[id].flags = flags;
        target_position[id].diagram_square = *bnp;
#if defined(DETAILS)
        target_position[id].usage = find_piece_usage(id);
#endif
      }
    }
  }

  /* solve the problem */
  ResetPosition(&initial_position);

#if defined(DETAILS)
  TraceText("target position:\n");
  trace_target_position(target_position,CapturesLeft[1]);
#endif

  reset_nr_solutions_per_target_position();

  if (solve(slices[current_start_slice].next1,nr_of_moves)<=nr_of_moves)
    solutions_found = true;

  /* reset the old mating position */
  {
    square const *bnp;
    for (bnp = boardnum; *bnp!=initsquare; bnp++)
      empty_square(*bnp);
  }

  {
    PieceIdType id;
    for (id = 0; id<=MaxPieceId; ++id)
      if (target_position[id].diagram_square != initsquare)
        occupy_square(target_position[id].diagram_square,
                      target_position[id].type,
                      target_position[id].flags);
  }

  {
    PieNam p;

    number_of_pieces[White][King] = 1;
    number_of_pieces[Black][King] = 1;

    for (p = King+1; p<=Bishop; ++p)
    {
      number_of_pieces[White][p] = 2;
      number_of_pieces[Black][p] = 2;
    }
  }

  king_square[White] = save_king_square[White];
  king_square[Black] = save_king_square[Black];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void GenerateBlackKing(void)
{
  Flags const king_flags = black[index_of_king].flags;
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(black[index_of_king].type==King);

  intelligent_init_reservations(MovesLeft[White],MovesLeft[Black],
                                MaxPiece[White],MaxPiece[Black]-1);

  for (bnp = boardnum; *bnp!=initsquare && !hasMaxtimeElapsed(); ++bnp)
    if (is_square_empty(*bnp) /* *bnp isn't a hole*/
        && intelligent_reserve_black_king_moves_from_to(black[index_of_king].diagram_square,
                                                        *bnp))
    {

      {
        square s;
        for (s = 0; s!=maxsquare+4; ++s)
        {
          if (nr_reasons_for_staying_empty[s]>0)
            WriteSquare(s);
          assert(nr_reasons_for_staying_empty[s]==0);
        }
      }

      occupy_square(*bnp,King,king_flags);
      king_square[Black] = *bnp;
      black[index_of_king].usage = piece_is_king;

      init_guard_dirs(*bnp);

      if (goal_to_be_reached==goal_mate)
      {
        intelligent_mate_generate_checking_moves();
        intelligent_mate_generate_doublechecking_moves();
      }
      else
        intelligent_guard_flights();

      empty_square(*bnp);

      intelligent_unreserve();
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void IntelligentRegulargoal_types(stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (king_square[Black]!=initsquare)
  {
    nr_of_moves = n;

    testcastling =
        TSTCASTLINGFLAGMASK(White,q_castling&castling_flags_no_castling)==q_castling
        || TSTCASTLINGFLAGMASK(White,k_castling&castling_flags_no_castling)==k_castling
        || TSTCASTLINGFLAGMASK(Black,q_castling&castling_flags_no_castling)==q_castling
        || TSTCASTLINGFLAGMASK(Black,k_castling&castling_flags_no_castling)==k_castling;

    assert(where_to_start_placing_black_pieces==boardnum);

    MaxPiece[Black] = 0;
    MaxPiece[White] = 0;

    black[index_of_king].type= get_walk_of_piece_on_square(king_square[Black]);
    black[index_of_king].flags= spec[king_square[Black]];
    black[index_of_king].diagram_square= king_square[Black];
    PieceId2index[GetPieceId(spec[king_square[Black]])] = index_of_king;
    ++MaxPiece[Black];

    if (king_square[White]==initsquare)
      white[index_of_king].usage = piece_is_missing;
    else
    {
      white[index_of_king].usage = piece_is_unused;
      white[index_of_king].type = get_walk_of_piece_on_square(king_square[White]);
      white[index_of_king].flags = spec[king_square[White]];
      white[index_of_king].diagram_square = king_square[White];
      PieceId2index[GetPieceId(spec[king_square[White]])] = index_of_king;
      assert(white[index_of_king].type==King);
    }

    ++MaxPiece[White];

    {
      square const *bnp;

      nextply(White);

      for (bnp = boardnum; *bnp!=initsquare; ++bnp)
        if (king_square[White]!=*bnp && TSTFLAG(spec[*bnp],White))
        {
          white[MaxPiece[White]].type = get_walk_of_piece_on_square(*bnp);
          white[MaxPiece[White]].flags = spec[*bnp];
          white[MaxPiece[White]].diagram_square = *bnp;
          white[MaxPiece[White]].usage = piece_is_unused;
          if (get_walk_of_piece_on_square(*bnp)==Pawn)
            moves_to_white_prom[MaxPiece[White]] = intelligent_count_moves_to_white_promotion(*bnp);
          PieceId2index[GetPieceId(spec[*bnp])] = MaxPiece[White];
          ++MaxPiece[White];
        }

      for (bnp = boardnum; *bnp!=initsquare; ++bnp)
        if (king_square[Black]!=*bnp && TSTFLAG(spec[*bnp],Black))
        {
          black[MaxPiece[Black]].type = get_walk_of_piece_on_square(*bnp);
          black[MaxPiece[Black]].flags = spec[*bnp];
          black[MaxPiece[Black]].diagram_square = *bnp;
          black[MaxPiece[Black]].usage = piece_is_unused;
          PieceId2index[GetPieceId(spec[*bnp])] = MaxPiece[Black];
          ++MaxPiece[Black];
        }

      finply();
    }

    StorePosition(&initial_position);

    /* clear board */
    {
      square const *bnp;
      for (bnp= boardnum; *bnp!=initsquare; ++bnp)
        if (get_walk_of_piece_on_square(*bnp)!=Invalid)
          empty_square(*bnp);
    }

    {
      PieNam p;
      for (p = King; p<=Bishop; ++p)
      {
        number_of_pieces[White][p] = 2;
        number_of_pieces[Black][p] = 2;
      }
    }

    /* generate final positions */
    GenerateBlackKing();

    ResetPosition(&initial_position);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void goal_to_be_reached_goal(slice_index si,
                                    stip_structure_traversal *st)
{
  goal_type * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(*goal==no_goal);
  *goal = slices[si].u.goal_handler.goal.type;

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the variable holding the goal to be reached
 */
static goal_type determine_goal_to_be_reached(slice_index si)
{
  stip_structure_traversal st;
  goal_type result = no_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&result);
  stip_structure_traversal_override_single(&st,
                                           STGoalReachedTester,
                                           &goal_to_be_reached_goal);
  stip_structure_traversal_override_single(&st,
                                           STTemporaryHackFork,
                                           &stip_traverse_structure_children_pipe);
  stip_traverse_structure(si,&st);

  TraceValue("%u",goal_to_be_reached);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise a STGoalReachableGuardFilter slice
 * @return identifier of allocated slice
 */
static slice_index alloc_goalreachable_guard_filter(goal_type goal)
{
  slice_index result;
  slice_type type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  switch (goal)
  {
    case goal_mate:
      type = STGoalReachableGuardFilterMate;
      break;

    case goal_stale:
      type = STGoalReachableGuardFilterStalemate;
      break;

    case goal_proofgame:
    case goal_atob:
      type = proof_make_goal_reachable_type();
      break;

    default:
      assert(0);
      type = no_slice_type;
      break;
  }

  if (type!=no_slice_type)
    result = alloc_pipe(type);
  else
    result = no_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static
void goalreachable_guards_inserter_help_move(slice_index si,
                                             stip_structure_traversal *st)
{
  goal_type const * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_goalreachable_guard_filter(*goal);
    if (prototype!=no_slice)
      help_branch_insert_slices(si,&prototype,1);

    if (is_max_nr_solutions_per_target_position_limited())
    {
      slice_index const prototype = alloc_intelligent_limit_nr_solutions_per_target_position_slice();
      help_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static
void
goalreachable_guards_duplicate_avoider_inserter(slice_index si,
                                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  if (slices[si].u.goal_handler.goal.type==goal_mate
      || slices[si].u.goal_handler.goal.type==goal_stale)
  {
    slice_index const prototype = alloc_intelligent_duplicate_avoider_slice();
    help_branch_insert_slices(si,&prototype,1);

    if (is_max_nr_solutions_per_target_position_limited())
    {
      slice_index const prototype = alloc_intelligent_nr_solutions_per_target_position_counter_slice();
      help_branch_insert_slices(si,&prototype,1);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor goalreachable_guards_inserters[] =
{
  { STReadyForHelpMove,  &goalreachable_guards_inserter_help_move         },
  { STGoalReachedTester, &goalreachable_guards_duplicate_avoider_inserter },
  { STTemporaryHackFork, &stip_traverse_structure_children_pipe                    }
};

enum
{
  nr_goalreachable_guards_inserters = (sizeof goalreachable_guards_inserters
                                       / sizeof goalreachable_guards_inserters[0])
};

/* Instrument stipulation with STgoal_typereachableGuard slices
 * @param si identifies slice where to start
 */
static void stip_insert_goalreachable_guards(slice_index si, goal_type goal)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  assert(goal!=no_goal);

  stip_structure_traversal_init(&st,&goal);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_conditional_pipe,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    goalreachable_guards_inserters,
                                    nr_goalreachable_guards_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static slice_index find_goal_tester_fork(slice_index si)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const branch_goal_fork = branch_find_slice(STEndOfBranchGoalImmobile,
                                                           si,
                                                           stip_traversal_context_intro);
    if (branch_goal_fork==no_slice)
    {
      slice_index const branch_goal = branch_find_slice(STEndOfBranch,
                                                        si,
                                                        stip_traversal_context_intro);
      assert(branch_goal!=no_slice);
      result = find_goal_tester_fork(slices[branch_goal].next2);
    }
    else
      result = branch_find_slice(STGoalReachedTester,
                                 slices[branch_goal_fork].next2,
                                 stip_traversal_context_intro);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void intelligent_filter_inserter(slice_index si,
                                        stip_structure_traversal *st)
{
  goal_type const * const goal = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (*goal)
  {
    case goal_proofgame:
    case goal_atob:
    {
      slice_index const prototype = alloc_intelligent_proof();
      branch_insert_slices(si,&prototype,1);
      break;
    }

    case goal_mate:
    {
      slice_index const prototype = alloc_intelligent_mate_filter(find_goal_tester_fork(si));
      branch_insert_slices(si,&prototype,1);
      break;
    }

    case goal_stale:
    {
      slice_index const prototype = alloc_intelligent_stalemate_filter();
      branch_insert_slices(si,&prototype,1);
      break;
    }

    default:
      assert(0);
      break;
  }

  {
    slice_index const prototype = alloc_intelligent_moves_left_initialiser();
    branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor intelligent_filters_inserters[] =
{
  { STHelpAdapter,       &intelligent_filter_inserter           },
  { STTemporaryHackFork, &stip_traverse_structure_children_pipe }
};

enum
{
  nr_intelligent_filters_inserters = (sizeof intelligent_filters_inserters
                                     / sizeof intelligent_filters_inserters[0])
};

/* Instrument stipulation with STgoal_typereachableGuard slices
 * @param si identifies slice where to start
 */
static void stip_insert_intelligent_filters(slice_index si, goal_type goal)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",goal);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&goal);
  stip_structure_traversal_override(&st,
                                    intelligent_filters_inserters,
                                    nr_intelligent_filters_inserters);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* How well does the stipulation support intelligent mode?
 */
typedef enum
{
  intelligent_not_supported,
  intelligent_not_active_by_default,
  intelligent_active_by_default
} support_for_intelligent_mode;

typedef struct
{
  support_for_intelligent_mode support;
  goal_type goal;
} detector_state_type;

static
void intelligent_mode_support_detector_or(slice_index si,
                                          stip_structure_traversal *st)
{
  detector_state_type * const state = st->param;
  support_for_intelligent_mode support1;
  support_for_intelligent_mode support2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->support!=intelligent_not_supported)
  {
    stip_traverse_structure_binary_operand1(si,st);
    support1 = state->support;

    stip_traverse_structure_binary_operand2(si,st);
    support2 = state->support;

    /* enumerators are ordered so that the weakest support has the
     * lowest enumerator etc. */
    assert(intelligent_not_supported<intelligent_not_active_by_default);
    assert(intelligent_not_active_by_default<intelligent_active_by_default);

    state->support = support1<support2 ? support1 : support2;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intelligent_mode_support_none(slice_index si,
                                          stip_structure_traversal *st)
{
  detector_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->support = intelligent_not_supported;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intelligent_mode_support_goal_tester(slice_index si,
                                                 stip_structure_traversal *st)
{
  detector_state_type * const state = st->param;
  goal_type const goal = slices[si].u.goal_handler.goal.type;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->goal==no_goal)
  {
    switch (goal)
    {
      case goal_mate:
      case goal_stale:
        if (state->support!=intelligent_not_supported)
          state->support = intelligent_not_active_by_default;
        break;

      case goal_proofgame:
      case goal_atob:
        if (state->support!=intelligent_not_supported)
          state->support = intelligent_active_by_default;
        break;

      default:
        state->support = intelligent_not_supported;
        break;
    }

    state->goal = goal;
  }
  else if (state->goal!=goal)
    state->support = intelligent_not_supported;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor intelligent_mode_support_detectors[] =
{
  { STAnd,               &intelligent_mode_support_none         },
  { STOr,                &intelligent_mode_support_detector_or  },
  { STNot,               &intelligent_mode_support_none         },
  { STConstraintSolver,  &intelligent_mode_support_none         },
  { STConstraintTester,  &intelligent_mode_support_none         },
  { STReadyForDefense,   &intelligent_mode_support_none         },
  { STGoalReachedTester, &intelligent_mode_support_goal_tester  },
  { STTemporaryHackFork, &stip_traverse_structure_children_pipe }
};

enum
{
  nr_intelligent_mode_support_detectors
  = (sizeof intelligent_mode_support_detectors
     / sizeof intelligent_mode_support_detectors[0])
};

/* Determine whether the stipulation supports intelligent mode, and
 * how much so
 * @param si identifies slice where to start
 * @return degree of support for ingelligent mode by the stipulation
 */
static support_for_intelligent_mode stip_supports_intelligent(slice_index si)
{
  detector_state_type state = { intelligent_not_active_by_default, no_goal };
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override(&st,
                                    intelligent_mode_support_detectors,
                                    nr_intelligent_mode_support_detectors);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",state.support);
  TraceFunctionResultEnd();
  return state.support;
}

/* Initialize intelligent mode if the user or the stipulation asks for
 * it
 * @param si identifies slice where to start
 * @return false iff the user asks for intelligent mode, but the
 *         stipulation doesn't support it
 */
boolean init_intelligent_mode(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  goal_to_be_reached = no_goal;

  switch (stip_supports_intelligent(si))
  {
    case intelligent_not_supported:
      result = !OptFlag[intelligent];
      break;

    case intelligent_not_active_by_default:
      result = true;
      if (OptFlag[intelligent])
      {
        goal_to_be_reached = determine_goal_to_be_reached(si);
        stip_insert_intelligent_filters(si,goal_to_be_reached);
        stip_insert_goalreachable_guards(si,goal_to_be_reached);
      }
      break;

    case intelligent_active_by_default:
      result = true;
      goal_to_be_reached = determine_goal_to_be_reached(si);
      stip_insert_intelligent_filters(si,goal_to_be_reached);
      stip_insert_goalreachable_guards(si,goal_to_be_reached);
      break;

    default:
      assert(0);
      result = false;
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
