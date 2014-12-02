/** @file sched.c
 * 
 * @brief Top level implementation of the scheduler.
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-20
 */

#include <types.h>
#include <assert.h>

#include <kernel.h>
#include <config.h>
#include "sched_i.h"

#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/physmem.h>

// idle stack (None of this should ever be used, so we make it small)
#define IDLE_STACK_SIZE 32

uint32_t idle_stack[IDLE_STACK_SIZE];

tcb_t system_tcb[OS_MAX_TASKS]; /*allocate memory for system TCBs */

void sched_init(task_t* main_task  __attribute__((unused)))
{
  //TODO: figure out what this does
}

/**
 * @brief This is the idle task that the system runs when no other task is runnable
 */
static void idle(void)
{
  enable_interrupts();
  while(1);
}

void request_reschedule()
{
  dispatch_save();
}

/**
 * @brief Allocate user-stacks and initializes the kernel contexts of the
 * given threads.
 *
 * This function assumes that:
 * - num_tasks < number of tasks allowed on the system.
 * - the tasks have already been deemed schedulable and have been appropriately
 *   scheduled.  In particular, this means that the task list is sorted in order
 *   of priority -- higher priority tasks come first.
 *
 * @param tasks  A list of scheduled task descriptors.
 * @param size   The number of tasks is the list.
 */
void allocate_tasks(task_t** tasks, size_t num_tasks)
{
  runqueue_init();

  size_t i;
	for (i = 0; i < num_tasks; i++)
  {
    task_t* task = tasks[i];
    tcb_t* tcb = &system_tcb[i];
    sched_context_t* context = &(tcb->context);
 
    tcb->native_prio = i;
    tcb->cur_prio = tcb->native_prio;
    tcb->holds_lock = 0;
    tcb->sleep_queue = 0;

    context->r4 = (int)task->lambda; // entry
    context->r5 = (int)task->data; // argument
    context->r6 = (int)task->stack_pos; // usr mode stack pointer
    context->sp = &tcb->kstack_high; // svc mode stack pointer
    context->lr = &launch_task; // kernel load code

    runqueue_add(tcb, i);
  }

  i = OS_MAX_TASKS-1;
  tcb_t* tcb = &system_tcb[i];
  sched_context_t* context = &(tcb->context);

  tcb->native_prio = i;
  tcb->cur_prio = tcb->native_prio;
  tcb->holds_lock = 0;
  tcb->sleep_queue = 0;

  context->r4 = (int)&idle; // entry
  context->r6 = (int)idle_stack; // usr mode stack pointer
  context->sp = &tcb->kstack_high; // svc mode stack pointer
  context->lr = &launch_task; // kernel load code
  runqueue_add(tcb, i);

  // sets the current task to the idle task
  dispatch_init(tcb);
}

