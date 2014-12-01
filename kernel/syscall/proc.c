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

int task_create(task_t* tasks  __attribute__((unused)), size_t num_tasks  __attribute__((unused)))
{
  size_t i;

  if (num_tasks > OS_MAX_TASKS - 2)
  	return -EINVAL;

  if (!valid_addr(&tasks, size_of(task_t) * num_tasks, USR_START_ADDR, USR_END_ADDR))
  	return -EFAULT;

  // TODO : ub_test.c assign_schedule(&tasks, num_tasks)

  disable_interrupts();
  allocate_tasks(&tasks, num_tasks);
  dispatch_nosave();


  // This should never return
  return -1;
}

int event_wait(unsigned int dev  __attribute__((unused)))
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
void invalid_syscall(unsigned int call_num  __attribute__((unused)))
{
	printf("Kernel panic: invalid syscall -- 0x%08x\n", call_num);

	disable_interrupts();
	while(1);
}
