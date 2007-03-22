/**
 * pedro-ipc.c
 * implementation for pedro commands, using IPC and named pipes.
 * todo <ref>
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 10, 2006
 */

#include <config.h>   // autoconf

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <ipc.h>

#include "named-pipes.h"
#include "pedrosh-i.h"
#include "plugins.h"
#include "debug.h"

#include "plugin-ipc.h"

//
// defines
//

#define IPC_PIPE_DIR_TEMPLATE "/tmp/pedro-ipc-XXXXXX"
#define IPC_PIPE_OUT_FN       "out"
#define IPC_PIPE_IN_FN        "in"

#define IPC_TIMEOUT						5000		// the timeout waiting for the process to send data
#define IPC_TIMEOUT_ONCE			1				// 'true' here indicates to timeout only once (once data received, no timeout)

//
// function definitions
//

/**
 * initialize the pedro ipc module.
 * @return 0 on success.
 */
int ipc_init(void)
{
	// libipc initialization
	char module_name[50];
	sprintf(module_name,"%s-%d",PEDROSH_MODULE_NAME,getpid());

	// make ipc not display any message.
        //   possible values: IPC_Silent,       IPC_Print_Warnings,
	//                    IPC_Print_Errors, IPC_Exit_On_Errors
	IPC_setVerbosity(IPC_Silent);
	
	// connect to the central server (libipc)
	IPC_connect(module_name);

	plugins_register("ipc", ipc_cmd_handler);

	return 0;
}

/**
 * this command handler publishes an ipc message corresponding to this
 * command. it also creates a named pipe (two temporary fifo files) that allow
 * the handler to communicate with the cli user.
 */
int ipc_cmd_handler(int argc, char* argv[])
{
  char pipe_dir[MAX_PATH+1] = "";
  char proc_out_fn[MAX_PATH+1] = "";
  char proc_in_fn[MAX_PATH+1] = "";
  mode_t prev_umask;
  int ret;

  // create a temporary directory for the pipe fifos
  strcpy(pipe_dir, IPC_PIPE_DIR_TEMPLATE);

  if (mkdtemp(pipe_dir) == NULL)
  {
    fprintf(stderr, "unable to create directory for "
	    "communicating with process. %s", strerror(errno));
    
    ret = 1;
    goto cleanup;
  }

  // chmod the temp directory to allow all access (in case we are accessing
  // from telnet or something, that everyone will be able to connect).
  if (chmod(pipe_dir, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
  {
    fprintf(stderr, "unable to chmod pipes directory. %s\n",
	    strerror(errno));
    ret = 1;
    goto cleanup;
  }

  // generate names for out and in fifos.
  snprintf(proc_out_fn, MAX_PATH, "%s/%s", pipe_dir, IPC_PIPE_OUT_FN);
  snprintf(proc_in_fn, MAX_PATH, "%s/%s", pipe_dir, IPC_PIPE_IN_FN);
  
  trace("output to '%s' input from '%s'\n", proc_out_fn, proc_in_fn);
   
  //
  // create two fifos, one for process output (my input) and one 
  // for process input (my output). 
  //

  // reset umask so that fifos will get all permissions.
  prev_umask = umask(0);

  if (mkfifo(proc_out_fn, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
  {
    fprintf(stderr, "unable to create '%s'. %s", proc_out_fn, strerror(errno));
    ret = 1;
    goto cleanup;
  }

  if (mkfifo(proc_in_fn, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
  {
    fprintf(stderr, "unable to create '%s'. %s", proc_in_fn, strerror(errno));
    ret = 1;
    goto cleanup;
  }

  // restore umask.
  umask(prev_umask);

  // publish over libipc that the user has entered a cli command
  {
    char msg_name[300];
    sprintf(msg_name, "%s-%s", PEDROSH_MSG_NAME, argv[0]);

    // define the message if its not already defined
    if (!IPC_isMsgDefined(msg_name))
    {
        printf("error: no task had registered to handle this command ('%s').\n", argv[0]);
	ret = 0;
	goto cleanup;
        //IPC_defineMsg(msg_name, IPC_VARIABLE_LENGTH, PEDROSH_MSG_FORM);
    }

    PEDROSH_MSG msg;
    argv[argc++] = pipe_dir; // add pipe_dir as the last parameter.
    msg.argc = argc;
    msg.argv = argv;
    IPC_publishData(msg_name, &msg);
  }

  // create the named pipes and block until one side closes to close.
  ret = named_pipes(proc_out_fn, proc_in_fn, IPC_TIMEOUT, IPC_TIMEOUT_ONCE);
  if (ret != 0)
  {
    fprintf(stderr, "error communicating with process: %s\n", strerror(errno));
    ret = -1;
    goto cleanup;
  }

  
 cleanup: 
  
  // delete the fifos.
  if (proc_in_fn[0] != 0) unlink(proc_in_fn);
  if (proc_out_fn[0] != 0) unlink(proc_out_fn);
  
  // remove the pipe directory
  if (pipe_dir[0] != 0) rmdir(pipe_dir);
  return ret;
}

int ipc_cleanup(void)
{
  IPC_disconnect();
  return 0;
}
