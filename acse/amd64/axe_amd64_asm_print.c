/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_amd64_asm_print.c
 * Formal Languages & Compilers Machine, 2007-2020
 */

#include "axe_target_asm_print.h"
#include "axe_target_info.h"
#include "axe_utils.h"

extern int errorcode;


void emitFunctionPrologue(t_program_infos *program, FILE *fp)
{
   fputs(
      "\tpush rbp\n"
      "\tmov rbp, rsp\n"
      "\tpush rbx\n"
      "\tpush r12\n"
      "\tpush r13\n"
      "\tpush r14\n"
      "\tpush r15\n"
      "\tsub rsp, 8\n", // keep stack aligned to 16-byte boundary
      fp);
}

void emitFunctionEpilogue(t_program_infos *program, FILE *fp)
{
   fputs(
      "\tadd rsp, 8\n"
      "\tpop r15\n"
      "\tpop r14\n"
      "\tpop r13\n"
      "\tpop r12\n"
      "\tpop rbx\n"
      "\tmov rsp, rbp\n"
      "\tpop rbp\n",
      fp);
}

const char *translateAMD64_regName_64bit(int rid)
{
   const char *registerNames[] = {
      "0", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
      "r11", "r12", "r13", "r14", "r15"};
   assert(rid >= 0 && rid < NUM_REGISTERS+1);
   return registerNames[rid];
}

const char *translateAMD64_regName(int rid)
{
   const char *registerNames[] = {
      "0", "eax", "ebx", "ecx", "edx", "esi", "edi", "r8d", "r9d", "r10d",
      "r11d", "r12d", "r13d", "r14d", "r15d"};
   assert(rid >= 0 && rid < NUM_REGISTERS+1);
   return registerNames[rid];
}

const char *translateAMD64_regName_8bit(int rid)
{
   const char *registerNames[] = {
      "0", "al", "bl", "cl", "dl", "sil", "dil", "r8b", "r9b", "r10b",
      "r11b", "r12b", "r13b", "r14b", "r15b"};
   assert(rid >= 0 && rid < NUM_REGISTERS+1);
   return registerNames[rid];
}

char *translateAMD64_regValOrPtr(t_axe_register *reg, char *dest, int bufSize)
{
   if (reg->indirect) {
      snprintf(dest, bufSize, "dword [%s]", translateAMD64_regName_64bit(reg->ID));
   } else {
      if (reg->type >= 0 && (reg->type & PTR_TYPE_FLAG))
         snprintf(dest, bufSize, "%s", translateAMD64_regName_64bit(reg->ID));
      else
         snprintf(dest, bufSize, "%s", translateAMD64_regName(reg->ID));
   }
   return dest;
}

char *translateLabel(t_axe_label *label, char *dest, int bufSize)
{
   if (!label) {
      if (bufSize >= 1) dest[0] = '\0'; 
      return NULL;
   }
   snprintf(dest, bufSize, "L%d", label->labelID);
   return dest;
}

char *translateLabelOrAddress(t_axe_address *address, char *dest, int bufSize)
{
   if (address->type == ADDRESS_TYPE) {
      snprintf(dest, bufSize, "%d", address->addr);
   } else {
      translateLabel(address->labelID, dest, bufSize);
   }
   return dest;
}

void emitCast(t_axe_register *reg, int newtype, FILE *fp)
{
   /* FIXME: handling casts in the backend is horrendously wrong for a
    * plentitude of reasons, but at the moment it is unavoidable because
    * there is no cast instruction in the IR. The problem is that we don't even
    * want that instruction in the IR because adding casts would necessarily 
    * snowball into changes of the API in axe_gencode, which we definitively do
    * not want to do. Thus, we will try to live with this hack and hope that
    * the assert won't trigger. */
   if (reg->type == newtype)
      return;
   if (newtype < 0)
      return;
   assert(!reg->indirect);
   if (reg->type == INTEGER_TYPE && newtype == INTEGER_PTR_TYPE) {
      reg->type = INTEGER_PTR_TYPE;
      fprintf(fp, "\tmovsx %s, %s\n", 
         translateAMD64_regName_64bit(reg->ID), 
         translateAMD64_regName(reg->ID)); 
   }
}

