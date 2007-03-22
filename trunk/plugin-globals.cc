/**
 * plugin-global.c
 * implements a few global commands as plugins. 
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 18, 2006
 */

#include <config.h>     // autoconf

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#include "plugins.h"
#include "plugin-globals.h"

// mgmt_path
#include "pedrosh-i.h"
#include "debug.h"
#include "utils.h"

//
// internal functions
//

static int _system(const char* prog, int argc, char* argv[], const char* dir);

static int _exit_command(int argc, char* argv[]);
static int _telnet_command(int argc, char* argv[]);
static int _shell_command(int argc, char* argv[]);
static int _system_command(int argc, char* argv[]);
static int _cdup_command(int argc, char* argv[]);


/**
 * initializes the globals command module, so that the global command will be registered.
 */
extern void globals_init(void)
{
	plugins_register("exit", _exit_command);
	plugins_register("telnet", _telnet_command);
	plugins_register("shell", _shell_command);
	plugins_register("system", _system_command);
	plugins_register("cdup", _cdup_command);
	trace("global commands registered.\n");
}

static int _exit_command(int argc, char* argv[])
{
	exit(0);
	return 0;
}

static int _shell_command(int argc, char* argv[])
{
	return _system("sh", argc, argv, NULL);
}

static int _telnet_command(int argc, char* argv[])
{
	return _system("telnet", argc, argv, NULL);
}

static int _system_command(int argc, char* argv[])
{
	char dir[MAX_PATH+1];
	int ret = PEDRO_EXEC_SUCCESS;
	
	trace("system called: %s\n", argv[0]);

	snprintf(dir, MAX_PATH, "%s/%s", g_mgmt_path.c_str(), argv[0]);
	ret = _system("./" PEDRO_META_SYSTEM, argc, argv, dir);
	
	return ret;
}

/**
 * executes <prog> with the <argc> arguments in <argv> as command line arguments.
 */
static int _system(const char* prog, int argc, char* argv[], const char* dir)
{
	char cmdline[MAX_PATH+1];
	char prev_dir[MAX_PATH+1];
	int i, ret;

	if (dir != NULL)
	{
		getcwd(prev_dir, MAX_PATH);
		chdir(dir);
	}
	
	strncpy(cmdline, prog, MAX_PATH);
	strncat(cmdline, " ", MAX_PATH);
	
	for (i = 1; i < argc; ++i)
	{
		strncat(cmdline, argv[i], MAX_PATH);
		strncat(cmdline, " ", MAX_PATH);
	}

	trace("executing <%s>\n", cmdline);
	ret = system(cmdline);

	if (dir != NULL) chdir(prev_dir);
	return ret;
}

static int _cdup_command(int argc, char* argv[])
{
	if (g_curr_dir != "")
	{
		// this is a workaround because cdup() operates on pathes inside g_mgmt_path
		string temp = cdup(g_mgmt_path + '/' + g_curr_dir, g_mgmt_path);
		
		int skip_len = g_mgmt_path.size();
		if (temp.size() > g_mgmt_path.size())
			skip_len++;  // skip the additional '/' after g_mgmt_path
		g_curr_dir = temp.substr(skip_len);
	}
	return 0;
}
