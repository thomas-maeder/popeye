/****************** MODIFICATIONS to pymsg.c **************************
**
** Date       Who  What
**
** 1999/05/25 NG   Moved all remarks on modifications of pymsg.c to
**		   pymsg-c.mod. Look there for remarks before 1998/01/01.
**
**************************** End of List ******************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DOS
#ifdef MSC
#	ifdef SHARING		/* Import defines for locking regions  StH */
#		include <sys/locking.h>
#	endif
#endif	/* MSC */
#endif	/* DOS */

#define PYMSG

#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "pytime.h"

typedef unsigned int UnInt;
typedef unsigned char UnChar;
typedef unsigned long UnLong;


#ifndef SEEK_SET
#	define SEEK_SET 0
#endif	/* not SEEK_SET */

static int StringCnt;

static char *AdditionalArg;
/* This is used to record an argument which is used
 * in a message-string. There are only message strings
 * that contain at most one format specifier.  Therefore
 * one pointer is sufficient.
 * Three small routines are provided to assign a value:
 */

void logStrArg(char *arg) {
    AdditionalArg= arg;
    return;
}

void logIntArg(int arg) {
    static char IntBuffer[10];
    sprintf(IntBuffer, "%d", arg);
    AdditionalArg= IntBuffer;
    return;
}

void logLngArg(long arg) {				/* V3.22  NG */
    static char LngBuffer[16];
    sprintf(LngBuffer, "%ld", arg);
    AdditionalArg= LngBuffer;
    return;
}

void logChrArg(char arg) {
    static char CharBuffer[2];
    CharBuffer[0]= arg;
    CharBuffer[1]= '\0';
    AdditionalArg= CharBuffer;
    return;
}

#ifdef MSG_IN_MEM
#include "pyallmsg.h"
static char **ActualMsgTab;

boolean InitMsgTab(Lang l, boolean Force) {
    int i;
    StringCnt= STRINGCNT;
    ActualMsgTab= MessageTabs[l];
    for (i=0; i<20; i++) {
	GetMsgString(i);
    }
    return True;
}

char *GetMsgString(int nr) {
    return ActualMsgTab[nr];
}
#else /* not MSG_IN_MEM */

static FILE *fstring;
static char *StringBuf=NULL;
static UnInt *MsgOffset= (UnInt *)0;
static int MaxLeng;

UnInt GetUnInt(char **s) {
    UnInt c;

#define GetByte(s) ((*(*(UnChar **)(s))++) & 0xff)

    c= GetByte(s);
    switch (c & 0xc0) {

      case 0x00:
	return c;

      case 0x40:
	return c;

      case 0x80:
	c= (c<<8);
	c+= GetByte(s);
	return c ^ 0x8000;

      case 0xC0:
	c= (c<<8) + GetByte(s);
	c= (c<<8) + GetByte(s);
	c= (c<<8) + GetByte(s);
	return c ^ 0xC0000000;
    }
}

char *GetMsgString(int nr) {
    int l;
    char *spt;
    boolean OutOfRange= False;

    if (nr < 0 || StringCnt <= nr) {
	OutOfRange= True;
	fseek(fstring, (UnLong)MsgOffset[53], SEEK_SET);
    }
    else {
	fseek(fstring, (UnLong)MsgOffset[nr], SEEK_SET);
    }

#ifdef DOS
#ifdef MSC
#ifdef SHARING		 /* Lock the file region, which should be read */
    locking(fstring,LK_RLCK,MaxLeng);
#endif	/* SHARING */
#endif	/* MSC */
#endif	/* DOS */

    fread(StringBuf, MaxLeng, 1, fstring);

#ifdef DOS
#ifdef MSC
#ifdef SHARING		 /* Unlock the file region */
    locking(fstring,LK_UNLCK,MaxLeng);
#endif
#endif	/* MSC */
#endif	/* DOS */

    spt= StringBuf;
    l= GetUnInt(&spt);
    spt[l]='\0';
    if (OutOfRange) {
	static char ErrStr[64];
	sprintf(ErrStr,spt,nr);
	return ErrStr;
    }
    return spt;
}

