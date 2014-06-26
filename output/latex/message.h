#if !defined(OUTPUT_PLAINTEXT_MESSAGE_H)
#define OUTPUT_PLAINTEXT_MESSAGE_H

#include "output/message.h"

#include <stdio.h>

void output_latex_message(FILE *file, message_id_t id, ...);

#endif
