/*
 * copyright (c) 2006 by embrix.
 * 
 * all rights reserved.
 * 
 * developed by embrix for corrigent systems ltd.
 */

#include <config.h> /* this should be the first include file */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ipc.h>

#include "pedrosh.h"

#define THIS_MODULE_NAME __FILE__

#define MGMTFS_PATH "/tmp/clifs"

// handlers for cli commands
void on_argv_cmd(int argc, char *argv[],
                 FILE *fin, FILE *fout,
                 pedro_callback_func_args_t *pedro);
void on_version(int argc, char *argv[],
                FILE *fin, FILE *fout,
                pedro_callback_func_args_t *pedro);
void on_modify(int argc, char *argv[],
               FILE *fin, FILE *fout,
               pedro_callback_func_args_t *pedro);
void on_cmd1(int argc, char *argv[],
             FILE *fin, FILE *fout,
             pedro_callback_func_args_t *pedro);



void register_cli_commands_method1()
{

	static pedro_arg_t cmd1_args[] = {
	// arg_name, is_optional,  arg_type,       min,     max
	//-------------------------------------------------------
	{"string_arg",     1,    PEDRO_ARG_STRING                     },
	{"int_arg1",       1,    PEDRO_ARG_INT                        },
	{"int_arg2",       1,    PEDRO_ARG_INT,     0,      100       },
	{"ipv4_arg",       1,    PEDRO_ARG_IPV4                       },
	{"ipv6_arg",       1,    PEDRO_ARG_IPV6                       },
	{"enum_arg1",      1,    PEDRO_ARG_ENUM,   (int)"one\n"
	                                                "two\n"
	                                                "three\n"
	                                                "four"
	                                                              },
	{"enum_arg2",      1,    PEDRO_ARG_ENUM,   (int)"one"   " do something once\n"
	                                                "two"   " do something twice\n"
	                                                "three" " do something three times\n"
	                                                "four"  " do somthing four times\n"
	                                                              },
	{"date_arg",       1,    PEDRO_ARG_DATETIME,(int)"%Y-%m-%d"},
	{"time_arg",       1,    PEDRO_ARG_DATETIME,(int)"%H:%M:%S"},
	{0}
	};
	
	// this is for commands without arguments
	static pedro_arg_t cmd_no_args[] = {
	{0}
	};
	
	
	static pedro_cmd_t tree[] = {
	//nodepath                     callback       args       shorthelp  longhelp
	//-------------------------------------------------------------------------------------
	
	// create a directory that the user may make his current directory
	{"example_task2",                NULL,          NULL,        NULL,  NULL},
	
	// this is argc/argv command (since 'args' is null)
	{"example_task2/a/argv_cmd",     on_argv_cmd,  NULL,         NULL,  NULL},
	
	{"example_task2/a/version",      on_version,   cmd_no_args,
													"display the verison of example_task2", // shorthelp
													NULL},                                  // longhelp
	{"example_task2/a/modify",       on_modify,    NULL,
				"vxworks modify clone", // shorthelp
				// longhelp:
				"  this command behaves like vxworks' modify command.\n"
				"  the <address> parameter is an integer.\n"
				"  the command prompts the user for an integer value to put into address <address>.\n"
				"  after the user enter a valid data to put into <Address>, the fnction prompts\n"
				"  the user for data to be placed in <Address>+1. to stop, the enters an empty\n"
				"  string (i.e. just presses the enter key).\n"
				},
	
	// this is a command with many arguments
	{"example_task2/b/cmd1",      on_cmd1,         cmd1_args,  NULL,  NULL},

#if 0	
	//TODO: remove. TEMP - for debugging
	{"example_task2/b/DEBUG", on_argv_cmd,     cmd1_args,  NULL,  NULL},
#endif

	{0}  // end of list
	};


	// register commands we are prepared to handle (and directories)
	pedro_add_cmds(tree, MGMTFS_PATH);

	// add some internal commands.
	// puttings them under PEDRO_GLOBALS makes these commands accessiable regardless
	// of the current directory.
	pedro_add_internal_help_cmd("shorthelp", "longhelp", PEDRO_GLOBALS"/help", MGMTFS_PATH);
	pedro_add_internal_exit_cmd("shorthelp", "longhelp", PEDRO_GLOBALS"/exit", MGMTFS_PATH);
	pedro_add_internal_cdup_cmd("shorthelp", "longhelp", PEDRO_GLOBALS"/cdup", MGMTFS_PATH);
}

