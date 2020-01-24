#include "axe_target_transform.h"
#include "axe_gencode.h"

t_list *fix_destination_register_instr(t_program_infos *program,
                                       t_list *position)
{
   t_axe_instruction *instr = LDATA(position);
   assert(instr);
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

void doTargetSpecificTransformations(t_program_infos *program)
{
   fix_destination_register(program);
}
