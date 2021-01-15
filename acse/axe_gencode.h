/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_gencode.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Code generation functions. See also axe_utils.h for gen_load_immediate()
 * and gen_move_immediate().
 */

#ifndef _AXE_GENCODE_H
#define _AXE_GENCODE_H

#include "axe_engine.h"
#include "axe_struct.h"

/*----------------------------------------------------
 *                   NOP & HALT
 *---------------------------------------------------*/

/* By calling this function, a new NOP instruction will be added
 * to `program'. A NOP instruction doesn't make use of
 * any kind of parameter */
extern t_axe_instruction *gen_nop_instruction(t_program_infos *program);

/* By calling this function, a new HALT instruction will be added
 * to `program'. An HALT instruction doesn't require
 * any kind of parameter */
extern t_axe_instruction *gen_halt_instruction(t_program_infos *program);

/*----------------------------------------------------
 *                   UNARY OPERATIONS
 *---------------------------------------------------*/

/* A LOAD instruction requires the following parameters:
 * 1.  A destination register where the requested value will be loaded
 * 2.  A label information (can be a NULL pointer. If so, the addess
 *     value will be taken into consideration)
 * 3.  A direct address (if label is different from NULL) */
extern t_axe_instruction *gen_load_instruction(
      t_program_infos *program, int r_dest, t_axe_label *label, int address);

/* A READ instruction requires only one parameter:
 * A destination register (where the value
 * read from standard input will be loaded). */
extern t_axe_instruction *gen_read_instruction(
      t_program_infos *program, int r_dest);

/* A WRITE instruction requires only one parameter:
 * A destination register (where the value
 * that will be written to the standard output is located). */
extern t_axe_instruction *gen_write_instruction(
      t_program_infos *program, int r_dest);

/* A STORE instruction copies a value from a register to a
 * specific memory location. The memory location can be
 * either a label identifier or a address reference.
 * In order to create a STORE instruction the caller must
 * provide a valid register location (`r_dest') and an
 * instance of `t_axe_label' or a numeric address */
extern t_axe_instruction *gen_store_instruction(
      t_program_infos *program, int r_dest, t_axe_label *label, int address);

/* A MOVA instruction copies an address value into a register.
 * An address can be either an instance of `t_axe_label'
 * or a number (numeric address) */
extern t_axe_instruction *gen_mova_instruction(
      t_program_infos *program, int r_dest, t_axe_label *label, int address);

/* 
 * STATUS REGISTER TEST INSTRUCTIONS
 */

/* A SGE instruction tests the content of the STATUS REGISTER. To be more
 * specific, a SGE instruction sets to #1 the content of the register
 * `r_dest' if the condition (N.V + ~N.~V) is TRUE; otherwise the content
 * of `r_dest' is set to 0.
 * (I.e.: r_dest will be set to #1 only if the value computed by
 * the last numeric operation returned a value
 * greater or equal to zero). */
extern t_axe_instruction *gen_sge_instruction(
      t_program_infos *program, int r_dest);

/* A SEQ instruction tests the content of the STATUS REGISTER. In particular,
 * a SEQ instruction sets to #1 the content of the register
 * `r_dest' if the condition Z is TRUE; otherwise the content of `r_dest' is set
 * to 0. (I.e.: r_dest will be set to #1 only if the value computed by
 * the last numeric operation returned a value equal to zero). */
extern t_axe_instruction *gen_seq_instruction(
      t_program_infos *program, int r_dest);

/* A SGT instruction tests the content of the STATUS REGISTER. In particular,
 * a SGT instruction sets to #1 the content of the register
 * `r_dest' if the condition (N.V.~Z + ~N.~V.~Z) is TRUE;
 * otherwise the content of `r_dest' is set to 0. (I.e.: r_dest will be
 * set to #1 only if the value computed by the last numeric operation
 * returned a value greater than zero). */
extern t_axe_instruction *gen_sgt_instruction(
      t_program_infos *program, int r_dest);

/* A SLE instruction tests the content of the STATUS REGISTER. In particular,
 * a SLE instruction sets to #1 the content of the register
 * `r_dest' if the condition (Z + N.~V + ~N.V) is TRUE;
 * otherwise the content of `r_dest' is set to 0. (I.e.: r_dest will be
 * set to #1 only if the value computed by the last numeric operation
 * returned a value less than zero). */
extern t_axe_instruction *gen_sle_instruction(
      t_program_infos *program, int r_dest);

/* A SLT instruction tests the content of the STATUS REGISTER. In particular,
 * a SLT instruction sets to #1 the content of the register
 * `r_dest' if the condition (N.~V + ~N.V) is TRUE;
 * otherwise the content of `r_dest' is set to 0. (I.e.: r_dest will be
 * set to #1 only if the value computed by the last numeric operation
 * returned a value less than or equal to zero). */
