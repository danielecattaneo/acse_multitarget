/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_debug.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_debug.h"
#include "collections.h"
#include "reg_alloc_constants.h"
#include "cflow_constants.h"

#define LABEL_WIDTH (3*2)
#define INSTR_WIDTH (3*7)

static void printArrayOfVariables(t_cflow_var **array, int size, FILE *fout);
static void printListOfVariables(t_list *variables, FILE *fout);
static void printCFlowGraphVariable(t_cflow_var *var, FILE *fout);
static void printBBlockInfos(t_basic_block *block, FILE *fout, int verbose);
static void printLiveIntervals(t_list *intervals, FILE *fout);
static void printBindings(int *bindings, int numVars, FILE *fout);
static void printLabel(t_axe_label *label, int printInline, FILE *fout);
static off_t printFormPadding(off_t formBegin, int formSize, FILE *fout);

void printBindings(int *bindings, int numVars, FILE *fout)
{
   int counter;
   
   if (bindings == NULL)
      return;

   if (fout == NULL)
      return;

   fprintf(fout, "BINDINGS : \n");
   for (counter = 0; counter < numVars; counter++) {
      if (bindings[counter] != RA_SPILL_REQUIRED)
      {
         fprintf(fout, "VAR T%d will be assigned to register R%d \n"
                  , counter, bindings[counter]);
      }
      else
      {
         fprintf(fout, "VAR T%d will be spilled \n", counter);
      }
   }

   fflush(fout);
}

void printRegAllocInfos(t_reg_allocator *RA, FILE *fout)
{
   if (RA == NULL)
      return;
   if (fout == NULL)
      return;
   fprintf(fout, "\n\n*************************\n");
   fprintf(fout, "REGISTER ALLOCATION INFOS\n");
   fprintf(fout, "*************************\n");
   fprintf(fout, "AVAILABLE REGISTERS : %d \n", RA->regNum + 3);
   fprintf(fout, "USED VARIABLES : %d \n", RA->varNum);
   fprintf(fout, "-------------------------\n");
   printLiveIntervals(RA->live_intervals, fout);
   fprintf(fout, "-------------------------\n");
   printBindings(RA->bindings, RA->varNum, fout);
   fprintf(fout, "*************************\n\n");
   fflush(fout);
}

void printLiveIntervals(t_list *intervals, FILE *fout)
{
   t_list *current_element;
   t_live_interval *interval;

   /* precondition */
   if (fout == NULL)
      return;

   fprintf(fout, "LIVE_INTERVALS:\n");

   /* retireve the first element of the list */
   current_element = intervals;
   while (current_element != NULL)
   {
      interval = (t_live_interval *) LDATA(current_element);

      fprintf(fout, "\tLIVE_INTERVAL of T%d : [%d, %d] \n"
            , interval->varID, interval->startPoint, interval->endPoint);
      
      /* retrieve the next element in the list of intervals */
      current_element = LNEXT(current_element);
   }
   fflush(fout);
}

void printBBlockInfos(t_basic_block *block, FILE *fout, int verbose)
{
   t_list *current_element;
   t_cflow_Node *current_node;
   int count;
   
   /* preconditions */
   if (block == NULL)
      return;
   if (fout == NULL)
      return;

   fprintf(fout,"NUMBER OF PREDECESSORS : %d \n"
         , getLength(block->pred) );
   fprintf(fout,"NUMBER OF SUCCESSORS : %d \n"
         , getLength(block->succ) );
   fprintf(fout,"NUMBER OF INSTRUCTIONS : %d \n"
         , getLength(block->nodes) );

   count = 1;
   current_element = block->nodes;
   while(current_element != NULL)
   {
      current_node = (t_cflow_Node *) LDATA(current_element);
      fprintf(fout,"\t%d.  ", count);
      debug_printInstruction(current_node->instr, fout);
      if (verbose != 0)
      {
         fprintf(fout, "\n\t\t\tDEFS = [");
         printArrayOfVariables(current_node->defs, CFLOW_MAX_DEFS, fout);
         fprintf(fout, "]");
         fprintf(fout, "\n\t\t\tUSES = [");
         printArrayOfVariables(current_node->uses, CFLOW_MAX_USES, fout);
         fprintf(fout, "]");

         fprintf(fout, "\n\t\t\tLIVE IN = [");
         printListOfVariables(current_node->in, fout);
         fprintf(fout, "]");
         fprintf(fout, "\n\t\t\tLIVE OUT = [");
         printListOfVariables(current_node->out, fout);
         fprintf(fout, "]");
      }
      
      fprintf(fout, "\n");
      count++;
      current_element = LNEXT(current_element);
   }
   fflush(fout);
}

