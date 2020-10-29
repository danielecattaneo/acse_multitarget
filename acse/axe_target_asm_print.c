/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_mace_asm_print.h
 * Formal Languages & Compilers Machine, 2007-2020
 */

#include "axe_target_asm_print.h"

extern int errorcode;

/* print out a label to the file `fp' */
void printLabel(t_axe_label *label, FILE *fp);

/* print out an opcode to the file `fp' */
static void printOpcode(int opcode, FILE *fp);

/* print out a register information to the file `fp' */
static void printRegister(t_axe_register *reg, FILE *fp);

/* Translate the assembler directives (definitions inside the data segment) */
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
      /* set "output.asm" as output file name */
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

   /* print the data segment */
   translateDataSegment(program, fp);

   /* print the code segment */
   translateCodeSegment(program, fp);

   /* close the file and return */
   _error = fclose(fp);
   if (_error == EOF)
      notifyError(AXE_FCLOSE_ERROR);
}

/* translate each instruction in his assembler symbolic representation */
int translateInstruction(t_program_infos *program, t_axe_instruction *current_instruction, FILE *fp)
{
   if (current_instruction->labelID != NULL)
   {
      printLabel(current_instruction->labelID, fp);
      fprintf(fp, ":\t");
   }
   else
   {
      /* create a string identifier for the label */
      if (fprintf(fp, "\t") < 0)
      {
         return 1;
      }
   }

   /* print the opcode */
   printOpcode(current_instruction->opcode, fp);

   if (  (current_instruction->opcode == HALT)
         || (current_instruction->opcode == NOP) )
   {
      /* do nothing */
   }
   else
   {
      if (fputc(' ', fp) == EOF)
      {
         return 1;
      }

      if (current_instruction->reg_1 != NULL)
      {
         printRegister(current_instruction->reg_1, fp);
         
         if (fputc(' ', fp) == EOF)
         {
            return 1;
         }
      }
      if (current_instruction->reg_2 != NULL)
      {
         printRegister(current_instruction->reg_2, fp);
         if (errorcode != AXE_OK)
            return 1;

         if (fputc(' ', fp) == EOF)
         {
            return 1;
         }
      }
      if (current_instruction->reg_3 != NULL)
      {
         printRegister(current_instruction->reg_3, fp);
      }
      else if (current_instruction->address != NULL)
      {
         if ((current_instruction->address)->type == ADDRESS_TYPE)
         {
            if (fprintf(fp, "%d", (current_instruction->address)->addr) < 0)
            {
               return 1;
            }
         }
         else
         {
            assert((current_instruction->address)->type == LABEL_TYPE);
            printLabel((current_instruction->address)->labelID, fp);
         }
      }
      else if (fprintf(fp, "#%d", current_instruction->immediate) < 0)
      {
         return 1;
      }
   }

   if (current_instruction->user_comment)
   {
      fprintf(fp, "\t\t/* %s */", current_instruction->user_comment);
   }

   if (fprintf(fp, "\n") < 0) {
      return 1;
   }
   return 0;
}

