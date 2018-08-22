#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include "defines.h"
#include "sapi.h"        // <= sAPI header
#include "mp3dec.h"
#include "mp3.h"
#include "ff.h"

extern FIL mp3File;

/* Ping-Pong specific */
volatile bool getNextFrame = false;
volatile bool Enable_DAC = false;

/* MP3 decode specific */
HMP3Decoder mp3Instance;
MP3FrameInfo mp3FrameInfo;

int bytesLeft;
uint8_t *readPtr;
volatile int16_t outOfData;

short *outBuf0;
short *outBuf1;

volatile bool eofReached;
uint8_t readBuf[READBUF_SIZE];

/***************************************************************************//**
 * @brief
 *   Store MP3 data from SD card to buffer for decoding.
 * @param[in] *readBuf
 *   Pointer to the buffer to store MP3 data.
 * @param[in] *readPtr
 *   Pointer to last read location.
 * @param[in] bufSize
 *   Size of buffer to store MP3 data.
 * @param[in] byteLeft
 *   Bytes left in the buffer not yet processed.
 * @param[in] *infile
 *   Pointer to the open MP3 file object.
 * @param[out] buff
 *   Data buffer to store received data.
 * @return
 *   Number of bytes read.
******************************************************************************/
static int FillReadBuffer(unsigned char *readBuf, unsigned char *readPtr, int bufSize, int bytesLeft, FIL *infile)
{
//  uint16_t nRead;
  UINT nRead;

  /* Move last, small chunk from end of buffer to start, then fill with new data */
  memmove(readBuf, readPtr, bytesLeft);
  f_read(infile, readBuf + bytesLeft, bufSize - bytesLeft, &nRead);

  /* Zero-pad to avoid finding false sync word after last frame (from old data in readBuf) */
  if (nRead < bufSize - bytesLeft)
  {
    memset(readBuf + bytesLeft + nRead, 0, bufSize - bytesLeft - nRead);
  }
  return (int) nRead;
}

/***************************************************************************//**
 * @brief
 *   Store decoded PCM data to output buffer.
 * @param[in] *readFile (for MP3 file in uSD card)
 *   Pointer to the open MP3 file object.
 * @param[in] *outBuf
 *   Pointer to the buffer to store decoded PCM data.
 * @return
 *   1:Error or EOF, 0:OK, -1:Retry.
******************************************************************************/

int FillMP3DecodeBuffer(FIL *readFile, short *outbuf)
{
  int err, offset;
  int nRead;
  /* Somewhat arbitrary trigger to refill buffer - should always be enough for a full frame */
  if (bytesLeft < 2 * MAINBUF_SIZE && !eofReached)
  {
    nRead = FillReadBuffer(readBuf, readPtr, READBUF_SIZE, bytesLeft, readFile);

	static char uartBuff[10];
    uartWriteString( UART_USB," ");
       		uartWriteString( UART_USB,itoa(f_tell(&mp3File),uartBuff,10));
       		uartWriteString( UART_USB,"\n\r");

    bytesLeft += nRead;
    readPtr = readBuf;
    if (nRead == 0)
    {
      eofReached = true;
    }

  }

  /* Find start of next MP3 frame, retry or EOF if no sync found */
  offset = MP3FindSyncWord(readPtr, bytesLeft);
  if (offset < 0)
  {
    if (eofReached)
    {
      outOfData = 1;
    }
    else
    {
      bytesLeft = 0;
      outOfData = -1;
    }
    return outOfData;
  }

  readPtr += offset;
  bytesLeft -= offset;

  /* Retry if not a valid MP3 frame header */
  err = MP3GetNextFrameInfo(mp3Instance, &mp3FrameInfo, readPtr);
  if (err < 0)
  {
  static char uartBuff[10];
  uartWriteString( UART_USB,"!");

	readPtr++;
    bytesLeft--;
    outOfData = -1;
    return outOfData;
  }

  /* Retry if not enough data for decode */
  if (bytesLeft < FRAME320K_SIZE)
  {
    outOfData = -1;
    return outOfData;
  }

  /* Decode one MP3 frame */
  err = MP3Decode(mp3Instance, &readPtr, &bytesLeft, outbuf, 0);
  if (err < 0)
  {
    /* Error occurred */
    switch (err)
    {
    case ERR_MP3_INDATA_UNDERFLOW:
      outOfData = 1;
      uartWriteString( UART_USB,"ERR_MP3_INDATA_UNDERFLOW\n\r");
      break;

    case ERR_MP3_MAINDATA_UNDERFLOW:
    case ERR_MP3_INVALID_HUFFCODES:
  	  uartWriteString( UART_USB,"ERR_MP3_MAINDATA_UNDERFLOW\n\r");
      // Do nothing - next call to decode will provide more mainData
      outOfData = 0;
      break;

    case ERR_MP3_FREE_BITRATE_SYNC:
      outOfData = 1;
      uartWriteString( UART_USB,"ERR_MP3_BITRATE_SYNC\r\n");
      break;

    default:
      //outOfData = 0;
      outOfData = 1;
      uartWriteString( UART_USB, "MP3 DECODE ERROR\r\n" );
      break;
    }
  }
  else
  {
    outOfData = 0;
  }
  return outOfData;
}

/***************************************************************************//**
 * @brief
 *   Decode MP3 data and output to DAC.
******************************************************************************/
void MP3Play_Init(void)
{
  /* Reset parameters */
  bytesLeft = 0;
  eofReached = false;
  getNextFrame = false;

  /* Fill buffer in advance */

  do
  {
    FillMP3DecodeBuffer(&mp3File, outBuf1);
  } while (outOfData != 0);

  {
    FillMP3DecodeBuffer(&mp3File, outBuf0);
  } while (outOfData != 0);
 }

void MP3Play_Frame(void)
{
	/* Wait buffer ready to refill */

	while (!getNextFrame);
	FillMP3DecodeBuffer(&mp3File, outBuf1);
	getNextFrame = false;

	while (!getNextFrame);
	FillMP3DecodeBuffer(&mp3File, outBuf0);
	getNextFrame = false;

}
