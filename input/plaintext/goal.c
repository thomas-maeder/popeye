#include "input/plaintext/goal.h"
#include "input/plaintext/geometry/square.h"
#include "input/plaintext/token.h"
#include "output/output.h"
#include "output/plaintext/position.h"
#include "output/plaintext/message.h"
#include "conditions/imitator.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/goals/target/reached_tester.h"
#include "stipulation/goals/immobile/reached_tester.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/goals/mate/reached_tester.h"
#include "stipulation/goals/stalemate/reached_tester.h"
#include "stipulation/goals/doublestalemate/reached_tester.h"
#include "stipulation/goals/check/reached_tester.h"
#include "stipulation/goals/capture/reached_tester.h"
#include "stipulation/goals/steingewinn/reached_tester.h"
#include "stipulation/goals/enpassant/reached_tester.h"
#include "stipulation/goals/doublemate/reached_tester.h"
#include "stipulation/goals/countermate/reached_tester.h"
#include "stipulation/goals/castling/reached_tester.h"
#include "stipulation/goals/autostalemate/reached_tester.h"
#include "stipulation/goals/circuit/reached_tester.h"
#include "stipulation/goals/exchange/reached_tester.h"
#include "stipulation/goals/circuit_by_rebirth/reached_tester.h"
#include "stipulation/goals/exchange_by_rebirth/reached_tester.h"
#include "stipulation/goals/any/reached_tester.h"
#include "stipulation/goals/proofgame/reached_tester.h"
#include "stipulation/goals/atob/reached_tester.h"
#include "stipulation/goals/chess81/reached_tester.h"
#include "stipulation/goals/kiss/reached_tester.h"
#include "position/pieceid.h"
#include "debugging/trace.h"
#include "solving/proofgames.h"

#include "debugging/assert.h"
#include <string.h>

/* map input strings to goals */
typedef struct
{
    char const *inputText;
    goal_type goal;
 } goalInputConfig_t;

 /* make sure that input strings that are substrings of other strings
  * appear *after* them! */
 static goalInputConfig_t const goalInputConfig[nr_goals] =
 {
   {   "##!",  goal_countermate         }
   , { "##",   goal_doublemate          }
   , { "#=",   goal_mate_or_stale       }
   , { "#",    goal_mate                }
   , { "==",   goal_dblstale            }
   , { "!=",   goal_autostale           }
   , { "=",    goal_stale               }
   , { "z",    goal_target              }
   , { "+",    goal_check               }
   , { "x",    goal_capture             }
   , { "%",    goal_steingewinn         }
   , { "ep",   goal_ep                  }
   , { "ctr",  goal_circuit_by_rebirth  }
   , { "ct",   goal_circuit             }
   , { "<>r",  goal_exchange_by_rebirth }
   , { "<>",   goal_exchange            }
   , { "00",   goal_castling            }
   , { "~",    goal_any                 }
   , { "dia",  goal_proofgame           }
   , { "a=>b", goal_atob                }
   , { "c81",  goal_chess81             }
   , { "k",    goal_kiss                }
 };

 static goalInputConfig_t const *detectGoalType(char *tok)
 {
   goalInputConfig_t const *gic;

   TraceFunctionEntry(__func__);
   TraceFunctionParam("%s",tok);
   TraceFunctionParamListEnd();

   for (gic = goalInputConfig; gic!=goalInputConfig+nr_goals; ++gic)
     if (gic->inputText!=0 && token_starts_with(gic->inputText,tok))
       break;

   TraceFunctionExit(__func__);
   TraceFunctionResult("%u",(gic-goalInputConfig)<nr_goals);
   TraceFunctionResultEnd();
   return gic;
 }

