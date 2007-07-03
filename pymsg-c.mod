/*************************** MODIFICATIONS to pymsg.c *****************************
**
** made compilable with MPW on macintosh-computers.     14.02.91        SB
**
** made compilable with Microsoft C V6.0                  08.05.91      ElB
**
** when using any multitasking system under DOS, the 'PY-????.STR'
** files should be read with file sharing mode and locking file
** regions. To compile for this Systems define SHARING. 11.10.91        StH
**
** PrintTime(why) added due to more time info for users.  27.11.91      NG
**
** made functions 'ErrorMsg' and 'Message' ANSI conform, because
** the do not get the same number of parameters when they are called.
**                                                        27.03.92      StH
**
** Looking for *.msg-file in PATH not needed any more.	 3.05.1993	NG
**
** 1993/5/25  ElB  Moved all remarks on modifications of pymsg.c to
**                 pymsg-c.mod. Look there for remarks before this date.
**
** 1994/05/23 NG   LogLngArg added due to opyhash.c problems.
**
** 1996/01/01 NG   Moved all remarks on modifications of pymsg.c to
**                 pymsg-c.mod. Look there for remarks before this date.
**************************** INFORMATION END **************************/
