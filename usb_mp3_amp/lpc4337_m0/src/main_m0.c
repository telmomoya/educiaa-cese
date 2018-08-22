#include "sapi.h"        // <= sAPI header
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "memory_m0.h"
#include "tm_stm32f4_ili9341.h"

int16_t *FFTOutput;						//Aquí estará la salida de fft.c

int main(void)
{
FFTOutput = &(ram_loc40->FFTOutput);	//Ahora apunta al mismo FFTOutput del M4

		#define ANCHO	216
	    #define ALTO	40
		#define X0		(240-ANCHO)/2
		#define Y0		30

while (1)
	{
		while (ipc->stop_graph==1);				//Sólo dibuja el espectro si se lo permite M4
		/* Set cursor position */
	    TM_ILI9341_SetCursorPosition(X0, Y0, X0+ANCHO-1, Y0+ALTO-1);
	    /* Set command for GRAM data */
	    TM_ILI9341_SendCommand(0x2C);
	    Board_SSP_config(16,SSP_CLOCK_CPHA0_CPOL0 ,40000000);
	 while (1)
		{
		while(ipc->fft_writting==1);		// Espero si se está calculando la fft
		int x,y;
		uint16_t color;
		uint32_t valor;
		for (y=0;y<ALTO;y++)
			{
			for (x=0;x<ANCHO/8;x++)
				{
				valor=FFTOutput[30+x*2];
				if ((y%8)==0)
					color = ILI9341_COLOR_BLACK;
				else
				{
					if (valor > (ALTO-y))
					{
						if (y > ALTO/2)
							color = ILI9341_COLOR_GREEN;
						else
							color = ILI9341_COLOR_RED;
					}
					else
						color = ILI9341_COLOR_BLACK;
				}
				TM_ILI9341_SendRAM (color);
				TM_ILI9341_SendRAM (color);
				TM_ILI9341_SendRAM (color);
				TM_ILI9341_SendRAM (color);
				TM_ILI9341_SendRAM (color);
				TM_ILI9341_SendRAM (color);
				TM_ILI9341_SendRAM (color);
				TM_ILI9341_SendRAM (ILI9341_COLOR_BLACK);
				}
			}
	  if (ipc->stop_graph==1)		// Si M4 deshabilita el espectro
	  	  {
		  ipc->stop_graph_ack=1;	// le contesto con un ack y libero el SPI
		  break;
	  	  }
	  }
		Board_SSP_config(8,SSP_CLOCK_CPHA0_CPOL0 ,40000000);
	}
}
