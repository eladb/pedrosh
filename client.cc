/*
 * copyright (c) 2006 by embrix.
 * 
 * all rights reserved.
 * 
 * developed by embrix for corrigent systems ltd.
 *
 * this file contains implementation of functions used by client process
 * that wants to respond to a pedrosh command.
 */

#include <config.h> /* this should be the first include file */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h> // 	WIFEXITED(), WEXITSTATUS()
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <list>
using namespace std;

#include <ipc.h>
#include "pedrosh.h"
#include "pedrosh-i.h"  // get our private libipc message definition
#include "utils.h"
#include "plugin-globals.h"  // PEDRO_META_DOC, PEDRO_META_SYSTEM

#include "params-int.h"
//#include "params-ipv4.h"  // parsed as "int" from command handler
//#include "params-enum.h"  // parsed as "int" from command handler
#include "params-ipv6.h"


//
// internal function headers
//

int pedro_add_node(const char *nodepath, const char *mgmtfs);
int pedro_set_node_shorthelp(const char *help, const char *nodepath, const char *mgmtfs);
int pedro_set_node_longhelp(const char *shorthelp, const char *longhelp, const char *longhelp, const char *nodepath, const char *mgmtfs);
int pedro_set_node_dir_flag(int dir_flag, const char *nodepath, const char *mgmtfs);

typedef enum {
	PEDRO_EXECMOD_NOEXEC,   /* node is not executable */
	PEDRO_EXECMOD_ARGV,     /* node is executable, parameters are not validated,
	                         * handler gets all parameters in argv/argc style */
	PEDRO_EXECMOD_PARAMS    /* node is executable, parameters are validated */
} pedro_exec_mode_t;
int pedro_set_node_exec_mode(pedro_exec_mode_t exec_mode, const char *nodepath, const char *mgmtfs);

int pedro_subscribe(pedro_callback_func_t callback, const char *nodepath);
int pedro_unsubscribe(char *nodepath);

int pedro_set_node_textfile(const char *filename, const char *filecontent, const char *nodepath, const char *mgmtfs);

//
// function definition
//


int pedro_init(const char *mgmtfs)
{
	// put @dir in the mgmtfs root (and also - create the mgmtfs if needed)
	int err = pedro_add_enterable_dir(NULL, NULL, "", mgmtfs);
	if (err) return err;

	return 0;
}


// NOTE: if PEDROSH_MSG becomes bigger, maybe we should pass a pointer 
// and not the entire structure (now its only 8 bytes)
static void call_user_handler(pedro_callback_func_t callback, PEDROSH_MSG msg)
{
	// connect to the pipes provided by pedrosh, to allow the command handler to
	//  send and recv data from the terminal
	FILE *in, *out;
	char tmp[MAX_PATH];
	
	// the order of these two open calls is very important.
	// the other option will deadlock.
	sprintf(tmp, "%s/out", msg.argv[msg.argc-1]);
	out = fopen(tmp, "wb");
	if (!out)
	{
		fprintf(stderr, "in "__FILE__ "#%d, call_user_handler():\n"
			"\t error opening output pipe: %s\n", __LINE__, tmp);
		return;
	}
	
	sprintf(tmp, "%s/in", msg.argv[msg.argc-1]);
	in = fopen(tmp, "rb");
	if (!in)
	{
		fprintf(stderr, "in "__FILE__ "#%d, call_user_handler():\n"
			"\t error opening input pipe: %s\n", __LINE__, tmp);
		fclose(out);
		return;
	}

	pedro_callback_func_args_t pedro;
	pedro.argc = msg.argc-1;
	pedro.argv = msg.argv;
	pedro.fin  = in;
	pedro.fout = out;
	// call the command handler
	callback(pedro.argc, pedro.argv, pedro.fin, pedro.fout, &pedro);
	
	// close the handler - failing to do that will lock pedrosh forever
	fclose(out);
	fclose(in);
}

static void on_cli_specific_msg (MSG_INSTANCE msgRef, BYTE_ARRAY callData,
			 void *clientData)
{
	PEDROSH_MSG msg;
	
	pedro_callback_func_t callback = (pedro_callback_func_t) clientData;
	assert(callback);
	
	IPC_unmarshallData(IPC_msgInstanceFormatter(msgRef), callData,
			&msg, sizeof(msg));
	
	call_user_handler(callback, msg);
	
	IPC_freeByteArray(callData);
}

