#include "axe_target_transform.h"
#include "axe_target_info.h"
#include "axe_gencode.h"
#include "axe_utils.h"

#define R_ID(reg)       ((reg)->ID)
#define R_IND(reg)      ((reg)->indirect)
#define RD(inst)        ((inst)->reg_1)
#define RD_ID(inst)     ((inst)->reg_1->ID)
#define RD_IND(inst)    ((inst)->reg_1->indirect)
#define RS1(inst)       ((inst)->reg_2)
#define RS1_ID(inst)    ((inst)->reg_2->ID)
#define RS1_IND(inst)   ((inst)->reg_2->indirect)
#define RS2(inst)       ((inst)->reg_3)
#define RS2_ID(inst)    ((inst)->reg_3->ID)
#define RS2_IND(inst)   ((inst)->reg_3->indirect)

#define CG_DIRECT(dest, src2)    (!!(dest) | ((!!(src2)) << 1))
#define CG_DIRECT_R(dest, src2)  CG_DIRECT(R_IND(dest), R_IND(src2))

t_list *fixDestinationRegisterOfInstruction(t_program_infos *program,
      t_list *position)
{
   t_axe_instruction *instr = LDATA(position);
   assert(instr);

   if (isMoveInstruction(instr, NULL, NULL, NULL, NULL))
      return position;
   if (!instr->reg_2 || instr->reg_2->ID == REG_INVALID)
      return position;
   if (instr->reg_2->ID == instr->reg_1->ID && 
         instr->reg_2->indirect == instr->reg_1->indirect)  
      return position;

   /* For some strange reason, the IMUL x86_64 instruction DOES have a form
    * with an immediate, a source and a destination. In fact, it's the only
    * form it has with provisions for an immediate operand! */
   if (instr->opcode == MULI)
      return position;

   pushInstrInsertionPoint(program, LPREV(position));
   gen_add_instruction(program, RD_ID(instr), REG_0, RS1_ID(instr), 
         CG_DIRECT(RD_IND(instr), 0));
   instr->reg_2->ID = instr->reg_1->ID;
   instr->reg_2->indirect = instr->reg_1->indirect;
   popInstrInsertionPoint(program);
   return position;
}

