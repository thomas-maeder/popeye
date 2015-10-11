#if !defined(PRIORITY_H)
#define PRIORITY_H

/* Functions for controlling the priority of Popeye's process(es) and
 * threads.
 */

/* Set the priority of Popeye's main process to a level that causes
 * Popeye to "play" nicely.
 */
void platform_set_nice_priority();

#endif
