/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_amd64_transform.c
 * Formal Languages & Compilers Machine, 2007-2020
 */

#include "axe_target_transform.h"
#include "axe_target_info.h"
#include "axe_gencode.h"
#include "axe_utils.h"
#include "axe_cflow_graph.h"

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

/* move the label of one instruction to another. */
void moveLabel(t_axe_instruction *dest, t_axe_instruction *src)
{
   dest->labelID = src->labelID;
   src->labelID = NULL;
}

/* Remove instructions with three unique operands. 
 * Note: this pass may produce instructions where SRC1 is an indirectly
 * indexed register, which is otherwise impossible. */
void fixInstrOperands(t_program_infos *program)
{
   t_list *position = program->instructions;
   for (; position; position = LNEXT(position)) {
      t_axe_instruction *instr = LDATA(position);

      /* instructions that can be represented as a move are special-cased in the
      * ASM printer, and do not need any fix */
      if (isMoveInstruction(instr, NULL, NULL, NULL, NULL))
         continue;

      /* For some strange reason, the IMUL x86_64 instruction DOES have a form
      * with an immediate, a source and a destination. In fact, it's the only
      * form it has with provisions for an immediate operand! */
      if (instr->opcode == MULI)
         continue;

      /* check if the instruction has at least 2 operands */
      if (!RS1(instr) || RS1_ID(instr) == REG_INVALID)
         continue;

      /* check if the instruction is not already in the form RD = RS1 */
      if (RS1_ID(instr) == RD_ID(instr) && RS1_IND(instr) == RD_IND(instr))  
         continue;

      pushInstrInsertionPoint(program, LPREV(position));

      /* copy RS2 to another register if:
       *  - there are more than one indirect operand (amd64 does not allow it)
       *  - RD and RS2 are the same register (otherwise the ADD which moves
       *    RS1 to RD would overwrite one of the source regs otherwise) */
      if (RS2(instr) && ((RS2_IND(instr) && RD_IND(instr)) || (RD_ID(instr) == RS2_ID(instr)))) {
         int tmp = getNewRegister(program);
         moveLabel(gen_add_instruction(program, tmp, REG_0, RS2_ID(instr), 
               CG_DIRECT(0, RS2_IND(instr))), instr);
         RS2_ID(instr) = tmp;
         RS2_IND(instr) = 0;
      }

      moveLabel(gen_add_instruction(program, RD_ID(instr), REG_0, RS1_ID(instr), 
            CG_DIRECT(RD_IND(instr), 0)), instr);
      RS1_ID(instr) = RD_ID(instr);
      RS1_IND(instr) = RD_IND(instr);

      popInstrInsertionPoint(program);
   }
}

t_list *genRegisterClobberingForCall(t_program_infos *program, t_list *callLnk, int returns)
{
   int regList[] = {
      R_AMD64_EAX, R_AMD64_ECX, R_AMD64_EDX, R_AMD64_ESI, R_AMD64_EDI,
      R_AMD64_R8D, R_AMD64_R9D, R_AMD64_R10D, R_AMD64_R11D
   };
   pushInstrInsertionPoint(program, callLnk);
   for (int i = returns ? 1 : 0; i<sizeof(regList)/sizeof(int); i++) {
      int reg = regList[i];
      int var = getNewRegister(program);
      t_axe_instruction *instr = gen_addi_instruction(program, var, REG_0, 0);
      setMCRegisterWhitelist(RD(instr), reg, -1);
      instr->mcFlags = MCFLAG_DUMMY;
   }
   return popInstrInsertionPoint(program);
}

void fixReadWrite(t_program_infos *program)
{
   /* note: destination of read and write instructions 
    * cannot be indirect */

   t_list *cur = program->instructions;
   for (; cur; cur = LNEXT(cur)) {
      t_axe_instruction *inst = (t_axe_instruction *)LDATA(cur);

      if (inst->opcode == AXE_READ) {
         t_list *afterCall = genRegisterClobberingForCall(program, cur, 1);

         int destReg = RD_ID(inst);
         RD_ID(inst) = getNewRegister(program);
         setMCRegisterWhitelist(RD(inst), R_AMD64_EAX, -1);

         pushInstrInsertionPoint(program, afterCall);
         gen_add_instruction(program, destReg, RD_ID(inst), REG_0, CG_DIRECT_ALL);
         popInstrInsertionPoint(program);

      } else if (inst->opcode == AXE_WRITE) {
         genRegisterClobberingForCall(program, cur, 0);

         int srcReg = RD_ID(inst);
         RD_ID(inst) = getNewRegister(program);
         setMCRegisterWhitelist(RD(inst), R_AMD64_EDI, -1);

         pushInstrInsertionPoint(program, LPREV(cur));
         moveLabel(gen_add_instruction(program, RD_ID(inst), srcReg, REG_0, CG_DIRECT_ALL), inst);
         popInstrInsertionPoint(program);
      }
   }
}