void printArrayOfVariables(t_cflow_var **array, int size, FILE *fout)
{
   int foundVariables = 0;
   int i;
   
   for (i=0; i<size; i++) {
      if (!(array[i]))
         continue;
         
      if (foundVariables > 0)
         fprintf(fout, ", ");
         
      printCFlowGraphVariable(array[i], fout);
      foundVariables++;
   }
   
   fflush(fout);
}

void printListOfVariables(t_list *variables, FILE *fout)
{
   t_list *current_element;
   t_cflow_var *current_variable;
   
   if (variables == NULL)
      return;
   if (fout == NULL)
      return;

   current_element = variables;
   while(current_element != NULL)
   {
      current_variable = (t_cflow_var *) LDATA(current_element);
      printCFlowGraphVariable(current_variable, fout);
      if (LNEXT(current_element) != NULL)
         fprintf(fout, ", ");
      
      current_element = LNEXT(current_element);
   }
   fflush(fout);
}

void printCFlowGraphVariable(t_cflow_var *var, FILE *fout)
{
   if (var->ID == VAR_PSW)
      fprintf(fout, "PSW");
   else if (var->ID == VAR_UNDEFINED)
      fprintf(fout, "<!UNDEF!>");
   else
      fprintf(fout, "R%d", var->ID);
}

void printGraphInfos(t_cflow_Graph *graph, FILE *fout, int verbose)
{
   int counter;
   t_list *current_element;
   t_basic_block *current_bblock;
   
   /* preconditions */
   if (graph == NULL)
      return;
   if (fout == NULL)
      return;

   /* initialization of the local variables */
   counter = 1;
   
   fprintf(fout,"NOTE : Temporary registers are considered as\n"
                "       variables of the intermediate language. \n");
#if CFLOW_ALWAYS_LIVEIN_R0 == (1)
   fprintf(fout,"       Variable \'R0\' (that refers to the \n"
                "       physical register \'RO\') is always \n"
                "       considered LIVE-IN for each node of \n"
                "       a basic block. \n"
                "       Thus, in the following control flow graph, \n"
                "       \'R0\' will never appear as LIVE-IN or LIVE-OUT\n"
                "       variable for a statement.\n\n"
                "       If you want to consider \'R0\' as\n"
                "       a normal variable, you have to set\n"
                "       to 0 the value of the macro CFLOW_ALWAYS_LIVEIN_R0\n"
                "       defined in \"cflow_constants.h\".\n\n");
#endif
   fprintf(fout,"\n");
   fprintf(fout,"**************************\n");
   fprintf(fout,"     CONTROL FLOW GRAPH   \n");
   fprintf(fout,"**************************\n");
   fprintf(fout,"NUMBER OF BASIC BLOCKS : %d \n"
         , getLength(graph->blocks));
   fprintf(fout,"NUMBER OF USED VARIABLES : %d \n"
         , getLength(graph->cflow_variables));
   fprintf(fout,"--------------------------\n");
   fprintf(fout,"START BASIC BLOCK INFOS.  \n");
   fprintf(fout,"--------------------------\n");

   /* initialize `current_block' */
   current_element = graph->blocks;
   while(current_element != NULL)
   {
      current_bblock = (t_basic_block *) LDATA(current_element);
      fprintf(fout,"[BLOCK %d] \n", counter);
      printBBlockInfos(current_bblock, fout, verbose);
      if (LNEXT(current_element) != NULL)
         fprintf(fout,"--------------------------\n");
      else
         fprintf(fout,"**************************\n");

      counter++;
      current_element = LNEXT(current_element);
   }
   
   fprintf(fout,"\n\n");
   fflush(fout);
}