int pedro_subscribe(pedro_callback_func_t callback, const char *nodepath)
{
	char msg_name[MAX_PATH + 1]; // TODO: SVN: use string class instead
	IPC_RETURN_TYPE ret;
	
	assert(callback);
	
	sprintf(msg_name, "%s-%s", PEDROSH_MSG_NAME, nodepath);
	
	ret = IPC_defineMsg(msg_name, IPC_VARIABLE_LENGTH, PEDROSH_MSG_FORM);
	if (ret != IPC_OK) return -1;
	
	ret = IPC_subscribe(msg_name, on_cli_specific_msg, (void*)callback);
	if (ret != IPC_OK) return -1;
	
	return 0;
}

int pedro_unsubscribe(const char *nodepath)
{
	char msg_name[MAX_PATH + 1]; // TODO: SVN: use string class instead
	IPC_RETURN_TYPE ret;
	
	sprintf(msg_name, "%s-%s", PEDROSH_MSG_NAME, nodepath);
	
	ret = IPC_defineMsg(msg_name, IPC_VARIABLE_LENGTH, PEDROSH_MSG_FORM);
	if (ret != IPC_OK) return -1;
	
	ret = IPC_unsubscribe(msg_name, on_cli_specific_msg);
	if (ret != IPC_OK) return -1;

	return 0;
}

int pedro_add_cmds(pedro_cmd_t *cmds, const char *mgmtfs)
{
	int i, err;

	// put @dir in the mgmtfs root (and also - create the mgmtfs if needed)
	err = pedro_add_enterable_dir(NULL, NULL, "", mgmtfs);
	if (err) return err;

	// add the commands
	for (i=0; cmds[i].nodepath; i++)
	{
		err = pedro_add_cmd(
											cmds[i].callback,
											cmds[i].args != NULL,   //validate_args_flag,
											cmds[i].shorthelp,
											cmds[i].longhelp,
											cmds[i].nodepath,
											mgmtfs);
		if (err) return err;

		for (pedro_arg_t *arg = cmds[i].args; arg && arg->arg_name; arg++)
		{
			string arg_nodepath;
			arg_nodepath = cmds[i].nodepath;
			arg_nodepath += '/';
			arg_nodepath += arg->arg_name;
			
			if (arg->arg_type == PEDRO_ARG_STRING)
				err = pedro_add_param(NULL, //callback_param_name
															"string",
															arg->is_optional,
															arg_nodepath.c_str(),
															mgmtfs);
			else if (arg->arg_type == PEDRO_ARG_INT)
				err = pedro_add_int_param(
															arg->min, arg->max,
															NULL, //callback_param_name
															arg->is_optional,
															arg_nodepath.c_str(),
															mgmtfs);
			else if (arg->arg_type == PEDRO_ARG_IPV4)
				err = pedro_add_param(NULL, //callback_param_name
															"ipv4",
															arg->is_optional,
															arg_nodepath.c_str(),
															mgmtfs);
			else if (arg->arg_type == PEDRO_ARG_IPV6)
				err = pedro_add_param(NULL, //callback_param_name
															"ipv6",
															arg->is_optional,
															arg_nodepath.c_str(),
															mgmtfs);
			else if (arg->arg_type == PEDRO_ARG_ENUM)
			{
				assert(arg->min);
				err = pedro_add_enum_param((const char*)arg->min, // enum values
															NULL, //callback_param_name
															arg->is_optional,
															arg_nodepath.c_str(),
															mgmtfs);
			}
			else if (arg->arg_type == PEDRO_ARG_DATETIME)
			{
				assert(arg->min);
				err = pedro_add_datetime_param((const char*)arg->min, // format
															NULL, //callback_param_name
															arg->is_optional,
															arg_nodepath.c_str(),
															mgmtfs);
			}
			else
				assert(0);

			if (err) return err;
		} // for each aegument
	} // for each command

	return 0;
}
	
// create a node (directory) in mgmtfs
//
// both nodepath and mgmtfs should not include a trailing slash.
// also creates any parent node that does not already exist
// return:
//    0 - ok
//   -1 - error
int pedro_add_node(const char *nodepath, const char *mgmtfs)
{
	string fullpath = mgmtfs;
	fullpath += '/';
	fullpath += nodepath;
	return mkdir_recursive(fullpath) ? 0 : -1;
}