void rewriteLogicalOperations(t_program_infos *program)
{
   t_list *cur = program->instructions;
   while (cur) {
      t_axe_instruction *inst = (t_axe_instruction *)LDATA(cur);
      t_list *next = LNEXT(cur);
      assert((!RS1(inst) || !RS1_IND(inst)) && "found illegal instruction with IND on SRC1");
      
      if (inst->opcode == ANDL || inst->opcode == EORL) {
         pushInstrInsertionPoint(program, LPREV(cur));
         int rs1 = getNewRegister(program);
         int rs2 = getNewRegister(program);
         moveLabel(gen_andb_instruction(program, RS1_ID(inst), RS1_ID(inst), RS1_ID(inst), CG_DIRECT_ALL), inst);
         gen_sne_instruction(program, rs1);
         if (!RS2_IND(inst)) {
            gen_andb_instruction(program, RS2_ID(inst), RS2_ID(inst), RS2_ID(inst), CG_DIRECT_ALL);
         } else {
            int tmp = getNewRegister(program);
            gen_add_instruction(program, tmp, REG_0, RS2_ID(inst), CG_INDIRECT_SOURCE);
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

      } else if (inst->opcode == NOTL) {
         assert(!RD_IND(inst) && "found illegal NOTL instruction with IND on DST");
         pushInstrInsertionPoint(program, LPREV(cur));
         moveLabel(gen_andb_instruction(program, RS1_ID(inst), RS1_ID(inst), RS1_ID(inst), CG_DIRECT_ALL), inst);
         gen_seq_instruction(program, RD_ID(inst));
         popInstrInsertionPoint(program);
         removeInstructionLink(program, cur);

      } else if (inst->opcode == ANDLI) {
         /* this instruction is stupid #1 */
         pushInstrInsertionPoint(program, LPREV(cur));
         t_axe_instruction *firstInst;
         if (inst->immediate == 0) {
            firstInst = gen_eorb_instruction(program, RD_ID(inst), RD_ID(inst), RD_ID(inst), CG_DIRECT_ALL);
         } else {
            firstInst = gen_andb_instruction(program, RS1_ID(inst), RS1_ID(inst), RS1_ID(inst), CG_DIRECT_ALL);
            gen_sne_instruction(program, RD_ID(inst));
         }
         moveLabel(firstInst, inst);
         popInstrInsertionPoint(program);
         removeInstructionLink(program, cur);

      } else if (inst->opcode == ORLI) {
         /* this instruction is stupid #2 */
         pushInstrInsertionPoint(program, LPREV(cur));
         t_axe_instruction *firstInst;
         if (inst->immediate != 0) {
            firstInst = gen_addi_instruction(program, RD_ID(inst), REG_0, 1);
         } else {
            firstInst = gen_andb_instruction(program, RS1_ID(inst), RS1_ID(inst), RS1_ID(inst), CG_DIRECT_ALL);
            gen_sne_instruction(program, RD_ID(inst));
         }
         moveLabel(firstInst, inst);
         popInstrInsertionPoint(program);
         removeInstructionLink(program, cur);

      } else if (inst->opcode == EORLI) {
         /* this instruction is stupid #3 */
         pushInstrInsertionPoint(program, LPREV(cur));
         moveLabel(gen_andb_instruction(program, RS1_ID(inst), RS1_ID(inst), RS1_ID(inst), CG_DIRECT_ALL), inst);
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
         int rShAmt = RS2_ID(inst);
         int rShAmtInd = RS2_IND(inst);
         RS2_ID(inst) = getNewRegister(program);
         RS2_IND(inst) = 0;
         setMCRegisterWhitelist(RS2(inst), R_AMD64_ECX, -1);
         moveLabel(gen_add_instruction(program, inst->reg_3->ID, REG_0, rShAmt, CG_DIRECT(0, rShAmtInd)), inst);
         popInstrInsertionPoint(program);
      }

      if (inst->opcode == DIVI) {
         /* move the immediate to a register because x86_64 does not have a DIV
          * instruction with an immediate parameter. */
         pushInstrInsertionPoint(program, LPREV(cur));
         int rimm = getNewRegister(program);
         moveLabel(gen_addi_instruction(program, rimm, REG_0, inst->immediate), inst);
         inst->opcode = DIV;
         RS2(inst) = alloc_register(rimm, INTEGER_TYPE, 0);
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
         moveLabel(gen_add_instruction(program, rtmp, REG_0, rdest, CG_DIRECT(0, rdest_dir)), inst);
         gen_subi_instruction(program, REG_0, rtmp, 0);
         gen_slt_instruction(program, rEDX);
         t_axe_instruction *signExpEDX = gen_neg_instruction(program, rEDX, rEDX, CG_DIRECT_ALL);
         setMCRegisterWhitelist(RD(signExpEDX), R_AMD64_EDX, -1);
         popInstrInsertionPoint(program);

         RD_ID(inst) = RS1_ID(inst) = rtmp;
         RD_IND(inst) = RS1_IND(inst) = 0;
         setMCRegisterWhitelist(RD(inst), R_AMD64_EAX, -1);

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

void fixFlagUsers(t_program_infos *program)
{
   t_cflow_Graph *cfg = createFlowGraph(program->instructions);
   performLivenessAnalysis(cfg);

   t_list *bbLnk = cfg->blocks;
   for (; bbLnk; bbLnk = LNEXT(bbLnk)) {
      t_basic_block *bb = LDATA(bbLnk);
      t_list *nodeLnk = bb->nodes;
      for (; nodeLnk; nodeLnk = LNEXT(nodeLnk)) {
         t_cflow_Node *node = LDATA(nodeLnk);
         
         if (!(node->uses[0] && node->uses[0]->ID == VAR_PSW))
            continue;

         t_list *reachDefLst = reachingDefinitionsOfNode(cfg, bb, node);
         assert(reachDefLst && "invalid program; PSW used but never defined");

         t_list *reachDefLnk = reachDefLst;
         for (; reachDefLnk; reachDefLnk = LNEXT(reachDefLnk)) {
            t_cflow_reach_def *reachDef = LDATA(reachDefLnk);

            if (reachDef->var->ID != VAR_PSW)
               continue;

            t_axe_instruction *instr = reachDef->node->instr;
            int needsSettingFlags = 0;
            t_axe_register *dstReg;

            if (isMoveInstruction(instr, &dstReg, NULL, NULL, NULL)) {
               /* x86_64 MOV instructions do not set flags. However, MOVA does
                * not set flags even in the ACSE IR. */
               if (instr->opcode != MOVA)
                  needsSettingFlags = 1;
            } else if (instr->opcode == SEQ || instr->opcode == SGE ||
                  instr->opcode == SGT || instr->opcode == SLE ||
                  instr->opcode == SLT) {
               /* SETcc instructions do not set flags on x86_64. SNE does not
                * alter the state of the zero flag, thus we do not have to test
                * for that. */
               dstReg = RD(instr);
               needsSettingFlags = 1;
            }

            if (!needsSettingFlags)
               continue;

            t_list *instLnk = findElement(program->instructions, reachDef->node->instr);
            assert(instLnk && "instruction is in the CFG but not in the program");
            pushInstrInsertionPoint(program, instLnk);
            if (!R_IND(dstReg)) {
               gen_andb_instruction(program, R_ID(dstReg), R_ID(dstReg), R_ID(dstReg), CG_DIRECT_ALL);
            } else {
               int tmp = getNewRegister(program);
               gen_orb_instruction(program, tmp, REG_0, R_ID(dstReg), CG_DIRECT(0, R_IND(dstReg)));
            }
            popInstrInsertionPoint(program);
         }
      }
   }

   finalizeGraph(cfg);
}

void doTargetSpecificTransformations(t_program_infos *program)
{
   rewriteLogicalOperations(program);
   fixFlagUsers(program);
   fixReadWrite(program);
   fixInstrOperands(program);
   insertRegisterAllocationConstraints(program);
}