void fixDestinationRegister(t_program_infos *program)
{
   t_list *cur = program->instructions;
   while (cur) {
      cur = fixDestinationRegisterOfInstruction(program, cur);
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

void fixReadWrite(t_program_infos *program)
{
   t_list *cur = program->instructions;
   while (cur) {
      t_axe_instruction *inst = (t_axe_instruction *)LDATA(cur);
      if (inst->opcode == AXE_READ || inst->opcode == AXE_WRITE) {
         t_list *afterCall = genRegisterClobberingForCall(program, cur);

         /* note: destination of read and write instructions 
          * cannot be indirect */

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

void rewriteLogicalOperations(t_program_infos *program)
{
   t_list *cur = program->instructions;
   while (cur) {
      t_axe_instruction *inst = (t_axe_instruction *)LDATA(cur);
      t_list *next = LNEXT(cur);
      assert((!inst->reg_2 || !RS1_IND(inst)) && "found illegal instruction with IND on SRC1");
      
      if (inst->opcode == ANDL || inst->opcode == EORL) {
         pushInstrInsertionPoint(program, LPREV(cur));
         int rs1 = getNewRegister(program);
         int rs2 = getNewRegister(program);
         gen_andb_instruction(program, RS1_ID(inst), RS1_ID(inst), RS1_ID(inst), CG_DIRECT_ALL);
         gen_sne_instruction(program, rs1);
         if (!RS2_IND(inst)) {
            gen_andb_instruction(program, RS2_ID(inst), RS2_ID(inst), RS2_ID(inst), CG_DIRECT_ALL);
         } else {
            int tmp = getNewRegister(program);
            gen_add_instruction(program, tmp, REG_0, RS2_ID(inst), CG_INDIRECT_DEST);
         }
         gen_sne_instruction(program, rs2);
         RS1_ID(inst) = rs1;
         RS2_ID(inst) = rs2;
         RS2_IND(inst) = 0;
         popInstrInsertionPoint(program);
         inst->opcode = inst->opcode == ANDL ? ANDB : EORB;

      } else if (inst->opcode == ORL) {
         inst->opcode = ORB;
         int destId = RD_ID(inst);
         int destInd = RD_IND(inst);
         RD_ID(inst) = getNewRegister(program);
         RD_IND(inst) = 0;
         pushInstrInsertionPoint(program, cur);
         if (!destInd) {
            gen_sne_instruction(program, destId);
         } else {
            int tmp2 = getNewRegister(program);
            gen_sne_instruction(program, tmp2);
            gen_add_instruction(program, destId, REG_0, tmp2, CG_DIRECT(destInd, 0));
         }
         popInstrInsertionPoint(program);

      } else if (inst->opcode == ANDLI) {
         /* this instruction is stupid #1 */
         pushInstrInsertionPoint(program, LPREV(cur));
         if (inst->immediate == 0) {
            gen_eorb_instruction(program, RD_ID(inst), RD_ID(inst), RD_ID(inst), CG_DIRECT_ALL);
         } else {
            gen_andb_instruction(program, RS1_ID(inst), RS1_ID(inst), RS1_ID(inst), CG_DIRECT_ALL);
            gen_sne_instruction(program, RD_ID(inst));
         }
         popInstrInsertionPoint(program);
         removeInstructionLink(program, cur);

      } else if (inst->opcode == ORLI) {
         /* this instruction is stupid #2 */
         pushInstrInsertionPoint(program, LPREV(cur));
         if (inst->immediate != 0) {
            gen_addi_instruction(program, RD_ID(inst), REG_0, 1);
         } else {
            gen_andb_instruction(program, RS1_ID(inst), RS1_ID(inst), RS1_ID(inst), CG_DIRECT_ALL);
            gen_sne_instruction(program, RD_ID(inst));
         }
         popInstrInsertionPoint(program);
         removeInstructionLink(program, cur);

      } else if (inst->opcode == EORLI) {
         /* this instruction is stupid #3 */
         pushInstrInsertionPoint(program, LPREV(cur));
         gen_andb_instruction(program, RS1_ID(inst), RS1_ID(inst), RS1_ID(inst), CG_DIRECT_ALL);
         if (inst->immediate == 0) {
            gen_sne_instruction(program, RD_ID(inst));
         } else {
            gen_seq_instruction(program, RD_ID(inst));
         }
         popInstrInsertionPoint(program);
         removeInstructionLink(program, cur);
      }

      cur = next;
   }
}

void insertRegisterAllocationConstraints(t_program_infos *program)
{
   t_list *cur = program->instructions;
   while (cur) {
      t_axe_instruction *inst = (t_axe_instruction *)LDATA(cur);

      if (inst->opcode == SHL || inst->opcode == SHR || 
            inst->opcode == ROTL || inst->opcode == ROTR) {
         /* Force shift amount register to be ECX */
         pushInstrInsertionPoint(program, LPREV(cur));
         int rShAmt = inst->reg_3->ID;
         int rShAmtInd = inst->reg_3->indirect;
         inst->reg_3->ID = getNewRegister(program);
         inst->reg_3->indirect = 0;
         inst->reg_3->mcRegWhitelist = addElement(inst->reg_3->mcRegWhitelist, (void *)R_AMD64_ECX, 0);
         gen_add_instruction(program, inst->reg_3->ID, REG_0, rShAmt, CG_DIRECT(0, rShAmtInd));
         popInstrInsertionPoint(program);
      }

      if (inst->opcode == DIVI) {
         /* move the immediate to a register because x86_64 does not have a DIV
          * instruction with an immediate parameter. */
         pushInstrInsertionPoint(program, LPREV(cur));
         int rimm = gen_load_immediate(program, inst->immediate);
         inst->opcode = DIV;
         inst->reg_3 = alloc_register(rimm, INTEGER_TYPE, 0);
         popInstrInsertionPoint(program);
      }

      if (inst->opcode == DIV) {
         /* Force source1 and destination to be in EAX. Add instructions for
          * zeroing out EDX before the division. */
         int rdest = RD_ID(inst);
         int rdest_dir = RD_IND(inst);
         int rtmp = getNewRegister(program);  /* RAX at codegen stage */
         int rEDX = getNewRegister(program);

         pushInstrInsertionPoint(program, LPREV(cur));
         gen_add_instruction(program, rtmp, REG_0, rdest, CG_DIRECT(0, rdest_dir));
         t_axe_instruction *zeroEdx = gen_addi_instruction(program, rEDX, REG_0, 0);
         zeroEdx->reg_1->mcRegWhitelist = addElement(zeroEdx->reg_1->mcRegWhitelist, (void *)R_AMD64_EDX, 0);
         popInstrInsertionPoint(program);

         inst->reg_1->ID = inst->reg_2->ID = rtmp;
         inst->reg_1->indirect = inst->reg_2->ID = 0;
         inst->reg_1->mcRegWhitelist = addElement(inst->reg_1->mcRegWhitelist, (void *)R_AMD64_EAX, 0);

         pushInstrInsertionPoint(program, cur);
         /* tell the register allocator that DIV sets EDX */
         t_axe_instruction *dummyEdxSet = gen_addi_instruction(program, rEDX, REG_0, 0);
         dummyEdxSet->mcFlags = MCFLAG_DUMMY;
         gen_add_instruction(program, rdest, REG_0, rtmp, CG_DIRECT(rdest_dir, 0));
         popInstrInsertionPoint(program);
      }

      cur = LNEXT(cur);
   }
}

void doTargetSpecificTransformations(t_program_infos *program)
{
   rewriteLogicalOperations(program);
   fixDestinationRegister(program);
   fixReadWrite(program);
   insertRegisterAllocationConstraints(program);
}
