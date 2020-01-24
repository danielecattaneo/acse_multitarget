/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_target_info.h
 * Formal Languages & Compilers Machine, 2007-2020
 * 
 * Properties of the target machine
 */

#ifndef _AXE_TARGET_INFO_H
#define _AXE_TARGET_INFO_H

#define TARGET_NAME "MACE"

/* Number of registers for this target (excluding REG_0) */
#define NUM_REGISTERS 31
/* Number of registers to reserve for spilled temporaries. Shall be equal to
 * the maximum number of unique register operands in a single instruction. */
#define NUM_SPILL_REGS 3

/* number of bytes for each memory address */
#define TARGET_PTR_GRANULARITY 4

#endif


