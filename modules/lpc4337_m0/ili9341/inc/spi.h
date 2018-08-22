/*
 * uart.h
 *
 *  Created on: 31/01/2016
 *      Author: Matias Leandro Ferraro
 */
#include "stdint.h"
#include "stdbool.h"
#include "chip.h"
#include "scu_18xx_43xx.h"

#define LPC_SSP             LPC_SSP1
void write_spi_XXh(uint32_t LPC_SSP_n, uint32_t XXh);
