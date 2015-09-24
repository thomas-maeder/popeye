#if !defined(INPUT_PLAINTEXT_GOAL_H)
#define INPUT_PLAINTEXT_GOAL_H

#include "stipulation/stipulation.h"

char *ParseGoal(char *tok, slice_index start, slice_index proxy);

char const *get_goal_symbol(goal_type type);

#endif
