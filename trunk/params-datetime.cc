/**
 * params-datetime.c
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
#include <assert.h>
#include <time.h>
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
 * parses a string that contains date and/or time.
 * the format is specified according to strptime(P).
 * @param input - the input string.
 * @param format - the format.
 * @param out_value - will contain the result.
 * @param out_err, err_len - if out_err is not null. return the error.
 */
extern int datetime_parse(
	const string& input,
	const string& format,
	struct tm* out_value,
	string* out_err)
{
	int ret;

	assert(out_value != NULL);

	// reset out_value
	memset(out_value, 0, sizeof(struct tm));

	// use strptime to convert the input string into a time structure.
	if (strptime(input.c_str(), format.c_str(), out_value) == NULL)
	{
		if (out_err) stl_sprintf(out_err, "'%s': date/time syntax error (format should be '%s')", input.c_str(), format.c_str());
		ret = -1;
		goto cleanup;
	}

	// if successful, return 0.
	ret = 0;
	
cleanup:
	return ret;
}

/**
 * completes a datetime argument.
 */
int datetime_complete(
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

	string date_format = read_file(currpath+"/@format");
	if (date_format == "")
	{
		assert(0);
    //strptime("2001-11-12 18:31:01", "%Y-%m-%d %H:%M:%S", &tm);
		date_format = "%Y-%m-%d %H:%M:%S";
	}

	string     err;
	struct tm  out_value;
	int        ret = datetime_parse(token, date_format.c_str(), &out_value, &err);

	// print error, if we had one.
	if (ret != 0)
		fprintf(stderr, "warning: %s\n", err.c_str());
	
	return ret;
}

/**
 * called when the user executes a command with a datetime argument.
 */
int datetime_execute(const string& nodepath, const string& argname, string& token)
{
	int ret;
	string err;

	trace("nodepath=%s, token=%s\n", nodepath.c_str(), token.c_str());

	string date_format = read_file(nodepath+"/@format");

	if (date_format == "")
	{
		assert(0);
    //strptime("2001-11-12 18:31:01", "%Y-%m-%d %H:%M:%S", &tm);
		date_format = "%Y-%m-%d %H:%M:%S";
	}

	struct tm out_value;
	ret = datetime_parse(token, date_format.c_str(), &out_value, &err);

	// print error, if we had one.
	if (ret != 0)
	{
		fprintf(stderr, "error: %s: %s\n", argname.c_str(), err.c_str());
		return PEDRO_EXEC_ERROR;
	}

  // modify the string that is sent to the command handler
	
  char tmpstr[9 * 15];
	sprintf(tmpstr, "%d %d %d %d %d %d %d %d %d",
	  out_value.tm_sec,
	  out_value.tm_min,
	  out_value.tm_hour,
	  out_value.tm_mday,
	  out_value.tm_mon,
	  out_value.tm_year,
	  out_value.tm_wday,
	  out_value.tm_yday,
	  out_value.tm_isdst
	  );
	token = tmpstr;

	return PEDRO_EXEC_SUCCESS;
}

/**
 * registers the datetime type.
 */
void datetime_register()
{
	params_register("datetime", datetime_complete, datetime_execute);	
}
