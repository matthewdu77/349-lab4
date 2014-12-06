/** @file proc.c
 * 
 * @brief Implementation of `process' syscalls
 *
 * @author Mike Kasick <mkasick@andrew.cmu.edu>
 * @date   Sun, 14 Oct 2007 00:07:38 -0400
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-12
 */

#include <exports.h>
#include <bits/errno.h>
#include <config.h>
#include <kernel.h>
#include <syscall.h>
#include <sched.h>

#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/physmem.h>
#include <device.h>

int tasks_created = 0;

int task_create(task_t* tasks, size_t num_tasks)
{
  size_t i;

  if (num_tasks > OS_MAX_TASKS - 2)
  	return -EINVAL;

  if (!valid_addr(tasks, sizeof(task_t) * num_tasks, USR_START_ADDR, USR_END_ADDR))
  	return -EFAULT;

  task_t *(task_array[OS_MAX_TASKS]);
  for (i = 0; i < num_tasks; i++)
  {
    task_array[i] = &(tasks[i]);
  }

  if (!assign_schedule(task_array, num_tasks))
  {
    return -ESCHED;
  }

  disable_interrupts();
  tasks_created = 1;
  allocate_tasks(task_array, num_tasks);
  dispatch_nosave();

  // This should never return
  return -1;
}

int event_wait(unsigned int dev)
{
  tcb_t *task = get_cur_tcb();

  if (dev >= NUM_DEVICES)
  	return -EINVAL;

  if (task->holds_lock)
  	return -EDEADLOCK;

  disable_interrupts();
  dev_wait(dev);
  enable_interrupts();

  return 1;
}

/* An invalid syscall causes the kernel to exit. */
void invalid_syscall(unsigned int call_num)
{
	printf("Kernel panic: invalid syscall -- 0x%08x\n", call_num);

	disable_interrupts();
	while(1);
}
