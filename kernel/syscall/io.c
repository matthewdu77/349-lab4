/** @file io.c
 *
 * @brief Kernel I/O syscall implementations
 *
 * @author Mike Kasick <mkasick@andrew.cmu.edu>
 * @date   Sun, 14 Oct 2007 00:07:38 -0400
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date   2008-11-16
 */

#include <types.h>
#include <bits/errno.h>
#include <bits/fileno.h>
#include <arm/physmem.h>
#include <syscall.h>
#include <exports.h>
#include <kernel.h>

#include <constants.h>

#define EOT_CHAR 0x04
#define DEL_CHAR 0x7f

/* Verifies that the buffer is entirely in valid memory. */
int check_mem(char *buf, int count, unsigned start, unsigned end)
{
  unsigned start_buf = (unsigned) buf;
  unsigned end_buf = (unsigned)(buf + count);

  if ((start_buf < start) || (start_buf > end))
  {
    return false;
  }
  if ((end_buf < start) || (end_buf > end))
  {
    return false;
  }
  // Overflow case.
  if (start_buf > end_buf)
  {
    return false;
  }

  return true;
}

/* Read count bytes (or less) from fd into the buffer buf. */
ssize_t read_syscall(int fd, void *buf, size_t count)
{
  // Check for invalid memory range or file descriptors
  if (check_mem((char *) buf, (int) count, SDRAM_START, SDRAM_END) == false)
  {
    invalid_syscall(-EFAULT);
  }
  else if (fd != STDIN_FILENO)
  {
    invalid_syscall(-EBADF);
  }

  size_t i = 0;
  char *buffer = (char *) buf;
  char read_char;

  while (i < count)
  {
    read_char = getc();

    if (read_char == 4)
    { //EOT character
      return i;
    }
    else if (((read_char == 8) || (read_char == 127)))
    { // backspace or DEL character
      buffer[i] = 0; // '\0' character
      if(i > 0)
      {
        i--;
        puts("\b \b");
      }
    }
    else if ((read_char == 10) || (read_char == 13))
    { // '\n' newline or '\r' carriage return character
      buffer[i] = '\n';
      putc('\n');
      return (i+1);
    }
    else
    {
      // put character into buffer and putc
      buffer[i] = read_char;
      i++;
      putc(read_char);
    }
  }
  return i;
}

/* Write count bytes to fd from the buffer buf. */
ssize_t write_syscall(int fd, const void *buf, size_t count)
{
  // Check for invalid memory range or file descriptors
  if (check_mem((char *) buf, (int) count, SDRAM_START, SDRAM_END) == false &&
      check_mem((char *) buf, (int) count, SFROM_START, SFROM_END) == false)
  {
    invalid_syscall(-EFAULT);
  }
  else if (fd != STDOUT_FILENO)
  {
    invalid_syscall(-EBADF);
  }

  char *buffer = (char *) buf;
  size_t i;
  char read_char;
  for (i = 0; i < count; i++)
  {
    // put character into buffer and putc
    read_char = buffer[i];
    putc(read_char);
  }
  return i;
}

