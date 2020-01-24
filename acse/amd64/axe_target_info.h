#ifndef _AXE_TARGET_INFO_H
#define _AXE_TARGET_INFO_H

#define NUM_REGISTERS 14
/* Number of registers to reserve for spilled temporaries. Shall be equal to
 * the maximum number of unique register operands in a single instruction. */
#define NUM_SPILL_REGS 2
#define TARGET_NAME "x86_64"

/* number of bytes for each memory address */
#define TARGET_PTR_GRANULARITY 1

#endif


