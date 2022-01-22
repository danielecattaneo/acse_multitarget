/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_utils.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Contains important functions to access the list of symbols and other
 * utility functions and macros.
 */

#ifndef _AXE_UTILS_H
#define _AXE_UTILS_H

#include "axe_engine.h"
#include "axe_struct.h"
#include "axe_constants.h"
#include "collections.h"

/* maximum and minimum between two values */
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

/* create a variable for each `t_axe_declaration' inside
 * the list `variables'. Each new variable will be of type
 * `varType'. */
extern void set_new_variables(
      t_program_infos *program, int varType, t_list *variables);

/* Given a variable/symbol identifier (ID) this function
 * returns a register location where the value is stored
 * (the value of the variable identified by `ID').
 * If the variable/symbol has never been loaded from memory
 * to a register, first this function searches
 * for a free register, then it assign the variable with the given
 * ID to the register just found.
 * Once computed, the location (a register identifier) is returned
 * as output to the caller.
 * This function generates a LOAD instruction
 * only if the flag `genLoad' is set to 1; otherwise it simply reserve
 * a register location for a new variable in the symbol table.
 * If an error occurs, get_symbol_location returns a REG_INVALID errorcode */
extern int get_symbol_location(t_program_infos *program, char *ID, int genLoad);

/* Generate the instruction to load an `immediate' value into a new register.
 * It returns the new register identifier or REG_INVALID if an error occurs */
extern int gen_load_immediate(t_program_infos *program, int immediate);

/* Generate the instruction to move an `immediate' value into a register. */
extern void gen_move_immediate(t_program_infos *program, int dest, int imm);

/* Returns 1 if `instr` is a jump (branch) instruction. */
extern int isJumpInstruction(t_axe_instruction *instr);

/* Returns 1 if `instr` is a unconditional jump instruction (BT, BF) */
extern int isUnconditionalJump(t_axe_instruction *instr);

/* Returns 1 if `instr` is either the HALT instruction or the RET
 * instruction. */
extern int isHaltOrRetInstruction(t_axe_instruction *instr);

/* Returns 1 if `instr` is the LOAD instruction. */
extern int isLoadInstruction(t_axe_instruction *instr);

/* Returns 1 if the opcode corresponds to an instruction with an immediate
 * argument (i.e. if the instruction mnemonic ends with `I`). */
extern int isImmediateArgumentInstrOpcode(int opcode);

/* Switches the immediate form of an opcode. For example, ADDI is transformed
 * to ADD, and ADD is transformed to ADDI. Returns the original opcode in case
 * there is no immediate or non-immediate available. */
extern int switchOpcodeImmediateForm(int orig);

/* Set the list of allowed machine registers for a specific register object
 * to the specified list of register identifiers. The list must be terminated
 * by REG_INVALID or -1. */
extern void setMCRegisterWhitelist(t_axe_register *regObj, ...);

/* Notify the end of the program. This function is directly called
 * from the parser when the parsing process is ended */
extern void set_end_Program(t_program_infos *program);

/* Once called, this function destroys all the data structures
 * associated with the compiler (program, RA, etc.). This function
 * is typically automatically called before exiting from the main
 * or when the compiler encounters some error. */
extern void shutdownCompiler();

/* Once called, this function initialize all the data structures
 * associated with the compiler (program, RA etc..) and all the
 * global variables in the system. This function
 * is typically automatically called at the beginning of the main
 * and should NEVER be called from the user code */
extern void init_compiler(int argc, char **argv);

#endif
