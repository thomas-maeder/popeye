#if !defined(PYTIME_H)
#define PYTIME_H

/* Start the timer for measuring the solving time for one problem.
 */
void StartTimer(void);

/* Stop the timer for measuring the solving time for one problem.
 * @param seconds address of object that is assigned the number of
 *                seconds since last call of StartTimer()
 * @param msec address of object that is assigned the number of
 *              milliseconds (0-999, to be added to *seconds) since
 *              last call of StartTimer()
 * @note *msec is assigned 0 if the platform doesn't support timing at
 *       millisecond accuracy
 */
void StopTimer(unsigned long *seconds, unsigned long *msec);

#endif /*_PYTIME_INCLUDED*/
