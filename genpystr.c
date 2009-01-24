/* GENPYSTR.C	*/
/*************************** MODIFICATIONS *****************************
 **
 ** made compilable with MPW on macintosh-computers.	14.02.91 SB
 ** This file contains essantially two versions:
 **    - one which generates (at the moment) three *.str files
 **    - the other a header file to include all messages in memory
 **							04.02.93 ElB
 **
 **************************** INFORMATION END **************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pylang.h"

#if defined(__bsdi__)
#define strchr	index
#endif

#if defined(MSG_IN_MEM)

int main() {
  Language	lan;
  FILE	*hd;
  char	*hdFileName= "pyallmsg.h";
  int		StringCnt, MaxLeng;
  int		LastStringCnt= 0;

  if ((hd=fopen(hdFileName, "w")) == NULL) {
	fprintf(stderr,
            "Sorry, cannot open %s for writing\n", hdFileName);
	exit (1);
  }
  fprintf(hd, "/* This file is generated from: \n");
  for (lan=0; lan<LanguageCount; lan++) {
	fprintf(hd, "	 %s\n", MkMsgFileName(lan));
  }
  fprintf(hd, "*/\n");
  MaxLeng= 0;
  for (lan=0; lan<LanguageCount; lan++) {
	char *sLine, Line[512];
	int l;
	char *MsgFileName;
	FILE *MsgFile;

	MsgFileName= MkMsgFileName(lan);
	if ((MsgFile=fopen(MsgFileName,"r")) == NULL) {
      printf("Sorry, cannot open %s\n", MsgFileName);
      exit(2);
	}
	fprintf(hd,"char *%sMsg[] = {\n", GetLanguageName(lan));
	StringCnt= l= 0;
	while (fgets(Line,sizeof(Line),MsgFile) != NULL) {
      int snum;
      if (StringCnt>0) {
		fprintf(hd,",\n");
      }
      if (Line[l] == '#') {
		continue;
      }
      l= strlen(Line);
      if (Line[l-1] != '\n') {
		fprintf(stderr, "String %d is too long\n",StringCnt);
		exit(3);
      }
      Line[--l]= '\0';
      if (Line[l-1] == '\\') {
		l--;
		continue;
      }
      sscanf(Line, "%d= ", &snum);
      if (snum != StringCnt) {
		fprintf(stderr,
                "Wrong Format of string %d\n",StringCnt);
		exit(4);
      }
      sLine= strchr(Line, ' ')+1;
      fprintf(hd, "\t/*%2d*/\t\"%s\"", StringCnt, sLine);
      if ((l=strlen(sLine)) > MaxLeng) {
		MaxLeng= l;
      }
      l= 0;
      StringCnt++;
	}
	if (lan>0 && StringCnt != LastStringCnt) {
      fprintf(stderr, "Sorry, different StringCounts !\n");
      exit(5);
	}
	LastStringCnt=StringCnt;
	fprintf(hd, "\n};\n");
	fclose(MsgFile);
  }

  fprintf(hd, "char **MessageTabs[] = {\n");
  for (lan=0; lan<LanguageCount; lan++) {
	if (lan>0) {
      fprintf(hd, ",\n");
	}
	fprintf(hd, "\t/*%d*/\t%sMsg", lan, GetLanguageName(lan));
  }
  fprintf(hd, "\n};\n");
  exit(0);
}

#else	/* not MSG_IN_MEM */

typedef unsigned int Cardinal;

static int PutCardinal(Cardinal i, char *s) {
  if (i <= 0x7f) {
	*s++ = i;
	return 1;
  }
  if (i < 0x3fff) {
	*s++ = (i>>8) | 0x80;
	*s++ = i;
	return 2;
  }
  if (i < 0x3fffffffL) {
	*s++ = (i>>24) | 0xC0;
	*s++ = (i>>16);
	*s++ = (i>>8);
	*s++ = i;
	return 4;
  }
  fprintf(stderr,"Cannot put this large Cardinals\n");
  exit(2);
}

int main() {
  char Head[64], *hpt;
  char StrBuf[25256], *spt, *start, *p, *r;
  char OfsBuf[1024], *opt;
  char Line[256];

  Cardinal StringCnt;
  Cardinal MaxLeng, l;
  Language	 lan;
  int      snum;
  FILE     *MsgFile, *out;
  char     *FileName;

  for (lan=0; lan<LanguageCount; lan++) {

	FileName= MkMsgFileName(lan);

	if ((MsgFile=fopen(FileName,"r")) == NULL) {
      fprintf(stderr,"Cannot open %s\n",FileName);
      exit(3);
	}

	StringCnt= l= MaxLeng= 0;
	spt= StrBuf;
	opt= OfsBuf;
	while (fgets(Line+l,sizeof(Line)-l,MsgFile) != NULL) {
      if (Line[l] == '#') {
		continue;
      }
      l+= strlen(Line+l);
      if (Line[l-1] != '\n') {
		fprintf(stderr, "String %d is too long\n", StringCnt);
		exit(4);
      }
      Line[--l]= '\0';
      if (Line[l-1] == '\\') {
		l--;
		continue;
      }
      sscanf(Line, "%d= ", &snum);
      if (snum != StringCnt) {
		fprintf(stderr,
                "Wrong Format of string %d\n", StringCnt);
		exit(5);
      }
      r= p= start= strchr(Line,' ')+1;
      while (*p) {
		if (*p != '\\') {
          *r++= *p++;
		}
		else {
          p++;
          switch (*p) {
          case 'n': *r++= '\n'; break;
          case 't': *r++= '\t'; break;
          case 'f': *r++= '\f'; break;
          case 'r': *r++= '\r'; break;
          default:	*r++= *p;
          }
          p++;
		}
      }
      *r= '\0';
      l= r-start;
      opt+= PutCardinal(spt-StrBuf,opt);
      spt+= PutCardinal(l,spt);
      strcpy(spt, start);
      spt+= l;
      if (MaxLeng < l) {
		MaxLeng= l;
      }
      l= 0;
      StringCnt++;
	}

	for (hpt=Head; hpt<&(Head[sizeof(Head)]); hpt++)
      *hpt= '\0';

	hpt= Head;
	hpt+= PutCardinal(StringCnt,hpt);	      /* String Count */
	hpt+= PutCardinal(MaxLeng,hpt);		    /* Maximal Length */
	hpt+= PutCardinal(sizeof(Head), hpt);	  /* Start of Strings */
	hpt+= PutCardinal(spt-StrBuf, hpt);	   /* Size of Strings */
	hpt+= PutCardinal(
      sizeof(Head)+spt-StrBuf, hpt);	   /* Start of Offset */
	hpt+= PutCardinal(opt-OfsBuf, hpt);	    /* Size of Offset */

	strcpy(strchr(FileName,'.'),".str");

	if ((out=fopen(FileName,"wb")) == NULL) {
      fprintf(stderr,"Cannot open %s for writing\n", FileName);
      exit(6);
	}
	fwrite(Head, sizeof(Head), 1, out);
	fwrite(StrBuf, spt-StrBuf, 1, out);
	fwrite(OfsBuf, opt-OfsBuf, 1, out);
	fclose(out);

  }
  exit(0);

}
#endif /* MESSAGES_IN_MEM */
