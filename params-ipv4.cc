/**
 * params-ipv4.c
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
#include <regex.h>

// stl
#include <string>
#include <sstream>    // stringstream, istringstream, ostringstream
using namespace std;

// pedro
#include "utils.h"
#include "debug.h"
#include "pedrosh-i.h"
#include "params.h"
#include "params-ipv4.h"


//
// definitions
//

#define IPV4_RE "^([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})$"
#define IPV4_MAX_MATCH	10

//
// function definitions
//

/**
 * parses a string as an ip version 4 string in the format (a.b.c.d).
 * @param input - the input string.
 * @param out_value - the output ipv4 address.
 * @param out_err - if not null, returns a descriptive error.
 * @param err_len - size of out_err.
 * @return 0 upon success, -1 on failure.
 */
int ipv4_parse(
	const string& str,
	ipv4_t* out_value,
	string* out_err)
{
	regmatch_t matches[IPV4_MAX_MATCH];
	int ret;
	regex_t re;
	int reg_ret;
	int i;
	const char* input;
	
	assert(out_value != NULL);
	
	// reset output value and error.
	out_value->as_int = 0;
	if (out_err) out_err->clear();
	
	input = str.c_str();
	
	// compile the regular expression.
	reg_ret = regcomp(&re, IPV4_RE, REG_EXTENDED);
	if (reg_ret != 0)
	{
		if (out_err) stl_sprintf(out_err, "unable to parse ipv4 address. invalid regular expression used");
		ret = -1;
		goto cleanup;
	}

	// execute the regular expression on the input string.
	reg_ret = regexec(&re, input, IPV4_MAX_MATCH, matches, 0);
	if (reg_ret == REG_NOMATCH)
	{
		if (out_err) stl_sprintf(out_err, "%s: invald ipv4 address (expecting a.b.c.d)", input);
		
		ret = -1;
		goto cleanup;
	}

	// if a match was found, convert to integers and verify all are below 256.
	for (i = 0; i < 4; ++i)
	{
		long nibble;
		
		// convert a nibble to long. we use the fact that nstrtol stops at the next non-digit.
		nibble = strtol(input + matches[i + 1].rm_so, NULL, 10);

		// check that it is in range.
		if (nibble < 0 || nibble > 255)
		{
			if (out_err) stl_sprintf(out_err, "%s: invalid ipv4 address (out of range)", input);
			ret = -1;
			goto cleanup;
		}

		// store on output.
		out_value->as_bytes[i] = nibble & 0xff;
	}

	ret = 0;
	
cleanup:
	regfree(&re);
	return ret;
}

/**
 * completes an enum argument.
 */
int ipv4_complete(
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

	string err;
	ipv4_t out_value;
	int    ret = ipv4_parse(token, &out_value, &err);

	// print error, if we had one.
	if (ret != 0)
		fprintf(stderr, "warning: %s\n", err.c_str());
	
	return ret;
}

/**
 * called when the user executes a command with an ipv4 argument.
 */
int ipv4_execute(const string& nodepath, const string& argname, string& token)
{
	int ret;
	string err;

	trace("nodepath=%s, token=%s\n", nodepath.c_str(), token.c_str());

	ipv4_t out_value;
	ret = ipv4_parse(token, &out_value, &err);

	// print error, if we had one.
	if (ret != 0)
	{
		fprintf(stderr, "error: %s: %s\n", argname.c_str(), err.c_str());
		return PEDRO_EXEC_ERROR;
	}

	// change token, the value passed to the command handler, to the integer index
	ostringstream os;
	os << (int)out_value.as_int;
	token = os.str();

	return PEDRO_EXEC_SUCCESS;
}

/**
 * registers the ipv4 type.
 */
void ipv4_register()
{
	params_register("ipv4", ipv4_complete, ipv4_execute);	
}
