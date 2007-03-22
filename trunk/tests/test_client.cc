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
#include <time.h>       // struct tm, strftime()
#include <ipc.h>

#include "pedrosh.h"

#define THIS_MODULE_NAME __FILE__

#define MGMTFS_PATH "/tmp/clifs"

// handlers for cli commands
void cmd_version(int argc, char *argv[],
                 FILE *fin, FILE *fout,
                 pedro_callback_func_args_t *pedro);
void cmd_argv_print_args(int argc, char *argv[],
                         FILE *fin, FILE *fout,
                         pedro_callback_func_args_t *pedro);
void cmd_is_arg_defined(int argc, char *argv[],
                        FILE *fin, FILE *fout,
                        pedro_callback_func_args_t *pedro);
void cmd_show_arg_val(int argc, char *argv[],
                      FILE *fin, FILE *fout,
                      pedro_callback_func_args_t *pedro);

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
{"test",                       NULL,          NULL,        NULL,  NULL},
{"test/a/argv_cmd",     cmd_argv_print_args,  NULL,         NULL,  NULL},
{"test/a/version",      cmd_version,       cmd_no_args,
												"display the verison of example_task2", // shorthelp
												NULL},                                  // longhelp
{"test/b/cmd_show_val",   cmd_show_arg_val,   cmd1_args,  NULL,  NULL},
{"test/b/cmd_show_given", cmd_is_arg_defined, cmd1_args,  NULL,  NULL},

//TODO: remove. TEMP - for debugging
{"test/b/DEBUG",         cmd_argv_print_args, cmd1_args,  NULL,  NULL},

{0}  // end of list
};


//
// command handlers
//

void cmd_version(int argc, char *argv[],
                 FILE *fin, FILE *fout,
                 pedro_callback_func_args_t *pedro)
{
  fprintf(pedro->fout, "version: "  __FILE__ " " __DATE__ " " __TIME__ "\n");
}

void cmd_argv_print_args(int argc, char *argv[],
                         FILE *fin, FILE *fout,
                         pedro_callback_func_args_t *pedro)
{
  fprintf(pedro->fout, "%s:  argc = %d\n", "on_argv_cmd()", pedro->argc);
  for (int i=0; i<pedro->argc; i++)
    fprintf(pedro->fout, "\t argv[%d] = '%s'\n", i, pedro->argv[i]);
}

// show given/not given for each arg
void cmd_is_arg_defined(int argc, char *argv[],
                        FILE *fin, FILE *fout,
                        pedro_callback_func_args_t *pedro)
{
	for (pedro_arg_t *parg=cmd1_args; parg->arg_name; parg++)
	{
		int is_given = pedro_is_arg_defined(pedro,  parg->arg_name);
		fprintf(pedro->fout, "%s - %d\n", parg->arg_name, is_given);
	}
}

// for every given arg, show its value
void cmd_show_arg_val(int argc, char *argv[],
                      FILE *fin, FILE *fout,
                      pedro_callback_func_args_t *pedro)
{
	for (pedro_arg_t *parg=cmd1_args; parg->arg_name; parg++)
	{
		if (!pedro_is_arg_defined(pedro,  parg->arg_name))
			continue;

		fprintf(pedro->fout, "%s - ", parg->arg_name);

		switch(parg->arg_type)
		{
			case PEDRO_ARG_STRING:
				fprintf(pedro->fout, "'%s'", pedro_get_string_arg(pedro, parg->arg_name, NULL));
				break;
			case PEDRO_ARG_INT:
				fprintf(pedro->fout, "%d", pedro_get_int_arg(pedro, parg->arg_name, -1));
				break;
			case PEDRO_ARG_IPV4:
				fprintf(pedro->fout, "%08X", pedro_get_ipv4_arg(pedro, parg->arg_name, -1));
				break;
			case PEDRO_ARG_IPV6:
				{
					//__i = pedro_get_ipv6_arg(pedro, "ipv6_arg", def_ipv6, ipv6_val);
					//char def_ipv6[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
					//                     0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
					char ipv6_val[16] = {0};
					int __i = pedro_get_ipv6_arg(pedro, "ipv6_arg", NULL, ipv6_val);
					for (int i=0; i<16; i++) fprintf(pedro->fout, "%02X ", ipv6_val[i]);
				}
				break;
			case PEDRO_ARG_ENUM:
				fprintf(pedro->fout, "%d", pedro_get_enum_arg(pedro, parg->arg_name, -1));
				break;
			case PEDRO_ARG_DATETIME:
				{
					//__i = pedro_get_datetime_arg(pedro, "time_arg", &def_datetime, &datetime_val);
					//struct tm def_datetime = {-1,-1,-1,-1,-1,-1,-1,-1,-1};
					struct tm datetime_val = {0};
					int __i = pedro_get_datetime_arg(pedro, parg->arg_name, NULL, &datetime_val);
					//for (int i=0; i<9; i++) fprintf(pedro->fout, "%0d ", ((int*)&datetime_val)[i]);

					char datetime_as_string[200];
					strftime(datetime_as_string, sizeof(datetime_as_string),
					        (char*)parg->min, //format,
					        &datetime_val);
					fprintf(pedro->fout, "'%s'", datetime_as_string);
//struct tm {
//  int     tm_sec;         /* seconds */
//  int     tm_min;         /* minutes */
//  int     tm_hour;        /* hours */
//  int     tm_mday;        /* day of the month */
//  int     tm_mon;         /* month */
//  int     tm_year;        /* year */
//  int     tm_wday;        /* day of the week */
//  int     tm_yday;        /* day in the year */
//  int     tm_isdst;       /* daylight saving time */
//};
				}
				break;
			default:
				assert(0);
		}

		fprintf(pedro->fout, "\n");
	}
}


//
// main
//

int main (void)
{
	// remove any previous mgmtfs
	system("rm -r " MGMTFS_PATH); // rm -r /tmp/clifs
	
	/* Connect to the central server */
	printf("\nIPC_connect(%s)\n", THIS_MODULE_NAME);
	IPC_connect(THIS_MODULE_NAME);

	pedro_init(MGMTFS_PATH);

	// register commands we are prepared to handle (and directories)
	pedro_add_cmds(tree, MGMTFS_PATH);

/*
	pedro_add_internal_help_cmd("shorthelp", "longhelp", PEDRO_GLOBALS"/help", MGMTFS_PATH);
	pedro_add_internal_exit_cmd("shorthelp", "longhelp", PEDRO_GLOBALS"/exit", MGMTFS_PATH);
	pedro_add_internal_cdup_cmd("shorthelp", "longhelp", PEDRO_GLOBALS"/cdup", MGMTFS_PATH);
*/

	IPC_dispatch();
	IPC_disconnect();
	return 0;
}
