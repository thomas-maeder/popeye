#if !defined(INPUT_PLAINTEXT_LINE_H)
#define INPUT_PLAINTEXT_LINE_H

enum
{
  LINESIZE = 256
};

extern char InputLine[LINESIZE];    /* This array contains the input as is */

/* read into InputLine until the next1 end of line */
void ReadToEndOfLine(void);

void ReadRemark(void);

#endif
