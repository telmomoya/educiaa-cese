#ifndef _MP3_H
#define _MP3_H

#include "mp3dec.h"
#include "sapi.h"        // <= sAPI header
#include "ff.h"

extern short *outBuf0;
extern short *outBuf1;
extern volatile bool Enable_DAC;
extern HMP3Decoder mp3Instance;
extern MP3FrameInfo mp3FrameInfo;
extern volatile int16_t outOfData;
extern int bytesLeft;

/* Frame data, read buffer and MP3 decode buffer size */
#define FRAME128K_SIZE  430
#define FRAME320K_SIZE  1200
#define READBUF_SIZE    2048
#define DECODEBUF_SIZE  MAX_NCHAN * MAX_NGRAN * MAX_NSAMP

void MP3Play_Init(void);
void MP3Play_Frame(void);
int FillMP3DecodeBuffer(FIL *readFile, short *outbuf);
#endif
