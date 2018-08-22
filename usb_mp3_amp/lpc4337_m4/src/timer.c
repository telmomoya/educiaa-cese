#include "sapi.h"        // <= sAPI header
#include "timer.h"
#include "fm.h"
#include "mp3dec.h"
#include "tm_stm32f4_ili9341.h"

extern volatile bool getNextFrame;
extern volatile bool Enable_DAC;
extern short *outBuf0;
extern short *outBuf1;
extern MP3FrameInfo mp3FrameInfo;

void timer0Init(uint32_t MatchCount){
	Chip_TIMER_Init(LPC_TIMER0);
	Chip_TIMER_SetMatch(LPC_TIMER0, 0, MatchCount);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER0, 0);
	Chip_TIMER_MatchEnableInt(LPC_TIMER0, 0);
	Chip_TIMER_Enable(LPC_TIMER0);
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler(){
	int32_t tmp;
	if (Chip_TIMER_MatchPending(LPC_TIMER0, 0))
	{
		static uint16_t indice = 0;
		if (Enable_DAC)
			{
			if (indice >= mp3FrameInfo.outputSamps)
			   {
				tmp = outBuf0[indice-mp3FrameInfo.outputSamps]+32767;
				if (mp3FrameInfo.nChans == 2)		// Si es estereo promedio las muestras
					tmp = (tmp + outBuf0[++indice-mp3FrameInfo.outputSamps]+32767)/2;
			   }
			else
			   {
				tmp = outBuf1[indice]+32767;
				if (mp3FrameInfo.nChans == 2)			// Si es estereo promedio las muestras
					tmp = (tmp + outBuf1[++indice]+32767)/2;
			   }
			Chip_DAC_UpdateValue(LPC_DAC,(tmp>>6)/2);	// Mitad para no saturar line-in
			FM_Play (tmp>>9);
			indice++;
			if (indice == mp3FrameInfo.outputSamps*2)
				{
				indice=0;
				getNextFrame = true;
				}
			if (indice == mp3FrameInfo.outputSamps)
				{
				getNextFrame = true;
				}
			}
		else indice = 0;
		Chip_TIMER_ClearMatch(LPC_TIMER0, 0);
	}
}