int translateAMD64_mov(t_program_infos *p, t_axe_instruction *instr, FILE *fp)
{
   t_axe_register *dest;
   t_axe_register *srcReg;
   t_axe_address *srcAddr;
   int srcImm;

   if (!isMoveInstruction(instr, &dest, &srcReg, &srcAddr, &srcImm))
      return 0;

   if (srcAddr) {
      char flabel[20];
      translateLabelOrAddress(instr->address, flabel, 20);
      if (instr->address->type == ADDRESS_TYPE) {
         fprintf(fp, "\tmov %s, %s\n", translateAMD64_regName_64bit(dest->ID), flabel);
      } else {
         fprintf(fp, "\tlea %s, [%s]\n", translateAMD64_regName_64bit(dest->ID), flabel);
      }
      return 1;
   }

   char rdest[20];
   translateAMD64_regValOrPtr(dest, rdest, 20);

   if (srcReg) {
      if (srcReg->ID == dest->ID && 
            srcReg->indirect == dest->indirect &&
            srcReg->type == dest->type)
         return 1;
      char rsrc[20];
      translateAMD64_regValOrPtr(srcReg, rsrc, 20);
      fprintf(fp, "\tmov %s, %s\n", rdest, rsrc);
      return 1;
   }

   fprintf(fp, "\tmov %s, %d\n", rdest, srcImm);
   return 1;
}

int translateAMD64_acseLOAD_acseSTORE(t_program_infos *p, t_axe_instruction *instr, FILE *fp)
{
   char address[20];
   translateLabelOrAddress(instr->address, address, 20);
   const char *reg = translateAMD64_regName(instr->reg_1->ID);

   if (instr->opcode == STORE) {
      fprintf(fp, "\tmov dword [%s], %s\n", address, reg);
   } else {
      fprintf(fp, "\tmov %s, dword [%s]\n", reg, address);
   }

   return 1;
}

int translateAMD64_acseScc(t_program_infos *p, t_axe_instruction *instr, FILE *fp)
{
   const char *amd64opc;
   switch (instr->opcode) {
      case SEQ:
         amd64opc = "sete";
         break;
      case SGE:
         amd64opc = "setge";
         break;
      case SGT:
         amd64opc = "setg";
         break;
      case SLE:
         amd64opc = "setle";
         break;
      case SLT:
         amd64opc = "setl";
         break;
      case SNE:
         amd64opc = "setne";
         break;
   }
   const char *byte = translateAMD64_regName_8bit(instr->reg_1->ID);
   const char *dword = translateAMD64_regName(instr->reg_1->ID);
   fprintf(fp, "\t%s %s\n\tmovzx %s, %s\n", amd64opc, byte, dword, byte);
   return 1;
}

