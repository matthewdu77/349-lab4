/**
 * @file mutex.c
 *
 * @brief Implements mutices.
 *
 * @author Harry Q Bovik < PUT YOUR NAMES HERE
 *
 * 
 * @date  
 */

//#define DEBUG_MUTEX

#include <lock.h>
#include <task.h>
#include <sched.h>
#include <bits/errno.h>
#include <arm/psr.h>
#include <arm/exception.h>
#include <exports.h>
#ifdef DEBUG_MUTEX
#include <exports.h> // temp
#endif

static volatile int largest_mutex = 0;
mutex_t gtMutex[OS_NUM_MUTEX];

void mutex_init()
{
  int i;
	for (i = 0; i < OS_NUM_MUTEX; i++)
  {
    gtMutex[i].bAvailable = 0;
    gtMutex[i].pHolding_Tcb = 0;
    gtMutex[i].bLock = 0;
    gtMutex[i].pSleep_queue = 0;
  }
}

int mutex_create(void)
{
  if (largest_mutex >= OS_NUM_MUTEX)
  {
    return -ENOMEM;
  }

  // disable interrupts to prevent two mutex creates from getting the same mutex
  disable_interrupts();
  int i = largest_mutex++;
  enable_interrupts();

	return i;
}

int mutex_lock(int mutex)
{
  if (mutex < 0 || mutex >= largest_mutex)
  {
    return -EINVAL;
  }
  tcb_t* curTcb = get_cur_tcb();
  if (curTcb == gtMutex[mutex].pHolding_Tcb)
  {
    return -EDEADLOCK;
  }

  // disable interrupts to prevent two tasks from getting the same lock
  disable_interrupts();
  // sleep until mutex is released
  while (gtMutex[mutex].bLock != 0)
  {
    // priority inheritance
    if (gtMutex[mutex].pHolding_Tcb->cur_prio > curTcb->cur_prio)
      gtMutex[mutex].pHolding_Tcb->cur_prio = curTcb->cur_prio;
    curTcb->sleep_queue = gtMutex[mutex].pSleep_queue;
    gtMutex[mutex].pSleep_queue = curTcb;
    dispatch_sleep();
  }
  gtMutex[mutex].bLock = 1;
  gtMutex[mutex].pHolding_Tcb = curTcb;
  curTcb->holds_lock++;

  enable_interrupts();
	return 0;
}

int mutex_unlock(int mutex)
{
  if (mutex < 0 || mutex >= largest_mutex)
  {
    return -EINVAL;
  }
  tcb_t* curTcb = get_cur_tcb();
  if (curTcb != gtMutex[mutex].pHolding_Tcb)
  {
    return -EPERM;
  }

  // disable interrupts to process mutex
  disable_interrupts();
  // make all tasks in sleep queue runnable
  tcb_t *t;
  for (t = gtMutex[mutex].pSleep_queue; t != 0; t = t->sleep_queue)
  {
    runqueue_add(t, t->cur_prio);
  }
  
  // reset mutex
  gtMutex[mutex].pHolding_Tcb = 0;
  gtMutex[mutex].bLock = 0;
  gtMutex[mutex].pSleep_queue = 0;
  curTcb->holds_lock--;

  // priority inheritance
  curTcb->cur_prio = curTcb->native_prio;

  // schedule any higher priority tasks
  dispatch_save();
  enable_interrupts();

	return 0;
}

