#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>
#include "defines.h"
#include "arm_math.h"

#define	RamLoc32		0x10000000
#define	RamLoc40		0x10080000
#define	RamAHB32		0x20000000
#define	RamAHB16		0x20008000
#define	RamAHB_ETB16	0x2000c000

// Aquí armo una estructura con todas las variables IPC, que quiero ver con los 2 cores
struct ipc_s {
	int fft_writting;		// offset 0
	unsigned short dummy1;
	int stop_graph;			// offset 4
	unsigned short dummy2;
	int fft_writting_ack;	// offset 8
	unsigned short dummy3;
	int stop_graph_ack;		// offset 12
	unsigned short dummy;
};


// Estas son las estructuras de las seccciones de memoria
// En alguna debo incluir struct ipc_s, pero siempre como primer miembro
// Luego actualizo la última linea de memory.c con la sección elegida

struct Loc32 {};

struct Loc40 {
	float32_t Input    [SAMPLES];
	float32_t Output   [SAMPLES/2];
	float32_t FFTOutput[SAMPLES/4];
};

struct AHB32{
	struct ipc_s	ipc;
};

struct AHB16 {
	short outBuf0[2*DECODEBUF_SIZE]; // offset 0
	short outBuf1[2*DECODEBUF_SIZE];
};

struct AHB_ETB16{};


extern struct Loc32 * const ram_loc32;
extern struct Loc40 * const ram_loc40;
extern struct AHB32 * const ram_ahb32;
extern struct AHB16 * const ram_ahb16;
extern struct AHB_ETB16 * const ram_ahb_etb16;
extern volatile struct ipc_s * const ipc;

#endif
