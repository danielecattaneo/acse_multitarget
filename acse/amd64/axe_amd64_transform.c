#include "axe_target_transform.h"
#include "axe_target_info.h"
#include "axe_gencode.h"
#include "axe_utils.h"

t_list *fix_destination_register_instr(t_program_infos *program,
                                       t_list *position)
{
   t_axe_instruction *instr = LDATA(position);
   assert(instr);
   if (isMoveInstruction(instr, NULL, NULL, NULL, NULL))
      return position;
   if (!instr->reg_2 || instr->reg_2->ID == REG_INVALID)
      return position;
   if (instr->reg_2->ID == instr->reg_1->ID)
      return position;
   pushInstrInsertionPoint(program, LPREV(position));
   gen_add_instruction(program, instr->reg_1->ID, instr->reg_2->ID, REG_0, 0);
   instr->reg_2->ID = instr->reg_1->ID;
   popInstrInsertionPoint(program);
   return position;
}

void fix_destination_register(t_program_infos *program)
{
   t_list *cur = program->instructions;
   while (cur) {
      cur = fix_destination_register_instr(program, cur);
      cur = LNEXT(cur);
   }
}

t_list *genRegisterClobberingForCall(t_program_infos *program, t_list *callLnk)
{
   int regList[] = {
      R_AMD64_ECX, R_AMD64_EDX, R_AMD64_ESI, R_AMD64_EDI, 
      R_AMD64_R8D, R_AMD64_R9D, R_AMD64_R10D
   };
   pushInstrInsertionPoint(program, callLnk);
   for (int i = 0; i<sizeof(regList)/sizeof(int); i++) {
      int reg = regList[i];
      int var = getNewRegister(program);
      t_axe_instruction *instr = gen_addi_instruction(program, var, REG_0, 0);
      instr->reg_1->mcRegWhitelist = addElement(instr->reg_1->mcRegWhitelist, (void *)reg, 0);
      instr->mcFlags = MCFLAG_DUMMY;
   }
   return popInstrInsertionPoint(program);
}

void fixLoadStore(t_program_infos *program)
{
   t_list *cur = program->instructions;
   while (cur) {
      t_axe_instruction *inst = (t_axe_instruction *)LDATA(cur);
      if (inst->opcode == AXE_READ || inst->opcode == AXE_WRITE) {
         t_list *afterCall = genRegisterClobberingForCall(program, cur);

         if (inst->opcode == AXE_READ) {
            int destReg = inst->reg_1->ID;
            inst->reg_1->ID = getNewRegister(program);
            inst->reg_1->mcRegWhitelist = addElement(inst->reg_1->mcRegWhitelist, (void *)R_AMD64_EAX, 0);
            pushInstrInsertionPoint(program, afterCall);
            gen_add_instruction(program, destReg, inst->reg_1->ID, REG_0, CG_DIRECT_ALL);
            afterCall = popInstrInsertionPoint(program);
         }

         if (inst->opcode == AXE_WRITE) {
            int srcReg = inst->reg_1->ID;
            inst->reg_1->ID = getNewRegister(program);
            inst->reg_1->mcRegWhitelist = addElement(inst->reg_1->mcRegWhitelist, (void *)R_AMD64_EDI, 0);
            pushInstrInsertionPoint(program, LPREV(cur));
            gen_add_instruction(program, inst->reg_1->ID, srcReg, REG_0, CG_DIRECT_ALL);
            popInstrInsertionPoint(program);
         }
      }
      
      cur = LNEXT(cur);
   }
}

void fixShiftAmtRegister(t_program_infos *program)
{
   t_list *cur = program->instructions;
   while (cur) {
      t_axe_instruction *inst = (t_axe_instruction *)LDATA(cur);
      if (inst->opcode == SHL || inst->opcode == SHR || inst->opcode == ROTL || inst->opcode == ROTR) {
         pushInstrInsertionPoint(program, LPREV(cur));
         int rShAmt = inst->reg_3->ID;
         inst->reg_3->ID = getNewRegister(program);
         inst->reg_3->mcRegWhitelist = addElement(inst->reg_3->mcRegWhitelist, (void *)R_AMD64_ECX, 0);
         gen_add_instruction(program, inst->reg_3->ID, rShAmt, REG_0, CG_DIRECT_ALL);
         popInstrInsertionPoint(program);
      }
      cur = LNEXT(cur);
   }
}

void doTargetSpecificTransformations(t_program_infos *program)
{
   fix_destination_register(program);
   fixLoadStore(program);
   fixShiftAmtRegister(program);
}