extern t_axe_instruction *gen_slt_instruction(
      t_program_infos *program, int r_dest);

/* A SNE instruction tests the content of the STATUS REGISTER. In particular,
 * a SNE instruction sets to #1 the content of the register
 * `r_dest' if the condition ~N is TRUE;
 * otherwise the content of `r_dest' is set to 0. (I.e.: r_dest will be
 * set to #1 only if the value computed by the last numeric operation
 * returned a value different from zero). */
extern t_axe_instruction *gen_sne_instruction(
      t_program_infos *program, int r_dest);

/*----------------------------------------------------
 *                   BINARY OPERATIONS
 *---------------------------------------------------*/

/* Used in order to create and assign to the current `program'
 * an ADDI instruction. The semantic of an ADDI instruction
 * is the following: ADDI r_dest, r_source1, immediate. `RDest' is a register
 * location identifier: the result of the ADDI instruction will be
 * stored in that register. Using an RTL (Register Transfer Language)
 * representation we can say that an ADDI instruction of the form: 
 * ADDI R1 R2 #IMM can be represented in the following manner: R1 <-- R2 + IMM.
 * `Rsource1' and `#IMM' are the two operands of the binary numeric
 * operation. `r_dest' is a register location, `immediate' is an immediate
 * value. The content of `r_source1' is added to the value of `immediate'
 * and the result is then stored into the register `RDest'. */
extern t_axe_instruction *gen_addi_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a SUBI instruction. The semantic of an SUBI instruction
 * is the following: SUBI r_dest, r_source1, immediate. `RDest' is a register
 * location identifier: the result of the SUBI instruction will be
 * stored in that register. Using an RTL representation we can say
 * that a SUBI instruction of the form: SUBI R1 R2 #IMM can be represented
 * in the following manner: R1 <-- R2 - IMM.
 * `Rsource1' and `#IMM' are the two operands of the binary numeric
 * operation. `r_dest' is a register location, `immediate' is an immediate
 * value. The content of `r_source1' is subtracted to the value of `immediate'
 * and the result is then stored into the register `RDest'. */
extern t_axe_instruction *gen_subi_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * an ANDLI instruction. An example RTL representation of ANDLI R1 R2 #IMM is:
 * R1 <-- R2 && IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *gen_andli_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a ORLI instruction. An example RTL representation of ORLI R1 R2 #IMM is:
 * R1 <-- R2 || IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *gen_orli_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a EORLI instruction. An example RTL representation of EORLI R1 R2 #IMM is:
 * R1 <-- R2 XOR IMM (Where XOR is the operator: logical exclusive OR).
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *gen_eorli_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * an ANDBI instruction. An example RTL representation of ANDBI R1 R2 #IMM is:
 * R1 <-- R2 & IMM (bitwise AND).
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *gen_andbi_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a MULI instruction. An example RTL representation of MULI is:
 * R1 <-- R2 * IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *gen_muli_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * an ORBI instruction. An example RTL representation of ORBI R1 R2 #IMM is:
 * R1 <-- R2 | IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *gen_orbi_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a EORBI instruction. An example RTL representation of EORBI R1 R2 #IMM is:
 * R1 <-- R2 ^ IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *gen_eorbi_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a DIVI instruction. An example RTL representation of DIVI R1 R2 #IMM is:
 * R1 <-- R2 / IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *gen_divi_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a SHLI instruction. An example RTL representation of SHLI R1 R2 #IMM is:
 * R1 <-- R2 / IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *gen_shli_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a SHRI instruction. An example RTL representation of SHRI R1 R2 #IMM is:
 * R1 <-- R2 / IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *gen_shri_instruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a NOTL instruction. An example RTL representation of NOTL R1 R2 is:
 * R1 <-- !R2. */
extern t_axe_instruction *gen_notl_instruction(
      t_program_infos *program, int r_dest, int r_source1);

/* Used in order to create and assign to the current `program'
 * a NOTB instruction. An example RTL representation of NOTB R1 R2 is:
 * R1 <-- ~R2. */
extern t_axe_instruction *gen_notb_instruction(
      t_program_infos *program, int r_dest, int r_source1);

/*----------------------------------------------------
 *                   TERNARY OPERATIONS
 *---------------------------------------------------*/