void printProgramInfos(t_program_infos *program, FILE *fout)
{
   fprintf(fout,"**************************\n");
   fprintf(fout,"          PROGRAM         \n");
   fprintf(fout,"**************************\n\n");

   fprintf(fout,"-----------\n");
   fprintf(fout," VARIABLES\n");
   fprintf(fout,"-----------\n");
   t_list *cur_var = program->variables;
   while (cur_var) {
      t_axe_variable *var = LDATA(cur_var);
      fprintf(fout, "[%s]\n", var->ID);

      fprintf(fout, "   type = %s", dataTypeToString(var->type));
      if (var->isArray) {
         fprintf(fout, ", array size = %d", var->arraySize);
      } else {
         fprintf(fout, ", scalar initial value = %d", var->init_val);
      }
      fprintf(fout, "\n");

      fprintf(fout, "   label = ");
      printLabel(var->labelID, 0, fout);
      fprintf(fout, "\n");

      fprintf(fout, "   location = ");
      int sy_error;
      int reg = getLocation(program->sy_table, var->ID, &sy_error);
      if (reg == SY_LOCATION_UNSPECIFIED)
         fprintf(fout, "N/A");
      else
         fprintf(fout, "R%d", reg);
      fprintf(fout, "\n");

      cur_var = LNEXT(cur_var);
   }

   fprintf(fout,"\n--------------\n");
   fprintf(fout," INSTRUCTIONS\n");
   fprintf(fout,"--------------\n");
   t_list *cur_inst = program->instructions;
   while (cur_inst) {
      t_axe_instruction *instr = LDATA(cur_inst);
      debug_printInstruction(instr, fout);
      fprintf(fout, "\n");
      cur_inst = LNEXT(cur_inst);
   }

   fflush(fout);
}