boolean InitMsgTab(Lang l, boolean Force) {
    char Head[64], *hpt, *path, *OurFile;
    UnInt StrStart, StrSize, OfsStart, OfsSize;
    char    *OfsBuf,*opt;
    int     s;

    OurFile= MkStrFileName(l);

    if ((fstring=fopen(OurFile,"r")) == NULL) {
	if (Force == True) {
	    fprintf(stderr,"No %s - sorry\n",OurFile);
	    exit(-2);
	}
	else {
	    return False;
	}
    }

    fread(Head, sizeof(Head), 1, fstring);

    hpt= Head;
    StringCnt= GetUnInt(&hpt);
    MaxLeng  = GetUnInt(&hpt);
    StrStart = GetUnInt(&hpt);
    StrSize  = GetUnInt(&hpt);
    OfsStart = GetUnInt(&hpt);
    OfsSize  = GetUnInt(&hpt);

    MaxLeng+=20;		     /* V2.52  ElB */

    if (MsgOffset)
	    free(MsgOffset);
    MsgOffset= (UnInt *)malloc(sizeof(UnInt *)*StringCnt);
    if (StringBuf)
	    free(StringBuf);
    StringBuf= (char *)malloc(MaxLeng);
    OfsBuf= (char *)malloc(OfsSize+2);
    opt= OfsBuf;

    fseek(fstring, (UnLong)OfsStart, SEEK_SET);
    fread(OfsBuf, OfsSize, 1, fstring);

    for (s=0; s<StringCnt; s++)
	    MsgOffset[s]= GetUnInt(&opt) + sizeof(Head);

    free(OfsBuf);
    return True;
}
#endif	/* MSG_IN_MEM */



void Message(int nr) {
    DBG((stderr, "Mesage(%d) = %s\n", nr, GetMsgString(nr)));
    if (0 <= nr && nr < StringCnt)
	sprintf(GlobalStr, GetMsgString(nr), AdditionalArg);
    else
	sprintf(GlobalStr, GetMsgString(53),nr);
    AdditionalArg= (char *)0;
    StdString(GlobalStr);
}

void ErrorMsg(int nr) {
    DBG((stderr, "ErrorMsg(%d) = %s\n", nr, GetMsgString(nr)));
    if (0 <= nr && nr < StringCnt)
	sprintf(GlobalStr, GetMsgString(nr), AdditionalArg);
    else
	sprintf(GlobalStr, GetMsgString(53),nr);
    AdditionalArg= (char *)0;
    ErrString(GlobalStr);
}

void FtlMsg(int nr) {
    char *SaveArg= 0;
    if (AdditionalArg)
	SaveArg= AdditionalArg;
    ErrorMsg(ErrFatal);
    ErrorMsg(NewLine);
    if (SaveArg)
	AdditionalArg= SaveArg;
    ErrorMsg(nr);
    ErrorMsg(NewLine);
    exit(nr);
}

boolean VerifieMsg(int nr) {
    ErrorMsg(nr);
    ErrorMsg(NewLine);
    ErrorMsg(ProblemIgnored);
    ErrorMsg(NewLine);
    return false;
}

char *MakeTimeString(void) {
    long	Seconds;
    int		Hours, Minutes;
    static char	TmString[32];

    Seconds= StopTimer();
    Hours= Seconds/3600;
    Minutes= (Seconds%3600)/60;
    Seconds= (Seconds%60);
    if (Hours>0) {
	sprintf(TmString,
	  "%d:%02d:%02ld h:m:s", Hours, Minutes, Seconds);
    }
    else if (Minutes>0) {
#if defined(UNIX) || defined(WIN32)
	sprintf(TmString,"%d:%02ld.%03d m:s", Minutes, Seconds, MilliSec());
#else
	sprintf(TmString,"%d:%02ld m:s", Minutes, Seconds);
#endif /*UNIX || WIN32*/
    }
    else {
#if defined(UNIX) || defined(WIN32)
	sprintf(TmString,"%ld.%03d s", Seconds, MilliSec());
#else
	sprintf(TmString,"%ld s", Seconds);
#endif /*UNIX || WIN32*/
    }
    return TmString;
}

void PrintTime(int why) {
    sprintf(GlobalStr,"%s %s", GetMsgString(why), MakeTimeString());
    StdString(GlobalStr);
}
