#include "memory_m0.h"

struct Loc32 * const ram_loc32 =  (struct Loc32*)RamLoc32;
struct Loc40 * const ram_loc40 = (struct Loc40*)RamLoc40;
struct AHB32 * const ram_ahb32 = (struct ipc_s*)RamAHB32;
struct AHB16 * const ram_ahb16 =  (struct AHB16*)RamAHB16;
struct AHB_ETB16 * const ram_ahb_etb16 =  (struct AHB_ETB16*)RamAHB_ETB16;

volatile struct ipc_s * const ipc = (struct ipc_s*)RamAHB32;
