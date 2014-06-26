#if !defined(OUTPUT_PLAINTEXT_MESSAGE_H)
#define OUTPUT_PLAINTEXT_MESSAGE_H

#include "output/message.h"

#include <stdio.h>

void output_plaintext_verifie_message(message_id_t id);
void output_plaintext_error_message(message_id_t id, ...);
void output_plaintext_message(message_id_t id, ...);
void output_plaintext_fatal_message(message_id_t id);
void output_plaintext_io_error_message(message_id_t n, int val);

void output_plaintext_print_time(char const *header, char const *trail);
void output_plaintext_report_aborted(int signal);

#endif