void register_cli_commands_method2()
{
	// note: every function call we're going to use in here, returns an integer
	// value. if this value is non-zero, this means an error has occured.
	// this example does not check for errors.
	
	pedro_add_enterable_dir(NULL, NULL, "example_task2", MGMTFS_PATH);

	pedro_add_cmd(on_modify, 0, 
						"vxworks modify clone", // short help
						// long help
						"  this command behaves like vxworks' modify command.\n"
						"  the <address> parameter is an integer.\n"
						"  the command prompts the user for an integer value to put into address <address>.\n"
						"  after the user enter a valid data to put into <Address>, the fnction prompts\n"
						"  the user for data to be placed in <Address>+1. to stop, the enters an empty\n"
						"  string (i.e. just presses the enter key).\n",

						"example_task2/a/modify", MGMTFS_PATH);

	pedro_add_cmd(on_argv_cmd, 0, NULL, NULL, "example_task2/a/argv_cmd", MGMTFS_PATH);

	pedro_add_cmd(on_version, 0, "display the verison of example_task2",
	              NULL, "example_task2/a/version", MGMTFS_PATH);

	//
	// add commands (with parameters)
	//
	pedro_add_cmd(on_cmd1, 1, NULL, NULL, "example_task2/b/cmd1", MGMTFS_PATH);
	
	//
	// add parameters to commands
	//
	pedro_add_string_param(NULL, 0,
                    "example_task2/b/cmd1/string_arg", MGMTFS_PATH);

	pedro_add_int_param(-1, -1,  // min, max: if min==max, they are both ignored!
                    NULL, 0,
                    "example_task2/b/cmd1/int_arg1", MGMTFS_PATH);

	pedro_add_int_param(0, 100,  // min, max: if min==max, they are both ignored!
                    NULL, 0,
                    "example_task2/b/cmd1/int_arg2", MGMTFS_PATH);

	pedro_add_ipv4_param(NULL, 0,
                    "example_task2/b/cmd1/ipv4_arg", MGMTFS_PATH);

	pedro_add_ipv6_param(NULL, 0,
                    "example_task2/b/cmd1/ipv6_arg", MGMTFS_PATH);

	pedro_add_enum_param("one\0two\0three\0four\0", NULL, 
                    0, // optional_flag,
                    "example_task2/b/cmd1/enum_arg1", MGMTFS_PATH);

	pedro_add_enum_param("one\0two\0three\0four\0", NULL, 
                    0, // optional_flag,
                    "example_task2/b/cmd1/enum_arg2", MGMTFS_PATH);

	// add some internal commands.
	// puttings them under PEDRO_GLOBALS makes these commands accessiable regardless
	// of the current directory.
	pedro_add_internal_help_cmd("shorthelp", "longhelp", PEDRO_GLOBALS"/help", MGMTFS_PATH);
	pedro_add_internal_exit_cmd("shorthelp", "longhelp", PEDRO_GLOBALS"/exit", MGMTFS_PATH);
	pedro_add_internal_cdup_cmd("shorthelp", "longhelp", PEDRO_GLOBALS"/cdup", MGMTFS_PATH);
}


//
// command handlers
//

void on_argv_cmd(int argc, char *argv[],
                 FILE *fin, FILE *fout,
                 pedro_callback_func_args_t *pedro)
{
  fprintf(pedro->fout, "%s:  argc = %d\n", "on_argv_cmd()", pedro->argc);
  for (int i=0; i<pedro->argc; i++)
    fprintf(pedro->fout, "\t argv[%d] = '%s'\n", i, pedro->argv[i]);
}

