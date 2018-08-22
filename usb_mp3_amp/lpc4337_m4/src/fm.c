#include "sapi.h"        // <= sAPI header
#include <stdint.h>

#define		FREQUENCY		102		// Funciona razonablemente bien desde 101 hasta 108 mhz
#define     PLL_FRAC		0x20000 + (FREQUENCY-102)*1285 // Cambiar por algo que calcule correctamente la parte entera y fraccional


void FM_Play (uint8_t modulacion) 				// FM Modulator
{
	LPC_CGU->PLL0AUDIO_FRAC =  (uint32_t)(PLL_FRAC + modulacion);

	LPC_CGU->PLL[CGU_AUDIO_PLL].PLL_CTRL =			// (Table 132) 0x4005 0030 - PLL0AUDIO control register
										(0<<0)|		// PLL0 power UP
										(0<<1)|		// BYPASS
										(1<<2)|		// DIRECTI
										(0<<3)|		// DIRECTO
										(1<<4)|		// CLKEN
										(1<<12)|	// PLLFRACT_REQ
										(0x0C<<24);	// IDIVA (Clock source selection)
}

void FM_Init_PLL()
{
// FM con PLL0AUDIO
// IDIVA usa como fuente PPL1 (204MHz) y lo divide en 4
// PLLAUDIO en modo fraccional y usa como fuente a IDIVA (204/4=51MHz)

	LPC_CGU->IDIV_CTRL[CLK_IDIV_A] = 				// (Table 138) 0x4005 0048 - IDIVA control register
									(0x03<<2)|		// divider value = 4
									(0x09<<24);		// CLK_SEL = PLL1;

	LPC_CGU->PLL[CGU_AUDIO_PLL].PLL_NP_DIV  = 		// (Table 134) 0x4005 0038 - PLL0AUDIO_NP_DIV - divider register
										(66)|		// Post-Divider: [6:0]   PDEC Decoded P-divider coefficient value
										(0<<12);	// Pre-Divider:  [21:12] NDEC Decoded N-divider coefficient value

	LPC_CGU->PLL0AUDIO_FRAC = (uint32_t)(PLL_FRAC);	// (Table 135) 0x4005 003C bits 21:0 - PLL0AUDIO fractional divider register


	LPC_CGU->PLL[CGU_AUDIO_PLL].PLL_CTRL =			// (Table 132) 0x4005 0030 - PLL0AUDIO control register
										(0<<0)|		// PLL0 power UP
										(0<<1)|		// BYPASS
										(1<<2)|		// DIRECTI
										(0<<3)|		// DIRECTO
										(1<<4)|		// CLKEN
										(1<<12)|	// PLLFRACT_REQ
										(0x0C<<24);	// IDIVA (Clock source selection)


/* Ruteo PLLAUDIO al pin REF CLK de la EduCIAA*/
	Chip_Clock_SetBaseClock(CLK_BASE_OUT, CLKIN_AUDIOPLL, true, false);	// (Table 146) 0x400500AC bits 28:24 - BASE_OUT_CLK source selection: CLK_SEL = PLL0AUDIO
	Chip_SCU_PinMuxSet(0x1, 19, (SCU_PINIO_FAST | SCU_MODE_FUNC4));		// (Table 191) 0x400860CC bits 2:0 - PIN1.19 MODE: Function 4 = CLKOUT

}
