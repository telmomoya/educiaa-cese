#include "memory.h"

struct Loc32 * const ram_loc32 =  (struct Loc32*)RamLoc32;
struct Loc40 * const ram_loc40 = (struct Loc40*)RamLoc40;
struct AHB32 * const ram_ahb32 = (struct AHB32*)RamAHB32;
struct AHB16 * const ram_ahb16 =  (struct AHB16*)RamAHB16;
struct AHB_ETB16 * const ram_ahb_etb16 =  (struct AHB_ETB16*)RamAHB_ETB16;
//Para poder usar miembros de ipc_s sin tener que saber en cual seccción
//de memoria está puesta creo esta nueva estructura, entonces podré usar
//	ipc->variable en vez de seccion_memoria->ipc->variable
//Debe coincidir con la sección de memoria elegida para ipc_s en memory.h
volatile struct ipc_s * const ipc = (struct ipc_s*)RamAHB32;
