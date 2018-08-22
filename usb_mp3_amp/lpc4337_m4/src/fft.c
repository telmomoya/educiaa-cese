/**
 *	Keil project example for FFT on STM32F4 device.
 *
 *	Works on STM32F429-Discovery board because it has LCD
 *
 *	@author		Tilen Majerle
 *	@email		tilen@majerle.eu
 *	@website	http://stm32f4-discovery.net
 *	@ide		Keil uVision 5
 *	@packs		STM32F4xx Keil packs version 2.2.0 or greater required
 *	@stdperiph	STM32F4xx Standard peripheral drivers version 1.4.0 or greater required
 *
 *	Notes:
 *		- Under "Options for target" > "C/C++" > "Define" you must add 2 defines (I've already add them):
 *			- ARM_MATH_CM4
 *			- __FPU_PRESENT=1
 */
#include <stdio.h>
#include "sapi.h"        // <= sAPI header
#include "arm_math.h"	/* Include arm_math.h mathematic functions */
#include "fft.h"
#include "memory.h"

static char uartBuff[20];

float32_t maxValue;				/* Max FFT value is stored here */
uint32_t maxIndex;				/* Index in Output array where max value is */

int16_t   *FFTOutput;
float32_t *Input;
float32_t *Output;

extern short *outBuf0;


void fft(void) {

	Input=&(ram_loc40->Input);
	Output=&(ram_loc40->Output);
	FFTOutput=&(ram_loc40->FFTOutput);

	uint16_t i;
		for (i = 0; i < SAMPLES; i += 2) {
			/* Real part */
			Input[(uint16_t)i] = (float32_t)(outBuf0[i]+outBuf0[(i)+1]);
			/* Imaginary part */
			Input[(uint16_t)(i + 1)] = 0;
		}

		arm_cfft_radix4_instance_f32 S;	/* ARM CFFT module */

		/* Initialize the CFFT/CIFFT module, intFlag = 0, doBitReverse = 1 */
		arm_cfft_radix4_init_f32(&S, FFT_SIZE, 0, 1);

		/* Process the data through the CFFT/CIFFT module */
		arm_cfft_radix4_f32(&S, Input);

		/* Process the data through the Complex Magniture Module for calculating the magnitude at each bin */
		arm_cmplx_mag_f32(Input, Output, FFT_SIZE);

		/* Calculates maxValue and returns corresponding value */
		arm_max_f32(Output+30, (FFT_SIZE/2), &maxValue, &maxIndex);

		ipc->fft_writting=1;
		/* Escalo y copio los resultados pero como int para M0*/
		for (i = 0; i < FFT_SIZE/2; i++)
		{
			FFTOutput[i]=(int16_t)(40*(float32_t)((float32_t)Output[i]/(float32_t)maxValue));
		}
		ipc->fft_writting=0;

		if ((Output[1]) > 650000)
			{
			gpioWrite( LEDR, ON );
			gpioWrite( LEDB, ON );
			}
		else
			{
			gpioWrite( LEDR, OFF );
			gpioWrite( LEDB, OFF );
			}
}