// delete a node (directory) in mgmtfs
//
// both nodepath and mgmtfs should not include a trailing slash.
// also deletes any child node that does not already exist
// return:
//    0 - ok
//   -1 - error
int pedro_del_node(const char *nodepath, const char *mgmtfs)
{
	string fullpath = mgmtfs;
	fullpath += '/';
	fullpath += nodepath;
	return delfile_recursive(fullpath) ? 0 : -1;
}

// set the context of a text file inside a node (or deletes it).
// this function should be called after pedro_add_node().
//
// both nodepath and mgmtfs should not include a trailing slash.
// filename - the name of the file to create in the node (no path is allowed)
// filecontent - the text to put in the file. if NULL the file is deleted from the node,
//               if it exists.
// return:
//    0 - ok
//   -1 - error
int pedro_set_node_textfile(const char *filename, const char *filecontent, const char *nodepath, const char *mgmtfs)
{
	char path[MAX_PATH + 1];
	
	sprintf(path, "%s/%s/%s", mgmtfs, nodepath, filename);

	if (filecontent)
	{
		FILE *fp = fopen(path, "wt");
		if (!fp) return -1;
		if ( fputs(filecontent, fp) == EOF )
			return -1;
		return fclose(fp) ? -1 : 0;
	}
	else
	{
		// delete the file <path>
		/*int err = */ remove(path);
		if (0) // (err && errno == ENOENT)  <-- this causes problems
			return -1;
		else
			return 0;
	}
}

// set node longhelp
//
// both nodepath and mgmtfs should not include a trailing slash.
// this function should be called after pedro_add_node().
// 
// longhelp - the help text. if NULL the node is marked as having no longhelp.
// return:
//    0 - ok
//   -1 - error
int pedro_set_node_longhelp(const char *longhelp, const char *nodepath, const char *mgmtfs)
{
	return pedro_set_node_textfile(PEDRO_META_DOC, longhelp, nodepath, mgmtfs);
}

// set node longhelp
//
// see set_node_longhelp()
int pedro_set_node_shorthelp(const char *help, const char *nodepath, const char *mgmtfs)
{
	return pedro_set_node_textfile(PEDRO_META_HELP, help, nodepath, mgmtfs);
}


/////////////////////////////////////////////////////////////////////
// adding internal commands

