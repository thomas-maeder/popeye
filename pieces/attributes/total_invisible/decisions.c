#include "pieces/attributes/total_invisible/decisions.h"
#include "debugging/assert.h"

decision_level_type curr_decision_level = 2;
decision_level_type max_decision_level = decision_level_latest;

char decision_level_dir[decision_level_dir_capacity];

#if defined(REPORT_DECISIONS)

unsigned long report_decision_counter;
unsigned long prev_report_decision_counter;

#endif