void debug_printInstruction(t_axe_instruction *instr, FILE *fout)
{
   off_t formBegin = ftello(fout);

   /* preconditions */
   if (fout == NULL)
      return;
   
   if (instr == NULL)
   {
      fprintf(fout, "[NULL]\n");
      return;
   }

   if (instr->labelID != NULL)
      printLabel(instr->labelID, 1, fout);
   formBegin = printFormPadding(formBegin, LABEL_WIDTH, fout);
   
   switch(instr->opcode)
   {
      case ADD : fprintf(fout, "ADD "); break;
      case SUB : fprintf(fout, "SUB "); break;
      case ANDL : fprintf(fout, "ANDL "); break;
      case ORL : fprintf(fout, "ORL "); break;
      case EORL : fprintf(fout, "EORL "); break;
      case ANDB : fprintf(fout, "ANDB "); break;
      case ORB : fprintf(fout, "ORB "); break;
      case EORB : fprintf(fout, "EORB "); break;
      case MUL : fprintf(fout, "MUL "); break;
      case DIV : fprintf(fout, "DIV "); break;
      case SHL : fprintf(fout, "SHL "); break;
      case SHR : fprintf(fout, "SHR "); break;
      case ROTL : fprintf(fout, "ROTL "); break;
      case ROTR : fprintf(fout, "ROTR "); break;
      case NEG : fprintf(fout, "NEG "); break;
      case SPCL : fprintf(fout, "SPCL "); break;
      case ADDI : fprintf(fout, "ADDI "); break;
      case SUBI : fprintf(fout, "SUBI "); break;
      case ANDLI : fprintf(fout, "ANDLI "); break;
      case ORLI : fprintf(fout, "ORLI "); break;
      case EORLI : fprintf(fout, "EORLI "); break;
      case ANDBI : fprintf(fout, "ANDBI "); break;
      case ORBI : fprintf(fout, "ORBI "); break;
      case EORBI : fprintf(fout, "EORBI "); break;
      case MULI : fprintf(fout, "MULI "); break;
      case DIVI : fprintf(fout, "DIVI "); break;
      case SHLI : fprintf(fout, "SHLI "); break;
      case SHRI : fprintf(fout, "SHRI "); break;
      case ROTLI : fprintf(fout, "ROTLI "); break;
      case ROTRI : fprintf(fout, "ROTRI "); break;
      case NOTL : fprintf(fout, "NOTL "); break;
      case NOTB : fprintf(fout, "NOTB "); break;
      case NOP : fprintf(fout, "NOP "); break;
      case MOVA : fprintf(fout, "MOVA "); break;
      case JSR : fprintf(fout, "JSR "); break;
      case RET : fprintf(fout, "RET "); break;
      case HALT : fprintf(fout, "HALT "); break;
      case SEQ : fprintf(fout, "SEQ "); break;
      case SGE : fprintf(fout, "SGE "); break;
      case SGT : fprintf(fout, "SGT "); break;
      case SLE : fprintf(fout, "SLE "); break;
      case SLT : fprintf(fout, "SLT "); break;
      case SNE : fprintf(fout, "SNE "); break;
      case BT : fprintf(fout, "BT "); break;
      case BF : fprintf(fout, "BF "); break;
      case BHI : fprintf(fout, "BHI "); break;
      case BLS : fprintf(fout, "BLS "); break;
      case BCC : fprintf(fout, "BCC "); break;
      case BCS : fprintf(fout, "BCS "); break;
      case BNE : fprintf(fout, "BNE "); break;
      case BEQ : fprintf(fout, "BEQ "); break;
      case BVC : fprintf(fout, "BVC "); break;
      case BVS : fprintf(fout, "BVS "); break;
      case BPL : fprintf(fout, "BPL "); break;
      case BMI : fprintf(fout, "BMI "); break;
      case BGE : fprintf(fout, "BGE "); break;
      case BLT : fprintf(fout, "BLT "); break;
      case BGT : fprintf(fout, "BGT "); break;
      case BLE : fprintf(fout, "BLE "); break;
      case LOAD : fprintf(fout, "LOAD "); break;
      case STORE : fprintf(fout, "STORE "); break;
      case AXE_READ : fprintf(fout, "READ "); break;
      case AXE_WRITE : fprintf(fout, "WRITE "); break;
      case INVALID_OPCODE : fprintf(fout, "[INVALID] ");
   }

   if (instr->reg_1 != NULL)
   {
      if (!(instr->reg_1)->indirect)
         fprintf(fout, "R%d ", (instr->reg_1)->ID);
      else
         fprintf(fout, "(R%d) ", (instr->reg_1)->ID);
   }
   if (instr->reg_2 != NULL)
   {
      if (!(instr->reg_2)->indirect)
         fprintf(fout, "R%d ", (instr->reg_2)->ID);
      else
         fprintf(fout, "(R%d) ", (instr->reg_2)->ID);
      if (instr->reg_3 != NULL)
      {
         if (!(instr->reg_3)->indirect)
            fprintf(fout, "R%d ", (instr->reg_3)->ID);
         else
            fprintf(fout, "(R%d) ", (instr->reg_3)->ID);
      }
      else
         fprintf(fout, "#%d ", instr->immediate);
   }
   
   if (instr->address != NULL)
   {
      if ((instr->address)->type == LABEL_TYPE)
         printLabel(instr->address->labelID, 1, fout);
      else
         fprintf(fout, "%d", (instr->address)->addr);
   }

   if (instr->user_comment) {
      printFormPadding(formBegin, INSTR_WIDTH, fout);
      fprintf(fout, "/* %s */", instr->user_comment);
   }
}

char * dataTypeToString(int codedType)
{
   switch (codedType)
   {
      case INTEGER_TYPE : return "INTEGER";
      default : return "<INVALID_TYPE>";
   }
}

void printLabel(t_axe_label *label, int printInline, FILE *fout)
{
   if (printInline) {
      if (!label->name)
         fprintf(fout, "L%d", label->labelID);
      else
         fprintf(fout, "%s", label->name);
   } else {
      if (!label->name)
         fprintf(fout, "L%d", label->labelID);
      else
         fprintf(fout, "%s (ID=%d)", label->name, label->labelID);
   }
}

off_t printFormPadding(off_t formBegin, int formSize, FILE *fout)
{
   off_t currentLoc = ftello(fout);
   off_t padding = formSize - (currentLoc - formBegin);
   if (padding > 1) {
      off_t i;
      for (i = 0; i < padding - 1; i++) {
         putc(' ', fout);
      }
   }
   putc(' ', fout);
   return ftello(fout);
}
