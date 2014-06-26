#if !defined(OUTPUT_PLAINTEXT_MESSAGE_H)
#define OUTPUT_PLAINTEXT_MESSAGE_H

#include "output/message.h"

#include <stdio.h>

/* Issue a message text
 * @param id identifies the message
 * @param ... additional parameters according the printf() like conversion
 *            specifiers in message id
 */
void output_latex_message(FILE *file, message_id_t id, ...);

#endif
