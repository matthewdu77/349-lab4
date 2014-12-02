/** @file dagger.c
 *
 * @brief Creates two simple periodic tasks.
 *
 * @note  This is like knife -- but smaller :)
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date   2008-11-30
 */
#include <stdio.h>
#include <task.h>
#include <unistd.h>

int write_mutex;

void panic(const char* str)
{
	puts(str);
	while(1);
}

void fun1(void* str)
{
	while(1)
	{
    // require a lock to write the char every 50 ms
    mutex_lock(write_mutex);
		putchar((int)str);
    mutex_unlock(write_mutex);
		if (event_wait(3) < 0)
			panic("Dev 3 failed");
	}
}

void fun2(void* str)
{
	while(1)
	{
    mutex_lock(write_mutex);
    putchar((int)str);
    sleep(500);
    putchar((int)str);
    mutex_unlock(write_mutex);
    if (event_wait(2) < 0)
      panic("Dev 2 failed");
	}
}

int main(int argc, char** argv)
{
	task_t tasks[2];
	tasks[0].lambda = fun1;
	tasks[0].data = (void*)'@';
	tasks[0].stack_pos = (void*)0xa2000000;
	tasks[0].C = 1;
	tasks[0].T = PERIOD_DEV3;
	tasks[1].lambda = fun2;
	tasks[1].data = (void*)'<';
	tasks[1].stack_pos = (void*)0xa1000000;
	tasks[1].C = 1;
	tasks[1].T = PERIOD_DEV2;

  write_mutex = mutex_create();

	task_create(tasks, 2);
	argc=argc; /* remove compiler warning */
	argv[0]=argv[0]; /* remove compiler warning */

	puts("Why did your code get here!\n");
	return 0;
}
