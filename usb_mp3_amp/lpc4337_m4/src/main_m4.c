#include "main.h"
#include "sapi.h"        // <= sAPI header
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "MassStorageHost.h"
#include "ff.h"
#include "ff_extras.h"
#include "timer.h"
#include "fm.h"
#include "display.h"
#include "tm_stm32f4_ili9341.h"
#include "memory.h"
#include "cr_start_m0.h"
#include "mp3.h"


char text[30];
int tecla=2;

int seleccionado=0;
int cantidad_archivos;
#define CANTIDAD_ARCHIVOS_LISTADOS	6

/* File system specific */
FIL mp3File;
/* Return value, directory search object and file information */
DIR dj;
FATFS Fatfs;
FILINFO fno;
FRESULT res;

int main(void)
{
	outBuf0 = &(ram_ahb16->outBuf0);
	outBuf1 = &(ram_ahb16->outBuf1);

	SetupHardware();	// USB Setup

  /* ------------- INICIALIZACIONES ------------- */

  /* Inicializar la placa */
  boardConfig();

  /* Inicializar UART_USB a 115200 baudios */
  uartConfig( UART_USB, 115200 );
  uartWriteString( UART_USB,"UART OK\n\r");


  /* Inicializar FM */
  FM_Init_PLL();

  Display_init();

  /* Inicializar DAC */
  dacConfig( DAC_ENABLE ); /* DAC */
  Enable_DAC = false;

  /* Inicializar Timer0 */
  timer0Init(4626);

  ipc->stop_graph=1;

#define CIAA_MULTICORE_CORE_1_IMAGE ((uint8_t *)0x1B000000)	// Inicio código M0
  /* Run M0*/
  cr_start_m0(0, CIAA_MULTICORE_CORE_1_IMAGE);

  /* Initialize filesystem */
  f_mount(0, &Fatfs);		/* Register volume work area (never fails) */

  /* Initialize decoder */
  mp3Instance = MP3InitDecoder();

  if(mp3Instance == 0)
  {
	  uartWriteString( UART_USB,"Could not init MP3 decoder\n\r");
      while (1);
  }

/* Cuento la cantidad de archivos en el directorio*/
res = f_findfirst(&dj, &fno, "", "*.mp3");
cantidad_archivos=0;
while (res == FR_OK && fno.fname[0])
{
	res = f_findnext(&dj, &fno);
	cantidad_archivos++;
}

Seleccion();

while(1)
  {
  do				// Permite elegir archivo hasta que se presiona tec1 para play
  	  {
	  Revisar_teclas();
	  Seleccion();
  	  } while (tecla!=1);

  	  res = f_findfirst(&dj, &fno, "", "*.mp3");	//Busco el archivo seleccionado
  	  int i;
  	  for (i=1;i<seleccionado;i++) f_findnext(&dj, &fno);

  	  /* Open and play MP3 file */
  	  res = f_open(&mp3File, fno.fname, FA_READ);
  	  if (res != FR_OK)
  	  {
	  	  uartWriteString( UART_USB,"MP3 file open error\n");
  	  }
  	  else
  	  {
	  	  uartWriteString( UART_USB,"\nPlaying ");
	  	  uartWriteString( UART_USB,fno.fname);
	  MP3Play();
      f_close(&mp3File);
  	  }

  }

  /* Start playing */
  while(1)
  {
    /* Start to search for MP3 files */
    res = f_findfirst(&dj, &fno, "", "*.mp3");
    if (!fno.fname[0])
    {
      uartWriteString( UART_USB,"No MP3 file in USB\n");
      MP3FreeDecoder(mp3Instance);
      while (1);
    }

    while (res == FR_OK && fno.fname[0])
    {
      /* Open and play MP3 file */
      res = f_open(&mp3File, fno.fname, FA_READ);
      if (res != FR_OK)
      {
    	  uartWriteString( UART_USB,"MP3 file open error\n");
      }
      else
      {
    	  sprintf (text, "%s - %d KB         ", fno.fname, (int)fno.fsize/1024);
    	  TM_ILI9341_Puts(0, 300, text, &TM_Font_11x18, ILI9341_COLOR_BLUE2, ILI9341_COLOR_BLUE);
    	  uartWriteString( UART_USB,"\nPlaying ");
    	  uartWriteString( UART_USB,fno.fname);
    	  MP3Play();
      }
      /* Close current file and search for next item */
      f_close(&mp3File);
      res = f_findnext(&dj, &fno);
    }
  }
}