int translateInstruction(t_program_infos *program, t_axe_instruction *instr, FILE *fp)
{
   if (instr->mcFlags & MCFLAG_DUMMY)
      return 1;

   if (instr->labelID) {
      char labelBuffer[20];
      translateLabel(instr->labelID, labelBuffer, 20);
      fprintf(fp, "%s:\n", labelBuffer);
   }

   if (translateAMD64_mov(program, instr, fp))
      return 1;

   char addrBuf[20];
   char rDstBuf[20];
   char rSrcBuf[20];

   t_axe_register *rDst = instr->reg_1;
   t_axe_register *rSrc = instr->reg_3 ?: instr->reg_2;

   if (rDst)
      translateAMD64_regValOrPtr(rDst, rDstBuf, 20);
   if (rSrc) {
      emitCast(rSrc, rDst->type, fp);
      translateAMD64_regValOrPtr(rSrc, rSrcBuf, 20);
   }

   t_axe_address *addr = instr->address;
   if (addr)
      translateLabelOrAddress(addr, addrBuf, 20);

   switch (instr->opcode) {
      case BF:
      case NOP:
         fprintf(fp, "\tnop\n");
         break;
      case ADD:
         fprintf(fp, "\tadd %s, %s\n", rDstBuf, rSrcBuf);
         break;
      case SUB:
         fprintf(fp, "\tsub %s, %s\n", rDstBuf, rSrcBuf);
         break;
      case MUL:
         assert(!rDst->indirect);
         fprintf(fp, "\timul %s, %s\n", rDstBuf, rSrcBuf);
         break;
      case DIV:
         assert(!rDst->indirect && rDst->ID == R_AMD64_EAX);
         fprintf(fp, "\tidiv %s\n", rSrcBuf);
         break;
      case ANDB:
         fprintf(fp, "\tand %s, %s\n", rDstBuf, rSrcBuf);
         break;
      case ORB:
         fprintf(fp, "\tor %s, %s\n", rDstBuf, rSrcBuf);
         break;
      case EORB:
         fprintf(fp, "\txor %s, %s\n", rDstBuf, rSrcBuf);
         break;
      case SHL:
         assert(!rSrc->indirect && rSrc->ID == R_AMD64_ECX);
         fprintf(fp, "\tsal %s, cl\n", rDstBuf);
         break;
      case SHR:
         assert(!rSrc->indirect && rSrc->ID == R_AMD64_ECX);
         fprintf(fp, "\tsar %s, cl\n", rDstBuf);
         break;
      case ROTL:
         assert(!rSrc->indirect && rSrc->ID == R_AMD64_ECX);
         fprintf(fp, "\trol %s, cl\n", rDstBuf);
         break;
      case ROTR:
         assert(!rSrc->indirect && rSrc->ID == R_AMD64_ECX);
         fprintf(fp, "\tror %s, cl\n", rDstBuf);
         break;
      case NEG:
         fprintf(fp, "\tneg %s\n", rDstBuf);
         break;
      case NOTB:
         fprintf(fp, "\tnot %s\n", rDstBuf);
         break;
      case ADDI:
         fprintf(fp, "\tadd %s, %d\n", rDstBuf, instr->immediate);
         break;
      case SUBI:
         fprintf(fp, "\tsub %s, %d\n", rDstBuf, instr->immediate);
         break;
      case MULI:
         fprintf(fp, "\timul %s, %s, %d\n", rDstBuf, rSrcBuf, instr->immediate);
         break;
      case ANDBI:
         fprintf(fp, "\tand %s, %d\n", rDstBuf, instr->immediate);
         break;
      case ORBI:
         fprintf(fp, "\tor %s, %d\n", rDstBuf, instr->immediate);
         break;
      case EORBI:
         fprintf(fp, "\txor %s, %d\n", rDstBuf, instr->immediate);
         break;
      case SHLI:
         fprintf(fp, "\tsal %s, %d\n", rDstBuf, instr->immediate);
         break;
      case SHRI:
         fprintf(fp, "\tsar %s, %d\n", rDstBuf, instr->immediate);
         break;
      case ROTLI:
         fprintf(fp, "\trol %s, %d\n", rDstBuf, instr->immediate);
         break;
      case ROTRI:
         fprintf(fp, "\tror %s, %d\n", rDstBuf, instr->immediate);
         break;
      case SEQ:
      case SGE:
      case SGT:
      case SLE:
      case SLT:
      case SNE:
         return translateAMD64_acseScc(program, instr, fp);
      case LOAD:
      case STORE:
         return translateAMD64_acseLOAD_acseSTORE(program, instr, fp);
      case BT:
         fprintf(fp, "\tjmp %s\n", addrBuf);
         break;
      case BHI:
         fprintf(fp, "\tja %s\n", addrBuf);
         break;
      case BLS:
         fprintf(fp, "\tjbe %s\n", addrBuf);
         break;
      case BCC:
         fprintf(fp, "\tjnc %s\n", addrBuf);
         break;
      case BCS:
         fprintf(fp, "\tjc %s\n", addrBuf);
         break;
      case BNE:
         fprintf(fp, "\tjne %s\n", addrBuf);
         break;
      case BEQ:
         fprintf(fp, "\tje %s\n", addrBuf);
         break;
      case BVC:
         fprintf(fp, "\tjno %s\n", addrBuf);
         break;
      case BVS:
         fprintf(fp, "\tjo %s\n", addrBuf);
         break;
      case BPL:
         fprintf(fp, "\tjns %s\n", addrBuf);
         break;
      case BMI:
         fprintf(fp, "\tjs %s\n", addrBuf);
         break;
      case BGE:
         fprintf(fp, "\tjge %s\n", addrBuf);
         break;
      case BLT:
         fprintf(fp, "\tjl %s\n", addrBuf);
         break;
      case BGT:
         fprintf(fp, "\tjg %s\n", addrBuf);
         break;
      case BLE:
         fprintf(fp, "\tjle %s\n", addrBuf);
         break;
      case RET:
      case HALT:
         emitFunctionEpilogue(program, fp);
         fprintf(fp, "\tret\n");
         break;
      case AXE_READ:
         fprintf(fp, "\tcall __axe_read\n");
         break;
      case AXE_WRITE:
         fprintf(fp, "\tcall __axe_write\n");
         break;
      case SPCL:
      case ANDL:
      case ORL:
      case EORL:
      case ANDLI:
      case ORLI:
      case EORLI:
      case NOTL:
      case DIVI:
         fprintf(stderr, "illegal x86_64 instruction %d\n", instr->opcode);
         assert(0 && "instr cannot be encoded in x86_64; bug in axe_target_transform.c");
      case JSR:
      default:
         fprintf(fp, "; FIXME unimpl opcode %d\n", instr->opcode);
         break;
   }
   
   return 1;
}

