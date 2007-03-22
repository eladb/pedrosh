/**
 * named-pipes.c
 * implements named full-duplex pipes.
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 10, 2006
 */

#include <config.h>     // autoconf

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "debug.h"

//
// defines
//

/* the maximum size of buffer read at a time */
#define READ_BUFF_SIZE 1024

/* the maximum size of path */
#define MAX_PATH 2048

/* returns the maximum between 'a' and 'b' */
#define MAX(a,b) (a > b ? a : b)

//
// function definitions
//


/**
 * first opens the fifo 'incoming_fifo_name' for reading and whenever data is 
 * written into the fifo, this function writes it to stdout.
 *
 * opens the fifo 'outgoing_fifo_name' for writing, and whenever data is coming
 * to stdin, this function writes it to this fifo.
 *
 * note: in order to avoid a deadlock and support timeouts, we are opening the 
 * fifos in non-blocking mode. the reader fifo is opened for reading (posix says 
 * it will never block) and the writer fifo is opened for reading and writing, so 
 * that it doesn't return an error if there's no reader opened on the other side.
 * in POSIX, this behavior is not defined, while in Linux this behavior allows 
 * opening the fifo for writing without a reader.
 * 
 * when the fifo 'incoming_fifo_name' is closed by the other side or stdin is closed, 
 * the function exits successfuly.
 *
 * @param incoming_fifo_name - file name of the fifo to open for reading.
 * @param outgoing_fifo_name - file name of the fifo to open for writing.
 * @param timeout_ms - the timeout in milliseconds to wait for the other side to send data. set to -1 for no-timeout.
 * @param timeout_once - 
 * 		set this value to 1 if you don't want to use a timeout after data had arrived from incoming fifo.
 * 		set this value to 0 if you want to use a timeout every time we wait for data.
 *
 * @return 0 upon success, -1 upon failure (with errno).
 * 	ETIMEDOUT - if the timeout had expired before data was received from process.
 * 	
 */
int named_pipes(const char* incoming_fifo_name, const char* outgoing_fifo_name, int timeout_ms, int timeout_once)
{
	struct timeval timeout;
	struct timeval* timeout_p = NULL;
  char read_buff[READ_BUFF_SIZE+1];
  fd_set read_fds;  
  int proc_out_fd = -1;
  int proc_in_fd = -1;
  int select_ret;
  int read_ret;
  int ret = 0;
  int fd_n;
	int wind_timeout = 1; 

  //
  // reroute data coming from stdin into the process 'in' fifo.
  // reroute data coming from the process 'out' fifo into stdout.
  //

	// open incoming for reading and outgoing for read and write (Linux behavior that allows non-blocking writer).
  proc_out_fd = open(incoming_fifo_name, O_RDONLY | O_NONBLOCK);
  proc_in_fd = open(outgoing_fifo_name, O_RDWR | O_NONBLOCK);

  // if one of them didn't open, return with an error.
  if (proc_out_fd == -1 || proc_in_fd == -1)
  {
		// errno is set by open.
  	ret = -1;
    goto cleanup;
  }
  
  // wait for incoming data from stdin (file descriptor 0) and from
  // the process output. whenever data comes in, route it either to the
  // process input or stdout correspondingly.
  
  while (1)
	{
  	// set up fd-set for select.
    fd_n = 0;
    FD_ZERO(&read_fds);
    FD_SET(proc_out_fd, &read_fds); 
    fd_n = MAX(fd_n, proc_out_fd);
    FD_SET(0, &read_fds); 
    fd_n = MAX(fd_n, 0);

		if (timeout_ms != -1 && wind_timeout)
		{
			timeout.tv_sec = timeout_ms / 1000;
			timeout.tv_usec = (timeout_ms % 1000) * 1000;
			timeout_p = &timeout;
		}
		else
		{
			timeout_p = NULL;
		}
			
		trace("in select timeout=%d ms (%d sec and %d usec)\n", timeout_ms, timeout.tv_sec, timeout.tv_usec);
			
    // todo: we can add timeout support right here
    select_ret = select(fd_n + 1, &read_fds, NULL, NULL, timeout_p);

    // if select returned 0, it means the time out had expired.
    if (select_ret == 0) 
		{
	  	trace("select timed out\n");
			errno = ETIMEDOUT;
	  	ret = -1;
		  goto cleanup;
		}

    // if select returned with an error, break the while.
    if (select_ret < 0)
		{
	  	fprintf(stderr, "error in select. %s", strerror(errno));

			// errno has been set by select.
			ret = -1;
	    goto cleanup;
    }

    // if there's data in stdin, route it to the process input fifo.
    if (select_ret > 0 && FD_ISSET(0, &read_fds))
		{
	  	read_ret = read(0, read_buff, READ_BUFF_SIZE);

	  	// if we got data from stdin, write them into the process
	  	if (read_ret > 0) 
	    {
	    	write(proc_in_fd, read_buff, read_ret);
	    }

	  	// if stdin has been closed (could happen if its redirected
	  	// from somewhere), we'll close the proc_in_fd to notify
	  	// process that input has ended, and break.
	  	if (read_ret == 0)
	    {
	    	break;
	    }
		}

    // if there's data available on the process output fifo, route
    // it to stdout.
    if (select_ret > 0 && FD_ISSET(proc_out_fd, &read_fds))
		{
			// don't rewind timeout if user had asked to timeout only once.
			if (timeout_once) wind_timeout = 0;
					
	  	read_ret = read(proc_out_fd, read_buff, READ_BUFF_SIZE);

	  	// if process has some data for us, write them to stdout.
	  	if (read_ret > 0)
	    {
	      write(1, read_buff, read_ret);
	    }

	  	// if process closed output fifo, we are done.
	  	if (read_ret == 0)
	    {
	      break;
	    }

	  	// break in case of an error from process.
	  	if (read_ret < 0)
	    {
	      fprintf(stderr, "error reading data from process. %s\n",
		    	strerror(errno));
			
				// errno is set by read.
				ret = -1;
				goto cleanup;
	    }
		}
  }

cleanup:

  // close the file descriptors.
  if (proc_in_fd != -1) close(proc_in_fd);
  if (proc_out_fd != -1) close(proc_out_fd);
   
  return ret;
}

