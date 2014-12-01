/** @file time.c
 *
 * @brief Kernel timer based syscall implementations
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date   2008-11-16
 */

#include <types.h>
#include <config.h>
#include <bits/errno.h>
#include <arm/timer.h>
#include <arm/reg.h>
#include <syscall.h>

volatile unsigned long int system_time;

unsigned long time_syscall(void)
{
  return system_time;
}



/** @brief Waits in a tight loop for atleast the given number of milliseconds.
 *
 * @param millis  The number of milliseconds to sleep.
 *
 * 
 */
void sleep_syscall(unsigned long millis)
{
  unsigned long timer_start_time = system_time;
  while (system_time - timer_start_time < millis);
}

void C_IRQ_Handler()
{
  volatile uint32_t OSCR = reg_read(OSTMR_OSCR_ADDR);
  volatile uint32_t OSSR = reg_read(OSTMR_OSSR_ADDR);

  // reacting to an interrupt
  if (OSSR == 1)
  {
    uint32_t next_time;
    // increment uptime by 10ms
    system_time += 10;

    // increment the match register by 10 ms
    next_time = OSCR + (OSTMR_FREQ/100);

    // setting OSMR to the correct value
    reg_write(OSTMR_OSMR_ADDR(0), next_time);

    // clear OSSR
    reg_set(OSTMR_OSSR_ADDR, OSTMR_OSSR_M0);

  }
}