void MP3Play(void)
{
	MP3Play_Init();
	Muestra_data();
    timer0Init(Chip_Clock_GetBaseClocktHz (CLK_BASE_MX)/mp3FrameInfo.samprate);

    // Play until end of file

    gpioWrite( LED3, ON );
	Enable_DAC = 1;
	ipc->stop_graph=0;	// inicia dibujo en m0
  do
	{
	MP3Play_Frame();
	static int skip=0;
	if (++skip==3)
		{
		ipc->fft_writting=1;
		fft();
		ipc->fft_writting=0;
		skip=0;
		}
	Revisar_teclas();
    if (tecla>1) break;
    } while (outOfData == 0);

  // End of play
  	  ipc->stop_graph_ack=0;
  	  ipc->stop_graph=1;		// Paro el dibujo del M0
  	  while (ipc->stop_graph_ack==0);
  	  Enable_DAC = false;
  	  gpioWrite( LED3, OFF );
}

void Listar_archivos(int piso, int techo)
{
	TM_ILI9341_Puts(10, 150,"    File Browser    ", &TM_Font_11x18,ILI9341_COLOR_BLUE2,ILI9341_COLOR_BLUE);

	res = f_findfirst(&dj, &fno, "", "*.mp3");
	  int n,i;
	  for (i=1;i<=piso;i++)
	  {
		  if (res == FR_OK && fno.fname[0])
	  	    {
			n=sprintf (text, fno.fname);
			text[n-4]=0;			// Recorto últimos 4 que son la extensión
			n=sprintf (text, "%s          ", text);
			text[10]=0;				// Recorto nombre
		  	n=sprintf (text, "%s - %d KB         ", text, (int) fno.fsize/1024);
		  	text[20]=0;				// Recorto al ancho visible
		  	if (i>=techo) TM_ILI9341_Puts(10, 170+23*(i-techo), text, &TM_Font_11x18, (i==seleccionado)?ILI9341_COLOR_WHITE:ILI9341_COLOR_BLUE,(i==seleccionado)?ILI9341_COLOR_BLACK:ILI9341_COLOR_BLACK);
		    res = f_findnext(&dj, &fno);
	  	    }
	  }
}

void Revisar_teclas()
{
  tecla = 0;
  if (!gpioRead( TEC1 )) tecla=1;
  if (!gpioRead( TEC2 )) tecla=2;
  if (!gpioRead( TEC3 )) tecla=3;
  if (!gpioRead( TEC4 )) tecla=4;
}

void Seleccion()
{
static int techo = 1;
static int piso = CANTIDAD_ARCHIVOS_LISTADOS;

  if ((tecla == 2) && (seleccionado < cantidad_archivos))
  	  {
  		if (++seleccionado>piso)
  		{
  			techo++;
  			piso++;
  		}
  		Listar_archivos(piso, techo);
  	  }

  if (( tecla == 3 ) && (seleccionado>1))
  	  {
  		if (--seleccionado<techo)
  		{
  			techo--;
 			piso--;
  		}
  		Listar_archivos(piso, techo);
  	  }
}

void Muestra_data()
{
  int n;
  n=sprintf (text,"Playing: %s", fno.fname);
  text[n-4]=0;
  //n=sprintf (text, "%s - %d KB         ", text, fno.fsize/1024);
  TM_ILI9341_Puts(10, 80, text, &TM_Font_11x18, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);
  n=sprintf (text,"%d Kbps",mp3FrameInfo.bitrate/1000);
  n=sprintf (text,"%s - %d Hz", text, mp3FrameInfo.samprate);
  TM_ILI9341_Puts(10, 80+(23*1), text, &TM_Font_11x18, ILI9341_COLOR_WHITE, ILI9341_COLOR_BLACK);

  static char uartBuff[10];
  uartWriteString( UART_USB,"\n\rBitrate: ");
  uartWriteString( UART_USB,itoa(mp3FrameInfo.bitrate,uartBuff,10));
  uartWriteString( UART_USB,"\n\rCanales: ");
  uartWriteString( UART_USB,itoa(mp3FrameInfo.nChans,uartBuff,10));
  uartWriteString( UART_USB,"\n\rSamples: ");
  uartWriteString( UART_USB,itoa(mp3FrameInfo.samprate,uartBuff,10));
  uartWriteString( UART_USB,"\n\rBPS: ");
  uartWriteString( UART_USB,itoa(mp3FrameInfo.bitsPerSample,uartBuff,10));
  uartWriteString( UART_USB,"\n\rOutput Samples: ");
  uartWriteString( UART_USB,itoa(mp3FrameInfo.outputSamps,uartBuff,10));
  uartWriteString( UART_USB,"\n\rLayer: ");
  uartWriteString( UART_USB,itoa(mp3FrameInfo.layer,uartBuff,10));
  uartWriteString( UART_USB,"\n\rVersion: ");
  uartWriteString( UART_USB,itoa(mp3FrameInfo.version,uartBuff,10));
  uartWriteString( UART_USB,"\n\r");
}