/* translate each instruction in his assembler symbolic representation */
void translateCodeSegment(t_program_infos *program, FILE *fp)
{
   t_list *current_element;
   t_axe_instruction *current_instruction;
   int _error;
   
   /* preconditions */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);

   if (program == NULL)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   }

   /* initialize the current_element */
   current_element = program->instructions;

   /* write the .text directive */
   if (current_element != NULL)
   {
      fputs(
         "section .text\n"
         "__lance_start:\n",
         fp);
      emitFunctionPrologue(program, fp);
   }

   while (current_element != NULL)
   {
      /* retrieve the current instruction */
      current_instruction = (t_axe_instruction *) LDATA(current_element);
      assert(current_instruction != NULL);
      assert(current_instruction->opcode != INVALID_OPCODE);

      if (!translateInstruction(program, current_instruction, fp)) {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      /* loop termination condition */
      current_element = LNEXT(current_element);
   }
}

int translateDataObject(t_axe_data *current_data, FILE *fp)
{
   int fprintf_error;

   /* assertions */
   assert (current_data->directiveType != DIR_INVALID);

   /* create a string identifier for the label */
   char labelBuf[20];
   char *lab = translateLabel(current_data->labelID, labelBuf, 20);
   if (lab) {
      fprintf_error = fprintf(fp, "%s:\t", lab);
   } else {
      fprintf_error = fprintf(fp, "\t");
   }

   /* test if an error occurred while executing the `fprintf' function */
   if (fprintf_error < 0)
      return 0;

   /* print the directive identifier */
   if (current_data->directiveType == DIR_WORD) {
      if (fprintf(fp, "dd ") < 0)
         return 0;

   } else if (current_data->directiveType == DIR_SPACE) {
      if (fprintf(fp, "resb ") < 0)
         return 0;
   }

   /* print the value associated with the directive */
   if (fprintf(fp, "%d\n", current_data->value) < 0)
      return 0;
      
   return 1;
}

void translateDataSegment(t_program_infos *program, FILE *fp)
{
   t_list *current_element;
   t_axe_data *current_data;
   int _error;
   int fprintf_error;
   
   /* preconditions */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);

   /* initialize the local variable `fprintf_error' */
   fprintf_error = 0;
   
   if (!program)
      goto fail; 

   if (!program->data)
      return;

   fprintf(fp, "section .bss\n");

   /* iterate all the elements inside the data segment */
   current_element = program->data;
   for (; current_element; current_element = LNEXT(current_element)) {
      /* retrieve the current data element */
      current_data = (t_axe_data *) LDATA(current_element);
      if (current_data->directiveType != DIR_SPACE)
         continue;

      if (!translateDataObject(current_data, fp)) 
         goto fail;
   }

   fprintf(fp, "section .data\n");

   current_element = program->data;
   for (; current_element; current_element = LNEXT(current_element)) {
      current_data = (t_axe_data *) LDATA(current_element);
      if (current_data->directiveType != DIR_WORD)
         continue;

      if (!translateDataObject(current_data, fp)) 
         goto fail;
   }

   return;

fail:
   _error = fclose(fp);
   if (_error == EOF)
      notifyError(AXE_FCLOSE_ERROR);
   notifyError(AXE_FWRITE_ERROR);
   return;
}

void writeAssembly(t_program_infos *program, char *output_file)
{
   FILE *fp;
   int _error;

   /* test the preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   /* If necessary, set the value of `output_file' to "output.asm" */
   if (output_file == NULL)
   {
      /* set "output.o" as output file name */
      output_file = "output.asm";
   }

#ifndef NDEBUG
   fprintf(stdout, "\n\n*******************************************\n");
   fprintf(stdout, "INITIALIZING OUTPUT FILE: %s. \n", output_file);
   fprintf(stdout, "CODE SEGMENT has a size of %d instructions \n"
         , getLength(program->instructions));
   fprintf(stdout, "DATA SEGMENT has a size of %d elements \n"
         , getLength(program->data));
   fprintf(stdout, "NUMBER OF LABELS : %d. \n"
         , get_number_of_labels(program->lmanager));
   fprintf(stdout, "*******************************************\n\n");
#endif
   
   /* open a new file */
   fp = fopen(output_file, "w");
   if (fp == NULL)
      notifyError(AXE_FOPEN_ERROR);

   fputs(
      "bits 64\n"
      "default rel\n"
      "global __lance_start\n"
      "extern __axe_read\n"
      "extern __axe_write\n",
      fp);

   /* print the data segment */
   translateDataSegment(program, fp);

   /* print the code segment */
   translateCodeSegment(program, fp);

   /* close the file and return */
   _error = fclose(fp);
   if (_error == EOF)
      notifyError(AXE_FCLOSE_ERROR);
}

