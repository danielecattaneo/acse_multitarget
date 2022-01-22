/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_engine.h
 * Formal Languages & Compilers Machine, 2007-2020
 *
 * Contains t_program_infos and some functions for label management
 * (reserve, fix, assign) 
 */

#ifndef _AXE_ENGINE_H
#define _AXE_ENGINE_H

#include "axe_struct.h"
#include "axe_labels.h"
#include "collections.h"
#include "symbol_table.h"

typedef struct t_program_infos
{
  t_list *variables;
  t_list *instructions;
  t_list *instrInsPtrStack;
  t_list *data;
  t_axe_label_manager *lmanager;
  t_symbol_table *sy_table;
  int current_register;
} t_program_infos;


/* initialize the informations associated with the program. This function is
 * called at the beginning of the translation process. This function
 * is called once: its only purpouse is to initialize an instance of the struct
 * `t_program_infos' that will contain all the informations about the program
 * that will be compiled */
extern t_program_infos *allocProgramInfos();

/* add a new instruction to the current program. This function is directly
 * called by all the functions defined in `axe_gencode.h' */
extern void addInstruction(t_program_infos *program, t_axe_instruction *instr);

/* remove an instruction from the program, given its link in the instruction
 * list. */
extern void removeInstructionLink(t_program_infos *program, t_list *instrLi);

/* Save the current insertion point in the instruction list, and replace it
 * with `ip`. New instructions will be inserted after the `ip` instruction.
 * To insert instructions at the beginning of the program, ip shall be NULL. */
extern void pushInstrInsertionPoint(t_program_infos *p, t_list *ip);

/* Restore the last insertion point in the instruction list. Returns the
 * previous position of the instruction insertion point. */
extern t_list *popInstrInsertionPoint(t_program_infos *p);

/* reserve a new label identifier and return the identifier to the caller */
extern t_axe_label *newLabel(t_program_infos *program);

/* assign the given label identifier to the next instruction. Returns
 * the label assigned; otherwise (an error occurred) NULL */
extern t_axe_label *assignLabel(t_program_infos *program, t_axe_label *label);

/* reserve and fix a new label. It returns either the label assigned or
 * NULL if an error occurred */
extern t_axe_label *assignNewLabel(t_program_infos *program);

/* Like the above functions, but with the ability to give a name to the label.
 * If another label with the same name already exists, the name assigned to
 * the new label will be modified to remove any ambiguity. */
extern t_axe_label *newNamedLabel(t_program_infos *program, const char *name);
extern t_axe_label *assignNewNamedLabel(
      t_program_infos *program, const char *name);

/* add a variable to the program */
extern void createVariable(t_program_infos *program, char *ID, int type,
      int isArray, int arraySize, int init_val);

/* get a previously allocated variable */
extern t_axe_variable *getVariable(t_program_infos *program, char *ID);

/* get the label that marks the starting address of the variable
 * with name "ID" */
extern t_axe_label *getLabelFromVariableID(t_program_infos *program, char *ID);

/* get a register still not used. This function returns
 * the ID of the register found*/
extern int getNewRegister(t_program_infos *program);

/* finalize all the data structures associated with `program' */
extern void finalizeProgramInfos(t_program_infos *program);

#endif
