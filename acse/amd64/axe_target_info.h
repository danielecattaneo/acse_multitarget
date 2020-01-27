/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_target_info.h
 * Formal Languages & Compilers Machine, 2007-2020
 */

#ifndef _AXE_TARGET_INFO_H
#define _AXE_TARGET_INFO_H

#define TARGET_NAME "x86_64"

#define R_AMD64_EAX   1
#define R_AMD64_EBX   2  /* callee-save */
#define R_AMD64_ECX   3
#define R_AMD64_EDX   4
#define R_AMD64_ESI   5
#define R_AMD64_EDI   6
#define R_AMD64_R8D   7
#define R_AMD64_R9D   8
#define R_AMD64_R10D  9
#define R_AMD64_R11D 10
#define R_AMD64_R12D 11  /* callee-save */
#define R_AMD64_R13D 12  /* callee-save */
#define R_AMD64_R14D 13  /* callee-save */
#define R_AMD64_R15D 14  /* callee-save */

/* Number of registers for this target (excluding REG_0) */
#define NUM_REGISTERS R_AMD64_R15D
/* Number of registers to reserve for spilled temporaries. Shall be equal to
 * the maximum number of unique register operands in a single instruction. */
#define NUM_SPILL_REGS 2

/* number of bytes for each memory address */
#define TARGET_PTR_GRANULARITY 1

/* set on instructions that should not be output in
 * the resulting assembly code */
#define MCFLAG_DUMMY    1

#endif


