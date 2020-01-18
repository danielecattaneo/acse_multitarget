#include "axe_target_asm_print.h"
#include "axe_target_info.h"

extern int errorcode;

/* Translate the assembler directives (definitions inside the data segment */
static void translateDataSegment(t_program_infos *program, FILE *fp);

/* Translate all the instructions within the code segment */
static void translateCodeSegment(t_program_infos *program, FILE *fp);


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

const char *translateAMD64_regName(int rid)
{
   const char *registerNames[] = {
      "0", "eax", "ebx", "ecx", "edx", "esi", "edi", "r8d", "r9d", "r10d",
      "r11d", "r12d", "r13d", "r14d", "r15d"};
   assert(rid >= 0 && rid < NUM_REGISTERS);
   return registerNames[rid];
}

int translateAMD64_mov(t_program_infos *p, t_axe_instruction *instr, FILE *fp)
{
   if ((instr->opcode == ADD && (instr->reg_2->ID == REG_0 || instr->reg_3->ID == REG_0)) ||
       (instr->opcode == SUB && instr->reg_3->ID == REG_0)) {
      int rsrc = instr->reg_2->ID == REG_0 ? instr->reg_3->ID : instr->reg_2->ID;
      int rdest = instr->reg_1->ID;
      fprintf(fp, "\tmov %s, %s\n", translateAMD64_regName(rdest), translateAMD64_regName(rsrc));
      return 0;
   }
   return 1;
}

int translateAMD64_acseMOVA(t_program_infos *p, t_axe_instruction *instr, FILE *fp)
{
   int rdest = instr->reg_1->ID;
   if (instr->address->type == ADDRESS_TYPE) {
      fprintf(fp, "\tmov %s, %d\n", translateAMD64_regName(rdest), instr->address->addr);
   } else {
      fprintf(fp, "\tlea %s, dword [L%d]\n", translateAMD64_regName(rdest), instr->address->labelID->labelID);
   }
   return 0;
}

int translateAMD64_acseLOAD_acseSTORE(t_program_infos *p, t_axe_instruction *instr, FILE *fp)
{
   char address[80];
   if (instr->address->type == ADDRESS_TYPE) {
      snprintf(address, 80, "%d", instr->address->addr);
   } else {
      snprintf(address, 80, "dword [L%d]", instr->address->labelID->labelID);
   }
   const char *reg = translateAMD64_regName(instr->reg_1->ID);

   if (instr->opcode == STORE) {
      fprintf(fp, "\tmov %s, %s\n", address, reg);
   } else {
      fprintf(fp, "\tmov %s, %s\n", reg, address);
   }

   return 0;
}

int translateInstruction(t_program_infos *program, t_axe_instruction *current_instruction, FILE *fp)
{
   if (current_instruction->mcFlags & MCFLAG_DUMMY)
      return 0;

   if (current_instruction->labelID != NULL) {
      fprintf(fp, "L%d:\n", (current_instruction->labelID)->labelID);
   }

   if (!translateAMD64_mov(program, current_instruction, fp))
      return 0;

   int rdest = current_instruction->reg_1 ? current_instruction->reg_1->ID : -1;
   int rsrc1 = current_instruction->reg_3 ? current_instruction->reg_3->ID : -1;
   switch (current_instruction->opcode) {
      case NOP:
         fprintf(fp, "\tnop\n");
         break;
      case ADD:
         fprintf(fp, "\tadd %s, %s\n", translateAMD64_regName(rdest), translateAMD64_regName(rsrc1));
         break;
      case SUB:
         fprintf(fp, "\tsub %s, %s\n", translateAMD64_regName(rdest), translateAMD64_regName(rsrc1));
         break;
      case ANDB:
         fprintf(fp, "\tand %s, %s\n", translateAMD64_regName(rdest), translateAMD64_regName(rsrc1));
         break;
      case ORB:
         fprintf(fp, "\tor %s, %s\n", translateAMD64_regName(rdest), translateAMD64_regName(rsrc1));
         break;
      case EORB:
         fprintf(fp, "\txor %s, %s\n", translateAMD64_regName(rdest), translateAMD64_regName(rsrc1));
         break;
      case NEG:
         fprintf(fp, "\tneg %s\n", translateAMD64_regName(rdest));
         break;
      case MOVA:
         return translateAMD64_acseMOVA(program, current_instruction, fp);
      case ADDI:
         fprintf(fp, "\tadd %s, %d\n", translateAMD64_regName(rdest), current_instruction->immediate);
         break;
      case SUBI:
         fprintf(fp, "\tsub %s, %d\n", translateAMD64_regName(rdest), current_instruction->immediate);
         break;
      case SHLI:
         fprintf(fp, "\tshl %s, %d\n", translateAMD64_regName(rdest), current_instruction->immediate);
         break;
      case SHRI:
         fprintf(fp, "\tshr %s, %d\n", translateAMD64_regName(rdest), current_instruction->immediate);
         break;
      case LOAD:
      case STORE:
         return translateAMD64_acseLOAD_acseSTORE(program, current_instruction, fp);
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
      case ANDL:
      case ORL:
      case EORL:
      case MUL:
      case DIV:
      case ROTL:
      case ROTR:
      case SPCL:
      case ANDLI:
      case ORLI:
      case EORLI:
      case ANDBI:
      case ORBI:
      case EORBI:
      case MULI:
      case DIVI:
      case ROTLI:
      case ROTRI:
      case NOTL:
      case NOTB:
      case JSR:
      case SEQ:
      case SGE:
      case SGT:
      case SLE:
      case SLT:
      case SNE:
      case BT:
      case BF:
      case BHI:
      case BLS:
      case BCC:
      case BCS:
      case BNE:
      case BEQ:
      case BVC:
      case BVS:
      case BPL:
      case BMI:
      case BGE:
      case BLT:
      case BGT:
      case BLE:
         fprintf(fp, "; FIXME unimpl opcode %d\n", current_instruction->opcode);
         break;
   }
   
   return 0;
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

      if (translateInstruction(program, current_instruction, fp)) {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      /* loop termination condition */
      current_element = LNEXT(current_element);
   }
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
   
   if (program == NULL)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);

      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   }

   /* initialize the value of `current_element' */
   current_element = program->data;

   /* write the .data directive */
   if (current_element != NULL)
   {
      fprintf(fp, "section .bss\n");
   }

   /* iterate all the elements inside the data segment */
   while (current_element != NULL)
   {
      /* retrieve the current data element */
      current_data = (t_axe_data *) LDATA(current_element);

      /* assertions */
      assert (current_data->directiveType != DIR_INVALID);

      /* create a string identifier for the label */
      if ( (current_data->labelID != NULL)
            && ((current_data->labelID)->labelID != LABEL_UNSPECIFIED) )
      {
         fprintf_error = fprintf(fp, "L%d:\t"
                  , (current_data->labelID)->labelID);
      }
      else
      {
         fprintf_error = fprintf(fp, "\t");
      }

      /* test if an error occurred while executing the `fprintf' function */
      if (fprintf_error < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      /* print the directive identifier */
      if (current_data->directiveType == DIR_WORD)
      {
         if (fprintf(fp, "dd ") < 0)
         {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }
      }
      
      else if (current_data->directiveType == DIR_SPACE)
      {
         if (fprintf(fp, "resb ") < 0)
         {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }
      }

      /* print the value associated with the directive */
      if (fprintf(fp, "%d\n", current_data->value) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      /* loop termination condition */
      current_element = LNEXT(current_element);
   }
}

