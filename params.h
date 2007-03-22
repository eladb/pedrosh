/**
 * plugin-globals.h
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 18, 2006
 */

#ifndef __PARAMS_H__
#define __PARAMS_H__

#include <libtecla.h>
#include <string>
using namespace std;

//
// typedefs
//

/**
 * callback function which completes a specific param type
 */
typedef int (*cmpl_arg_func_t)(const string& nodepath,  // path to the node of this param
                               WordCompletion *cpl,
                               const char *line,
                               int word_start, int word_end);

/**
 * callback function which validates a specific parameter
 * during parsing ("execution") of a command line
 *
 * @param nodepath - path to the node (including mgmgtfs)
 * @param argname  - argument name (not the callback name!!) to 
 *                   use for error messages.
 * @param token    - [in/out] token to parse.
 *                   the function may change this variable.
 *                   the output of this function will be sent over to the
 *                   command handler.
 * @return PEDRO_EXEC_SUCCESS
 *         PEDRO_EXEC_ERROR    - the function will also print an error message
 */
typedef int (*exec_arg_func_t)(const string& nodepath, // path to the node of this param
                               const string& argname,
															       string& token);   // token to parse, the param value.

/**
 * parameter type descriptor
 */
typedef struct
{
	string name;
				
	/**
	 * the function called to complete arguments of this type.
	 */
	cmpl_arg_func_t cmpl_func;

	/**
	 * the function called to execute the argument.
	 */
	exec_arg_func_t exec_func;
	
} param_type_t;

/**
 * param descriptor
 */
typedef struct
{
	const param_type_t *param_type;
	string param_name;           // the name the user enters. different nodes on the
	                             //   same command line may have parameters with the
															 //   same param_name;
	string param_callback_name;  // the name by which the callback function refers to
	                             //   this param. this must be unique across a single
	                             //   command line, or the callback function won't be
	                             //   able to differentiate between the parameters.
	bool is_optional;
	//string constraints;
} param_t;

//
// function declarataions
// 

extern void params_init(void);

// command_path is full path. see comment in code.
extern const param_type_t *get_param_type (const string &command_path);

extern const param_type_t* params_register(const string& name, cmpl_arg_func_t cmpl_func, exec_arg_func_t exec_func);
extern const param_type_t* params_get_type(const string& name);
extern const param_type_t* params_set_default(const param_type_t* def);


#endif /* __PARAMS_H__ */