char *ParseGoal(char *tok, slice_index start, slice_index proxy)
{
  goalInputConfig_t const *gic;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParam("%u",start);
  TraceFunctionParam("%u",proxy);
  TraceFunctionParamListEnd();

  gic = detectGoalType(tok);
  if (gic-goalInputConfig==nr_goals)
    tok = 0;
  else
  {
    Goal goal = { gic->goal, initsquare };
    tok += strlen(gic->inputText);
    TraceValue("%s",gic->inputText);
    TraceValue("%s\n",tok);

    switch (gic->goal)
    {
      case goal_target:
      {
        tok = ParseSquare(tok,&goal.target);

        if (goal.target==initsquare)
        {
          output_plaintext_input_error_message(MissngSquareList, 0);
          tok = 0;
        }
        else
          pipe_link(proxy,alloc_goal_target_reached_tester_system(goal.target));
        break;
      }

      case goal_mate_or_stale:
      {
        Goal const goal = { goal_mate_or_stale, initsquare };
        slice_index const immobile_tester = alloc_goal_immobile_reached_tester_system();
        pipe_link(proxy,alloc_goal_reached_tester_slice(goal,immobile_tester));
        break;
      }

      case goal_mate:
      {
        Goal const goal = { goal_mate, initsquare };
        slice_index const mate_tester = alloc_goal_mate_reached_tester_system();
        pipe_link(proxy,alloc_goal_reached_tester_slice(goal,mate_tester));
        break;
      }

      case goal_stale:
        pipe_link(proxy,alloc_goal_stalemate_reached_tester_system());
        break;

      case goal_dblstale:
        pipe_link(proxy,alloc_goal_doublestalemate_reached_tester_system());
        break;

      case goal_check:
        pipe_link(proxy,alloc_goal_check_reached_tester_system());
        break;

      case goal_capture:
        pipe_link(proxy,alloc_goal_capture_reached_tester_system());
        break;

      case goal_steingewinn:
        pipe_link(proxy,alloc_goal_steingewinn_reached_tester_system());
        break;

      case goal_ep:
        pipe_link(proxy,alloc_goal_enpassant_reached_tester_system());
        break;

      case goal_doublemate:
        pipe_link(proxy,alloc_doublemate_mate_reached_tester_system());
        break;

      case goal_countermate:
        pipe_link(proxy,alloc_goal_countermate_reached_tester_system());
        break;

      case goal_castling:
        pipe_link(proxy,alloc_goal_castling_reached_tester_system());
        break;

      case goal_autostale:
        pipe_link(proxy,alloc_goal_autostalemate_reached_tester_system());
        break;

      case goal_circuit:
        pipe_link(proxy,alloc_goal_circuit_reached_tester_system());
        stipulation_remember_pieceids_required();
        break;

      case goal_exchange:
        pipe_link(proxy,alloc_goal_exchange_reached_tester_system());
        stipulation_remember_pieceids_required();
        break;

      case goal_circuit_by_rebirth:
        pipe_link(proxy,alloc_goal_circuit_by_rebirth_reached_tester_system());
        stipulation_remember_pieceids_required();
        break;

      case goal_exchange_by_rebirth:
        pipe_link(proxy,alloc_goal_exchange_by_rebirth_reached_tester_system());
        stipulation_remember_pieceids_required();
        break;

      case goal_any:
        pipe_link(proxy,alloc_goal_any_reached_tester_system());
        break;

      case goal_proofgame:
      {
        slice_index const prototype = alloc_pipe(STProofSolverBuilder);
        slice_insertion_insert(start,&prototype,1);

        if (output_plaintext_are_there_position_writers(start))
        {
          output_plaintext_remove_position_writers(start);
          output_plaintext_build_proof_position_writers(start);
        }


        pipe_link(proxy,alloc_goal_proofgame_reached_tester_system());
        break;
      }

      case goal_atob:
      {
        slice_index const prototype = alloc_pipe(STAToBSolverBuilder);
        slice_insertion_insert(start,&prototype,1);

        if (output_plaintext_are_there_position_writers(start))
        {
          output_plaintext_remove_position_writers(start);
          output_plaintext_build_proof_position_writers(start);
          output_plaintext_build_atob_start_position_writers(start);
        }

        pipe_link(proxy,alloc_goal_atob_reached_tester_system());

        ProofSaveStartPosition();

        {
          /* used to call InitBoard(), which does much more than the following: */
          int i;
          for (i = 0; i<nr_squares_on_board; i++)
            empty_square(boardnum[i]);
          for (i = 0; i<maxinum; i++)
            being_solved.isquare[i] = initsquare;
        }

        break;
      }

      case goal_chess81:
        pipe_link(proxy,alloc_goal_chess81_reached_tester_system());
        break;

      case goal_kiss:
        {
          tok = ParseSquare(tok,&goal.target);

          if (goal.target==initsquare)
          {
            output_plaintext_input_error_message(MissngSquareList, 0);
            tok = 0;
          }
          else
            pipe_link(proxy,alloc_goal_kiss_reached_tester_system(goal.target));
          break;
        }
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%s",tok);
  TraceFunctionResultEnd();
  return tok;
}
