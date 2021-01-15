/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_target_asm_print.h
 * Formal Languages & Compilers Machine, 2007-2020
 * 
 * Generation of the output assembly program.
 */

#ifndef _AXE_TARGET_ASM_PRINT_H
#define _AXE_TARGET_ASM_PRINT_H

#include "axe_engine.h"
#include "axe_errors.h"

/* write the corresponding assembly for the given program */
extern void writeAssembly(t_program_infos *program, char *output_file);

#endif
