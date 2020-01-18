/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_struct.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_struct.h"

/* create an expression */
t_axe_expression create_expression (int value, int type)
{
   t_axe_expression expression;

   expression.value = value;
   expression.expression_type = type;

   return expression;
}

/* create and initialize an instance of `t_axe_register' */
t_axe_register * alloc_register(int ID, int indirect)
{
   t_axe_register *result;

   /* create an instance of `t_axe_register' */
   result = (t_axe_register *)
            malloc(sizeof(t_axe_register));
   
   /* check the postconditions */
   if (result == NULL)
      return NULL;

   /* initialize the new label */
   result->ID = ID;
   result->mcRegWhitelist = NULL;
   result->indirect = indirect;

   /* return the label */
   return result;
}

/* create and initialize an instance of `t_axe_instruction' */
t_axe_instruction * alloc_instruction(int opcode)
{
   t_axe_instruction *result;

   /* create an instance of `t_axe_data' */
   result = (t_axe_instruction *) malloc(sizeof(t_axe_instruction));
   
   /* check the postconditions */
   if (result == NULL)
      return NULL;

   /* ininitialize the fields of `result' */
   result->opcode = opcode;
   result->reg_1 = NULL;
   result->reg_2 = NULL;
   result->reg_3 = NULL;
   result->immediate = 0;
   result->labelID = NULL;
   result->address = NULL;
   result->user_comment = NULL;

   /* return `result' */
   return result;
}

/* create and initialize an instance of `t_axe_data' */
t_axe_data * alloc_data(int directiveType, int value, t_axe_label *label)
{
   t_axe_data *result;

   /* create an instance of `t_axe_data' */
   result = (t_axe_data *) malloc(sizeof(t_axe_data));
   
   /* check the postconditions */
   if (result == NULL)
      return NULL;

   /* initialize the new directive */
   result->directiveType = directiveType;
   result->value = value;
   result->labelID = label;

   /* return the new data */
   return result;
}

t_while_statement create_while_statement()
{
   t_while_statement statement;

   /* initialize the WHILE informations */
   statement.label_condition = NULL;
   statement.label_end = NULL;

   /* return a new instance of `t_while_statement' */
   return statement;
}

t_axe_label * alloc_label(int value)
{
   t_axe_label *result;

   /* create an instance of t_axe_label */
   result = (t_axe_label *)
         malloc(sizeof(t_axe_label));

   /* initialize the internal value of `result' */
   result->labelID = value;
   result->name = NULL;

   /* return the just initialized new instance of `t_axe_label' */
   return result;
}

void free_label(t_axe_label *lab)
{
   if (lab->name)
      free(lab->name);
   free(lab);
}

t_axe_declaration * alloc_declaration
      (char *ID, int isArray, int arraySize, int init_val)
{
   t_axe_declaration *result;

   /* allocate memory for the new declaration */
   result = (t_axe_declaration *)
         malloc(sizeof(t_axe_declaration));

   /* check the postconditions */
   if (result == NULL)
      return NULL;

   /* initialize the content of `result' */
   result->isArray = isArray;
   result->arraySize = arraySize;
   result->ID = ID;
   result->init_val = init_val;

   /* return the just created and initialized instance of t_axe_declaration */
   return result;
}

/* finalize an instance of `t_axe_variable' */
void free_variable (t_axe_variable *variable)
{
   free(variable);
}

/* create and initialize an instance of `t_axe_variable' */
t_axe_variable * alloc_variable
      (char *ID, int type, int isArray, int arraySize, int init_val)
{
   t_axe_variable *result;

   /* allocate memory for the new variable */
   result = (t_axe_variable *)
         malloc(sizeof(t_axe_variable));

   /* check the postconditions */
   if (result == NULL)
      return NULL;

   /* initialize the content of `result' */
   result->type = type;
   result->isArray = isArray;
   result->arraySize = arraySize;
   result->ID = ID;
   result->init_val = init_val;
   result->labelID = NULL;

   /* return the just created and initialized instance of t_axe_variable */
   return result;
}

/* finalize an instruction info. */
void free_Instruction(t_axe_instruction *inst)
{
   /* preconditions */
   if (inst == NULL)
      return;
   
   /* free memory */
   if (inst->reg_1 != NULL) {
      freeList(inst->reg_1->mcRegWhitelist);
      free(inst->reg_1);
   }
   if (inst->reg_2 != NULL) {
      freeList(inst->reg_2->mcRegWhitelist);
      free(inst->reg_2);
   }
   if (inst->reg_3 != NULL) {
      freeList(inst->reg_3->mcRegWhitelist);
      free(inst->reg_3);
   }
   if (inst->address != NULL) {
      free(inst->address);
   }
   if (inst->user_comment != NULL) {
      free(inst->user_comment);
   }

   free(inst);
}

/* finalize a data info. */
void free_Data(t_axe_data *data)
{
   if (data != NULL)
      free(data);
}

t_axe_address * alloc_address(int type, int address, t_axe_label *label)
{
   t_axe_address *result;

   result = (t_axe_address *)
         malloc(sizeof(t_axe_address));

   if (result == NULL)
      return NULL;

   /* initialize the new instance of `t_axe_address' */
   result->type = type;
   result->addr = address;
   result->labelID = label;

   /* return the new address */
   return result;
}
