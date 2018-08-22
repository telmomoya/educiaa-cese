#include "tm_stm32f4_ili9341.h"

void Display_init(void)
{
TM_ILI9341_Init();
TM_ILI9341_Puts(0, 0, "  EduCIAA MP3 PLAYER ", &TM_Font_11x18, ILI9341_COLOR_BLACK, ILI9341_COLOR_GREEN2);
}
