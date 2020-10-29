/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_mace_transform.h
 * Formal Languages & Compilers Machine, 2007-2020
 */

#include "axe_target_transform.h"
#include "axe_gencode.h"
#include "axe_struct.h"
#include "axe_utils.h"
#include "collections.h"

void moveLabel(t_axe_instruction *dest, t_axe_instruction *src)
{
   assert(dest->labelID == NULL && "moveLabel failed: destination already is labeled");
   dest->labelID = src->labelID;
   src->labelID = NULL;
   
   if (!dest->user_comment) {
      dest->user_comment = src->user_comment;
      src->user_comment = NULL;
   }
}

int is_int16(int immediate)
{
   return immediate < (1 << 15) && immediate >= -(1 << 15);
}

t_axe_instruction *genLoweredImmediateMove(
      t_program_infos *program, int dest, int immediate)
{
   t_axe_instruction *firstInstr = NULL;
   
   int imm0 = immediate;
   int imm1 = 0;
   if (!is_int16(imm0)) {
      /* cast to int16_t to perform sign-extension */
      imm0 = (int16_t)(immediate & 0xFFFF); 
      imm1 = (immediate - imm0) >> 16;
   }

   int basereg = REG_0;
   if (imm1) {
      firstInstr = gen_addi_instruction(program, dest, basereg, imm1);
      gen_shli_instruction(program, dest, dest, 16);
      basereg = dest;
   }
   if (imm0 || basereg == REG_0) {
      t_axe_instruction *i = gen_addi_instruction(program, dest, basereg, imm0);
      if (!firstInstr)
         firstInstr = i;
   }
   
   return firstInstr;
}

void fixLargeImmediates(t_program_infos *program)
{
   t_list *curi = program->instructions;
   
   while (curi) {
      t_axe_instruction *instr = LDATA(curi);
      
      if (!isImmediateArgumentInstrOpcode(instr->opcode) || is_int16(instr->immediate)) {
         curi = LNEXT(curi);
         continue;
      }
      t_list *nexti = LNEXT(curi);
      
      if (instr->opcode == ADDI && instr->reg_2->ID == REG_0) {
         pushInstrInsertionPoint(program, curi);
         int reg = instr->reg_1->ID;
         genLoweredImmediateMove(program, reg, instr->immediate);
         removeInstructionLink(program, curi);
         popInstrInsertionPoint(program);
         
      } else {
         pushInstrInsertionPoint(program, curi->prev);
         int reg = getNewRegister(program);
         moveLabel(genLoweredImmediateMove(program, reg, instr->immediate), instr);
         instr->immediate = 0;
         instr->reg_3 = alloc_register(reg, 0);
         instr->opcode = switchOpcodeImmediateForm(instr->opcode);
         popInstrInsertionPoint(program);
      }
      
      curi = nexti;
   }
}

void doTargetSpecificTransformations(t_program_infos *program)
{
   fixLargeImmediates(program);
}
