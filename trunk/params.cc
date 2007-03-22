/**
 * params.c
 * handles the execution of commands, along with the registration of plugins. 
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author talba@embrix.com
 * @since july 27, 2006
 */

#include <config.h>     // autoconf

#include <stdio.h>
#include <stdlib.h>     // strtoul()
#include <assert.h>
#include <string.h>
#include <strings.h>

#include <vector>
#include <string>
#include <map>
using namespace std;

#include <libtecla.h>
#include "pedrosh-i.h"
#include "debug.h"
#include "utils.h"
#include "params.h"

#include "params-enum.h"
#include "params-ipv4.h"
#include "params-ipv6.h"
#include "params-string.h"
#include "params-int.h"
#include "params-datetime.h"

//
// type definitions
// 


//
// prototypes
//

static int cmpl_arg_bool(const string& currpath,
                         WordCompletion *cpl, const char *line,
                         int word_start, int word_end);
static int exec_arg_bool(const string& nodepath, const string& arg_name,
                         string& token);

//
// static variables
//

//
// global variables
//

typedef map<string,param_type_t> param_types_map_t;
param_types_map_t g_param_types;
static const param_type_t* _default_param_type = NULL;

//
// function definitions
//

/**
 * initializes the params handling module.
 */
extern void params_init()
{
	const param_type_t* def;
	
	def = params_register("bool", cmpl_arg_bool, exec_arg_bool);
	params_set_default(def);

	enum_register();
	ipv4_register();
	ipv6_register();
	string_register();
	int_register();
	datetime_register();

	return;
}

/**
 * returns the param_type_t struct by type name.
 */
extern const param_type_t* params_get_type(const string& name)
{
	param_types_map_t::iterator it = g_param_types.find(name);
	if (it == g_param_types.end()) return NULL;
	return &(it->second);
}

/**
 * sets the default param type.
 */
extern const param_type_t* params_set_default(const param_type_t* def)
{
	const param_type_t* prev = _default_param_type;
	_default_param_type = def;
	return prev;
}

/**
 * registers a parameter type into the params module.
 */
extern const param_type_t* params_register(const string& name, cmpl_arg_func_t cmpl_func, exec_arg_func_t exec_func)
{
	g_param_types[name].name = name;
	g_param_types[name].cmpl_func = cmpl_func;
	g_param_types[name].exec_func = exec_func;
	return &(g_param_types[name]);
}

/**
 * looks for the file @arg under the command's path and fill a param_t structure.
 * @param command_path - a full path to the command (not relative to g_mgmt_path)
 * @return pointer if ok, null if not found or some other error.
 */
const param_type_t *get_param_type(const string& command_path)
{
	string prmtype = read_file(command_path + "/"PEDRO_META_ARG);

	// if there's not file or the argument type is empty, return NULL.
	if (prmtype == "") return NULL;
	
	prmtype = prmtype.substr( prmtype.find_first_not_of(" \t\n") );
	prmtype = prmtype.substr( 0, prmtype.find_last_not_of(" \t\n")+1 );

	trace("param_type=<%s>\n", prmtype.c_str());

	const param_type_t *res = params_get_type(prmtype);
	if (res == NULL)
	{
		fprintf(stderr, "warning: unable to identify argument type '%s'. assuming '%s'\n", 
										prmtype.c_str(), _default_param_type->name.c_str());
		res = _default_param_type;
	}

	return res;	
}

/////////////////////// bool parameter type ///////////////////////

static int cmpl_arg_bool(const string& currpath, WordCompletion *cpl, const char *line,
								             int word_start, int word_end)
{
	string line_wrapper = line; // wrap line with string object
	string token(line+word_start, word_end-word_start);

	string tr = "true";
	string fl = "false";

	if (tr.find(token) == 0)
	{
		cpl_add_completion(cpl, line, word_start, word_end,
		                   tr.c_str() + token.size(), // suffix
                       "", // type_suffix (must be malloced)
                       " "); // cont_suffix
	}
	
	if (fl.find(token) == 0)
	{
		cpl_add_completion(cpl, line, word_start, word_end,
		                   fl.c_str() + token.size(), // suffix
                       "", // type_suffix (must be malloced)
                       " "); // cont_suffix
	}
	return 0;
}

static int exec_arg_bool(const string& nodepath, // path to the node of this param
												 const string& argname,
                         string& token)    // token to parse, the param value.
{
  if (token == "true" || token == "false")
		return PEDRO_EXEC_SUCCESS;
	else
	{
		printf("error: bad boolean value - '%s'\n", token.c_str());
		return PEDRO_EXEC_ERROR;
	}
}

