/**
 * params-int.c
 *
 * copyright (c) 2006 by embrix.
 * all rights reserved.
 * developed by embrix for corrigent systems ltd.
 *
 * @author eladb@embrix.com
 * @since july 29, 2006
 */

#include <config.h>     // autoconf

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>

// stl
#include <string>
using namespace std;

// pedro
#include "utils.h"
#include "debug.h"
#include "params.h"
#include "pedrosh-i.h"


//
// function definitions
//

/**
 * checks if a string corresponds to come length restrictions.
 * @param str - the input string.
 * @param out_value - the input string, without leading whitespace.
 * @param value_len - the length of the out_value buffer.\
 * @param use_min, min - if use_min is true, min is the required minimum length of the string.
 * @param use_max, max - if use_max is true, max is the required maximum length of the string.
 * @param out_err, err_len - if out_err is not null, it will contain a descriptive error of maximum size err_len.
 * @return returns 0 if the string had been parsed successfuly or -1 if not.
 */
extern int string_parse(
	const string& str,
	string* out_value,
	int use_min, int min,
	int use_max, int max,
	string* out_err)
{
	const char* str_start;
	int ret;
	int len;

	assert(out_value != NULL);

	// reset out_value && err.
	out_value->clear();
	if (out_err) out_err->clear();
	
	// jump over any whitespace at the beginning of the string.
	str_start = str.c_str();
	while (*str_start != '\0' && isspace(*str_start))
	{
		str_start++;
	}
	
	len = strlen(str_start);
	
	// check min
	if (use_min && len < min)
	{
		if (out_err) stl_sprintf(out_err, "'%s': string must be longer than %d characters", str_start, min);
		ret = -1;
		goto cleanup;
	}

	// check max
	if (use_max && len > max)
	{
		if (out_err) stl_sprintf(out_err, "'%s': string must be shorter than %d characters", str_start, max);
		ret = -1;
		goto cleanup;
	}

	// we're ok.
	ret = 0;
	*out_value = str_start;
	
cleanup:
	return ret;
}

/**
 * load @min, @max from file system and calls int_parse().
 * this is a helper function called by both int_execute() and int_complete()
 */
static int string_helper(const string& nodepath, const string& token, string *out_value, string* out_err)
{
	int use_min = 0;
	int min     = 0;
	int use_max = 0;
	int max     = 0;

	if (file_exists(nodepath+"/@min"))
	{
		use_min=1;
		min = atoi(read_file(nodepath+"/@min").c_str());
	}

	if (file_exists(nodepath+"/@max"))
	{
		use_max=1;
		max= atoi(read_file(nodepath+"/@max").c_str());
	}

	int ret = string_parse(token, out_value, 
	                       use_min, min,
	                       use_max, max,
	                       out_err);
	return ret;
}

/**
 * completes a string argument.
 */
int string_complete(
 const string& currpath, WordCompletion *cpl, const char *line, int word_start, int word_end)
{
	// we don't complete this token type.
	// we do check, however, if it contains an illegal value.
	// if it does - simply print an error message.
	// extract last token from line.
	
  string token(line + word_start, word_end - word_start);
	trace("currpath=<%s> line=<%s> token=<%s>\n", currpath.c_str(), line, token.c_str());

	if (token == "")
		return 0;

	string out_value;
	string err;
	int ret = string_helper(currpath, token, &out_value, &err);

	// print error, if we had one.
	if (ret != 0)
		fprintf(stderr, "warning: %s\n", err.c_str());

	return ret;
}

/**
 * called when the user executes a command with an string argument.
 */
int string_execute(const string& nodepath, const string& argname, string& token)
{
	trace("nodepath=%s, token=%s\n", nodepath.c_str(), token.c_str());

	string out_value;
	string err;
	int ret = string_helper(nodepath, token, &out_value, &err);

	// print error, if we had one.
	if (ret != 0)
		fprintf(stderr, "error: %s: %s\n", argname.c_str(), err.c_str());
	
	return ret ? PEDRO_EXEC_ERROR : PEDRO_EXEC_SUCCESS;
}

/**
 * registers the string type.
 */
void string_register()
{
	params_register("string", string_complete, string_execute);	
}
