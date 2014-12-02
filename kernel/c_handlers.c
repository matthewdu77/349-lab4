#include <bits/swi.h>
#include <bits/errno.h>
#include <syscall.h>
#include <constants.h>
#include <exports.h>

/* C_SWI_Handler uses SWI number to call the appropriate function. */
int C_SWI_Handler(int swiNum, int *regs)
{
  int count = 0;
  switch (swiNum)
  {
    case READ_SWI:
      // ssize_t read(int fd, void *buf, size_t count);
      count = read_syscall(regs[0], (void *) regs[1], (size_t) regs[2]);
      break;
    case WRITE_SWI:
      // ssize_t write(int fd, const void *buf, size_t count);
      count = write_syscall((int) regs[0], (void *) regs[1], (size_t) regs[2]);
      break;
    case TIME_SWI:
      // unsigned long time();
      count = time_syscall();
      break;
    case SLEEP_SWI:
      // void sleep(unsigned long millis);
      sleep_syscall((unsigned long) regs[0]);
      break;
    case CREATE_SWI:
      // int task_create(task_t* tasks, size_t num_tasks);
      count = task_create((void*) regs[0], (size_t) regs[1]);
      break;
    case EVENT_WAIT:
      // int event_wait(unsigned int dev)
      count = event_wait((unsigned int) regs[0]);
      break;
    default:
      invalid_syscall(BAD_CODE);
  }
  return count;
}
