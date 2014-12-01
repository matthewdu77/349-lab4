/** @file main.c
 *
 * @brief kernel main
 *
 * @author 
 *	   
 *	   
 * @date   
 */

#include <kernel.h>
#include <task.h>
#include <sched.h>
#include <device.h>
#include <assert.h>
#include <arm/exception.h>
#include <arm/interrupt.h>
#include <arm/timer.h>
#include <arm/reg.h>

#include <constants.h>
#include "swi_handler.h"
#include "irq_handler.h"
#include "user_setup.h"

// 0xe59ff014 (LDR pc, [pc, 0x14]) --> 0x014 through masking
#define SWI_VECT_ADDR 0x08
#define PC_OFFSET 0x08
// Cannot write to this address. kernel.bin loaded here. Stack grows down.
#define USER_STACK_TOP 0xa3000000

// (LDR pc, [pc, 0x000]) 0xe59ff000 --> 0x000 through masking
#define LDR_PC_PC_INSTR 0xe59ff000
#define LDR_SIGN_MASK 0x00800000

uint32_t global_data;

/* Checks the Vector Table. */
bool check_vector(int exception_num)
{
  int vector_instr = *((int *)(exception_num * 4));

  // Check if the offset is negative.
  if ((vector_instr & LDR_SIGN_MASK) == 0)
  {
    return false;
  }

  // Check that the instruction is a (LDR pc, [pc, 0x000])
  if ((vector_instr & 0xFFFFF000) != LDR_PC_PC_INSTR)
  {
    return false;
  }

  return true;
}

void restore_exception_handler(int exception_num, int* old_instructions)
{
  int vector_address = exception_num * 4;

  // Jump offset already incorporates PC offset. Usually 0x10 or 0x14.
  int jmp_offset = (*((int *) vector_address))&(0xFFF);

  // &Handler" in Jump Table.
  int *handler_addr = *(int **)(vector_address + PC_OFFSET + jmp_offset);

  // Restore original Uboot handler instructions.
  *handler_addr = old_instructions[0];
  *(handler_addr + 1) = old_instructions[1];
}

void _install_exception_handler(int exception_num, int new_handler, int* old_instructions)
{
  int vector_address = exception_num * 4;

  // Jump offset already incorporates PC offset. Usually 0x10 or 0x14.
  int jmp_offset = (*((int *) vector_address))&(0xFFF);

  // &Handler" in Jump Table.
  int *handler_addr = *(int **)(vector_address + PC_OFFSET + jmp_offset);

  // Save original Uboot handler instructions.
  old_instructions[0] = *handler_addr;
  old_instructions[1] = *(handler_addr + 1);

  // Wire in our own: LDR pc, [pc, #-4] = 0xe51ff004
  *handler_addr = 0xe51ff004;
  *(handler_addr + 1) = new_handler;
}


int kmain(int argc __attribute__((unused)), char** argv  __attribute__((unused)), uint32_t table)
{

  app_startup();
  global_data = table;
  /* add your code up to assert statement */

  // Installs the swi handler
  if (check_vector(EX_SWI) == false)
  {
    return BAD_CODE;
  }
  int swi_instrs[2] = {0};
  _install_exception_handler(EX_SWI, (int) &swi_handler, swi_instrs);

  // Installs the irq handler
  if (check_vector(EX_IRQ) == false)
  {
    return BAD_CODE;
  }
  int irq_instrs[2] = {0};
  _install_exception_handler(EX_IRQ, (int) &irq_handler, irq_instrs);

  // Copy argc and argv to user stack in the right order.
  int *spTop = ((int *) USER_STACK_TOP) - 1;
  int i = 0;
  for (i = argc-1; i >= 0; i--)
  {
    *spTop = (int)argv[i];
    spTop--;
  }
  *spTop = argc;
  uint32_t ostmr1_bit = 1 << INT_OSTMR_0;
  // activates interrupts on the timer match register
  reg_set(INT_ICMR_ADDR, ostmr1_bit);
  reg_clear(INT_ICCR_ADDR, ostmr1_bit);
  reg_set(OSTMR_OIER_ADDR, OSTMR_OIER_E0);

  // resets clock to 0 and schedules an interrupt in 10 ms
  reg_write(OSTMR_OSCR_ADDR, 0);
  uint32_t next_time = OSTMR_FREQ/100;
  reg_write(OSTMR_OSMR_ADDR(0), next_time);

  /** Jump to user program. **/
  user_setup(spTop);

  assert(0);        /* should never get here */
}
