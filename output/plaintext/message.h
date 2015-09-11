#if !defined(OUTPUT_PLAINTEXT_MESSAGE_H)
#define OUTPUT_PLAINTEXT_MESSAGE_H

#include "output/message.h"
#include "stipulation/stipulation.h"

#include <stdio.h>

/* Suppress output that is supposed to be different in two runs with the same
 * input, e.g. timing information
 */
void output_plaintext_suppress_variable(void);

/* Report a verification error that causes the current problem to be ignored
 * @param id identiifes the diagnostic message
 */
void output_plaintext_verifie_message(message_id_t id);

/* Report an error
 * @param id identifies the diagnostic message
 * @param ... additional parameters according the printf() like conversion
 *            specifiers in message id
 */
void output_plaintext_error_message(message_id_t id, ...);

/* Issue a message text
 * @param id identifies the message
 * @param ... additional parameters according the printf() like conversion
 *            specifiers in message id
 */
void output_plaintext_message(message_id_t id, ...);

/* Issue a fatal message
 * @param id identifies the message
 * @note terminates the program with exit status code id
 */
void output_plaintext_fatal_message(message_id_t id);

/* Issue an input error message
 * @param id identifies the diagnostic message
 * @param val additional parameter according to the printf() conversion
 *            specifier in message id
 */
void output_plaintext_input_error_message(message_id_t n, int val);

/* Issue a solving time indication
 * @param header text printed before the time
 * @param trail text printed after the time
 * @note nothing is issued if we are in regression testing mode
 */
void output_plaintext_print_time(char const *header, char const *trail);

/* Print information about the program version, platform, maximum memory ...
 */
void output_plaintext_print_version_info(FILE *file);

void output_plaintext_version_info_printer_solve(slice_index si);

slice_index output_plaintext_alloc_version_info_printer(FILE *file);

/* Issue a message that the program is being aborted
 * @param signal identifies the aborting signal
 * @note can be used inside a signal handler; does *not* terminate the program
 */
void output_plaintext_report_aborted(int signal);

#endif
