/*************************** MODIFICATIONS to pytime.c *****************************
**
** Ergaenzungen fuer ATARI ST - Zeitmessung.            06.01.1991      StHoe
**
** made compilable with MPW on macintosh-computers.     14.02.91        SB
**
** made compilable with Microsoft C V6.0                08.05.91        ElB
**
** made compilable with VAX-C on VMS-machines		12. 10. 1992   	TLi
**
** made compilable with C V2.0A on BS2000-machines       17.05.1993     NG, RB
**
** 1993/05/25  ElB  Moved all remarks on modifications of pytime.c to
**                  pytime-c.mod. See there for remarks before this date.
**
** 1995/10/24  ElB  Signal handling for UNIX systems implemented.
**
** 1996/01/01 NG   Moved all remarks on modifications of pytime.c to
**                 pytime-c.mod. Look there for remarks before this date.
**
** 1996/05/06  NG   Time handling for WIN32 with MilliSec() implemented.
**
** 1996/11/26  ElB  Introduced long ClockTick, which is initialized via
**                  sysinfo(2) call.
**
** 1996/12/10  TBa  Changes for WIN16 Popeye
**
** 1999/05/25 NG   Moved all remarks on modifications of pytime.c to
**                 pytime-c.mod. Look there for remarks before 1998/01/01.
**
** 2000/03/14 NG   Due to problems with 386DX processors without Coprocessor
**                 all floating point code eliminated.
**
**************************** INFORMATION END **************************/