/* translate each instruction in its assembler symbolic representation */
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
      if (fprintf(fp, "\t.text\n") < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
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
      if (fprintf(fp, "\t.data\n") < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
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
         printLabel(current_data->labelID, fp);
         fprintf_error = fprintf(fp, ":\t");
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
         if (fprintf(fp, ".WORD ") < 0)
         {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }
      }
      
      else if (current_data->directiveType == DIR_SPACE)
      {
         if (fprintf(fp, ".SPACE ") < 0)
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

void printOpcode(int opcode, FILE *fp)
{
   char *opcode_to_string;
   int _error;
   
   /* preconditions: fp must be different from NULL */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);

   switch(opcode)
   {
      case ADD : opcode_to_string = "ADD"; break;
      case SUB : opcode_to_string = "SUB"; break;
      case ANDL : opcode_to_string = "ANDL"; break;
      case ORL : opcode_to_string = "ORL"; break;
      case EORL : opcode_to_string = "EORL"; break;
      case ANDB : opcode_to_string = "ANDB"; break;
      case ORB : opcode_to_string = "ORB"; break;
      case EORB : opcode_to_string = "EORB"; break;
      case MUL : opcode_to_string = "MUL"; break;
      case DIV : opcode_to_string = "DIV"; break;
      case SHL : opcode_to_string = "SHL"; break;
      case SHR : opcode_to_string = "SHR"; break;
      case ROTL : opcode_to_string = "ROTL"; break;
      case ROTR : opcode_to_string = "ROTR"; break;
      case NEG : opcode_to_string = "NEG"; break;
      case SPCL : opcode_to_string = "SPCL"; break;
      case ADDI : opcode_to_string = "ADDI"; break;
      case SUBI : opcode_to_string = "SUBI"; break;
      case ANDLI : opcode_to_string = "ANDLI"; break;
      case ORLI : opcode_to_string = "ORLI"; break;
      case EORLI : opcode_to_string = "EORLI"; break;
      case ANDBI : opcode_to_string = "ANDBI"; break;
      case ORBI : opcode_to_string = "ORBI"; break;
      case EORBI : opcode_to_string = "EORBI"; break;
      case MULI : opcode_to_string = "MULI"; break;
      case DIVI : opcode_to_string = "DIVI"; break;
      case SHLI : opcode_to_string = "SHLI"; break;
      case SHRI : opcode_to_string = "SHRI"; break;
      case ROTLI : opcode_to_string = "ROTLI"; break;
      case ROTRI : opcode_to_string = "ROTRI"; break;
      case NOTL : opcode_to_string = "NOTL"; break;
      case NOTB : opcode_to_string = "NOTB"; break;
      case NOP : opcode_to_string = "NOP"; break;
      case MOVA : opcode_to_string = "MOVA"; break;
      case JSR : opcode_to_string = "JSR"; break;
      case RET : opcode_to_string = "RET"; break;
      case HALT : opcode_to_string = "HALT"; break;
      case BT : opcode_to_string = "BT"; break;
      case BF : opcode_to_string = "BF"; break;
      case BHI : opcode_to_string = "BHI"; break;
      case BLS : opcode_to_string = "BLS"; break;
      case BCC : opcode_to_string = "BCC"; break;
      case BCS : opcode_to_string = "BCS"; break;
      case BNE : opcode_to_string = "BNE"; break;
      case BEQ : opcode_to_string = "BEQ"; break;
      case BVC : opcode_to_string = "BVC"; break;
      case BVS : opcode_to_string = "BVS"; break;
      case BPL : opcode_to_string = "BPL"; break;
      case BMI : opcode_to_string = "BMI"; break;
      case BGE : opcode_to_string = "BGE"; break;
      case BLT : opcode_to_string = "BLT"; break;
      case BGT : opcode_to_string = "BGT"; break;
      case BLE : opcode_to_string = "BLE"; break;
      case LOAD : opcode_to_string = "LOAD"; break;
      case STORE : opcode_to_string = "STORE"; break;
      case SEQ : opcode_to_string = "SEQ"; break;
      case SGE : opcode_to_string = "SGE"; break;
      case SGT : opcode_to_string = "SGT"; break;
      case SLE : opcode_to_string = "SLE"; break;
      case SLT : opcode_to_string = "SLT"; break;
      case SNE : opcode_to_string = "SNE"; break;
      case AXE_READ : opcode_to_string = "READ"; break;
      case AXE_WRITE : opcode_to_string = "WRITE"; break;
      default :
         /* close the file and return */
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_INVALID_OPCODE);
         return;
   }
      
   /* postconditions */
   if (fprintf(fp, "%s", opcode_to_string) < 0)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_FWRITE_ERROR);
   }
}

void printLabel(t_axe_label *label, FILE *fp)
{
   if (!label || label->labelID == LABEL_UNSPECIFIED)
      return;
   
   if (label->name) {
      fputs(label->name, fp);
   } else {
      fprintf(fp, "L%d", label->labelID);
   }
}

void printRegister(t_axe_register *reg, FILE *fp)
{
   int _error;
   
   /* preconditions: fp must be different from NULL */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);
   if (reg == NULL)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_INVALID_REGISTER_INFO);
   }
   if (reg->ID == REG_INVALID)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_INVALID_REGISTER_INFO);
   }

   if (reg->indirect)
   {
      if (fprintf(fp, "(R%d)", reg->ID) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }
   else
   {
      if (fprintf(fp, "R%d", reg->ID) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }
}


