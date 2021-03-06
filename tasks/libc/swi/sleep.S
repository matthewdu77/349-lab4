/** @file sleep.S
 *
 * @brief sleep sycall wrapper
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-10-31
 */

#include <asm.h>
#include <bits/swi.h>

	.file "sleep.S"

FUNC(sleep)
  swi SLEEP_SWI @make the system call

  @ Return on success
  cmp r0, #0 @success
  movpl pc, lr @pl means >= 0

  @ Store the absolute value of return value in errno
  neg r0, r0
  ldr r1, =errno
  str r0, [r1]

  @ Store -1 in return value
  mov r0, #-1
  mov pc, lr