int pedro_add_internal_exit_cmd(const char *help, const char *longhelp, const char *nodepath, const char *mgmtfs)
{
	int err;

	err = pedro_add_node(nodepath, mgmtfs);
	if (err) return err;
	err = pedro_set_node_shorthelp(help, nodepath, mgmtfs);
	if (err) return err;
	err = pedro_set_node_longhelp(longhelp, nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_textfile(PEDRO_META_EXEC_ARGV, "", nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_textfile(PEDRO_META_PLUGIN, "exit", nodepath, mgmtfs);
	if (err) return err;

	return 0;
}

int pedro_add_internal_telnet_cmd(const char *help, const char *longhelp, const char *nodepath, const char *mgmtfs)
{
	int err;

	err = pedro_add_node(nodepath, mgmtfs);
	if (err) return err;
	err = pedro_set_node_shorthelp(help, nodepath, mgmtfs);
	if (err) return err;
			
	err = pedro_set_node_longhelp(longhelp, nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_textfile(PEDRO_META_EXEC_ARGV, "", nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_textfile(PEDRO_META_PLUGIN, "telnet", nodepath, mgmtfs);
	if (err) return err;

	return 0;
}

//      "system" (also needs a file "@system" with command to run)
int pedro_add_internal_system_cmd(const char *cmd_to_run, const char *help, const char *longhelp, const char *nodepath, const char *mgmtfs)
{
	int err;
	char tmp[MAX_PATH+1];
	
	err = pedro_add_node(nodepath, mgmtfs);
	if (err) return err;
	err = pedro_set_node_shorthelp(help, nodepath, mgmtfs);
	if (err) return err;
	err = pedro_set_node_longhelp(longhelp, nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_textfile(PEDRO_META_EXEC_ARGV, "", nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_textfile(PEDRO_META_PLUGIN, "system", nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_textfile("@system", cmd_to_run, nodepath, mgmtfs);
	if (err) return err;

	sprintf(tmp, "%s/%s/@system", mgmtfs, nodepath);
	err = chmod(tmp, 
			S_IRUSR | S_IWUSR | S_IXUSR // r,w,x by ownder
			|
			S_IRGRP | S_IWGRP | S_IXGRP // r,w,x by group
			|
			S_IROTH | S_IWOTH | S_IXOTH // r,w,x by others
			);
	if (err) return err;
 
	
	return 0;
}

int pedro_add_internal_help_cmd(const char *help, const char *longhelp, const char *nodepath, const char *mgmtfs)
{
	int err;
	static char cmd_to_run__src[] = 
		"#!/bin/bash\n"
		"path=%s\n"
		"for part in $@\n"
		"do\n"
		"        path=\"$path/$part\"\n"
		"done\n"
		"\n"
		"if [ ! -d $path ]\n"
		"then\n"
		"        echo \"'$@' command not found.\"\n"
		"        exit 2\n"
		"fi\n"
		"\n"
		"doc_file=$path/@doc\n"
		"if [ -f $doc_file ]\n"
		"then\n"
		"#       less $doc_file\n"
		"        more $doc_file\n"
		"else\n"
		"        echo \"no documentation for '$@'.\"\n"
		"        exit 1\n"
		"fi\n"
		;
	
	char tmp[MAX_PATH + 1];
	char cmd_to_run[MAX_PATH + 1];
	sprintf(cmd_to_run, cmd_to_run__src, mgmtfs);
	err = pedro_add_internal_system_cmd(cmd_to_run, help, longhelp, nodepath, mgmtfs);
	if (err) return err;
	
	
	// create a symbolic link under the new help node, which reffers to the top of the
	//  mgmtfs
	// same as running: ln -s $mgmtfs @void

	// first delete @void if it exists
	pedro_set_node_textfile(PEDRO_META_VOID, NULL, nodepath, mgmtfs);

	// now create the symlink
	sprintf(tmp, "%s/%s/" PEDRO_META_VOID , mgmtfs, nodepath);
	err = symlink(mgmtfs, tmp);
	if (err) return -1;				 
	
	return 0;
}

int pedro_add_internal_cdup_cmd(const char *help, const char *longhelp, const char *nodepath, const char *mgmtfs)
{
	int err;

	err = pedro_add_node(nodepath, mgmtfs);
	if (err) return err;
	err = pedro_set_node_shorthelp(help, nodepath, mgmtfs);
	if (err) return err;
			
	err = pedro_set_node_longhelp(longhelp, nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_textfile(PEDRO_META_EXEC, "", nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_textfile(PEDRO_META_PLUGIN, "cdup", nodepath, mgmtfs);
	if (err) return err;

	return 0;
}


int pedro_set_node_dir_flag(int dir_flag, const char *nodepath, const char *mgmtfs)
{
	char *p;
  if (dir_flag)
		p = "";
	else
		p = NULL;

	int err = pedro_set_node_textfile(PEDRO_META_DIR, p, nodepath, mgmtfs);
	return err;
}

int pedro_set_node_exec_mode(pedro_exec_mode_t exec_mode, const char *nodepath, const char *mgmtfs)
{
	int err1, err2;

	switch(exec_mode)
	{
		case PEDRO_EXECMOD_NOEXEC:
			err1 = pedro_set_node_textfile(PEDRO_META_EXEC,      NULL, nodepath, mgmtfs);
			err2 = pedro_set_node_textfile(PEDRO_META_EXEC_ARGV, NULL, nodepath, mgmtfs);
			return err1 || err2;

		case PEDRO_EXECMOD_ARGV:
			err1 = pedro_set_node_textfile(PEDRO_META_EXEC,      NULL, nodepath, mgmtfs);
			err2 = pedro_set_node_textfile(PEDRO_META_EXEC_ARGV, ""  , nodepath, mgmtfs);
			return err1 || err2;

		case PEDRO_EXECMOD_PARAMS:
			err1 = pedro_set_node_textfile(PEDRO_META_EXEC,      ""  , nodepath, mgmtfs);
			err2 = pedro_set_node_textfile(PEDRO_META_EXEC_ARGV, NULL, nodepath, mgmtfs);
			return err1 || err2;

		default:
			assert(0);
	}

	return 0;
}



// param_type: "bool", "string", "ipv4", "ipv6"
//             for "int"  - use pedro_add_int_param()
//             for "enum" - use pedro_add_enum_param()
int pedro_add_param(const char *callback_param_name, // may be NULL
                    const char *param_type,
                    int optional_flag,
                    const char *nodepath,
                    const char *mgmtfs)
{
	int res;

	assert(param_type && param_type[0]);
	assert(!callback_param_name || callback_param_name[0]);
	
	res = pedro_add_node(nodepath, mgmtfs);
	if (res) return res;
	
	res = pedro_set_node_textfile(PEDRO_META_ARG_CBNAME, callback_param_name,
	                                  nodepath, mgmtfs);
	if (res) return res;
	
	
	res = pedro_set_node_textfile(PEDRO_META_ARG, param_type, nodepath, mgmtfs);
	if (res) return res;
	
	res = pedro_set_node_textfile(PEDRO_META_ARG_OPT,
	                                  optional_flag ? "" : NULL,
	                                  nodepath, mgmtfs);
	if (res) return res;
	return 0;
}

int pedro_add_int_param(int min, int max,  // if min==max, they are both ignored!
                    // rest of the parameters - like pedro_add_param()
                    const char *callback_param_name,
                    int optional_flag,
                    const char *nodepath,
                    const char *mgmtfs)
{
	int res = pedro_add_param(callback_param_name, "int", optional_flag,
                            nodepath, mgmtfs);
	if (res)
		return res;

	// if min==max : the user does not want min/max validation
	if (min != max)
	{
		char buff[20];
		sprintf(buff, "%d", min);
		res = pedro_set_node_textfile("@min", buff, nodepath, mgmtfs); // TODO: SVN: @min
		if (res) return res;
		
		sprintf(buff, "%d", max);
		res = pedro_set_node_textfile("@max", buff, nodepath, mgmtfs); // TODO: SVN: @max
		if (res) return res;
	}

	return 0;
}

int pedro_add_enum_param(const char *strings, // '\n' separated list.
                                       // for example: "up\ndown\nleft\nright"
                    // rest of the parameters - like pedro_add_param()
                    const char *callback_param_name,
                    int optional_flag,
                    const char *nodepath,
                    const char *mgmtfs)
{
	assert(strings);
	
	int res = pedro_add_param(callback_param_name, "enum", optional_flag,
                            nodepath, mgmtfs);
	if (res)
		return res;

	res = pedro_set_node_textfile("@values", strings, nodepath, mgmtfs);
	if (res) return res;

	return 0;
}

// format - same as format parameter of strptime(). see man page strptime(3).
int pedro_add_datetime_param(const char *format,
                    // rest of the parameters - like pedro_add_param()
                    const char *callback_param_name,
                    int optional_flag,
                    const char *nodepath,
                    const char *mgmtfs)
{
	int res = pedro_add_param(callback_param_name, "datetime", optional_flag,
                            nodepath, mgmtfs);
	if (res)
		return res;

	assert(format && format[0]);

	res = pedro_set_node_textfile("@format", format, nodepath, mgmtfs);
	if (res) return res;

	return 0;
}


/////////////////////////////////////////////////////////////////////
// pedro_get_xxx_arg() functions

/**
 * returns a string argument to a command handler function.
 *
 * @param pedro - the value passed to the command handler.
 * @param param_cbname - the (callabck) name of the argument.
 * @param defval - default value to return if the desired parameter was not
 *                  entered on the command line. may be NULL.
 * @return pointer to the value of the string argument value, if it was given
 *         on the command line. otherwise, the value of defval.
 */
char *pedro_get_string_arg(pedro_callback_func_args_t *pedro, char *param_cbname, char *defval)
{
	int i = 1; // skip argv[0]
	
	for (; i < pedro->argc; i+=2)
	{
		if (!strcmp(pedro->argv[i], param_cbname))
		{
			assert(i+1 < pedro->argc); // shouldn't happen
			if (i+1 >= pedro->argc) return defval;
			return pedro->argv[i+1];
		}
	}
	
	return defval;
}

/**
 * returns an int argument to a command handler function.
 *
 * @param pedro - the value passed to the command handler.
 * @param param_cbname - the (callabck) name of the argument.
 * @param defval - default value to return if the desired parameter was not
 *                  entered on the command line.
 * @return argument value if available, otherwise defval.
 */
int pedro_get_int_arg(pedro_callback_func_args_t *pedro, char *param_cbname, int defval)
{
	char *str = pedro_get_string_arg(pedro, param_cbname, NULL);

	if (str == NULL)
		return defval;
	
	int val, err;
	err = int_parse(str,(long*) &val, 0, 0, 0, 0, NULL);
	if (err)
	{
		assert(0); // this should not happen!
		return defval;
	}
	
	return val;
}

/**
 * returns an ipv6 argument to a command handler function.
 *
 * @param pedro - the value passed to the command handler.
 * @param param_cbname - the (callabck) name of the argument.
 * @param defval - pointer to 16 bytes (may be NULL).
 *                 default value to return if the desired parameter was not
 *                  entered on the command line.
 * @param outval - pointer to 16 bytes that will receive the argument value.
 *                 if argument was not entered on the command line 
 *                 and defval!=NULL, defval is copied to outval.
 * @return zero if argument was found.
 *         nonzero if argument was not found.
 */
int pedro_get_ipv6_arg(pedro_callback_func_args_t *pedro, char *param_cbname,
                        void *defval, void *outval)
{
	char *str = pedro_get_string_arg(pedro, param_cbname, NULL);

	if (str == NULL)
		goto arg_not_found;

	int err;
	err = ipv6_parse(str,              // const string& input
	                 (ipv6_t*) outval, // ipv6_t* out_value  [ = byte[16] ]
	                 NULL);            // string* out_err
	if (err)
	{
		assert(0); // this should not happen!
		goto arg_not_found;
	}

	return 0;


arg_not_found:
	if (defval)
		memcpy(outval, defval, sizeof(ipv6_t));

	return -1;
}


/**
 * returns a datetime argument to a command handler function.
 *
 * @param pedro  - the value passed to the command handler.
 * @param param_cbname - the (callabck) name of the argument.
 * @param defval - struct tm (may be NULL).
 *                 default value to return if the desired parameter was not
 *                 entered on the command line.
 * @param outval - pointer to a struct tm that will receive the argument value.
 *                 if argument was not entered on the command line 
 *                 and defval!=NULL, defval is copied to outval.
 * @return zero if argument was found.
 *         nonzero if argument was not found.
 */
int pedro_get_datetime_arg(pedro_callback_func_args_t *pedro, char *param_cbname,
                           struct tm *defval, struct tm* outval)
{
	int n;
	char *str = pedro_get_string_arg(pedro, param_cbname, NULL);

	if (str == NULL)
		goto arg_not_found;

	n = sscanf(str, "%d %d %d %d %d %d %d %d %d",
	  &outval->tm_sec,
	  &outval->tm_min,
	  &outval->tm_hour,
	  &outval->tm_mday,
	  &outval->tm_mon,
	  &outval->tm_year,
	  &outval->tm_wday,
	  &outval->tm_yday,
	  &outval->tm_isdst
	  );
		
	if (n != 9)
	{
		assert(0);
		goto arg_not_found;
	}

	return 0;


arg_not_found:
	if (defval)
		memcpy(outval, defval, sizeof(*defval));

	return -1;
}


/**
 * check if an argument was given on the command line
 *
 * @param pedro - the value passed to the command handler.
 * @param param_cbname - the (callabck) name of the argument.
 * @return zero if the argument was not given.
 *         nonzero if the argument was given.
 */
int pedro_is_arg_defined(pedro_callback_func_args_t *pedro, char *param_cbname)
{
	char *str = pedro_get_string_arg(pedro, param_cbname, NULL);

	return (str != NULL);
}

///////////////////////////////////////////////////////////////

/**
 * add a command
 *
 * @param callback - callback handler to call to execute the command.
 * @param validate_args_flag - zero=argc/argv mode.
 * @param shorthelp - short help msg. displayed when pressing TAB.
 * @param longhelp  - for the "help" command.
 * @param nodepath  - 
 * @param mgmtfs    - 
 * @return zero if ok.
 */
int pedro_add_cmd(
                  pedro_callback_func_t callback,
                  int   validate_args_flag,
                  const char *shorthelp,
                  const char *longhelp,
                  const char *nodepath,
                  const char *mgmtfs)
{
	int err;
	
	err = pedro_add_node(nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_shorthelp(shorthelp, nodepath, mgmtfs);
	if (err) return err;

	err = pedro_set_node_longhelp(longhelp, nodepath, mgmtfs);
	if (err) return err;
	
	if (callback)	
	{
		if (validate_args_flag)
			err = pedro_set_node_exec_mode(PEDRO_EXECMOD_PARAMS, nodepath, mgmtfs);
		else
			err = pedro_set_node_exec_mode(PEDRO_EXECMOD_ARGV,   nodepath, mgmtfs);

		if (err) return err;

		err = pedro_subscribe(callback, nodepath);
	}
	else
	{
		err = pedro_set_node_exec_mode(PEDRO_EXECMOD_NOEXEC, nodepath, mgmtfs);
		if (err) return err;
		err = pedro_set_node_dir_flag(1, nodepath, mgmtfs);
		if (err) return err;
	}
		
	return 0;
}