/* Used in order to create and assign to the current `program'
 * a ADD instruction. An example RTL representation of ADD R1 R2 R3 is:
 * R1 <-- R2 + R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_add_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a SUB instruction. An example RTL representation of SUB R1 R2 R3 is:
 * R1 <-- R2 - R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_sub_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a ANDL instruction. An example RTL representation of ANDL R1 R2 R3 is:
 * R1 <-- R2 && R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_andl_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a ORL instruction. An example RTL representation of ORL R1 R2 R3 is:
 * R1 <-- R2 || R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_orl_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a EORL instruction. An example RTL representation of EORL R1 R2 R3 is:
 * R1 <-- R2 XORL R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_eorl_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a ANDB instruction. An example RTL representation of ANDB R1 R2 R3 is:
 * R1 <-- R2 & R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_andb_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a ORB instruction. An example RTL representation of ORB R1 R2 R3 is:
 * R1 <-- R2 | R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_orb_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a EORB instruction. An example RTL representation of EORB R1 R2 R3 is:
 * R1 <-- R2 XORB R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_eorb_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a MUL instruction. An example RTL representation of MUL R1 R2 R3 is:
 * R1 <-- R2 * R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_mul_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a DIV instruction. An example RTL representation of DIV R1 R2 R3 is:
 * R1 <-- R2 / R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_div_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a SHL instruction. An example RTL representation of SHL R1 R2 R3 is:
 * R1 <-- R2 shifted to left by R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_shl_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a SHR instruction. An example RTL representation of SHR R1 R2 R3 is:
 * R1 <-- R2 shifted to right by R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_shr_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a NEG instruction. An example RTL representation of NEG R1 R2 is:
 * as follows: R1 <-- (-R2).
 * `r_source' is the only operand for this instruction.
 * `r_dest' is a register location. `r_dest' and `r_source'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *gen_neg_instruction(
      t_program_infos *program, int r_dest, int r_source, int flags);

/* This instruction is reserved for future implementation. */
extern t_axe_instruction *gen_spcl_instruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/*----------------------------------------------------
 *                   JUMP INSTRUCTIONS
 *---------------------------------------------------*/

/* create a branch true instruction. By executing this instruction the control
 * is always passed to either the instruction with the label `label' associated
 * with, or (if `label' is a NULL pointer) to the explicit `address' */
extern t_axe_instruction *gen_bt_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a branch false instruction. By executing this instruction the control
 * is always passed to the next instruction in the program
 * (i.e.: the instruction pointed by PC + 1). */
extern t_axe_instruction *gen_bf_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on higher than" instruction. */
extern t_axe_instruction *gen_bhi_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on less than" instruction. According to the value
 * of the status register, the branch will be taken if the expression
 * (~C.~Z) is TRUE. */
extern t_axe_instruction *gen_bls_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on carry clear" instruction. If the bit `C' of the
 * status register is not set, then the branch is taken. */
extern t_axe_instruction *gen_bcc_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on carry clear" instruction. If the bit `C' of the
 * status register is set, then the branch is taken. */
extern t_axe_instruction *gen_bcs_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on not equal" instruction. If the bit `Z' of the
 * status register is not set, then the branch is taken. */
extern t_axe_instruction *gen_bne_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on equal" instruction. If the bit `Z' of the
 * status register is set, then the branch is taken. */
extern t_axe_instruction *gen_beq_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on overflow clear" instruction. If the bit `V' of the
 * status register is not set, then the branch is taken. */
extern t_axe_instruction *gen_bvc_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on overflow set" instruction. If the bit `V' of the
 * status register is set, then the branch is taken. */
extern t_axe_instruction *gen_bvs_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on plus (i.e. positive)" instruction. If the bit `N' of the
 * status register is not set, then the branch is taken. */
extern t_axe_instruction *gen_bpl_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on minus (i.e. negative)" instruction. If the bit `N' of the
 * status register is set, then the branch is taken. */
extern t_axe_instruction *gen_bmi_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on greater or equal" instruction. According to the value
 * of the status register, the branch will be taken if the expression
 * (N.V + ~N.~V) is TRUE. */
extern t_axe_instruction *gen_bge_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on less than" instruction. According to the value
 * of the status register, the branch will be taken if the expression
 * (N.~V + ~N.V) is TRUE. */
extern t_axe_instruction *gen_blt_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on less than" instruction. According to the value
 * of the status register, the branch will be taken if the expression
 * (N.V.~Z + ~N.~V.~Z) is TRUE. */
extern t_axe_instruction *gen_bgt_instruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on less than or equal" instruction. According to the value
 * of the status register, the branch will be taken if the expression
 * (Z + N.~V + ~N.V) is TRUE. */
extern t_axe_instruction *gen_ble_instruction(
      t_program_infos *program, t_axe_label *label, int addr);


#endif
