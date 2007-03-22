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

// stl
#include <string>
using namespace std;

// pedro
#include "utils.h"
#include "debug.h"
#include "params.h"
#include "pedrosh-i.h"
#include "params-int.h"

//
// function definitions
//

/**
 * parses an integer input.
 * @param str - the input string.
 * @param out_value - will contain the parsed integer if successful.
 * @param use_min, min - if use_min, min contains a minimum value.
 * @param use_max, max - if use_max, max contains a maximum value.
 * @param out_err - if not NULL, will contain a descriptive error message.
 * @param err_len - maximum size of out_err buffer.
 * @return 0 if successful, -1 if failed.
 */
extern int int_parse(
	const string& str, 
	long* out_value,
	int use_min, long min,
	int use_max, long max,
	string* out_err)
{
	long long result;
	int ret;
	char* endptr = NULL;
				
	// verify that we can store the output value.
	assert(out_value != NULL);

	// reset out_value to zero.
	*out_value = 0;

	// reset out_err to an empty string.
	if (out_err) out_err->clear();
	
	// reset errno, so that if there's an error we can see it.
	errno = 0;

	// invoke strtol.
	result = strtoll(str.c_str(), &endptr, 0);

	// check for errors.
	switch (errno)
	{
		// syntax error
		case EINVAL:
			if (out_err) stl_sprintf(out_err, "'%s': integer syntax error", str.c_str());
			ret = -1;
			goto cleanup;
			
		// out of range
		case ERANGE:
			if (out_err) stl_sprintf(out_err, "'%s': integer is out of range", str.c_str());
			ret = -1;
			goto cleanup;
	}

	// check that there are no extra chars after the number (strtol accepts it but we don't).
	if (endptr != NULL && *endptr != '\0' && !isspace(*endptr))
	{
		if (out_err) stl_sprintf(out_err, "'%s': integer is followed by non-digits", str.c_str());
		ret = -1;
		goto cleanup;
	}
	
	// check that the min constraint applies.
	if (use_min && result < min)
	{
		if (out_err) stl_sprintf(out_err, "'%s': integer must be greater than or equal to %ld", str.c_str(), min);
		ret = -1;
		goto cleanup;
	}

	// check that the max constraint applies.
	if (use_max && result > max)
	{
		if (out_err) stl_sprintf(out_err, "'%s': integer must be less than or equal to %ld", str.c_str(), max);
		ret = -1;
		goto cleanup;
	}

	// the number has been parsed.
	*out_value = result;
	ret = 0;

cleanup:
	return ret;
}


//int int_execute(const string& nodepath, const string& argname, const string& token)


/**
 * load @min, @max from file system and calls int_parse().
 * this is a helper function called by both int_execute() and int_complete()
 */
static int int_helper(const string& nodepath, const string& token, long* out_value, string* out_err)
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

	int ret = int_parse(token, out_value, 
	                       use_min, min,
	                       use_max, max,
	                       out_err);
	return ret;
}

/**
 * completes an int argument.
 */
int int_complete(
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

	long out_value;
	string err;
	int ret = int_helper(currpath, token, &out_value, &err);

	// print error, if we had one.
	if (ret != 0)
		fprintf(stderr, "warning: %s\n", err.c_str());

	return ret;
}

/**
 * called when the user executes a command with an int argument.
 */
int int_execute(const string& nodepath, const string& argname, string& token)
{
	trace("nodepath=%s, token=%s\n", nodepath.c_str(), token.c_str());

	long out_value;
	string err;
	int ret = int_helper(nodepath, token, &out_value, &err);

	// print error, if we had one.
	if (ret != 0)
		fprintf(stderr, "error: %s: %s\n", argname.c_str(), err.c_str());
	
	return ret ? PEDRO_EXEC_ERROR : PEDRO_EXEC_SUCCESS;
}

/**
 * registers the int type.
 */
void int_register()
{
	params_register("int", int_complete, int_execute);	
}