void on_cmd1(int argc, char *argv[],
             FILE *fin, FILE *fout,
             pedro_callback_func_args_t *pedro)
{
  fprintf(pedro->fout, "on_cmd1()\n");

	int i;
	
	i = pedro_is_arg_defined(pedro,  "string_arg");
  fprintf(pedro->fout, "pedro_is_arg_defined(string_arg)=%d\n",i);
	
	i = pedro_is_arg_defined(pedro,  "int_arg1");
  fprintf(pedro->fout, "pedro_is_arg_defined(int_arg1)=%d\n",i);

	i = pedro_is_arg_defined(pedro,  "int_arg2");
  fprintf(pedro->fout, "pedro_is_arg_defined(int_arg2)=%d\n",i);

	i = pedro_is_arg_defined(pedro,  "ipv4_arg");
  fprintf(pedro->fout, "pedro_is_arg_defined(ipv4_arg)=%d\n",i);

	i = pedro_is_arg_defined(pedro,  "ipv6_arg");
  fprintf(pedro->fout, "pedro_is_arg_defined(ipv6_arg)=%d\n",i);

	i = pedro_is_arg_defined(pedro,  "enum_arg1");
  fprintf(pedro->fout, "pedro_is_arg_defined(enum_arg1)=%d\n",i);

	i = pedro_is_arg_defined(pedro,  "enum_arg2");
  fprintf(pedro->fout, "pedro_is_arg_defined(enum_arg2)=%d\n",i);

  fprintf(pedro->fout, "string_arg='%s'\n", pedro_get_string_arg(pedro, "string_arg", "defval") );
  fprintf(pedro->fout, "int_arg1=%d\n", pedro_get_int_arg(pedro, "int_arg1", -1) );
  fprintf(pedro->fout, "int_arg2=%d\n", pedro_get_int_arg(pedro, "int_arg2", -2) );
	
  fprintf(pedro->fout, "ipv4_arg=0x%08X\n", pedro_get_ipv4_arg(pedro, "ipv4_arg", -1) );
  fprintf(pedro->fout, "enum_arg (as string)='%s'\n", pedro_get_string_arg(pedro, "enum_arg", "defval") );
  fprintf(pedro->fout, "enum_arg1='%d'\n", pedro_get_enum_arg(pedro, "enum_arg1", -1)); 
  fprintf(pedro->fout, "enum_arg2='%d'\n", pedro_get_enum_arg(pedro, "enum_arg2", -1)); 
}

void on_version(int argc, char *argv[],
                FILE *fin, FILE *fout,
                pedro_callback_func_args_t *pedro)
{
  fprintf(pedro->fout, "version: "  __FILE__ " " __DATE__ " " __TIME__ "\n");
}

void on_modify(int argc, char *argv[],
               FILE *fin, FILE *fout,
               pedro_callback_func_args_t *pedro)
{
  // cli command: modify <address>
  int addr, data;
  char *endptr;
  
  //__debug_print_params(argc, argv, "on_strlen()");
  if (pedro->argc != 2)
  {
	  fprintf(pedro->fout,
		 "usgage:\n"
                 " \t modify <address>\n");
	  return;
  }


  addr = strtol(pedro->argv[1], &endptr, 0 /*base*/);
  if (*endptr)
    fprintf(pedro->fout, " bad integer value - %s\n", pedro->argv[1]);
  
  while (1)
  {
    char data_str[40];
    fprintf(pedro->fout, "0x%08X - ", addr); fflush(pedro->fout);
    if (!fgets(data_str, sizeof(data_str), pedro->fin))
      return;
    if (!strcmp(data_str, "\n"))
      return;
    data = strtol(data_str, &endptr, 0 /*base*/);
    if (*endptr != '\n' && *endptr)
    {
      fflush(pedro->fin);
      fprintf(pedro->fout, " bad integer value - %s\n", data_str); fflush(pedro->fout);
      continue;
    }
    fprintf(pedro->fout, "<setting address 0x%08X to 0x%X>\n", addr, data); fflush(pedro->fout);
    addr++;
  }
}

//
// main
//

int main (void)
{
	/* Connect to the central server */
	printf("\nIPC_connect(%s)\n", THIS_MODULE_NAME);
	IPC_connect(THIS_MODULE_NAME);

	pedro_init(MGMTFS_PATH);

#if 1
	register_cli_commands_method1();
#else
	register_cli_commands_method2();
#endif
	
	IPC_dispatch();
	IPC_disconnect();
	return 0;
}
